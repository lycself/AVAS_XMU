// fp32 particle state of the CUDA back end ("precision fp32").
//
// Coordinates are kept relative to the synchronous particle -- position x - x_s, momentum u - u_s --
// in float, the fields of the elements are evaluated in float with the same code as fp64
// (kernels.h, lattice positions relative to the synchronous particle computed in double on the
// host every step), and everything that needs more digits stays in double: time, RF phase, the
// synchronous particle, crossing records, the reconstruction of absolute coordinates and the sums
// of the statistics.  The Boris push is written for the differences to the synchronous particle's
// own push (same formula, see boris_push in physics.h), so no large number is subtracted in float.
#pragma once
#include "core/device.h"
#include "core/kernels.h"

namespace avas {
namespace cu {

// Lattice positions relative to the synchronous particle after the step (float).
struct RelZ {
    const float* es;  // elemZStart - zs
    const float* ee;  // elemZEnd - zs
    const float* e0;  // element zStart - zs
    const float* s0;  // sub-field zStart - zs
    float z0, spx, tot;
    __host__ __device__ float elem_start(int k) const { return es[k]; }
    __host__ __device__ float elem_end(int k) const { return ee[k]; }
    __host__ __device__ float estart(int k, const kern::ElemV&) const { return e0[k]; }
    __host__ __device__ float sstart(int q, const kern::SubV&) const { return s0[q]; }
    __host__ __device__ float zero() const { return z0; }
    __host__ __device__ float sp_exit() const { return spx; }
    __host__ __device__ float total() const { return tot; }
};

// The synchronous particle's push of this step (host, double, then float).
struct SyncStep {
    float hEs[3];  // first half kick h_s E_s
    float usm[3];  // momentum after it
    float gsm;     // its gamma
    float Ks[9];   // Boris rotation minus identity
    float usp[3];  // momentum after the step
    float gsp;     // gamma after the step
    float cdt;     // c dt
    double pre[6], post[6];
};

struct F32View {
    float* d[3];   // x - xs, y - ys, z - zs
    float* du[3];  // ux - uxs, uy - uys, uz - uzs
    float* E[3];
    float* B[3];
    float* sc[3];
    float* rel[6];  // rel[] of the last step (statistics of the next one)
    float* hq;      // q/m 0.5/c dt
    float* tq;      // dt q/m
};

struct F32Step {
    kern::LatV L;
    RelZ zo;
    kern::PartV P;
    F32View F;
    kern::StepArgs a;
    kern::RfSrc rf;
    SyncStep S;
    float cen[6];
    float zFieldPre;  // zField - zs before the step
    float segEnd, zNeg;
    const float* evRel;
    const float* evOk;
    unsigned long long* counters;

    __device__ void neutral(double* nt) const {
        for (int j = 0; j < 6; ++j) nt[j] = 0.0;
        nt[6] = -1e10;
        nt[7] = -1e10;
    }

    __device__ static void K_of(float tx, float ty, float tz, float* K) {
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

    __device__ void operator()(int i, double* s, double* m) const {
        const int lost0 = P.lost[i];
        const float d0[3] = {F.d[0][i], F.d[1][i], F.d[2][i]};
        const float u0[3] = {F.du[0][i], F.du[1][i], F.du[2][i]};
        if (lost0 != 1) {
            const double w = P.weight[i];
            const float inv = 1.0f / F.rel[5][i];
            double d[6];
            d[0] = F.rel[0][i] - cen[0];
            d[2] = F.rel[2][i] - cen[2];
            d[4] = F.rel[4][i] - cen[4];
            d[1] = (F.rel[1][i] - cen[1]) * inv;
            d[3] = (F.rel[3][i] - cen[3]) * inv;
            d[5] = (F.rel[5][i] - cen[5]) * inv;
            const double zabs = S.pre[4] + static_cast<double>(d0[2]);
            m[6] = -zabs;
            m[7] = zabs;
            const double gp = a.gp;
            s[9] = gp * d[4] * gp * d[4] * w;
            for (int j = 0; j < 6; ++j) {
                m[j] = d[j] < 0 ? -d[j] : d[j];
                s[j] = d[j] * d[j] * w;
            }
            s[6] = d[1] * d[0] * w;
            s[7] = d[3] * d[2] * w;
            s[8] = d[5] * d[4] * w;
        }
        float tot[6];
        if (a.zMinPrev <= a.zField && !(zFieldPre < d0[2])) {
            for (int j = 0; j < 6; ++j) tot[j] = 0.0f;
        } else {
            for (int j = 0; j < 3; ++j) {
                tot[j] = F.E[j][i] + F.sc[j][i];
                tot[3 + j] = F.B[j][i];
            }
        }
        if (a.writeOld) {
            double* old = P.old + 6 * static_cast<long long>(i);
            for (int j = 0; j < 3; ++j) {
                old[2 * j] = S.pre[2 * j] + static_cast<double>(d0[j]);
                old[2 * j + 1] = S.pre[2 * j + 1] + static_cast<double>(u0[j]);
            }
        }
        if (lost0 == 1) {
            s[kern::kStepSums + 1] = P.rec[8 * static_cast<long long>(i) + 7] == 1.0 ? 1.0 : 0.0;
            return;
        }
        // Boris push relative to the synchronous particle's push
        const float h = F.hq[i];
        float dk[3], dm[3];
        for (int j = 0; j < 3; ++j) {
            dk[j] = h * tot[j] - S.hEs[j];
            dm[j] = u0[j] + dk[j];
        }
        float N = 0.0f;
        for (int j = 0; j < 3; ++j) N += (S.usm[j] + S.usm[j] + dm[j]) * dm[j];
        const float gm = hm::sqrt(S.gsm * S.gsm + N);
        const float sq = F.tq[i] / gm;
        float K[9];
        K_of(sq * tot[3], sq * tot[4], sq * tot[5], K);
        float up[3];
        for (int r = 0; r < 3; ++r) {
            float acc = dm[r];
            for (int c = 0; c < 3; ++c) acc += K[3 * r + c] * dm[c] + (K[3 * r + c] - S.Ks[3 * r + c]) * S.usm[c];
            up[r] = acc + dk[r];
        }
        float Np = 0.0f;
        for (int j = 0; j < 3; ++j) Np += (S.usp[j] + S.usp[j] + up[j]) * up[j];
        const float gpi = hm::sqrt(S.gsp * S.gsp + Np);
        const float dg = Np / (gpi + S.gsp);
        float dn[3];
        for (int j = 0; j < 3; ++j) {
            dn[j] = d0[j] + S.cdt * ((up[j] * S.gsp - S.usp[j] * dg) / (gpi * S.gsp));
            F.d[j][i] = dn[j];
            F.du[j][i] = up[j];
        }
        // rel[] of this step (absolute momenta), turned in a superposeout block
        float rel[6];
        const float ux = S.usp[0] + up[0], uy = S.usp[1] + up[1], uz = S.usp[2] + up[2];
        if (a.rotMode == 0) {
            rel[0] = dn[0]; rel[2] = dn[1]; rel[4] = dn[2];
            rel[1] = ux; rel[3] = uy; rel[5] = uz;
        } else {
            const float sn = static_cast<float>(a.rs), cs = static_cast<float>(a.rc);
            if (a.rotMode == 2) {
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
        // field of the elements at the new position
        kern::FieldIn<float> in;
        in.x = static_cast<float>(S.post[0]) + dn[0];
        in.y = static_cast<float>(S.post[2]) + dn[1];
        in.fz = dn[2];
        in.rel0 = rel[0];
        in.rel2 = rel[2];
        in.spZ0 = P.spOutZ0[i] - S.post[4];
        in.spZ1 = P.spOutZ1[i] - S.post[4];
        in.boundaryMode = P.boundaryMode[i];
        float E[3], B[3];
        int k = P.elemIndex[i];
        const int lostNow = P.lost[i];
        const int f = kern::external_field_t<PortableMath, float, RelZ>(L, zo, in, lostNow, false, k, rf, E, B);
        if (!(dn[2] < zo.z0 || zo.spx < dn[2] || lostNow != 0)) P.elemIndex[i] = k;
        if (f & kern::kFieldLost) P.lost[i] = 1;
        if (f & kern::kFieldLostEnd) P.lost[i] = 2;
        if (f & kern::kFieldWrite)
            for (int j = 0; j < 3; ++j) {
                F.E[j][i] = E[j];
                F.B[j][i] = B[j];
            }
        if (dn[2] > segEnd)
            for (int j = 0; j < 3; ++j) F.E[j][i] = F.B[j][i] = 0.0f;
        if (uz < 0.0f && L.spOutCount <= L.spOutCur && dn[2] < zNeg) P.lost[i] = 1;
        // crossing of the next output plane, recorded in double
        const int mp = P.nextPlane[i];
        const bool cross = mp + 1 <= a.nEvents && evOk[mp] != 0.0f && evRel[mp] <= dn[2] + 1e-4f * hm::fabs(dn[2]) + 1e-9f;
        const bool needAbs = cross ||
                             (L.spOutCur < L.spOutCount);
        if (needAbs) {
            double cur[6], old[6];
            for (int j = 0; j < 3; ++j) {
                cur[2 * j] = S.post[2 * j] + static_cast<double>(dn[j]);
                cur[2 * j + 1] = S.post[2 * j + 1] + static_cast<double>(up[j]);
                old[2 * j] = S.pre[2 * j] + static_cast<double>(d0[j]);
                old[2 * j + 1] = S.pre[2 * j + 1] + static_cast<double>(u0[j]);
            }
            if (cross) {
                const double pl = a.events[mp];
                if (pl <= cur[4]) {
                    const double r = (pl - old[4]) / (cur[4] - old[4]);
                    double* rec = P.rec + 8 * static_cast<long long>(i);
                    for (int j = 0; j < 6; ++j) rec[j] = (cur[j] - old[j]) * r + old[j];
                    kern::interp_momentum_norm(rec, old, cur, r);
                    rec[6] = (r - 1.0) * a.dt + a.tEnd;
                    rec[7] = 1.0;
                    P.nextPlane[i] += 1;
                }
            }
            if (L.spOutCur < L.spOutCount && L.elems[P.elemIndex[i]].inSuperposeOut != 0 &&
                P.spRec[8 * static_cast<long long>(i) + 7] == 0.0 && cur[4] > P.spOutZ0[i]) {
                const kern::SpOutV& row = L.spOut[L.spOutCur];
                const double zs = L.elems[P.elemIndex[i]].zStart;
                double lc[6], lp[6];
                kern::sp_local(row, zs, cur, lc);
                kern::sp_local(row, zs, old, lp);
                if (lc[4] > 0.0) {
                    const double r = lp[4] / (lp[4] - lc[4]);
                    double* sr = P.spRec + 8 * static_cast<long long>(i);
                    for (int j = 0; j < 6; ++j) sr[j] = (lc[j] - lp[j]) * r + lp[j];
                    kern::interp_momentum_norm(sr, lp, lc, r);
                    sr[6] = (r - 1.0) * a.dt + a.tEnd;
                    sr[7] = 1.0;
                    s[kern::kStepSums] = 1.0;
                }
            }
        }
        if (P.lost[i] != 1) {
            const double w = P.weight[i];
            s[10] = 1.0;
            s[11] = w;
            for (int j = 0; j < 6; ++j) s[12 + j] = w * static_cast<double>(rel[j]);
            s[18] = (static_cast<double>(S.gsp) + static_cast<double>(dg)) * w;
        }
        s[kern::kStepSums + 1] = P.rec[8 * static_cast<long long>(i) + 7] == 1.0 ? 1.0 : 0.0;
    }
};

// fp64 arrays -> float state (relative to the synchronous particle "ref")
__global__ void k_f32_from64(kern::PartV P, F32View F, int n, SyncStep S, double dt) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    const double c[6] = {P.x[i], P.ux[i], P.y[i], P.uy[i], P.z[i], P.uz[i]};
    for (int j = 0; j < 3; ++j) {
        F.d[j][i] = static_cast<float>(c[2 * j] - S.pre[2 * j]);
        F.du[j][i] = static_cast<float>(c[2 * j + 1] - S.pre[2 * j + 1]);
    }
    F.E[0][i] = static_cast<float>(P.ex[i]); F.E[1][i] = static_cast<float>(P.ey[i]); F.E[2][i] = static_cast<float>(P.ez[i]);
    F.B[0][i] = static_cast<float>(P.bx[i]); F.B[1][i] = static_cast<float>(P.by[i]); F.B[2][i] = static_cast<float>(P.bz[i]);
    F.sc[0][i] = static_cast<float>(P.scx[i]); F.sc[1][i] = static_cast<float>(P.scy[i]); F.sc[2][i] = static_cast<float>(P.scz[i]);
    for (int j = 0; j < 6; ++j) F.rel[j][i] = static_cast<float>(P.rel[j][i]);
    F.hq[i] = static_cast<float>(P.qOverM[i] * k::half_inv_c * dt);
    F.tq[i] = static_cast<float>(dt * P.qOverM[i]);
}

// float state -> fp64 arrays (absolute coordinates from the reference "post")
__global__ void k_f32_to64(kern::PartV P, F32View F, int n, SyncStep S) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    double* c[6] = {P.x, P.ux, P.y, P.uy, P.z, P.uz};
    for (int j = 0; j < 3; ++j) {
        c[2 * j][i] = S.post[2 * j] + static_cast<double>(F.d[j][i]);
        c[2 * j + 1][i] = S.post[2 * j + 1] + static_cast<double>(F.du[j][i]);
    }
    P.ex[i] = F.E[0][i]; P.ey[i] = F.E[1][i]; P.ez[i] = F.E[2][i];
    P.bx[i] = F.B[0][i]; P.by[i] = F.B[1][i]; P.bz[i] = F.B[2][i];
    P.scx[i] = F.sc[0][i]; P.scy[i] = F.sc[1][i]; P.scz[i] = F.sc[2][i];
    for (int j = 0; j < 6; ++j) P.rel[j][i] = F.rel[j][i];
}

__global__ void k_f32_deposit(kern::MeshV M, kern::PartV P, F32View F, int n, unsigned long long* grid) {
    const int p = blockIdx.x * blockDim.x + threadIdx.x;
    if (p >= n || P.lost[p] == 1) return;
    int i, j, k;
    float w[8];
    if (!kern::mesh_cell_t<float>(M, F.rel[0][p], F.rel[2][p], F.rel[4][p], i, j, k, w)) return;
    const double q = P.charge[p];
    for (int m = 0; m < 8; ++m) {
        const long long v = kern::fix_charge(static_cast<double>(w[m]) * q, M.fixScaleExp);
        atomicAdd(grid + kern::mesh_corner(M, i, j, k, m), static_cast<unsigned long long>(v));
    }
}

__global__ void k_f32_interp(kern::MeshV M, kern::PartV P, F32View F, int n, const double* ex, const double* ey,
                             const double* ez) {
    const int p = blockIdx.x * blockDim.x + threadIdx.x;
    if (p >= n) return;
    if (P.lost[p] == 1) {
        F.sc[0][p] = F.sc[1][p] = F.sc[2][p] = 0.0f;
        return;
    }
    float sc[3];
    kern::mesh_interp_t<float>(M, F.rel[0][p], F.rel[2][p], F.rel[4][p], ex, ey, ez, 8987551787.368176, sc);
    F.sc[0][p] = sc[0];
    F.sc[1][p] = sc[1];
    F.sc[2][p] = sc[2];
}

__global__ void k_to_float(const double* in, float* out, long long n) {
    const long long e = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (e < n) out[e] = static_cast<float>(in[e]);
}

}  // namespace cu
}  // namespace avas
