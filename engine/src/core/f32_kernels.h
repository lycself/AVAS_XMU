// fp32 tier of the t-code step shared by the Metal back end (src/metal/kernels.metal, compiled as MSL),
// the CUDA fp32 tier (src/cuda/cuda_f32_device.cu) and the fp32 tier of the CPU device back end
// (core/f32_cpu.cpp, compiled as C++), all in the same "double = float" world: the same source gives
// the same float operations, so Metal, CUDA fp32 and emu fp32 runs agree bit for bit (M6c phases 3
// and 4).  Only compiled where AVAS_FLOAT_ONLY.
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
#elif defined(__CUDACC__)
typedef unsigned avas_atomic_u32;
AVAS_HD inline unsigned avas_fetch_add(avas_atomic_u32* c, unsigned v) {
#if defined(__CUDA_ARCH__)
    return atomicAdd(c, v);
#else
    const unsigned o = *c;
    *c = o + v;
    return o;
#endif
}
// atan / cos / sin only in the boundary.txt pre-selection (margins; the decisions are made in double)
AVAS_HD inline float avas_atanf(float x) { return ::atanf(x); }
AVAS_HD inline float avas_cosf(float x) { return ::cosf(x); }
AVAS_HD inline float avas_sinf(float x) { return ::sinf(x); }
AVAS_HD inline float avas_maxf(float a, float b) { return a < b ? b : a; }
AVAS_HD inline float avas_rint(float x) { return ::rintf(x); }
AVAS_HD inline float avas_ldexp(float x, int e) { return ::ldexpf(x, e); }
AVAS_HD inline float avas_inf() { return INFINITY; }
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
    AVAS_HD float elem_start(int k) const { return es[k]; }
    AVAS_HD float elem_end(int k) const { return ee[k]; }
    AVAS_HD float estart(int k, const AVAS_G kern::ElemV&) const { return e0[k]; }
    AVAS_HD float sstart(int q, const AVAS_G kern::SubV&) const { return s0[q]; }
    AVAS_HD float zero() const { return z0; }
    AVAS_HD float sp_exit() const { return spx; }
    AVAS_HD float total() const { return tot; }
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
    AVAS_G float* oldS;   // 6 per particle: d0, u0 (writeOld)
    AVAS_G avas_atomic_u32* crossCount;  // [0] plane crossings, [1] superposeout candidates
    AVAS_G unsigned* crossIdx;            // plane crossings
    AVAS_G unsigned* spIdx;               // superposeout candidates
    AVAS_G unsigned* bdIdx;               // boundary.txt candidates (count in crossCount[2])
    AVAS_G float* row[8];  // z-code rows {x, ux, y, uy, dt, uz, t - T0, flag} (phase 4)
    AVAS_G float* qm;      // q/m
    // compact candidate entries (slot = position in crossIdx / spIdx): the host reads only these, so a
    // GPU with its own memory (CUDA) moves no particle pages to the host
    AVAS_G float* cSnap;  // 12 per plane crossing: d0, u0, dn, up
    AVAS_G int* cNp;      // next plane of the crossing
    AVAS_G float* sSnap;  // 12 per superposeout candidate
    AVAS_G int* sElem;    // element index of the candidate
    AVAS_G int* aIdx;     // host results applied by kZApply: 4 i + kind (0 plane record, 1 superposeout record, 2 boundary)
    AVAS_G float* aRow;   // 8 per entry: the float row of a plane record
    AVAS_G float* scTmp;  // 3 per particle: the interpolated space-charge field (kZSc copies it into sc)
    AVAS_G float* bEnt;   // 16 per boundary.txt candidate: d, du, old (6), lost, spOutCount, geomIndex
    AVAS_G float* rm;     // rest mass (MeV, longlimits)
};

struct StepU {
    float hEs[3], usm[3], gsm, Ks[9], usp[3], gsp, cdt;
    float post0, post2, zsPost;
    float cen[6];
    float gp, zFieldPre, segEnd, zNeg, rs, rc;
    float z0, spx, tot;
    int zeroField;  // a.zMinPrev <= a.zField (evaluated on the host in double)
    int rotMode, nEvents, writeOld, n;
    int stride;  // of the C and Mx columns (n rounded up to whole reduction groups)
};

AVAS_HD inline void K_of(float tx, float ty, float tz, AVAS_T float* K) {
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
    const unsigned nn = unsigned(U.stride);
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
    if (cross) {
        const unsigned c = avas_fetch_add(F.crossCount, 1u);
        F.crossIdx[c] = i;
        F.cNp[c] = mp;
        AVAS_G float* sn = F.cSnap + 12 * c;
        for (int j = 0; j < 3; ++j) {
            sn[j] = d0[j];
            sn[3 + j] = u0[j];
            sn[6 + j] = dn[j];
            sn[9 + j] = up[j];
        }
    }
    if (spCand) {
        const unsigned c = avas_fetch_add(F.crossCount + 1, 1u);
        F.spIdx[c] = i;
        F.sElem[c] = F.elemIndex[i];
        AVAS_G float* sn = F.sSnap + 12 * c;
        for (int j = 0; j < 3; ++j) {
            sn[j] = d0[j];
            sn[3 + j] = u0[j];
            sn[6 + j] = dn[j];
            sn[9 + j] = up[j];
        }
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

// a boundary.txt candidate: its index and what the host needs of it (compact entry)
AVAS_HD inline void bound_cand(AVAS_T const PartF& F, int i) {
    const unsigned c = avas_fetch_add(F.crossCount + 2, 1u);
    F.bdIdx[c] = unsigned(i);
    AVAS_G float* e = F.bEnt + 16 * c;
    for (int j = 0; j < 3; ++j) {
        e[j] = F.d[j][i];
        e[3 + j] = F.du[j][i];
    }
    for (int j = 0; j < 6; ++j) e[6 + j] = F.oldS[6 * i + j];
    e[12] = float(F.lost[i]);
    e[13] = float(F.spOutCount[i]);
    e[14] = float(F.geomIndex[i]);
    e[15] = 0.0f;
}

// boundary.txt pre-selection of one particle (see BoundU).
AVAS_HD inline void bound_particle(AVAS_T const BoundU& U, AVAS_T const PartF& F, const AVAS_G GeomF* geoms,
                                   const AVAS_G float* segStart, const AVAS_G float* segEnd, const AVAS_G float* bndZ,
                                   const AVAS_G int* bndR1, const AVAS_G int* bndR2, int i) {

    if (int(i) >= U.n || F.lost[i] != 0) return;
    const float z = U.zs + F.d[2][i];
    const float spz = F.spz0[i];
    bool cand = near_f(z, 0.0f) || near_f(z, spz) || near_f(z, U.segEndLast);
    if (!cand && (z < 0.0f || z >= U.segEndLast || z < spz)) {
        // lost = 2 past the lattice end is decided on the host
        if (z > U.totalLength - 1e-5f * avas_maxf(1.0f, U.totalLength) && U.spOutCur == U.spOutSize)
            bound_cand(F, i);
        return;
    }
    if (!cand) {
        const int ri = F.spOutCount[i];
        if (ri >= U.nBnd) return;
        // boundary file z of the section and of the geometry (corrected engine: z - row offset, M10 D5)
        const float zz = z - bndZ[ri];
        const int r1 = bndR1[ri], r2 = bndR2[ri];
        if (near_f(zz, segStart[r1]) || near_f(zz, segEnd[r2])) {
            cand = true;
        } else {
            if (!(segStart[r1] <= zz && zz <= segEnd[r2])) return;
            int gi = F.geomIndex[i];
            if (gi < r1 || r2 < gi) gi = r1;
            const int g0 = gi;
            while (gi > 0 && zz < segStart[gi]) --gi;
            while (gi + 1 < U.nSeg && segEnd[gi] <= zz) ++gi;
            // a changed segment: segments crossed in one step are tested on the host (M10 D5)
            if (gi != g0 || near_f(zz, segStart[gi]) || near_f(zz, segEnd[gi])) {
                cand = true;
            } else {
                F.geomIndex[i] = gi;
                const AVAS_G GeomF& g = geoms[gi];
                const float x = U.post0 + F.d[0][i], y = U.post2 + F.d[1][i];
                const float dx = x - g.place[1], dy = y - g.place[2];
                const float u0 = g.s3 * dy + g.c3 * dx;
                const float v0 = g.c3 * dy - g.s3 * dx;
                const float dz = (zz - g.zStart) - g.place[0];
                const float w = dz * g.c5 + u0 * g.s5;
                const float u = u0 * g.c5 - dz * g.s5;
                const float v = w * g.s4 + v0 * g.c4;
                const float zl = w * g.c4 - v0 * g.s4;
                const int need = g.shape == 1 ? 2 : (g.shape == 2 ? 4 : 3);
                if (g.shape == 0 || g.npar < need || near_f(zl, 0.0f) || near_f(zl, g.par[0]) ||
                    (g.shape != 2 && zl >= g.par[0])) {
                    // composite (shape 0), missing parameters, ends of the pipe, beyond a straight pipe
                    cand = true;
                } else if (0.0f < zl && zl < g.par[0]) {
                    if (g.shape == 1) {
                        cand = hm::sqrt(v * v + u * u) >= g.par[1] * (1.0f - 1e-5f) - 1e-9f;
                    } else if (g.shape == 3) {
                        cand = hm::fabs(u) >= g.par[1] * (1.0f - 1e-5f) - 1e-9f || hm::fabs(v) >= g.par[2] * (1.0f - 1e-5f) - 1e-9f;
                    } else if (g.shape == 4) {
                        const float a = u / g.par[1], c = v / g.par[2];
                        cand = a * a + c * c >= 1.0f - 1e-4f;
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
    if (cand) bound_cand(F, i);
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

// ---- z-code rows on the device (M6c phase 4) ----
// The rows are kept in float with the time relative to T0 (held by the host in double and moved to the
// reference time after every element, `shift`); the transfers are the shared kern::het_row /
// kick_row / ztot_row in the float world.  Sums over particles go through the C / Mx columns and the
// fixed-point group sums like the t-code statistics.
enum : int { kZHet = 1, kZKick = 2, kZTref = 3, kZHesc1 = 4, kZHesc2 = 5, kZTot = 6, kZUbp1 = 7, kZUbp2 = 8, kZApply = 9, kZClear = 10, kZSc = 11, kZLp = 12, kZLong = 13 };

struct ZArgs {
    kern::RowOp op;
    int code, kind, stride, n;
    float shift, t, zpl, uzs, gc, wx, wy, len;
    float off[6], cen[6];
};

AVAS_HD inline void zrow_particle(AVAS_T const ZArgs& A, AVAS_T const PartF& F, int i) {
    if (i >= A.n) return;
    if (A.code == kZApply) {  // i: entry of the host's result list (each particle at most once per kind)
        const int e = F.aIdx[i];
        if (e < 0) return;  // a slot without a result
        const int p = e >> 2, kind = e & 3;
        if (kind == 0) {
            F.recFlag[p] = 1;
            F.nextPlane[p] = F.nextPlane[p] + 1;
            for (int j = 0; j < 8; ++j) F.row[j][p] = F.aRow[8 * i + j];
        } else if (kind == 1) {
            F.spFlag[p] = 1;
        } else {
            F.geomIndex[p] = int(F.aRow[8 * i]);
            F.lost[p] = int(F.aRow[8 * i + 1]);
        }
        return;
    }
    if (A.code == kZLp) {  // lpboundary (kern::lp_fold_particle): z folded into one period around the reference
        if (F.lost[i] != 0) return;
        float d = F.d[2][i];
        const float o = A.off[4], full = A.wx, hf = A.wy;  // ("half" is a type in MSL)
        while (true) {
            while (d + o > hf) d = d - full;
            if (-hf <= d + o) break;
            d = d + full;
        }
        F.d[2][i] = d;
        return;
    }
    if (A.code == kZLong) {  // longlimits (kern::long_loss_particle); gamma - 1 = p^2 / (gamma + 1) without cancellation
        float p2 = 0.0f;
        for (int j = 0; j < 3; ++j) {
            const float u = A.cen[2 * j + 1] + F.du[j][i];
            p2 = p2 + u * u;
        }
        const float gm1 = p2 / (hm::sqrt(p2 + 1.0f) + 1.0f);
        if (hm::fabs(gm1 * F.rm[i] - A.t) > A.wx) F.lost[i] = 1;
        if (hm::fabs(F.d[2][i] + A.off[4]) > A.wy) F.lost[i] = 1;
        return;
    }
    if (A.code == kZSc) {
        for (int j = 0; j < 3; ++j) F.sc[j][i] = F.scTmp[3 * i + j];
        return;
    }
    if (A.code == kZClear) {  // records cleared by the tracker (kind 0: plane records, 1: superposeout)
        if (A.kind == 0) {
            F.recFlag[i] = 0;
            F.row[7][i] = 0.0f;
        } else {
            F.spFlag[i] = 0;
        }
        return;
    }
    const unsigned nn = unsigned(A.stride);
    const bool on = F.row[7][i] == 1.0f;
    float r[8];
    for (int j = 0; j < 8; ++j) r[j] = F.row[j][i];
    if (on && A.shift != 0.0f && A.code <= kZTot && A.code != kZHesc2) {
        // the row times move to the new origin (exact: they are close to the shift)
        r[6] = r[6] - A.shift;
        F.row[6][i] = r[6];
    }
    if (A.code == kZHet) {
        if (!on) return;
        kern::het_row<PortableMath>(A.op, r, F.qm[i], F.sc[0][i], F.sc[1][i], F.sc[2][i]);
        for (int j = 0; j < 7; ++j) F.row[j][i] = r[j];
    } else if (A.code == kZKick) {
        if (!on) return;
        kern::kick_row(r, A.kind, A.wx, A.wy, A.len * F.qm[i], false);
        F.row[1][i] = r[1];
        F.row[3][i] = r[3];
        F.row[5][i] = r[5];
    } else if (A.code == kZTref) {
        F.C[0 * nn + i] = on ? r[6] : 0.0f;
        F.C[1 * nn + i] = on ? 1.0f : 0.0f;
        F.Mx[0 * nn + i] = on ? -r[6] : -avas_inf();
    } else if (A.code == kZHesc1) {
        // HESCFieldPre pass 1 (kern::hesc_pass1): rel at the reference time from the row
        if (!on) {
            for (int k = 0; k < 7; ++k) F.C[k * nn + i] = 0.0f;
            return;
        }
        const float dt = r[6] - A.t;
        const float g = hm::sqrt(r[1] * r[1] + 1.0f + r[3] * r[3] + r[5] * r[5]);
        const float be = hm::sqrt((r[1] * r[1] + r[3] * r[3]) + r[5] * r[5]) / g;  // without 1 - 1/g^2 (see zrow_b2)
        const float xp = r[1] / r[5], yp = r[3] / r[5];
        const float c = float(k::c);
        const float vz = hm::sqrt((be * c * be * c) / (xp * xp + yp * yp + 1.0f));
        float rel[6];
        rel[4] = -dt * vz;
        rel[0] = r[0] - ((r[1] * dt) / g) * c;
        rel[2] = r[2] - ((dt * r[3]) / g) * c;
        rel[1] = r[1];
        rel[3] = r[3];
        rel[5] = r[5];
        for (int k = 0; k < 6; ++k) {
            F.rel[k][i] = rel[k];
            F.C[k * nn + i] = rel[k];
        }
        F.C[6 * nn + i] = 1.0f;
    } else if (A.code == kZHesc2) {
        if (!on) {
            for (int k = 0; k < 3; ++k) F.C[k * nn + i] = 0.0f;
            return;
        }
        const float ax = F.rel[0][i] - A.cen[0], ay = F.rel[2][i] - A.cen[2], az = (F.rel[4][i] - A.cen[4]) * A.gc;
        F.C[0 * nn + i] = ax * ax;
        F.C[1 * nn + i] = ay * ay;
        F.C[2 * nn + i] = az * az;
    } else if (A.code == kZTot) {
        // zTotcode: the row back to the particle at the reference time, relative to the new synchronous
        // particle {0, 0, 0, 0, z_s, uz_s} (zpl = zPlane - z_s); particles without a row keep their place
        F.nextPlane[i] = 0;
        if (on) {
            float x, ux, y, uy, z, uz;
            ztot_row(r, A.zpl, A.t, true, x, ux, y, uy, z, uz);
            // zPlane - z_s and uz_s as float + remainder (cen[4], cen[5]): their rounding would be the same for
            // every particle and move the whole bunch against the synchronous particle
            F.d[0][i] = x;
            F.d[1][i] = y;
            F.d[2][i] = z + A.cen[4];
            F.du[0][i] = ux;
            F.du[1][i] = uy;
            F.du[2][i] = (uz - A.uzs) - A.cen[5];
        } else {
            for (int j = 0; j < 3; ++j) {
                F.d[j][i] = F.d[j][i] + A.off[2 * j];
                F.du[j][i] = F.du[j][i] + A.off[2 * j + 1];
            }
        }
        for (int j = 0; j < 8; ++j) F.row[j][i] = 0.0f;
        F.recFlag[i] = 0;
    } else if (A.code == kZUbp1) {
        // UpdateBeamParameters pass 1 (kern::ubp_pass1): off = reference - s6 (positions), absolute
        // momentum of the reference (cen 1, 3, 5)
        if (F.lost[i] == 1) {
            for (int k = 0; k < 9; ++k) F.C[k * nn + i] = 0.0f;
            return;
        }
        float rel[6];
        for (int j = 0; j < 3; ++j) {
            rel[2 * j] = F.d[j][i] + A.off[2 * j];
            rel[2 * j + 1] = A.cen[2 * j + 1] + F.du[j][i];
        }
        const float w = F.w[i];
        F.C[0 * nn + i] = w;
        for (int j = 0; j < 6; ++j) {
            F.rel[j][i] = rel[j];
            F.C[(1 + j) * nn + i] = w * rel[j];
        }
        F.C[7 * nn + i] = hm::sqrt(rel[1] * rel[1] + rel[3] * rel[3] + rel[5] * rel[5] + 1.0f) * w;
        F.C[8 * nn + i] = 1.0f;
    } else if (A.code == kZUbp2) {
        // pass 2 (kern::ubp_pass2) and the extent in z of every particle (relative to the reference)
        F.Mx[0 * nn + i] = F.d[2][i];
        F.Mx[1 * nn + i] = -F.d[2][i];
        if (F.lost[i] == 1) {
            for (int k = 0; k < 3; ++k) F.C[k * nn + i] = 0.0f;
            return;
        }
        const float w = F.w[i];
        const float ax = F.rel[0][i] - A.cen[0], ay = F.rel[2][i] - A.cen[2], az = (F.rel[4][i] - A.cen[4]) * A.gc;
        F.C[0 * nn + i] = w * ax * ax;
        F.C[1 * nn + i] = w * ay * ay;
        F.C[2 * nn + i] = w * az * az;
    }
}

// ---- FFT solve on the device (M6c phase 5): the direct transforms of sc_kernels.h, one pass per call ----
struct FftP {
    AVAS_G float* rho;
    AVAS_G float* A;
    AVAS_G float* B;
    AVAS_G float* C;
    AVAS_G float* D;
    AVAS_G float* sx;
    AVAS_G float* sy;
    AVAS_G float* cz;
    AVAS_G float* sz;
    AVAS_G float* lx;
    AVAS_G float* ly;
    AVAS_G float* lz;
    AVAS_G float* phi;
    AVAS_G float* ex;
    AVAS_G float* ey;
    AVAS_G float* ez;
    int pass;
    float scale, norm, hx, hy, hz;
};

// passes 0 .. 9 in the order of the device back end (device_emu.cpp): scale, DST x, DST y, DFT z, Green
// factor, inverse DFT z, DST x, DST y, normalisation, gradient
AVAS_HD inline void fft_node(AVAS_T const sck::SpGrid& g, AVAS_T const FftP& P, int id) {
    if (id >= int(g.n3())) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    float re, im;
    switch (P.pass) {
        case 0: P.A[id] = P.rho[id] * P.scale; break;
        case 1: P.B[id] = sck::dst_node(g, 0, P.A, P.sx, i, j, k); break;
        case 2: P.A[id] = sck::dst_node(g, 1, P.B, P.sy, i, j, k); break;
        case 3:
            sck::dft_z_node(g, P.A, nullptr, P.cz, P.sz, -1.0f, i, j, k, re, im);
            P.C[id] = re;
            P.D[id] = im;
            break;
        case 4: {
            const float f = sck::green_factor(P.lx, P.ly, P.lz, i, j, k);
            P.C[id] = P.C[id] * f;
            P.D[id] = P.D[id] * f;
            break;
        }
        case 5:
            sck::dft_z_node(g, P.C, P.D, P.cz, P.sz, 1.0f, i, j, k, re, im);
            P.A[id] = re;
            P.B[id] = im;
            break;
        case 6: P.C[id] = sck::dst_node(g, 0, P.A, P.sx, i, j, k); break;
        case 7: P.D[id] = sck::dst_node(g, 1, P.C, P.sy, i, j, k); break;
        case 8: P.phi[id] = P.D[id] * P.norm; break;
        default: sck::grad_fft_node(g, P.phi, P.hx, P.hy, P.hz, i, j, k, P.ex, P.ey, P.ez); break;
    }
}

}  // namespace f32
}  // namespace avas
#endif  // AVAS_FLOAT_ONLY
