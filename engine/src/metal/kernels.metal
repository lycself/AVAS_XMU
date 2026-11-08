// Metal kernels of the space-charge mesh work (M6c phase 1).  Compiled at run time by
// metal_backend.mm after core/hd.h, core/mesh_kernels.h and core/sc_kernels.h, with `double`
// defined as float, fast math off and no FP contraction: the arithmetic is that of the shared
// headers in fp32.  The fixed-point deposit adds 64-bit integers as two 32-bit atomics with carry
// (Apple GPUs have no 64-bit atomic add), so the mesh charge does not depend on the order.

using namespace metal;
using namespace avas;

kernel void k_sizes(device uint* o [[buffer(0)]]) {
    o[0] = sizeof(kern::MeshV);
    o[1] = sizeof(sck::SpGrid);
}

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
