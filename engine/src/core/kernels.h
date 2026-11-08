// Per-particle work of the bunch tracking, shared by the CPU tracker and the GPU back ends.
//
// Everything here works on plain views (structures of pointers and numbers, see LatV / PartV) and is
// marked AVAS_HD, so the CPU build and the CUDA build compile the very same source.  The arithmetic
// is written term by term in the order of the original AVAS.dll (see physics.h); the CPU in the
// corrected mode and the GPU in fp64 therefore produce bit-identical particles.
//
// Policies (template parameter P) select the elementary functions that are evaluated per particle:
//   DllMath      host only, compat=dll: the C library and AVAS.dll's static sincos (dllmath.h);
//   PortableMath corrected engine on every device: pmath.h (+ - * / only).
// Values that are the same for all particles (RF phase of a step, rotation angles of elements,
// bend matrices) are computed once on the host and passed in, so the corrected engine uses the C
// library for them exactly as before.
#pragma once
#include <cstdint>

#include "core/hd.h"
#include "core/physics.h"
#include "core/pmath.h"
#include "core/reduce.h"

namespace avas {

#if defined(AVAS_PMATH_LIBM_FOR_TEST)
struct PortableMath {  // verification build only: the C library instead of pmath.h
    static constexpr bool compat = false;
    static void sincos(double a, double& s, double& c) { s = std::sin(a); c = std::cos(a); }
    static double cos(double a) { return std::cos(a); }
    static double sinh(double a) { return std::sinh(a); }
    static double cosh(double a) { return std::cosh(a); }
    static double atan(double a) { return std::atan(a); }
};
#else
struct PortableMath {
    static constexpr bool compat = false;
    AVAS_HD static void sincos(double a, double& s, double& c) { p_sincos(a, s, c); }
    AVAS_HD static double cos(double a) { return p_cos(a); }
    AVAS_HD static double sinh(double a) { return p_sinh(a); }
    AVAS_HD static double cosh(double a) { return p_cosh(a); }
    AVAS_HD static double atan(double a) { return p_atan(a); }
};
#endif

namespace kern {

// ElemKind values (lattice.h)
enum : int { kDrift = 0, kField = 1, kQuad = 2, kSolenoid = 3, kBend = 4, kEdge = 5, kSteerer = 6, kRfq = 7 };

// One RF phase "slot": cos(phase) and cos(phase - pi/2) of a cavity at the current time, where
// phase = (t - cavTime[c]) f 2 pi + (cavPhase[c] [+ add]) pi/180 (spec 30 2.4).
struct RfSlotV {
    int cav;
    int hasAdd;
    double freq;
    double add;
};

struct SubV {  // a sub-field of a superpose block (field type 4)
    int type, map;
    int hasErr;   // AVAS.dll sub-field error flag (se[0] == 1)
    int rfErr;    // RF slot with the sub-field phase error (compat)
    double ke, kb, zStart, radius, length;
    double place[6];               // z0, x0, y0, thz, thx, thy
    double s3, c3, s4, c4, s5, c5;  // sincos of place[3], place[4], place[5]
    double err[12];                // {flag, dx, dy, dz, thx, thy, thz, kb, ke, phase, 0, 0}
    double es4, ec4, es5, ec5, es6, ec6, esin6;  // sincos of err[4..6], sin(err[6])
};

struct ElemV {
    int kind, fieldType, mapIndex, inSuperposeOut, rotMode;
    int nsub, sub0;      // sub-fields [sub0, sub0 + nsub) of a superpose block
    int rfPlain, rfErr;  // RF slots: phase without / with the element's phase error
    int errUse, hetErr;  // t-code error field / z-code misalignment
    double zStart, length, radius, strength, ke, kb, keErr, kbErr, phaseErr;
    double errDx, errDy, errDz;
    double esx, ecx, esy, ecy, esz, ecz;  // sincos of errRotX, errRotY, errRotZ
};

struct SpOutV {  // superposeout exit {z0, x0, y0, thz, thx, thy, z_exit} and sincos
    double row[7];
    double s3, c3, s4, c4, s5, c5;
};

struct GeomV {  // boundary.txt geometry
    int shape;
    int npar;
    double zStart;
    double par[6];
    double place[6];
    double s3, c3, s4, c4, s5, c5;
};

struct LatV {
    const ElemV* elems;
    const ElemV* eff;  // corrected engine: superpose blocks with the sub-field errors applied
    const SubV* subs;
    const MapView<double>* maps;
    const MapView<float, float>* mapsF;  // fp32 GPU path: the maps in float
    const double* elemZStart;
    const double* elemZEnd;
    const SpOutV* spOut;
    const double* cavTime;
    const double* cavPhase;
    const RfSlotV* rfSlots;
    int nElem;
    int errOn;
    int spOutCur, spOutCount, spOutSize;
    double totalLength, spExit;
    // boundary.txt
    const GeomV* geoms;
    const double* segStart;
    const double* segEnd;
    int nSeg;
    const double* bndZ;  // per superposeout section: z offset, first and last geometry
    const int* bndR1;
    const int* bndR2;
    int nBnd;
};

// Particle arrays.  rel[] are the coordinates relative to the synchronous particle, old the
// coordinates before the last step, rec / spRec the crossing records {x, ux, y, uy, z, uz, t, flag}.
struct PartV {
    double *x, *ux, *y, *uy, *z, *uz;
    double *ex, *ey, *ez, *bx, *by, *bz;
    double *scx, *scy, *scz;
    double* rel[6];
    int *elemIndex, *lost, *nextPlane, *boundaryMode, *geomIndex, *spOutCount;
    double *weight, *qOverM, *charge, *spOutZ0, *spOutZ1;
    double* old;    // 6 per particle
    double* rec;    // 8 per particle
    double* spRec;  // 8 per particle (null without superposeout blocks)
    double* t;      // own time (secondary particles)
    double* dt;     // own time step (secondary particles)
    int sync;       // index of the synchronous particle in this view, -1 when it is not part of it
};

// RF field factors of a slot: from the per-step table, or from the particle's own time.
struct RfSrc {
    const double* table;  // 2 per slot, at the step time (null: compute from t)
    double t;
};

template <class P>
AVAS_HD inline void rf_factors(const LatV& L, const RfSrc& rf, int slot, double& ce, double& cb) {
    if (rf.table) {
        ce = rf.table[2 * slot];
        cb = rf.table[2 * slot + 1];
        return;
    }
    const RfSlotV& s = L.rfSlots[slot];
    const double ph = s.hasAdd ? (rf.t - L.cavTime[s.cav]) * s.freq * k::two_pi + (L.cavPhase[s.cav] + s.add) * k::deg_to_rad
                               : (rf.t - L.cavTime[s.cav]) * s.freq * k::two_pi + L.cavPhase[s.cav] * k::deg_to_rad;
    ce = P::cos(ph);
    cb = P::cos(ph - k::half_pi);
}

// ---- field of an element at a particle -----------------------------------------------------
// The functions take the particle as FieldIn: x, y and fz = z - zr.  The fp64 engine passes zr = 0
// and the absolute lattice positions (AbsZ), so every expression is exactly the original one; the
// fp32 GPU path passes coordinates relative to the synchronous particle and the lattice positions
// relative to it (computed in double on the host once per step).  They return kFieldWrite when the
// particle's field is replaced by E, B and kFieldLost when the particle is lost.

enum : int { kFieldWrite = 1, kFieldLost = 2, kFieldLostEnd = 4 };

template <class R>
struct FieldIn {
    R x, y, fz;
    R rel0, rel2;        // coordinates relative to the synchronous particle (rotated beam frame)
    double spZ0, spZ1;   // superposeout section of the particle, minus zr
    int boundaryMode;
};

struct AbsZ {
    const LatV* L;
    AVAS_HD double elem_start(int k) const { return L->elemZStart[k]; }
    AVAS_HD double elem_end(int k) const { return L->elemZEnd[k]; }
    AVAS_HD double estart(int, const ElemV& e) const { return e.zStart; }
    AVAS_HD double sstart(int, const SubV& S) const { return S.zStart; }
    AVAS_HD double zero() const { return 0.0; }
    AVAS_HD double sp_exit() const { return L->spExit; }
    AVAS_HD double total() const { return L->totalLength; }
};

template <class R>
struct MapsOf {
    AVAS_HD static const MapView<double>& get(const LatV& L, int k) { return L.maps[k]; }
};
template <>
struct MapsOf<float> {
    AVAS_HD static const MapView<float, float>& get(const LatV& L, int k) { return L.mapsF[k]; }
};

template <class R>
AVAS_HD inline void zero3(R* E, R* B) {
    E[0] = E[1] = E[2] = R(0);
    B[0] = B[1] = B[2] = R(0);
}

template <class R, class Z>
AVAS_HD inline bool aperture_lost_t(const Z& zo, int k, const ElemV& e, const FieldIn<R>& in) {
    const R r = hm::sqrt(in.x * in.x + in.y * in.y);
    return R(e.radius) <= r && in.boundaryMode == 0 && zo.estart(k, e) + 1e-06 < in.spZ1 && in.spZ0 < in.fz;
}

// Field::GetField type 4: sum of the sub-fields of a superpose block (spec 30 2.4.2).
template <class P, class R, class Z>
AVAS_HD inline int superpose_field_t(const LatV& L, const Z& zo, int k, const ElemV& e, const FieldIn<R>& in,
                                     const RfSrc& rf, R* E, R* B) {
    zero3(E, B);
    const R x = in.x, y = in.y;
    for (int j = 0; j < e.nsub; ++j) {
        const SubV& S = L.subs[e.sub0 + j];
        const double* Pl = S.place;
        const R dz = in.fz - R(zo.sstart(e.sub0 + j, S));
        const R X0 = x - R(Pl[1]), Y0 = y - R(Pl[2]);
        const R s3 = R(S.s3), c3 = R(S.c3), s4 = R(S.s4), c4 = R(S.c4), s5 = R(S.s5), c5 = R(S.c5);
        const R u = c3 * X0 + s3 * Y0;
        const R v = Y0 * c3 - X0 * s3;
        const R xl = c5 * u - dz * s5;
        const R w = u * s5 + dz * c5;
        const R zl = w * c4 - v * s4;
        const R yl = s4 * w + v * c4;
        if (R(0) < zl && zl < R(S.length) && zo.estart(k, e) + 1e-06 < in.spZ1 && in.spZ0 < in.fz) {
            const R r = e.rotMode == 0 ? hm::sqrt(xl * xl + yl * yl) : hm::sqrt(in.rel0 * in.rel0 + in.rel2 * in.rel2);
            if (R(S.radius) <= r) return kFieldWrite | kFieldLost;  // the partial sum stays in the particle
        }
        const int st = S.type;
        if (st != 1 && st != 2 && st != 3) continue;
        const auto& m = MapsOf<R>::get(L, S.map);
        if (!(m.xmin < xl && xl < m.xmax && m.ymin < yl && yl < m.ymax && R(0) < zl && zl < m.zlen)) continue;
        R Sv[3] = {R(0), R(0), R(0)}, SB[3] = {R(0), R(0), R(0)};
        if (st == 1) {
            const R kb = R(S.kb);
            trilinear(m, xl, yl, zl, [&](R wt, const auto* f) {
                for (int q = 0; q < 3; ++q) Sv[q] = wt * f[q] * kb + Sv[q];
            });
        } else if (st == 3) {
            const R ke = R(S.ke);
            trilinear(m, xl, yl, zl, [&](R wt, const auto* f) {
                for (int q = 0; q < 3; ++q) Sv[q] = wt * f[q] * ke * R(1000000.0) + Sv[q];
            });
        } else {
            const R ke6 = R(S.ke * 1000000.0), kb = R(S.kb);
            trilinear(m, xl, yl, zl, [&](R wt, const auto* f) {
                Sv[0] = wt * f[0] * ke6 + Sv[0];
                Sv[1] = f[1] * wt * ke6 + Sv[1];
                Sv[2] = f[2] * wt * ke6 + Sv[2];
                SB[0] = f[3] * wt * kb + SB[0];
                SB[1] = f[4] * wt * kb + SB[1];
                SB[2] = f[5] * wt * kb + SB[2];
            });
        }
        if (st == 1 || st == 3) {
            R* D = st == 1 ? B : E;
            R* O = st == 1 ? E : B;
            const R a = Sv[1] * c4 - s4 * Sv[2];
            const R bb = Sv[1] * s4 + c4 * Sv[2];
            const R cz = bb * c5;
            const R d = Sv[0] * c5 + s5 * bb;
            D[0] = (c3 * d - a * s3) + D[0];
            D[1] = (s3 * d + a * c3) + D[1];
            D[2] = (cz - s5 * Sv[0]) + D[2];
            for (int q = 0; q < 3; ++q) O[q] = O[q] + R(0);
        } else {
            double ced, cbd;
            rf_factors<P>(L, rf, e.rfErr, ced, cbd);
            const R cE = R(ced), cB = R(cbd);
            const R a = cE * Sv[1] * c4 - cE * Sv[2] * s4;
            const R bb = s4 * cE * Sv[1] + cE * Sv[2] * c4;
            const R cz = bb * c5;
            const R d = Sv[0] * cE * c5 + s5 * bb;
            E[0] = (c3 * d - a * s3) + E[0];
            E[1] = (d * s3 + a * c3) + E[1];
            E[2] = (cz - s5 * Sv[0] * cE) + E[2];
            const R a2 = cB * SB[1] * c4 - cB * SB[2] * s4;
            const R b2 = cB * SB[1] * s4 + cB * SB[2] * c4;
            const R cz2 = b2 * c5;
            const R d2 = SB[0] * cB * c5 + s5 * b2;
            B[0] = (c3 * d2 - a2 * s3) + B[0];
            B[1] = (s3 * d2 + a2 * c3) + B[1];
            B[2] = (cz2 - s5 * SB[0] * cB) + B[2];
        }
    }
    return kFieldWrite;
}

// Field::GetField of element k (spec 30 2.4).
template <class P, class R, class Z>
AVAS_HD inline int element_field_t(const LatV& L, const Z& zo, int k, const ElemV& e, const FieldIn<R>& in,
                                   const RfSrc& rf, R* E, R* B) {
    if (e.kind == kDrift) {
        const bool lost = aperture_lost_t(zo, k, e, in);
        zero3(E, B);
        return kFieldWrite | (lost ? kFieldLost : 0);
    }
    if (e.kind != kField) {
        // t-code fields of the hard-edge elements: quad and solenoid simplified, others none
        zero3(E, B);
        if (e.kind == kQuad) {
            B[0] = in.y * R(e.strength);
            B[1] = R(e.strength) * in.x;
        } else if (e.kind == kSolenoid) {
            B[2] = R(e.strength);
        }
        return kFieldWrite;
    }
    const int type = e.fieldType;
    if (type == 4) return superpose_field_t<P>(L, zo, k, e, in, rf, E, B);
    if (type != 1 && type != 2 && type != 3) return 0;
    if (aperture_lost_t(zo, k, e, in)) return kFieldLost;  // fields keep their previous values
    const auto& m = MapsOf<R>::get(L, e.mapIndex);
    const R x = in.x, y = in.y;
    const R zs = R(zo.estart(k, e));
    if (!(m.xmin < x && x < m.xmax && m.ymin < y && y < m.ymax && zs < in.fz && in.fz < zs + m.zlen)) {
        zero3(E, B);
        return kFieldWrite;
    }
    const R zl = in.fz - zs;
    if (type == 1) {
        R se[3] = {R(0), R(0), R(0)}, sb[3] = {R(0), R(0), R(0)};
        const R ke6 = R(e.ke * 1000000.0), kb = R(e.kb);
        trilinear(m, x, y, zl, [&](R w, const auto* f) {
            se[0] = w * f[0] * ke6 + se[0];
            se[1] = f[1] * w * ke6 + se[1];
            se[2] = f[2] * w * ke6 + se[2];
            sb[0] = f[3] * w * kb + sb[0];
            sb[1] = f[4] * w * kb + sb[1];
            sb[2] = f[5] * w * kb + sb[2];
        });
        double ced, cbd;
        rf_factors<P>(L, rf, e.rfPlain, ced, cbd);
        const R ce = R(ced), cb = R(cbd);
        E[0] = ce * se[0];
        B[0] = cb * sb[0];
        E[1] = ce * se[1];
        B[1] = cb * sb[1];
        E[2] = ce * se[2];
        B[2] = cb * sb[2];
    } else if (type == 2) {
        R se[3] = {R(0), R(0), R(0)};
        const R ke = R(e.ke);
        trilinear(m, x, y, zl, [&](R w, const auto* f) {
            for (int j = 0; j < 3; ++j) se[j] = w * f[j] * (ke * R(1000000.0)) + se[j];
        });
        E[0] = se[0]; E[1] = se[1]; E[2] = se[2];
        B[0] = B[1] = B[2] = R(0);
    } else {
        R sb[3] = {R(0), R(0), R(0)};
        const R kb = R(e.kb);
        trilinear(m, x, y, zl, [&](R w, const auto* f) {
            for (int j = 0; j < 3; ++j) sb[j] = w * f[j] * kb + sb[j];
        });
        E[0] = E[1] = E[2] = R(0);
        B[0] = sb[0]; B[1] = sb[1]; B[2] = sb[2];
    }
    return kFieldWrite;
}

// Field::GetErrField type 4 of AVAS.dll (spec 70 6.3; compat only): the sub-field placement of the
// block is not used; an erroneous sub-field is displaced and rotated by its error only.
template <class P, class R, class Z>
AVAS_HD inline int superpose_err_field_dll_t(const LatV& L, const Z& zo, const ElemV& e, const FieldIn<R>& in,
                                             const RfSrc& rf, R* E, R* B) {
    zero3(E, B);
    const R x = in.x, y = in.y, z = in.fz;
    for (int q = 0; q < e.nsub; ++q) {
        const SubV& S = L.subs[e.sub0 + q];
        const double* se = S.err;
        const R szs = R(zo.sstart(e.sub0 + q, S));
        R xl = x, yl = y, zl = z - szs;
        const bool err = S.hasErr != 0;
        R sx = 0, cx = 1, sy = 0, cy = 1, sz = 0, cz = 1;
        if (err) {
            const R X0 = x - R(se[1]), Y0 = y - R(se[2]);
            sz = R(S.es6); cz = R(S.ec6);
            const R u = sz * Y0 + cz * X0;
            const R v = cz * Y0 - sz * X0;
            const R Z0 = (z - R(se[3])) - szs;
            sy = R(S.es5); cy = R(S.ec5);
            const R w = cy * Z0 + sy * u;
            xl = cy * u - sy * Z0;
            sx = R(S.es4); cx = R(S.ec4);
            yl = sx * w + cx * v;
            zl = cx * w - sx * v;
        }
        if (R(0) < zl && zl < R(S.length) && R(S.radius) <= hm::sqrt(yl * yl + xl * xl)) return kFieldWrite | kFieldLost;
        const int st = S.type;
        if (st != 1 && st != 2 && st != 3) continue;
        const auto& m = MapsOf<R>::get(L, S.map);
        if (!(m.xmin < xl && xl < m.xmax && m.ymin < yl && yl < m.ymax && R(0) < zl && zl < m.zlen)) continue;
        R F[3] = {0, 0, 0}, G[3] = {0, 0, 0};
        const R e7 = R(se[7]);
        if (st == 1) {
            trilinear(m, xl, yl, zl, [&](R wt, const auto* f) {
                for (int j = 0; j < 3; ++j) F[j] = f[j] * wt * e7 + F[j];
            });
        } else if (st == 3) {
            const R ke6 = R(se[8] * 1000000.0);
            trilinear(m, xl, yl, zl, [&](R wt, const auto* f) {
                for (int j = 0; j < 3; ++j) F[j] = f[j] * wt * ke6 + F[j];
            });
        } else {
            const R ke6 = R(se[8] * 1000000.0);
            trilinear(m, xl, yl, zl, [&](R wt, const auto* f) {
                for (int j = 0; j < 3; ++j) {
                    F[j] = f[j] * wt * ke6 + F[j];
                    G[j] = f[3 + j] * wt * e7 + G[j];
                }
            });
            double ced, cbd;
            rf_factors<P>(L, rf, S.rfErr, ced, cbd);
            for (int j = 0; j < 3; ++j) {
                F[j] = R(ced) * F[j];
                G[j] = R(cbd) * G[j];
            }
        }
        const R esin6 = R(S.esin6);
        auto turn = [&](R* V) {
            if (!err) return;
            const R v0 = V[0];
            const R a = sx * V[1] + cx * V[2];
            const R bb = sy * a + cy * v0;
            const R yy = cx * V[1] - sx * V[2];
            V[0] = cz * bb - sz * yy;
            V[1] = esin6 * bb + cz * yy;
            V[2] = cy * a - sy * v0;
        };
        if (st == 1) {
            turn(F);
            for (int j = 0; j < 3; ++j) B[j] = (j == 0 ? B[j] + F[j] : F[j] + B[j]);
        } else if (st == 3) {
            turn(F);
            for (int j = 0; j < 3; ++j) E[j] = (j == 0 ? E[j] + F[j] : F[j] + E[j]);
        } else {
            turn(F);
            turn(G);
            for (int j = 0; j < 3; ++j) {
                E[j] = (j == 0 ? E[j] + F[j] : F[j] + E[j]);
                B[j] = (j == 0 ? B[j] + G[j] : G[j] + B[j]);
            }
        }
    }
    return kFieldWrite;
}

// Field::GetErrField (spec 70 6.3): the particle in the frame of the displaced and rotated field
// map, the field rotated back to the laboratory.  Other elements: as GetField.  k = element index.
template <class P, class R, class Z>
AVAS_HD inline int element_err_field_t(const LatV& L, const Z& zo, int k, const FieldIn<R>& in, const RfSrc& rf, R* E,
                                       R* B) {
    const ElemV& e = L.elems[k];
    if (e.kind == kField && e.fieldType == 4) {
        if (P::compat) return superpose_err_field_dll_t<P>(L, zo, e, in, rf, E, B);
        return superpose_field_t<P>(L, zo, k, L.eff[k], in, rf, E, B);
    }
    if (e.kind != kField || (e.fieldType != 1 && e.fieldType != 2 && e.fieldType != 3))
        return element_field_t<P>(L, zo, k, e, in, rf, E, B);
    const R X0 = in.x - R(e.errDx);
    const R Y0 = in.y - R(e.errDy);
    const R sz = R(e.esz), cz = R(e.ecz), sy = R(e.esy), cy = R(e.ecy), sx = R(e.esx), cx = R(e.ecx);
    const R X1 = X0 * cz + Y0 * sz;
    const R Y1 = Y0 * cz - X0 * sz;
    const R Z0 = (in.fz - R(e.errDz)) - R(zo.estart(k, e));
    const R xl = cy * X1 - Z0 * sy;
    const R T = Z0 * cy + sy * X1;
    const R yl = sx * T + cx * Y1;
    const R zl = cx * T - sx * Y1;
    const R r = hm::sqrt(yl * yl + xl * xl);
    // AVAS.dll checks the aperture without the boundary mode and the superposeout planes
    const bool inside = R(0) < zl && zl < R(e.length);
    const bool lostHere = P::compat ? (inside && !(r < R(e.radius)))
                                    : (inside && R(e.radius) <= r && in.boundaryMode == 0 &&
                                       zo.estart(k, e) + 1e-06 < in.spZ1 && in.spZ0 < in.fz);
    if (lostHere) return kFieldLost;  // fields keep their previous values
    const auto& m = MapsOf<R>::get(L, e.mapIndex);
    if (!(m.xmin < xl && xl < m.xmax && m.ymin < yl && yl < m.ymax && R(0) < zl && zl < m.zlen)) {
        zero3(E, B);
        return kFieldWrite;
    }
    zero3(E, B);
    if (e.fieldType == 1) {
        const R ke6 = R(e.keErr * 1000000.0), kb = R(e.kbErr);
        trilinear(m, xl, yl, zl, [&](R w, const auto* f) {
            E[0] = f[0] * w * ke6 + E[0];
            E[1] = f[1] * w * ke6 + E[1];
            E[2] = f[2] * w * ke6 + E[2];
            B[0] = f[3] * w * kb + B[0];
            B[1] = f[4] * w * kb + B[1];
            B[2] = f[5] * w * kb + B[2];
        });
        double ced, cbd;
        rf_factors<P>(L, rf, e.rfErr, ced, cbd);
        for (int j = 0; j < 3; ++j) {
            E[j] = R(ced) * E[j];
            B[j] = R(cbd) * B[j];
        }
    } else if (e.fieldType == 3) {
        const R kb = R(e.kbErr);
        trilinear(m, xl, yl, zl, [&](R w, const auto* f) {
            for (int j = 0; j < 3; ++j) B[j] = f[j] * w * kb + B[j];
        });
    } else if (!P::compat) {
        // static electric map: AVAS.dll returns zero here (never an error target there)
        const R ke6 = R(e.keErr * 1000000.0);
        trilinear(m, xl, yl, zl, [&](R w, const auto* f) {
            for (int j = 0; j < 3; ++j) E[j] = f[j] * w * ke6 + E[j];
        });
    }
    auto back = [&](R* F) {
        const R a = F[2] * cx + F[1] * sx;  // z after the x rotation
        const R bb = a * sy + F[0] * cy;   // x after the y rotation
        const R yy = F[1] * cx - F[2] * sx;
        const R zz = a * cy - F[0] * sy;
        F[0] = bb * cz - yy * sz;
        F[1] = yy * cz + bb * sz;
        F[2] = zz;
    };
    back(E);
    back(B);
    return kFieldWrite;
}

// Lattice::GetExternalField of one particle: element search (k in/out), then its field.
template <class P, class R, class Z>
AVAS_HD inline int external_field_t(const LatV& L, const Z& zo, const FieldIn<R>& in, int lost, bool isSync, int& k,
                                    const RfSrc& rf, R* E, R* B) {
    const R z = in.fz;
    if (z < R(zo.zero()) || R(zo.sp_exit()) < z || lost != 0) {
        zero3(E, B);
        if (R(zo.total()) < z && L.spOutCur == L.spOutSize && lost == 0) return kFieldWrite | kFieldLostEnd;
        return kFieldWrite;
    }
    const int last = L.nElem - 1;
    if (z < R(zo.elem_start(k))) {
        do { --k; } while (k > 0 && z < R(zo.elem_start(k)));
    } else if (R(zo.elem_end(k)) <= z) {
        do { ++k; } while (k < last && R(zo.elem_end(k)) <= z);
    }
    if (k < 0) k = 0;
    if (k > last) k = last;
    // Lattice::GetErrExternalField: elements marked in the error sample use GetErrField; the
    // synchronous particle always uses GetField
    if (L.errOn && !isSync && L.elems[k].errUse) return element_err_field_t<P>(L, zo, k, in, rf, E, B);
    return element_field_t<P>(L, zo, k, L.elems[k], in, rf, E, B);
}

// ---- fp64 wrappers on the particle arrays ----

AVAS_HD inline FieldIn<double> field_in(const PartV& b, int i) {
    FieldIn<double> in;
    in.x = b.x[i];
    in.y = b.y[i];
    in.fz = b.z[i];
    in.rel0 = b.rel[0][i];
    in.rel2 = b.rel[2][i];
    in.spZ0 = b.spOutZ0[i];
    in.spZ1 = b.spOutZ1[i];
    in.boundaryMode = b.boundaryMode[i];
    return in;
}

AVAS_HD inline void zero_field(const PartV& b, int i) {
    b.ex[i] = b.ey[i] = b.ez[i] = b.bx[i] = b.by[i] = b.bz[i] = 0.0;
}

AVAS_HD inline void apply_field(const PartV& b, int i, int f, const double* E, const double* B) {
    if (f & kFieldLost) b.lost[i] = 1;
    if (f & kFieldLostEnd) b.lost[i] = 2;
    if (f & kFieldWrite) {
        b.ex[i] = E[0]; b.ey[i] = E[1]; b.ez[i] = E[2];
        b.bx[i] = B[0]; b.by[i] = B[1]; b.bz[i] = B[2];
    }
}

template <class P>
AVAS_HD inline void element_field(const LatV& L, const PartV& b, int i, int k, const RfSrc& rf) {
    double E[3], B[3];
    const int f = element_field_t<P>(L, AbsZ{&L}, k, L.elems[k], field_in(b, i), rf, E, B);
    apply_field(b, i, f, E, B);
}

template <class P>
AVAS_HD inline void element_err_field(const LatV& L, const PartV& b, int i, int k, const RfSrc& rf) {
    double E[3], B[3];
    const int f = element_err_field_t<P>(L, AbsZ{&L}, k, field_in(b, i), rf, E, B);
    apply_field(b, i, f, E, B);
}

template <class P>
AVAS_HD inline void external_field(const LatV& L, const PartV& b, int i, const RfSrc& rf) {
    double E[3], B[3];
    int k = b.elemIndex[i];
    const int f = external_field_t<P>(L, AbsZ{&L}, field_in(b, i), b.lost[i], i == b.sync, k, rf, E, B);
    if (!(b.z[i] < 0.0 || L.spExit < b.z[i] || b.lost[i] != 0)) b.elemIndex[i] = k;
    apply_field(b, i, f, E, B);
}

// Coordinates {x, ux, y, uy, z, uz} in the frame of a superposeout exit (spec 20 2.6.6).
AVAS_HD inline void sp_local(const SpOutV& S, double zStart, const double* c, double* o) {
    const double* row = S.row;
    const double s3 = S.s3, c3 = S.c3, s4 = S.s4, c4 = S.c4, s5 = S.s5, c5 = S.c5;
    auto turn = [&](double X, double Y, double Z, double& xo, double& yo, double& zo) {
        const double X1 = c3 * X + s3 * Y;
        const double Y1 = c3 * Y - s3 * X;
        const double X2 = c5 * X1 - s5 * Z;
        const double Z2 = s5 * X1 + c5 * Z;
        zo = c4 * Z2 - s4 * Y1;
        yo = c4 * Y1 + s4 * Z2;
        xo = X2;
    };
    turn(c[0] - row[1], c[2] - row[2], (c[4] - row[0]) - zStart, o[0], o[2], o[4]);
    turn(c[1], c[3], c[5], o[1], o[3], o[5]);
}

// A linear interpolation of the momentum vector within a step shortens it when a magnetic field
// turns it; the corrected engine interpolates the direction linearly and the magnitude separately.
AVAS_HD inline void interp_momentum_norm(double* rec, const double* a, const double* b, double r) {
    const double pa = hm::sqrt(a[1] * a[1] + a[3] * a[3] + a[5] * a[5]);
    const double pb = hm::sqrt(b[1] * b[1] + b[3] * b[3] + b[5] * b[5]);
    const double pr = hm::sqrt(rec[1] * rec[1] + rec[3] * rec[3] + rec[5] * rec[5]);
    if (!(pr > 0.0)) return;
    const double f = ((pb - pa) * r + pa) / pr;
    rec[1] *= f;
    rec[3] *= f;
    rec[5] *= f;
}

// ---- one time step of one particle (TcodeTrans) ----

struct StepArgs {
    double dt, tEnd, zField, zMinPrev, gp, segEnd;
    double sync[6];
    double cen[6];
    const double* events;
    int nEvents;
    int rotMode;
    double rs, rc;  // sincos(rotAngle)
    // fp32 GPU path only: the synchronous particle before its step, the field of its push, q/m
    double syncPre[6];
    double syncTot[6];
    double qmSync;
    int writeOld;   // fp32: keep the fp64 coordinates before the step (boundary.txt)
};

// Contributions of one particle to the beam statistics of a step.
// s: 0-5 sum of d^2 w, 6-8 cross terms, 9 (gp dz)^2 w, 10 count, 11 weight, 12-17 sum of w rel, 18 gamma w
constexpr int kStepSums = 19;
struct Contrib {
    double s[kStepSums];
    double mx[6];
    double zmin, zmax;
    int sp;
    int hasA, hasB;
};

template <class P>
AVAS_HD inline void trans_particle(const LatV& L, const PartV& b, int i, const StepArgs& a, const RfSrc& rf,
                                   Contrib& c) {
    c.hasA = c.hasB = 0;
    c.sp = 0;
    if (b.lost[i] != 1 && i != b.sync) {
        c.hasA = 1;
        const double* cen = a.cen;
        const double w = b.weight[i];
        const double d0 = b.rel[0][i] - cen[0];
        const double d2 = b.rel[2][i] - cen[2];
        const double d4 = b.rel[4][i] - cen[4];
        const double inv = 1.0 / b.rel[5][i];
        const double d1 = (b.rel[1][i] - cen[1]) * inv;
        const double d3 = (b.rel[3][i] - cen[3]) * inv;
        const double d5 = (b.rel[5][i] - cen[5]) * inv;
        c.zmin = c.zmax = b.z[i];
        const double gp = a.gp;
        c.s[9] = gp * d4 * gp * d4 * w;
        const double d[6] = {d0, d1, d2, d3, d4, d5};
        for (int j = 0; j < 6; ++j) {
            c.mx[j] = hm::fabs(d[j]);
            c.s[j] = d[j] * d[j] * w;
        }
        c.s[6] = d1 * d0 * w;
        c.s[7] = d3 * d2 * w;
        c.s[8] = d5 * d4 * w;
    }
    double tot[6];
    if (a.zMinPrev <= a.zField && !(a.zField < b.z[i])) {
        for (int j = 0; j < 6; ++j) tot[j] = 0.0;
    } else {
        tot[0] = b.ex[i] + b.scx[i]; tot[1] = b.ey[i] + b.scy[i]; tot[2] = b.ez[i] + b.scz[i];
        tot[3] = b.bx[i] + 0.0; tot[4] = b.by[i] + 0.0; tot[5] = b.bz[i] + 0.0;
    }
    double* old = b.old + 6 * static_cast<long long>(i);
    old[0] = b.x[i]; old[1] = b.ux[i]; old[2] = b.y[i]; old[3] = b.uy[i]; old[4] = b.z[i]; old[5] = b.uz[i];
    if (b.lost[i] == 1) return;
    boris_push(b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i], tot, tot + 3, b.qOverM[i], a.dt);
    const double* sync = a.sync;
    if (a.rotMode == 0) {
        b.rel[0][i] = b.x[i] - sync[0];
        b.rel[2][i] = b.y[i] - sync[2];
        b.rel[4][i] = b.z[i] - sync[4];
        b.rel[1][i] = b.ux[i];
        b.rel[3][i] = b.uy[i];
        b.rel[5][i] = b.uz[i];
    } else {
        // coordinates relative to the synchronous particle, turned by the beam angle
        const double sn = a.rs, cs = a.rc;
        const double dz = b.z[i] - sync[4];
        if (a.rotMode == 2) {
            const double dx = b.x[i] - sync[0];
            b.rel[0][i] = cs * dx - sn * dz;
            b.rel[4][i] = sn * dx + cs * dz;
            b.rel[1][i] = b.ux[i] * cs - b.uz[i] * sn;
            b.rel[5][i] = b.uz[i] * cs + b.ux[i] * sn;
            b.rel[2][i] = b.y[i] - sync[2];
            b.rel[3][i] = b.uy[i];
        } else {
            const double dy = b.y[i] - sync[2];
            b.rel[2][i] = cs * dy + sn * dz;
            b.rel[4][i] = dz * cs - dy * sn;
            b.rel[3][i] = b.uy[i] * cs + b.uz[i] * sn;
            b.rel[5][i] = b.uz[i] * cs - b.uy[i] * sn;
            b.rel[0][i] = b.x[i] - sync[0];
            b.rel[1][i] = b.ux[i];
        }
    }
    external_field<P>(L, b, i, rf);
    if (!P::compat && b.z[i] > a.segEnd) {
        // past the end of this time-step segment the particle is already recorded for the next
        // (hard-edge) segment: it drifts without field (AVAS.dll applies the simplified t-code
        // fields of the hard-edge elements there)
        zero_field(b, i);
    }
    if (b.uz[i] < 0.0 && L.spOutCount <= L.spOutCur && b.z[i] < 0.0) b.lost[i] = 1;
    const int m = b.nextPlane[i];
    if (m + 1 <= a.nEvents) {
        const double pl = a.events[m];
        if (pl <= b.z[i] && pl <= L.spExit) {
            const double r = (pl - old[4]) / (b.z[i] - old[4]);
            double* rec = b.rec + 8 * static_cast<long long>(i);
            const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
            for (int j = 0; j < 6; ++j) rec[j] = (cur[j] - old[j]) * r + old[j];
            if (!P::compat) interp_momentum_norm(rec, old, cur, r);
            rec[6] = (r - 1.0) * a.dt + a.tEnd;
            rec[7] = 1.0;
            b.nextPlane[i] += 1;
        }
    }
    // superposeout exit crossing (recorded in the exit frame; applied by tcode)
    if (L.spOutCur < L.spOutCount && L.elems[b.elemIndex[i]].inSuperposeOut != 0 &&
        b.spRec[8 * static_cast<long long>(i) + 7] == 0.0 && b.z[i] > b.spOutZ0[i]) {
        const SpOutV& row = L.spOut[L.spOutCur];
        const double zs = L.elems[b.elemIndex[i]].zStart;
        const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
        double lc[6], lp[6];
        sp_local(row, zs, cur, lc);
        sp_local(row, zs, old, lp);
        if (lc[4] > 0.0) {
            const double r = lp[4] / (lp[4] - lc[4]);
            double* sr = b.spRec + 8 * static_cast<long long>(i);
            for (int j = 0; j < 6; ++j) sr[j] = (lc[j] - lp[j]) * r + lp[j];
            if (!P::compat) interp_momentum_norm(sr, lp, lc, r);
            sr[6] = (r - 1.0) * a.dt + a.tEnd;
            sr[7] = 1.0;
            c.sp = 1;
        }
    }
    if (b.lost[i] != 1 && i != b.sync) {
        c.hasB = 1;
        const double w = b.weight[i];
        c.s[10] = 1.0;
        c.s[11] = w;
        for (int j = 0; j < 6; ++j) c.s[12 + j] = w * b.rel[j][i];
        const double u2 = P::compat ? b.uz[i] * b.uz[i] : b.ux[i] * b.ux[i] + b.uy[i] * b.uy[i] + b.uz[i] * b.uz[i];
        c.s[18] = hm::sqrt(u2 + 1.0) * w;
    }
}

// The step contributions in the fixed-order layout: missing parts are +0.0, extrema neutral.
AVAS_HD inline void contrib_sums(const Contrib& c, double* s) {
    for (int j = 0; j < 10; ++j) s[j] = c.hasA ? c.s[j] : 0.0;
    for (int j = 10; j < kStepSums; ++j) s[j] = c.hasB ? c.s[j] : 0.0;
}

// ---- UpdateBeamParameters (corrected engine): pass 1 sets rel and gives the sums ----
// sums: 0 weight, 1-6 w rel, 7 gamma w, 8 count
constexpr int kUbpSums = 9;
AVAS_HD inline void ubp_pass1(const PartV& b, int i, const double* s6, double* c) {
    for (int k = 0; k < kUbpSums; ++k) c[k] = 0.0;
    if (b.lost[i] == 1) return;
    b.rel[0][i] = b.x[i] - s6[0];
    b.rel[2][i] = b.y[i] - s6[2];
    b.rel[4][i] = b.z[i] - s6[4];
    b.rel[1][i] = b.ux[i];
    b.rel[3][i] = b.uy[i];
    b.rel[5][i] = b.uz[i];
    const double w = b.weight[i];
    c[0] = w;
    for (int j = 0; j < 6; ++j) c[1 + j] = w * b.rel[j][i];
    c[7] = hm::sqrt(b.ux[i] * b.ux[i] + b.uy[i] * b.uy[i] + b.uz[i] * b.uz[i] + 1.0) * w;
    c[8] = 1.0;
}

// pass 2: rms of the first space-charge mesh (relative coordinates, live particles)
AVAS_HD inline void ubp_pass2(const PartV& b, int i, const double* cen, double igz, double* c) {
    c[0] = c[1] = c[2] = 0.0;
    if (b.lost[i] == 1) return;
    const double w = b.weight[i];
    const double ax = b.rel[0][i] - cen[0];
    const double ay = b.rel[2][i] - cen[2];
    const double az = (b.rel[4][i] - cen[4]) * igz;
    c[0] = w * ax * ax;
    c[1] = w * ay * ay;
    c[2] = w * az * az;
}

// ---- z-code rows ----

// z-code misalignment (spec 30 2.8.3): entrance transform T_in into the element frame; returns the
// local longitudinal position, which T_in drops.
AVAS_HD inline double het_err_in(double* r, const ElemV& e) {
    const double X0 = r[0] - e.errDx, Y0 = r[2] - e.errDy, mdz = -e.errDz;
    const double sz = e.esz, cz = e.ecz, sy = e.esy, cy = e.ecy, sx = e.esx, cx = e.ecx;
    const double u = sz * Y0 + cz * X0;
    const double v = cz * Y0 - sz * X0;
    const double px1 = cz * r[1] + sz * r[3];
    const double py1 = cz * r[3] - sz * r[1];
    const double pz1 = cy * r[5] + sy * px1;
    const double T = sy * u + cy * mdz;
    r[0] = cy * u - sy * mdz;
    r[1] = cy * px1 - sy * r[5];
    r[2] = v * cx + T * sx;
    r[3] = py1 * cx + sx * pz1;
    r[5] = pz1 * cx - sx * py1;
    return cx * T - sx * v;
}

// Exit transform T_out.  AVAS.dll adds the offsets before rotating back (and takes +dz as the local
// z), so T_out is not the inverse of T_in when the element is rotated; the corrected engine rotates
// back and then adds the offsets (exact inverse, zl from T_in).
AVAS_HD inline void het_err_out(double* r, const ElemV& e, double zl, bool compat) {
    const double sx = e.esx, cx = e.ecx, sy = e.esy, cy = e.ecy, sz = e.esz, cz = e.ecz;
    double q, w;
    if (compat) {
        const double Y1 = e.errDy + r[2];
        q = (Y1 * sx + e.errDz * cx) * sy + (e.errDx + r[0]) * cy;
        w = Y1 * cx - e.errDz * sx;
    } else {
        w = cx * r[2] - sx * zl;
        const double T = sx * r[2] + cx * zl;
        q = cy * r[0] + sy * T;
    }
    const double pxo = r[1];
    const double pz1 = r[5] * cx + r[3] * sx;
    const double px1 = pz1 * sy + pxo * cy;
    const double py1 = r[3] * cx - r[5] * sx;
    r[0] = q * cz - sz * w;
    r[1] = px1 * cz - py1 * sz;
    r[2] = q * sz + cz * w;
    r[3] = py1 * cz + px1 * sz;
    r[5] = pz1 * cy - pxo * sy;
    if (!compat) {
        r[0] = r[0] + e.errDx;
        r[2] = r[2] + e.errDy;
    }
}

// One hard-edge transfer of the rows (MethodMP::HardEdgeTrans), prepared by the host.
struct RowOp {
    int kind;      // kQuad, kSolenoid, kBend, kEdge
    int scKick;    // space-charge kick of both half steps before the transfer
    int err;       // misaligned element (T_in / T_out)
    int hv;
    double strength, len, uzs, k1, k2;
    BendStep<double> bs;
    ElemV e;       // misalignment of the element
};

template <class P>
struct PSinCos {
    AVAS_HD void operator()(double a, double& s, double& c) const { P::sincos(a, s, c); }
};

template <class P>
AVAS_HD inline void quad_row_p(double* r, double qOverM, double G, double len) {
    double xa, ya;
    const double g2 = zrow_advance(r, len, xa, ya);
    const double qG = qOverM * G;
    const double gb = hm::sqrt((1.0 - 1.0 / g2) * g2);
    const double kq = hm::sqrt(hm::fabs(qG / (gb * k::c)));
    const double x = r[0], y = r[2];
    const double xp = r[1] / r[5];
    const double yp = r[3] / r[5];
    const double ik = 1.0 / kq;
    double X, Xp, Y, Yp;
    double s, c;
    P::sincos(len * kq, s, c);
    const double sh = P::sinh(kq * len), ch = P::cosh(kq * len);
    if (qG > 0.0) {
        X = c * x + (s * xp) * ik;
        Xp = c * xp - s * (kq * x);
        Y = (sh * yp) * ik + ch * y;
        Yp = sh * (kq * y) + ch * yp;
    } else {
        X = (sh * xp) * ik + ch * x;
        Xp = sh * (kq * x) + ch * xp;
        Y = c * y + (s * yp) * ik;
        Yp = c * yp - s * (kq * y);
    }
    r[0] = X;
    r[1] = r[5] * Xp;
    r[2] = Y;
    r[3] = r[5] * Yp;
}

template <class P>
AVAS_HD inline void het_row(const RowOp& op, double* r, double qOverM, double scx, double scy, double scz) {
    if (op.kind == kEdge) {
        const double p2 = zrow_p2(r);
        const double zl = op.err ? het_err_in(r, op.e) : 0.0;
        edge_row(r, op.k1, op.k2, op.hv);
        if (op.err) het_err_out(r, op.e, zl, P::compat);
        if (!P::compat) zrow_keep_momentum(r, p2);
        return;
    }
    // corrected engine: |gamma*beta| of the row before the magnetic transfer is restored after it.
    // T_in is a rotation (keeps |p|); the space-charge kick changes |p|, so after a kick the value
    // is taken again (M6 decision 4, 2026-11-08: until then the kick's energy change was undone).
    double p2 = P::compat ? 0.0 : zrow_p2(r);
    double zl = 0.0;
    if (op.err) zl = het_err_in(r, op.e);
    if (op.scKick) {
        // space-charge kick of both half steps (dt of the first half twice), lab frame
        double ak = (qOverM / k::c) * r[4];
        ak = ak + ak;
        r[1] = scx * ak + r[1];
        r[3] = scy * ak + r[3];
        r[5] = ak * scz + r[5];
        if (!P::compat) p2 = zrow_p2(r);
    }
    if (op.kind == kQuad) quad_row_p<P>(r, qOverM, op.strength, op.len);
    else if (op.kind == kSolenoid) solenoid_row(r, qOverM, op.strength, op.len, PSinCos<P>());
    else bend_row(r, op.uzs, op.bs);
    if (op.err) het_err_out(r, op.e, zl, P::compat);
    if (!P::compat) zrow_keep_momentum(r, p2);
}

// Steerer kick of a row; lq = (length of the kicked element) * q/m.
AVAS_HD inline void kick_row(double* r, int kind, double wx, double wy, double lq, bool compat) {
    const double p2 = zrow_p2(r);
    steerer_kick(r, kind, wx, wy, lq);
    // thin kick: the particle does not move across the field, so |p| is kept (AVAS.dll keeps uz)
    if (!compat) zrow_keep_momentum(r, p2);
}

// HESCFieldPre (corrected engine), pass 1: rel at time tRef from the row; sums 0-5 rel, 6 count.
constexpr int kHescSums = 7;
AVAS_HD inline void hesc_pass1(const PartV& b, int i, double tRef, double* c) {
    for (int k = 0; k < kHescSums; ++k) c[k] = 0.0;
    const double* r = b.rec + 8 * static_cast<long long>(i);
    if (r[7] != 1.0) return;
    const double dt = r[6] - tRef;
    const double g = hm::sqrt(r[1] * r[1] + 1.0 + r[3] * r[3] + r[5] * r[5]);
    const double be = hm::sqrt(1.0 - (1.0 / g) / g);
    const double xp = r[1] / r[5], yp = r[3] / r[5];
    const double vz = hm::sqrt((be * k::c * be * k::c) / (xp * xp + yp * yp + 1.0));
    b.rel[4][i] = -dt * vz;
    b.rel[0][i] = r[0] - ((r[1] * dt) / g) * k::c;
    b.rel[2][i] = r[2] - ((dt * r[3]) / g) * k::c;
    b.rel[1][i] = r[1];
    b.rel[3][i] = r[3];
    b.rel[5][i] = r[5];
    for (int k = 0; k < 6; ++k) c[k] = b.rel[k][i];
    c[6] = 1.0;
}

AVAS_HD inline void hesc_pass2(const PartV& b, int i, const double* cen, double gc, double* c) {
    c[0] = c[1] = c[2] = 0.0;
    if (b.rec[8 * static_cast<long long>(i) + 7] != 1.0) return;
    const double ax = b.rel[0][i] - cen[0], ay = b.rel[2][i] - cen[2], az = (b.rel[4][i] - cen[4]) * gc;
    c[0] = ax * ax;
    c[1] = ay * ay;
    c[2] = az * az;
}

// ---- space-charge mesh: CIC deposit cell and weights, interpolation (PIC::Weight / Allocate) ----

struct MeshV {
    int nx, ny, nz;
    double dx, dy, dz, vol, gamma, igamma;
    double c0, c2, c4;    // centroid x, y, z
    int lpRel;            // lpboundary: stretch about the synchronous particle
    int lpc;              // lpboundary: out-of-mesh distance from the mesh centre
    int compat;
    const double* xs;     // node coordinates
    const double* ys;
    const double* zs;
    double Q;             // total mesh charge (for particles outside the mesh)
    int rotMode;
    double rs, rc;
    long long fixScaleExp;  // corrected engine: fixed-point deposit, charge * 2^fixScaleExp
};

// Cell (i, j, k) and the 8 CIC weights of a particle at rel = (x, y, z4); true inside the mesh.
template <class R>
AVAS_HD inline bool mesh_cell_t(const MeshV& M, R x, R y, R z4, int& i, int& j, int& k, R* w) {
    // lpboundary 1: AVAS.dll stretches about the synchronous particle only in the threaded deposit,
    // the serial one about the centroid; the corrected engine always about the synchronous particle
    const R g = R(M.gamma), c4 = R(M.c4);
    const R zr = M.lpRel ? (M.compat ? z4 * g + c4 : z4 * g) : (z4 - c4) * g + c4;
    i = static_cast<int>(hm::floor((x - R(M.xs[0])) / R(M.dx)));
    j = static_cast<int>(hm::floor((y - R(M.ys[0])) / R(M.dy)));
    k = static_cast<int>(hm::floor((zr - R(M.zs[0])) / R(M.dz)));
    if (0 <= i && i < M.nx - 1 && 0 <= j && j < M.ny - 1 && 0 <= k && k < M.nz - 1) {
        for (int m = 0; m < 8; ++m) {
            const int ii = i + ((m & 1) ^ 1);
            const int jj = j + (((m & 3) == 1 || (m & 3) == 0) ? 1 : 0);
            const int kk = k + (m < 4 ? 1 : 0);
            w[m] = hm::fabs(x - R(M.xs[ii])) * hm::fabs(y - R(M.ys[jj])) * hm::fabs(zr - R(M.zs[kk])) / R(M.vol);
        }
        return true;
    }
    for (int m = 0; m < 8; ++m) w[m] = R(0);
    return false;
}

AVAS_HD inline bool mesh_cell(const MeshV& M, const PartV& b, int p, int& i, int& j, int& k, double* w) {
    return mesh_cell_t<double>(M, b.rel[0][p], b.rel[2][p], b.rel[4][p], i, j, k, w);
}

AVAS_HD inline long long mesh_at(const MeshV& M, int i, int j, int k) {
    return (static_cast<long long>(i) * M.ny + j) * M.nz + k;
}

// Node of corner m of cell (i, j, k), in the order of the deposit.
AVAS_HD inline long long mesh_corner(const MeshV& M, int i, int j, int k, int m) {
    const int di = m & 1, dj = (m >> 1) & 1, dk = (m >> 2) & 1;
    return mesh_at(M, i + di, j + dj, k + dk);
}

// Space-charge field of a particle at rel = (x, y, z4) from the mesh field (lab frame, E_perp / gamma).
template <class R>
AVAS_HD inline void mesh_interp_t(const MeshV& M, R x, R y, R z4, const double* ex, const double* ey, const double* ez,
                                  double kE, R* sc) {
    int i, j, k;
    R w[8];
    if (mesh_cell_t<R>(M, x, y, z4, i, j, k, w)) {
        auto interp = [&](const double* g) {
            return R(g[mesh_at(M, i, j, k)]) * w[0] + R(g[mesh_at(M, i + 1, j, k)]) * w[1] +
                   R(g[mesh_at(M, i, j + 1, k)]) * w[2] + R(g[mesh_at(M, i + 1, j + 1, k)]) * w[3] +
                   R(g[mesh_at(M, i, j, k + 1)]) * w[4] + R(g[mesh_at(M, i + 1, j, k + 1)]) * w[5] +
                   R(g[mesh_at(M, i, j + 1, k + 1)]) * w[6] + R(g[mesh_at(M, i + 1, j + 1, k + 1)]) * w[7];
        };
        const R Ez = interp(ez), Ey = interp(ey), Ex = interp(ex);
        R sx = Ex * R(M.igamma), sy = Ey * R(M.igamma), sz = Ez;
        const R rc = R(M.rc), rs = R(M.rs);
        if (M.rotMode == 1) {
            const R ny = sy * rc - sz * rs, nz = sy * rs + sz * rc;
            sy = ny;
            sz = nz;
        } else if (M.rotMode != 0) {
            const R nx = sx * rc + sz * rs, nz = sz * rc - sx * rs;
            sx = nx;
            sz = nz;
        }
        sc[0] = sx;
        sc[1] = sy;
        sc[2] = sz;
    } else {
        const R ddx = M.lpc ? x : x - R(M.c0);
        const R ddy = M.lpc ? y : y - R(M.c2);
        const R ddz = M.lpc ? z4 : z4 - R(M.c4);
        const R r = hm::sqrt(ddx * ddx + ddy * ddy + ddz * ddz);
        const R f = (((R(M.Q) * R(kE)) / r) / r) / r * R(M.igamma);
        sc[0] = ddx * f;
        sc[1] = ddy * f;
        sc[2] = f * ddz * R(M.gamma);
    }
}

AVAS_HD inline void mesh_interp(const MeshV& M, const PartV& b, int p, const double* ex, const double* ey,
                                const double* ez, double kE) {
    if (b.lost[p] == 1) {
        b.scx[p] = b.scy[p] = b.scz[p] = 0.0;
        return;
    }
    double sc[3];
    mesh_interp_t<double>(M, b.rel[0][p], b.rel[2][p], b.rel[4][p], ex, ey, ez, kE, sc);
    b.scx[p] = sc[0];
    b.scy[p] = sc[1];
    b.scz[p] = sc[2];
}

// Fixed-point value of one deposit (corrected engine): the same integer on every device, and
// integer sums do not depend on the order, the thread count or the number of GPUs.
AVAS_HD inline long long fix_charge(double v, long long scaleExp) {
    return round_to_ll(pm::scale2(v, static_cast<int>(scaleExp)));
}

// ---- boundary.txt (Geometry::Boundary) ----

AVAS_HD inline void geom_local(const GeomV& g, double x, double y, double z, double& u, double& v, double& zl) {
    const double* p = g.place;
    const double dx = x - p[1], dy = y - p[2];
    const double u0 = g.s3 * dy + g.c3 * dx;
    const double v0 = g.c3 * dy - g.s3 * dx;
    const double dz = (z - g.zStart) - p[0];
    const double w = dz * g.c5 + u0 * g.s5;
    u = u0 * g.c5 - dz * g.s5;
    v = w * g.s4 + v0 * g.c4;
    zl = w * g.c4 - v0 * g.s4;
}

// Collision of particle i with geometry g during the step that ended at time t (step dt).
// Returns 1 for a hit (P = coordinates at the wall, tHit), 0 for none, and -(10 + j) when shape
// parameter j is missing (the host then raises the error of the original, vector::at).
template <class P>
AVAS_HD inline int geometry_hit(const GeomV& g, const PartV& b, int i, double t, double dt, double* Pp, double& tHit) {
    double u, v, zl;
    geom_local(g, b.x[i], b.y[i], b.z[i], u, v, zl);
    if (g.npar < 1) return -10;
    if (!(0.0 < zl && zl < g.par[0])) return 0;
    Pp[0] = b.x[i]; Pp[1] = b.ux[i]; Pp[2] = b.y[i]; Pp[3] = b.uy[i]; Pp[4] = b.z[i]; Pp[5] = b.uz[i];
    tHit = t;
    const double* old = b.old + 6 * static_cast<long long>(i);
    if (g.shape == 1) {
        const double r = hm::sqrt(v * v + u * u);
        if (g.npar < 2) return -11;
        const double R = g.par[1];
        if (r < R) return 0;
        double u0, v0, zl0;
        geom_local(g, old[0], old[2], old[4], u0, v0, zl0);
        const double r0 = hm::sqrt(v0 * v0 + u0 * u0);
        double f = (R - r0) / (r - r0);
        const double zh = (zl - zl0) * f + zl0;
        if (zh < 0.0) f = -(zl0 / (zl - zl0));
        for (int j = 0; j < 6; ++j) Pp[j] = (Pp[j] - old[j]) * f + old[j];
        tHit = t - (1.0 - f) * dt;
    } else {
        if (g.npar < 3) return -12;
        const double rho = g.par[2];
        double phi = 0.0, cx = 0.0, cy = 0.0, cz = 0.0;
        double s, c;
        const double plane = g.npar > 4 ? g.par[4] : 0.0, dir = g.npar > 5 ? g.par[5] : 0.0;
        if (plane == 0.0) {
            if (dir <= 0.0) {
                phi = P::atan(zl / (u + rho));
                P::sincos(phi, s, c);
                cx = (c - 1.0) * rho;
            } else {
                phi = P::atan(zl / (rho - u));
                P::sincos(phi, s, c);
                cx = (1.0 - c) * rho;
            }
            cz = s * rho;
        } else if (plane == 1.0) {
            if (dir <= 0.0) {
                phi = P::atan(zl / (rho + v));
                P::sincos(phi, s, c);
                cy = (c - 1.0) * rho;
            } else {
                phi = P::atan(zl / (rho - v));
                P::sincos(phi, s, c);
                cy = (1.0 - c) * rho;
            }
            cz = s * rho;
        }
        if (g.npar < 4) return -13;
        if (!(phi <= hm::fabs(g.par[3]) * 0.017453292519943295)) return 0;
        const double du = u - cx;
        const double d = hm::sqrt((v - cy) * (v - cy) + du * du + (zl - cz) * (zl - cz));
        if (g.npar < 2) return -11;
        if (d < g.par[1]) return 0;
    }
    return 1;
}

// tcodeRun 4.2.4 for particle i: geometry search and Geometry::Boundary.  Returns the code of
// geometry_hit (1 hit) and the geometry index; may set lost = 2 past the lattice end.
template <class P>
AVAS_HD inline int boundary_particle(const LatV& L, const PartV& b, int i, double t, double dt, bool compat, int& gOut,
                                     double* Pp, double& tHit) {
    const double z = b.z[i];
    if (z < 0.0 || b.lost[i] != 0 || z >= L.segEnd[L.nSeg - 1] || z < b.spOutZ0[i]) {
        if (z > L.totalLength && L.spOutCur == L.spOutSize && b.lost[i] == 0) b.lost[i] = 2;
        return 0;
    }
    const int ri = b.spOutCount[i];
    if (ri >= L.nBnd) return 0;
    const double zz = L.bndZ[ri] + z;
    const int r1 = L.bndR1[ri], r2 = L.bndR2[ri];
    if (!(L.segStart[r1] <= zz && zz <= L.segEnd[r2])) return 0;
    int gi = b.geomIndex[i];
    if (compat ? (r2 < gi && gi < r1) : (gi < r1 || r2 < gi)) gi = compat ? r2 : r1;
    while (gi > 0 && zz < L.segStart[gi]) --gi;
    while (gi + 1 < L.nSeg && L.segEnd[gi] <= zz) ++gi;
    b.geomIndex[i] = gi;
    gOut = gi;
    return geometry_hit<P>(L.geoms[gi], b, i, t, dt, Pp, tHit);
}

// ---- lpboundary and longlimits ----

// lpboundary 1: particle folded into one RF period around the synchronous particle.
AVAS_HD inline void lp_fold_particle(const PartV& b, int i, double zs, double full, double half) {
    if (b.lost[i] != 0) return;
    while (true) {
        while (b.z[i] - zs > half) b.z[i] = b.z[i] - full;
        if (-half <= b.z[i] - zs) break;
        b.z[i] = b.z[i] + full;
    }
}

// longlimits: particles outside the energy or phase window are lost (LongitudinalBunchLoss).
AVAS_HD inline void long_loss_particle(const PartV& b, int i, const double* restMass, double zs, double ws,
                                       double dW, double dzMax, bool compat) {
    const double gi = compat ? hm::sqrt(b.uz[i] * b.uz[i] + 1.0)
                             : hm::sqrt(b.ux[i] * b.ux[i] + b.uy[i] * b.uy[i] + b.uz[i] * b.uz[i] + 1.0);
    if (hm::fabs((gi - 1.0) * restMass[i] - ws) > dW) b.lost[i] = 1;
    if (hm::fabs(b.z[i] - zs) > dzMax) b.lost[i] = 1;
}

}  // namespace kern
}  // namespace avas
