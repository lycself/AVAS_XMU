// fp32 tier of the t-code step shared by the Metal back end (src/metal/kernels.metal, compiled as MSL)
// and the fp32 tier of the CPU device back end (core/f32_cpu.cpp, compiled as C++ with the same
// "double = float" world): the same source gives the same float operations on both, so a Metal
// run and an emu fp32 run can agree bit for bit (M6c phase 3).  Only compiled where AVAS_FLOAT_ONLY.
#pragma once
#include "core/hd.h"
#include "core/kernels.h"
#include "core/mesh_kernels.h"
#include "core/sc_kernels.h"

#ifdef AVAS_FLOAT_ONLY
namespace avas {
namespace f32 {

#if defined(AVAS_METAL)
typedef metal::atomic_uint avas_atomic_u32;
inline unsigned avas_fetch_add(AVAS_G avas_atomic_u32* c, unsigned v) { return atomic_fetch_add_explicit(c, v, metal::memory_order_relaxed); }
inline float avas_atanf(float x) { return metal::precise::atan(x); }
inline float avas_cosf(float x) { return metal::precise::cos(x); }
inline float avas_sinf(float x) { return metal::precise::sin(x); }
inline float avas_maxf(float a, float b) { return metal::max(a, b); }
inline float avas_rint(float x) { return metal::rint(x); }
inline float avas_ldexp(float x, int e) { return metal::ldexp(x, e); }
inline float avas_inf() { return INFINITY; }
#else
typedef unsigned avas_atomic_u32;  // the CPU fp32 tier runs one particle after the other
inline unsigned avas_fetch_add(avas_atomic_u32* c, unsigned v) { const unsigned o = *c; *c = o + v; return o; }
inline float avas_atanf(float x) { return std::atan(x); }
inline float avas_cosf(float x) { return std::cos(x); }
inline float avas_sinf(float x) { return std::sin(x); }
inline float avas_maxf(float a, float b) { return a < b ? b : a; }
inline float avas_rint(float x) { return std::nearbyint(x); }
inline float avas_ldexp(float x, int e) { return std::ldexp(x, e); }
inline float avas_inf() { return HUGE_VALF; }
#endif

// charge * 2^e rounded to the nearest integer (ties to even); exact power-of-two scaling in float
AVAS_HD inline avas_i64 fix_charge_f(float v, avas_i64 e) { return avas_i64(avas_rint(avas_ldexp(v, int(e)))); }

AVAS_HD inline void add64(AVAS_G avas_atomic_u32* lo, AVAS_G avas_atomic_u32* hi, avas_i64 idx, avas_i64 v) {
    const avas_u64 u = static_cast<avas_u64>(v);
    const unsigned l = static_cast<unsigned>(u & 0xffffffffull), h = static_cast<unsigned>(u >> 32);
    const unsigned old = avas_fetch_add(lo + idx, l);
    const unsigned carry = (old + l < old) ? 1u : 0u;
    avas_fetch_add(hi + idx, h + carry);
}

struct RelZM {  // lattice positions relative to the synchronous particle after the step
    const AVAS_G float* es;
    const AVAS_G float* ee;
    const AVAS_G float* e0;
    const AVAS_G float* s0;
    float z0, spx, tot;
    float elem_start(int k) const { return es[k]; }
    float elem_end(int k) const { return ee[k]; }
    float estart(int k, const AVAS_G kern::ElemV&) const { return e0[k]; }
    float sstart(int q, const AVAS_G kern::SubV&) const { return s0[q]; }
    float zero() const { return z0; }
    float sp_exit() const { return spx; }
    float total() const { return tot; }
};

struct PartF {
    AVAS_G float* d[3];
    AVAS_G float* du[3];
    AVAS_G float* E[3];
    AVAS_G float* B[3];
    AVAS_G float* sc[3];
    AVAS_G float* rel[6];
    AVAS_G float* hq;
    AVAS_G float* tq;
    AVAS_G float* w;
    AVAS_G float* spz0;
    AVAS_G float* spz1;
    AVAS_G int* lost;
    AVAS_G int* elemIndex;
    AVAS_G int* nextPlane;
    AVAS_G int* boundaryMode;
    AVAS_G int* recFlag;
    AVAS_G int* spFlag;
    AVAS_G int* geomIndex;
    AVAS_G int* spOutCount;
    AVAS_G float* C;      // 21 per particle
    AVAS_G float* Mx;     // 8 per particle
    AVAS_G float* snap;   // 12 per particle: d0, u0, dn, up of a plane crossing
    AVAS_G float* oldS;   // 6 per particle: d0, u0 (writeOld)
    AVAS_G avas_atomic_u32* crossCount;  // [0] plane crossings, [1] superposeout candidates
    AVAS_G unsigned* crossIdx;            // plane crossings
    AVAS_G unsigned* spIdx;               // superposeout candidates
    AVAS_G unsigned* bdIdx;               // boundary.txt candidates (count in crossCount[2])
};

struct StepU {
    float hEs[3], usm[3], gsm, Ks[9], usp[3], gsp, cdt;
    float post0, post2, zsPost;
    float cen[6];
    float gp, zFieldPre, segEnd, zNeg, rs, rc;
    float z0, spx, tot;
    int zeroField;  // a.zMinPrev <= a.zField (evaluated on the host in double)
    int rotMode, nEvents, writeOld, n;
};

inline void K_of(float tx, float ty, float tz, AVAS_T float* K) {
    const float tx2 = tx * tx, ty2 = ty * ty, tz2 = tz * tz;
    const float d = 1.0f / ((tx2 + ty2 + tz2) * 0.25f + 1.0f);
    const float txy = ty * tx * 0.5f, txz = tz * tx * 0.5f, tyz = tz * ty * 0.5f;
    K[0] = -(ty2 + tz2) * 0.5f * d;
    K[1] = (txy + tz) * d;
    K[2] = (txz - ty) * d;
    K[3] = (txy - tz) * d;
    K[4] = -(tz2 + tx2) * 0.5f * d;
    K[5] = (tx + tyz) * d;
    K[6] = (txz + ty) * d;
    K[7] = (tyz - tx) * d;
    K[8] = -(ty2 + tx2) * 0.5f * d;
}

// One particle of the t-code step (port of cuda_f32.cuh F32Step; the Metal kernel k_step and the
// fp32 tier of the CPU device back end call it).
AVAS_HD inline void step_particle(AVAS_T const kern::LatV& L, AVAS_T const StepU& U, AVAS_T const PartF& F,
                                  const AVAS_G float* off, const AVAS_G float* rft, AVAS_T const int* sizes, int i) {

    const int nE = sizes[0], nEv = sizes[2];
    RelZM zo;
    zo.es = off;
    zo.ee = off + nE;
    zo.e0 = off + 2 * nE;
    zo.s0 = off + 3 * nE;
    zo.z0 = U.z0;
    zo.spx = U.spx;
    zo.tot = U.tot;
    const AVAS_G float* evRel = off + 3 * nE + sizes[1];
    const AVAS_G float* evOk = evRel + nEv;
    kern::RfSrc rf;
    rf.table = rft;
    rf.t = 0.0f;
    // column-major (coalesced): C[j n + i], Mx[j n + i]
    const unsigned nn = unsigned(U.n);
    float C[21], Mx[8];
    for (int j = 0; j < 21; ++j) C[j] = 0.0f;
    for (int j = 0; j < 6; ++j) Mx[j] = 0.0f;
    Mx[6] = Mx[7] = -avas_inf();
    const int lost0 = F.lost[i];
    const float d0[3] = {F.d[0][i], F.d[1][i], F.d[2][i]};
    const float u0[3] = {F.du[0][i], F.du[1][i], F.du[2][i]};
    if (lost0 != 1) {
        const float w = F.w[i];
        const float inv = 1.0f / F.rel[5][i];
        float d[6];
        d[0] = F.rel[0][i] - U.cen[0];
        d[2] = F.rel[2][i] - U.cen[2];
        d[4] = F.rel[4][i] - U.cen[4];
        d[1] = (F.rel[1][i] - U.cen[1]) * inv;
        d[3] = (F.rel[3][i] - U.cen[3]) * inv;
        d[5] = (F.rel[5][i] - U.cen[5]) * inv;
        Mx[6] = -d0[2];  // the host adds the synchronous z (zmin / zmax in double)
        Mx[7] = d0[2];
        C[9] = U.gp * d[4] * U.gp * d[4] * w;
        for (int j = 0; j < 6; ++j) {
            Mx[j] = d[j] < 0.0f ? -d[j] : d[j];
            C[j] = d[j] * d[j] * w;
        }
        C[6] = d[1] * d[0] * w;
        C[7] = d[3] * d[2] * w;
        C[8] = d[5] * d[4] * w;
    }
    float tot[6];
    if (U.zeroField != 0 && !(U.zFieldPre < d0[2])) {
        for (int j = 0; j < 6; ++j) tot[j] = 0.0f;
    } else {
        for (int j = 0; j < 3; ++j) {
            tot[j] = F.E[j][i] + F.sc[j][i];
            tot[3 + j] = F.B[j][i];
        }
    }
    if (U.writeOld != 0) {
        for (int j = 0; j < 3; ++j) {
            F.oldS[6 * i + 2 * j] = d0[j];
            F.oldS[6 * i + 2 * j + 1] = u0[j];
        }
    }
    if (lost0 == 1) {
        C[20] = F.recFlag[i] != 0 ? 1.0f : 0.0f;
        for (int j = 0; j < 21; ++j) F.C[j * nn + i] = C[j];
        for (int j = 0; j < 8; ++j) F.Mx[j * nn + i] = Mx[j];
        return;
    }
    const float h = F.hq[i];
    float dk[3], dm[3];
    for (int j = 0; j < 3; ++j) {
        dk[j] = h * tot[j] - U.hEs[j];
        dm[j] = u0[j] + dk[j];
    }
    float N = 0.0f;
    for (int j = 0; j < 3; ++j) N += (U.usm[j] + U.usm[j] + dm[j]) * dm[j];
    const float gm = hm::sqrt(U.gsm * U.gsm + N);
    const float sq = F.tq[i] / gm;
    float K[9];
    K_of(sq * tot[3], sq * tot[4], sq * tot[5], K);
    float up[3];
    for (int r = 0; r < 3; ++r) {
        float acc = dm[r];
        for (int c = 0; c < 3; ++c) acc += K[3 * r + c] * dm[c] + (K[3 * r + c] - U.Ks[3 * r + c]) * U.usm[c];
        up[r] = acc + dk[r];
    }
    float Np = 0.0f;
    for (int j = 0; j < 3; ++j) Np += (U.usp[j] + U.usp[j] + up[j]) * up[j];
    const float gpi = hm::sqrt(U.gsp * U.gsp + Np);
    const float dg = Np / (gpi + U.gsp);
    float dn[3];
    for (int j = 0; j < 3; ++j) {
        dn[j] = d0[j] + U.cdt * ((up[j] * U.gsp - U.usp[j] * dg) / (gpi * U.gsp));
        F.d[j][i] = dn[j];
        F.du[j][i] = up[j];
    }
    float rel[6];
    const float ux = U.usp[0] + up[0], uy = U.usp[1] + up[1], uz = U.usp[2] + up[2];
    if (U.rotMode == 0) {
        rel[0] = dn[0]; rel[2] = dn[1]; rel[4] = dn[2];
        rel[1] = ux; rel[3] = uy; rel[5] = uz;
    } else {
        const float sn = U.rs, cs = U.rc;
        if (U.rotMode == 2) {
            rel[0] = cs * dn[0] - sn * dn[2];
            rel[4] = sn * dn[0] + cs * dn[2];
            rel[1] = ux * cs - uz * sn;
            rel[5] = uz * cs + ux * sn;
            rel[2] = dn[1];
            rel[3] = uy;
        } else {
            rel[2] = cs * dn[1] + sn * dn[2];
            rel[4] = dn[2] * cs - dn[1] * sn;
            rel[3] = uy * cs + uz * sn;
            rel[5] = uz * cs - uy * sn;
            rel[0] = dn[0];
            rel[1] = ux;
        }
    }
    for (int j = 0; j < 6; ++j) F.rel[j][i] = rel[j];
    kern::FieldIn<float> in;
    in.x = U.post0 + dn[0];
    in.y = U.post2 + dn[1];
    in.fz = dn[2];
    in.rel0 = rel[0];
    in.rel2 = rel[2];
    in.spZ0 = F.spz0[i] - U.zsPost;
    in.spZ1 = F.spz1[i] - U.zsPost;
    in.boundaryMode = F.boundaryMode[i];
    float E[3], B[3];
    int k = F.elemIndex[i];
    const int lostNow = F.lost[i];
    const int f = kern::external_field_t<PortableMath, float, RelZM>(L, zo, in, lostNow, false, k, rf, E, B);
    if (!(dn[2] < zo.z0 || zo.spx < dn[2] || lostNow != 0)) F.elemIndex[i] = k;
    if (f & kern::kFieldLost) F.lost[i] = 1;
    if (f & kern::kFieldLostEnd) F.lost[i] = 2;
    if (f & kern::kFieldWrite)
        for (int j = 0; j < 3; ++j) {
            F.E[j][i] = E[j];
            F.B[j][i] = B[j];
        }
    if (dn[2] > U.segEnd)
        for (int j = 0; j < 3; ++j) F.E[j][i] = F.B[j][i] = 0.0f;
    if (uz < 0.0f && L.spOutCount <= L.spOutCur && dn[2] < U.zNeg) F.lost[i] = 1;
    const int mp = F.nextPlane[i];
    const bool cross = mp + 1 <= U.nEvents && evOk[mp] != 0.0f && evRel[mp] <= dn[2] + 1e-4f * hm::fabs(dn[2]) + 1e-9f;
    // superposeout exit: candidates (the host decides in double, as the CUDA fp32 tier does)
    const bool spCand = L.spOutCur < L.spOutCount && L.elems[F.elemIndex[i]].inSuperposeOut != 0 && F.spFlag[i] == 0 &&
                        dn[2] + 1e-4f * hm::fabs(dn[2]) + 1e-9f > F.spz0[i] - U.zsPost;
    if (cross || spCand) {
        AVAS_G float* sn = F.snap + 12 * i;
        for (int j = 0; j < 3; ++j) {
            sn[j] = d0[j];
            sn[3 + j] = u0[j];
            sn[6 + j] = dn[j];
            sn[9 + j] = up[j];
        }
        if (cross) F.crossIdx[avas_fetch_add(F.crossCount, 1u)] = i;
        if (spCand) F.spIdx[avas_fetch_add(F.crossCount + 1, 1u)] = i;
    }
    if (F.lost[i] != 1) {
        const float w = F.w[i];
        C[10] = 1.0f;
        C[11] = w;
        for (int j = 0; j < 6; ++j) C[12 + j] = w * rel[j];
        C[18] = (U.gsp + dg) * w;
    }
    C[20] = F.recFlag[i] != 0 ? 1.0f : 0.0f;
    for (int j = 0; j < 21; ++j) F.C[j * nn + i] = C[j];
    for (int j = 0; j < 8; ++j) F.Mx[j * nn + i] = Mx[j];
}

struct GeomF {  // kern::GeomV with double = float
    int shape, npar;
    float zStart;
    float par[6];
    float place[6];
    float s3, c3, s4, c4, s5, c5;
};

struct BoundU {
    float zs, post0, post2, totalLength, segEndLast;
    int nSeg, nBnd, spOutCur, spOutSize, n;
};


AVAS_HD inline bool near_f(float a, float b) { return hm::fabs(a - b) <= 1e-5f * avas_maxf(1.0f, hm::fabs(b)) + 1e-9f; }

// boundary.txt pre-selection of one particle (see BoundU).
AVAS_HD inline void bound_particle(AVAS_T const BoundU& U, AVAS_T const PartF& F, const AVAS_G GeomF* geoms,
                                   const AVAS_G float* segStart, const AVAS_G float* segEnd, const AVAS_G float* bndZ,
                                   const AVAS_G int* bndR1, const AVAS_G int* bndR2, int i) {

    if (int(i) >= U.n || F.lost[i] != 0) return;
    AVAS_G avas_atomic_u32* cnt = F.crossCount + 2;
    const float z = U.zs + F.d[2][i];
    const float spz = F.spz0[i];
    bool cand = near_f(z, 0.0f) || near_f(z, spz) || near_f(z, U.segEndLast);
    if (!cand && (z < 0.0f || z >= U.segEndLast || z < spz)) {
        // lost = 2 past the lattice end is decided on the host
        if (z > U.totalLength - 1e-5f * avas_maxf(1.0f, U.totalLength) && U.spOutCur == U.spOutSize)
            F.bdIdx[avas_fetch_add(cnt, 1u)] = i;
        return;
    }
    if (!cand) {
        const int ri = F.spOutCount[i];
        if (ri >= U.nBnd) return;
        const float zz = bndZ[ri] + z;
        const int r1 = bndR1[ri], r2 = bndR2[ri];
        if (near_f(zz, segStart[r1]) || near_f(zz, segEnd[r2])) {
            cand = true;
        } else {
            if (!(segStart[r1] <= zz && zz <= segEnd[r2])) return;
            int gi = F.geomIndex[i];
            if (gi < r1 || r2 < gi) gi = r1;
            while (gi > 0 && zz < segStart[gi]) --gi;
            while (gi + 1 < U.nSeg && segEnd[gi] <= zz) ++gi;
            if (near_f(zz, segStart[gi]) || near_f(zz, segEnd[gi])) {
                cand = true;
            } else {
                F.geomIndex[i] = gi;
                const AVAS_G GeomF& g = geoms[gi];
                const float x = U.post0 + F.d[0][i], y = U.post2 + F.d[1][i];
                const float dx = x - g.place[1], dy = y - g.place[2];
                const float u0 = g.s3 * dy + g.c3 * dx;
                const float v0 = g.c3 * dy - g.s3 * dx;
                const float dz = (z - g.zStart) - g.place[0];
                const float w = dz * g.c5 + u0 * g.s5;
                const float u = u0 * g.c5 - dz * g.s5;
                const float v = w * g.s4 + v0 * g.c4;
                const float zl = w * g.c4 - v0 * g.s4;
                if (g.npar < (g.shape == 1 ? 2 : 4) || near_f(zl, 0.0f) || near_f(zl, g.par[0])) {
                    cand = true;
                } else if (0.0f < zl && zl < g.par[0]) {
                    if (g.shape == 1) {
                        cand = hm::sqrt(v * v + u * u) >= g.par[1] * (1.0f - 1e-5f) - 1e-9f;
                    } else {
                        const float rho = g.par[2];
                        float phi = 0.0f, cx = 0.0f, cy = 0.0f, cz = 0.0f;
                        const float plane = g.npar > 4 ? g.par[4] : 0.0f, dir = g.npar > 5 ? g.par[5] : 0.0f;
                        if (plane == 0.0f) {
                            phi = dir <= 0.0f ? avas_atanf(zl / (u + rho)) : avas_atanf(zl / (rho - u));
                            cx = dir <= 0.0f ? (avas_cosf(phi) - 1.0f) * rho : (1.0f - avas_cosf(phi)) * rho;
                            cz = avas_sinf(phi) * rho;
                        } else if (plane == 1.0f) {
                            phi = dir <= 0.0f ? avas_atanf(zl / (rho + v)) : avas_atanf(zl / (rho - v));
                            cy = dir <= 0.0f ? (avas_cosf(phi) - 1.0f) * rho : (1.0f - avas_cosf(phi)) * rho;
                            cz = avas_sinf(phi) * rho;
                        }
                        const float lim = hm::fabs(g.par[3]) * 0.017453292519943295f;
                        const float du = u - cx;
                        const float dd = hm::sqrt((v - cy) * (v - cy) + du * du + (zl - cz) * (zl - cz));
                        cand = phi <= lim * (1.0f + 1e-5f) + 1e-7f && dd >= g.par[1] * (1.0f - 1e-4f) - 1e-9f;
                    }
                }
            }
        }
    }
    if (cand) F.bdIdx[avas_fetch_add(cnt, 1u)] = i;
}

// Fixed-point CIC deposit of one particle (64-bit integers as two 32-bit words with carry).
AVAS_HD inline void deposit_particle(AVAS_T const kern::MeshV& M, const AVAS_G float* x, const AVAS_G float* y,
                                     const AVAS_G float* z, const AVAS_G float* q, const AVAS_G int* lost,
                                     AVAS_G avas_atomic_u32* lo, AVAS_G avas_atomic_u32* hi, int p) {

    if (lost[p] == 1) return;
    int i, j, k;
    float w[8];
    if (!kern::mesh_cell_t<float>(M, x[p], y[p], z[p], i, j, k, w)) return;
    for (int m = 0; m < 8; ++m) add64(lo, hi, kern::mesh_corner(M, i, j, k, m), fix_charge_f(w[m] * q[p], M.fixScaleExp));
}

// Field of the mesh at one particle (sc: 3 per particle).
AVAS_HD inline void interp_particle(AVAS_T const kern::MeshV& M, const AVAS_G float* x, const AVAS_G float* y,
                                    const AVAS_G float* z, const AVAS_G int* lost, const AVAS_G float* ex,
                                    const AVAS_G float* ey, const AVAS_G float* ez, AVAS_G float* sc, int p) {

    if (lost[p] == 1) {
        sc[3 * p] = sc[3 * p + 1] = sc[3 * p + 2] = 0.0f;
        return;
    }
    float s[3];
    kern::mesh_interp_t<float>(M, x[p], y[p], z[p], ex, ey, ez, sck::kE, s);
    sc[3 * p] = s[0];
    sc[3 * p + 1] = s[1];
    sc[3 * p + 2] = s[2];
}

}  // namespace f32
}  // namespace avas
#endif  // AVAS_FLOAT_ONLY
