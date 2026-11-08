// Metal kernels (M6c).  The per-particle work is in core/f32_kernels.h (shared with the fp32 tier of
// the CPU device back end); this file holds the kernel entry points and the group sums.  Compiled at run time by
// metal_backend.mm after core/hd.h, core/mesh_kernels.h and core/sc_kernels.h, with `double`
// defined as float, fast math off and no FP contraction: the arithmetic is that of the shared
// headers in fp32.  The fixed-point deposit adds 64-bit integers as two 32-bit atomics with carry
// (Apple GPUs have no 64-bit atomic add), so the mesh charge does not depend on the order.

using namespace metal;
using namespace avas;


kernel void k_deposit(constant kern::MeshV& Mc [[buffer(0)]], device const float* x [[buffer(1)]],
                      device const float* y [[buffer(2)]], device const float* z [[buffer(3)]],
                      device const float* q [[buffer(4)]], device const int* lost [[buffer(5)]],
                      device atomic_uint* lo [[buffer(6)]], device atomic_uint* hi [[buffer(7)]],
                      constant uint& n [[buffer(8)]], uint p [[thread_position_in_grid]]) {
    if (p >= n) return;
    kern::MeshV M = Mc;
    f32::deposit_particle(M, x, y, z, q, lost, lo, hi, int(p));
}

kernel void k_interp(constant kern::MeshV& Mc [[buffer(0)]], device const float* x [[buffer(1)]],
                     device const float* y [[buffer(2)]], device const float* z [[buffer(3)]],
                     device const int* lost [[buffer(4)]], device const float* ex [[buffer(5)]],
                     device const float* ey [[buffer(6)]], device const float* ez [[buffer(7)]],
                     device float* sc [[buffer(8)]], constant uint& n [[buffer(9)]],
                     uint p [[thread_position_in_grid]]) {
    if (p >= n) return;
    kern::MeshV M = Mc;
    f32::interp_particle(M, x, y, z, lost, ex, ey, ez, sc, int(p));
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

kernel void k_step(constant kern::LatV& Lc [[buffer(0)]], constant f32::StepU& Uc [[buffer(1)]],
                   constant f32::PartF& Fc [[buffer(2)]], device const float* off [[buffer(3)]], device const float* rft [[buffer(4)]],
                   constant int* sizesc [[buffer(5)]], uint i [[thread_position_in_grid]]) {
    const f32::StepU U = Uc;
    if (int(i) >= U.n) return;
    const kern::LatV L = Lc;
    const f32::PartF F = Fc;
    const int sizes[4] = {sizesc[0], sizesc[1], sizesc[2], sizesc[3]};
    f32::step_particle(L, U, F, off, rft, sizes, int(i));
}

// ---- boundary.txt (Geometry::Boundary) on the GPU: a conservative float pre-selection ----
// Every particle runs the geometry search and the wall test in float with margins; only those that
// may hit a wall, sit near a segment edge or pass the lattice end ("candidates") are decided by the
// host in double with the shared kern::boundary_particle.  The others cannot hit (their geometry
// index is updated here; the search converges to the same segment away from the edges).
kernel void k_bound(constant f32::BoundU& Uc [[buffer(0)]], constant f32::PartF& Fc [[buffer(1)]],
                    device const f32::GeomF* geoms [[buffer(2)]], device const float* segStart [[buffer(3)]],
                    device const float* segEnd [[buffer(4)]], device const float* bndZ [[buffer(5)]],
                    device const int* bndR1 [[buffer(6)]], device const int* bndR2 [[buffer(7)]], uint i [[thread_position_in_grid]]) {
    const f32::BoundU U = Uc;
    if (int(i) >= U.n) return;
    const f32::PartF F = Fc;
    f32::bound_particle(U, F, geoms, segStart, segEnd, bndZ, bndR1, bndR2, int(i));
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
    o[7] = sizeof(kern::SubV);
    o[8] = sizeof(f32::GeomF);
    o[9] = sizeof(f32::BoundU);
    o[4] = sizeof(MapView<float, float>);
    o[5] = sizeof(f32::PartF);
    o[6] = sizeof(f32::StepU);
}
