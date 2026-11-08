// Metal kernels of the space-charge mesh work (M6c phase 1).  Compiled at run time by
// metal_backend.mm after core/hd.h, core/mesh_kernels.h and core/sc_kernels.h, with `double`
// defined as float, fast math off and no FP contraction: the arithmetic is that of the shared
// headers in fp32.  The fixed-point deposit adds 64-bit integers as two 32-bit atomics with carry
// (Apple GPUs have no 64-bit atomic add), so the mesh charge does not depend on the order.

using namespace metal;
using namespace avas;


// charge * 2^e rounded to the nearest integer (ties to even); exact power-of-two scaling in float
inline long fix_charge_f(float v, long e) { return long(metal::rint(metal::ldexp(v, int(e)))); }

inline void add64(device atomic_uint* lo, device atomic_uint* hi, long idx, long v) {
    const ulong u = ulong(v);
    const uint l = uint(u & 0xffffffffull), h = uint(u >> 32);
    const uint old = atomic_fetch_add_explicit(lo + idx, l, memory_order_relaxed);
    const uint carry = (old + l < old) ? 1u : 0u;
    atomic_fetch_add_explicit(hi + idx, h + carry, memory_order_relaxed);
}

kernel void k_deposit(constant kern::MeshV& Mc [[buffer(0)]], device const float* x [[buffer(1)]],
                      device const float* y [[buffer(2)]], device const float* z [[buffer(3)]],
                      device const float* q [[buffer(4)]], device const int* lost [[buffer(5)]],
                      device atomic_uint* lo [[buffer(6)]], device atomic_uint* hi [[buffer(7)]],
                      constant uint& n [[buffer(8)]], uint p [[thread_position_in_grid]]) {
    if (p >= n || lost[p] == 1) return;
    kern::MeshV M = Mc;
    int i, j, k;
    float w[8];
    if (!kern::mesh_cell_t<float>(M, x[p], y[p], z[p], i, j, k, w)) return;
    for (int m = 0; m < 8; ++m) add64(lo, hi, kern::mesh_corner(M, i, j, k, m), fix_charge_f(w[m] * q[p], M.fixScaleExp));
}

kernel void k_interp(constant kern::MeshV& Mc [[buffer(0)]], device const float* x [[buffer(1)]],
                     device const float* y [[buffer(2)]], device const float* z [[buffer(3)]],
                     device const int* lost [[buffer(4)]], device const float* ex [[buffer(5)]],
                     device const float* ey [[buffer(6)]], device const float* ez [[buffer(7)]],
                     device float* sc [[buffer(8)]], constant uint& n [[buffer(9)]],
                     uint p [[thread_position_in_grid]]) {
    if (p >= n) return;
    if (lost[p] == 1) {
        sc[3 * p] = sc[3 * p + 1] = sc[3 * p + 2] = 0.0f;
        return;
    }
    kern::MeshV M = Mc;
    float s[3];
    kern::mesh_interp_t<float>(M, x[p], y[p], z[p], ex, ey, ez, sck::kE, s);
    sc[3 * p] = s[0];
    sc[3 * p + 1] = s[1];
    sc[3 * p + 2] = s[2];
}

kernel void k_picnic(constant sck::SpGrid& gc [[buffer(0)]], device const float* rho [[buffer(1)]],
                     device const float* G [[buffer(2)]], device float* phi [[buffer(3)]],
                     uint id [[thread_position_in_grid]]) {
    sck::SpGrid g = gc;
    if (long(id) >= g.n3()) return;
    int i, j, k;
    g.ijk(long(id), i, j, k);
    phi[id] = sck::picnic_node<float>(g, rho, G, i, j, k);
}

kernel void k_sym(constant sck::SpGrid& gc [[buffer(0)]], device const float* rho [[buffer(1)]],
                  device const float* G [[buffer(2)]], constant float& thr [[buffer(3)]], device float* phi [[buffer(4)]],
                  uint id [[thread_position_in_grid]]) {
    sck::SpGrid g = gc;
    if (long(id) >= g.n_targets()) return;
    int i, j, k;
    g.target(long(id), i, j, k);
    sck::sym_target<float>(g, rho, G, thr, i, j, k, phi);
}

kernel void k_corr(constant sck::SpGrid& gc [[buffer(0)]], device const float* G [[buffer(1)]],
                   device const int* corr [[buffer(2)]], device const float* dval [[buffer(3)]],
                   constant long& nc [[buffer(4)]], device float* phi [[buffer(5)]], uint id [[thread_position_in_grid]]) {
    sck::SpGrid g = gc;
    if (long(id) >= g.n3()) return;
    int a, b, c;
    g.ijk(long(id), a, b, c);
    sck::corr_node<float>(g, G, corr, dval, nc, a, b, c, phi);
}

kernel void k_grad(constant sck::SpGrid& gc [[buffer(0)]], device const float* phi [[buffer(1)]],
                   constant float* h [[buffer(2)]], device float* ex [[buffer(3)]], device float* ey [[buffer(4)]],
                   device float* ez [[buffer(5)]], uint id [[thread_position_in_grid]]) {
    sck::SpGrid g = gc;
    if (long(id) >= g.n3()) return;
    int i, j, k;
    g.ijk(long(id), i, j, k);
    sck::grad_spicnic_node(g, phi, h[0], h[1], h[2], i, j, k, ex, ey, ez);
}

// ---- M6c phase 2: the t-code step in fp32 (port of cuda_f32.cuh F32Step) ----
// Particle state relative to the synchronous particle; the synchronous particle's push, the lattice
// positions relative to it and the RF factors come from the host every step.  Statistics are
// written per particle (C: 21 sums, Mx: 8 maxima) and summed per reduction group by k_group in
// fixed point; crossings of output planes leave a snapshot that the host completes in double.

struct RelZM {  // lattice positions relative to the synchronous particle after the step
    const device float* es;
    const device float* ee;
    const device float* e0;
    float z0, spx, tot;
    float elem_start(int k) const { return es[k]; }
    float elem_end(int k) const { return ee[k]; }
    float estart(int k, device const kern::ElemV&) const { return e0[k]; }
    float zero() const { return z0; }
    float sp_exit() const { return spx; }
    float total() const { return tot; }
};

struct PartF {
    device float* d[3];
    device float* du[3];
    device float* E[3];
    device float* B[3];
    device float* sc[3];
    device float* rel[6];
    device float* hq;
    device float* tq;
    device float* w;
    device float* spz0;
    device float* spz1;
    device int* lost;
    device int* elemIndex;
    device int* nextPlane;
    device int* boundaryMode;
    device int* recFlag;
    device float* C;      // 21 per particle
    device float* Mx;     // 8 per particle
    device float* snap;   // 12 per particle: d0, u0, dn, up of a plane crossing
    device float* oldS;   // 6 per particle: d0, u0 (writeOld)
    device atomic_uint* crossCount;
    device uint* crossIdx;
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

inline void K_of(float tx, float ty, float tz, thread float* K) {
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

kernel void k_step(constant kern::LatV& Lc [[buffer(0)]], constant StepU& U [[buffer(1)]], constant PartF& F [[buffer(2)]],
                   device const float* off [[buffer(3)]], device const float* rft [[buffer(4)]],
                   constant int* sizes [[buffer(5)]], uint i [[thread_position_in_grid]]) {
    if (int(i) >= U.n) return;
    kern::LatV L = Lc;
    const int nE = sizes[0], nEv = sizes[2];
    RelZM zo;
    zo.es = off;
    zo.ee = off + nE;
    zo.e0 = off + 2 * nE;
    zo.z0 = U.z0;
    zo.spx = U.spx;
    zo.tot = U.tot;
    device const float* evRel = off + 3 * nE + sizes[1];
    device const float* evOk = evRel + nEv;
    kern::RfSrc rf;
    rf.table = rft;
    rf.t = 0.0f;
    // column-major (coalesced): C[j n + i], Mx[j n + i]
    const uint nn = uint(U.n);
    float C[21], Mx[8];
    for (int j = 0; j < 21; ++j) C[j] = 0.0f;
    for (int j = 0; j < 6; ++j) Mx[j] = 0.0f;
    Mx[6] = Mx[7] = -INFINITY;
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
    if (cross) {
        device float* sn = F.snap + 12 * i;
        for (int j = 0; j < 3; ++j) {
            sn[j] = d0[j];
            sn[3 + j] = u0[j];
            sn[6 + j] = dn[j];
            sn[9 + j] = up[j];
        }
        F.crossIdx[atomic_fetch_add_explicit(F.crossCount, 1u, memory_order_relaxed)] = i;
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

// One reduction group (4096 particles) per threadgroup of 256: per column the largest |value|
// fixes a power of two (|v| 2^e < 2^50), the values are summed as 64-bit integers (exact, so
// independent of the order) and the host gets sum and e; maxima are exact in float.
kernel void k_group(device const float* C [[buffer(0)]], device const float* Mx [[buffer(1)]],
                    device long* sums [[buffer(2)]], device int* exps [[buffer(3)]], device float* mxo [[buffer(4)]],
                    constant int& n [[buffer(5)]], uint g [[threadgroup_position_in_grid]],
                    uint t [[thread_position_in_threadgroup]]) {
    threadgroup float tf[256];
    threadgroup long tl[256];
    const int lo = int(g) * 4096, hi = min(n, lo + 4096);
    for (int col = 0; col < 21; ++col) {
        float m = 0.0f;
        for (int p = lo + int(t); p < hi; p += 256) m = max(m, fabs(C[col * n + p]));
        tf[t] = m;
        threadgroup_barrier(mem_flags::mem_threadgroup);
        for (uint s2 = 128; s2 > 0; s2 >>= 1) {
            if (t < s2) tf[t] = max(tf[t], tf[t + s2]);
            threadgroup_barrier(mem_flags::mem_threadgroup);
        }
        const float gm = tf[0];
        threadgroup_barrier(mem_flags::mem_threadgroup);
        int ex = 0;
        if (gm > 0.0f) {
            frexp(gm, ex);  // gm < 2^ex
        }
        const int e = gm > 0.0f ? 50 - ex : 0;
        long acc = 0;
        for (int p = lo + int(t); p < hi; p += 256) acc += long(rint(ldexp(C[col * n + p], e)));
        tl[t] = acc;
        threadgroup_barrier(mem_flags::mem_threadgroup);
        for (uint s2 = 128; s2 > 0; s2 >>= 1) {
            if (t < s2) tl[t] = tl[t] + tl[t + s2];
            threadgroup_barrier(mem_flags::mem_threadgroup);
        }
        if (t == 0) {
            sums[21 * g + col] = tl[0];
            exps[21 * g + col] = e;
        }
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }
    for (int col = 0; col < 8; ++col) {
        float m = col < 6 ? 0.0f : -INFINITY;
        for (int p = lo + int(t); p < hi; p += 256) m = max(m, Mx[col * n + p]);
        tf[t] = m;
        threadgroup_barrier(mem_flags::mem_threadgroup);
        for (uint s2 = 128; s2 > 0; s2 >>= 1) {
            if (t < s2) tf[t] = max(tf[t], tf[t + s2]);
            threadgroup_barrier(mem_flags::mem_threadgroup);
        }
        if (t == 0) mxo[8 * g + col] = tf[0];
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }
}

kernel void k_sizes(device uint* o [[buffer(0)]]) {
    o[0] = sizeof(kern::MeshV);
    o[1] = sizeof(sck::SpGrid);
    o[2] = sizeof(kern::LatV);
    o[3] = sizeof(kern::ElemV);
    o[4] = sizeof(MapView<float, float>);
    o[5] = sizeof(PartF);
    o[6] = sizeof(StepU);
}
