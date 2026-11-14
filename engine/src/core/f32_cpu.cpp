// fp32 tier of the CPU device back end (M6c phase 3): the per-particle and mesh work of the Metal
// back end, compiled as C++ from the same source in the same "double = float" world (see hd.h,
// AVAS_F32_CPU).  The namespace is renamed (avas -> avas_f32) so that these float definitions never
// meet the fp64 ones of the rest of the engine; the interface below takes untyped pointers to the
// host mirrors of the Metal structures (metal_backend.mm), whose layout is checked by f32cpu_sizes.
//
// Contraction is off for the whole engine (-ffp-contract=off), and every operation used here is
// correctly rounded in IEEE 754 (+ - * /, sqrt, floor, ceil, rint, ldexp), so a CPU run and a Metal run
// compute the same floats.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#define AVAS_F32_CPU 1
#if defined(_MSC_VER)
// "double = float" turns the double literals and conversions of the shared kernels into float ones on
// purpose; MSVC would warn about each (C4305 truncation, C4244 conversion)
#pragma warning(push)
#pragma warning(disable : 4305 4244)
#endif
#define avas avas_f32
#define double float
#include "core/hd.h"
#include "core/mesh_kernels.h"
#include "core/physics.h"
#include "core/kernels.h"
#include "core/sc_kernels.h"
#include "core/f32_kernels.h"
#undef double
#undef avas
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace avas {
namespace f32cpu {

using namespace avas_f32;

void sizes(unsigned* o) {
    o[0] = sizeof(kern::MeshV);
    o[1] = sizeof(sck::SpGrid);
    o[2] = sizeof(kern::LatV);
    o[3] = sizeof(kern::ElemV);
    o[4] = sizeof(MapView<float, float>);
    o[5] = sizeof(f32::PartF);
    o[6] = sizeof(f32::StepU);
    o[7] = sizeof(kern::SubV);
    o[8] = sizeof(f32::GeomF);
    o[9] = sizeof(f32::BoundU);
    o[10] = sizeof(f32::ZArgs);
    o[11] = sizeof(f32::FftP);
}

void step(const void* L, const void* U, const void* F, const float* off, const float* rft, const int* sizes, int n) {
    const auto& LV = *static_cast<const kern::LatV*>(L);
    const auto& UV = *static_cast<const f32::StepU*>(U);
    const auto& FV = *static_cast<const f32::PartF*>(F);
    for (int i = 0; i < n; ++i) f32::step_particle(LV, UV, FV, off, rft, sizes, i);
}

void bound(const void* U, const void* F, const void* geoms, const float* segStart, const float* segEnd, const float* bndZ,
           const int* bndR1, const int* bndR2, int n) {
    const auto& UV = *static_cast<const f32::BoundU*>(U);
    const auto& FV = *static_cast<const f32::PartF*>(F);
    for (int i = 0; i < n; ++i)
        if (FV.lost[i] == 0)
            f32::bound_particle(UV, FV, static_cast<const f32::GeomF*>(geoms), segStart, segEnd, bndZ, bndR1, bndR2, i);
}

void deposit(const void* M, const float* x, const float* y, const float* z, const float* q, const int* lost, unsigned* lo,
             unsigned* hi, int n) {
    const auto& MV = *static_cast<const kern::MeshV*>(M);
    for (int p = 0; p < n; ++p) f32::deposit_particle(MV, x, y, z, q, lost, lo, hi, p);
}

void interp(const void* M, const float* x, const float* y, const float* z, const int* lost, const float* ex, const float* ey,
            const float* ez, float* sc, int n) {
    const auto& MV = *static_cast<const kern::MeshV*>(M);
    for (int p = 0; p < n; ++p) f32::interp_particle(MV, x, y, z, lost, ex, ey, ez, sc, p);
}

void picnic(const void* g, const float* rho, const float* G, float* phi) {
    const auto& S = *static_cast<const sck::SpGrid*>(g);
    int i, j, k;
    for (long long id = 0; id < S.n3(); ++id) {
        S.ijk(id, i, j, k);
        phi[id] = sck::picnic_node<float>(S, rho, G, i, j, k);
    }
}

void sym(const void* g, const float* rho, const float* G, float thr, float* phi) {
    const auto& S = *static_cast<const sck::SpGrid*>(g);
    int i, j, k;
    for (long long id = 0; id < S.n_targets(); ++id) {
        S.target(id, i, j, k);
        sck::sym_target<float>(S, rho, G, thr, i, j, k, phi);
    }
}

void corr(const void* g, const float* G, const int* corr, const float* dval, long long nc, float* phi) {
    const auto& S = *static_cast<const sck::SpGrid*>(g);
    int a, b, c;
    for (long long id = 0; id < S.n3(); ++id) {
        S.ijk(id, a, b, c);
        sck::corr_node<float>(S, G, corr, dval, nc, a, b, c, phi);
    }
}

void grad(const void* g, const float* phi, const float* h, float* ex, float* ey, float* ez) {
    const auto& S = *static_cast<const sck::SpGrid*>(g);
    int i, j, k;
    for (long long id = 0; id < S.n3(); ++id) {
        S.ijk(id, i, j, k);
        sck::grad_spicnic_node(S, phi, h[0], h[1], h[2], i, j, k, ex, ey, ez);
    }
}

// The group sums of k_group: per column the largest |value| of the group fixes e with |v| 2^e < 2^50,
// the values are summed as 64-bit integers; the maxima in float.  Max and integer sums do not depend
// on the order, so this sequential loop gives the threadgroup tree's result.
void fft(const void* g, const void* P, long long n3) {
    const auto& S = *static_cast<const sck::SpGrid*>(g);
    const auto& PV = *static_cast<const f32::FftP*>(P);
    for (long long id = 0; id < n3; ++id) f32::fft_node(S, PV, static_cast<int>(id));
}

void zrow(const void* A, const void* F, int n) {
    const auto& AV = *static_cast<const f32::ZArgs*>(A);
    const auto& FV = *static_cast<const f32::PartF*>(F);
    for (int i = 0; i < n; ++i) f32::zrow_particle(AV, FV, i);
}

void group(const float* C, const float* Mx, long long* sums, int* exps, float* mxo, int n, int stride, int ncol, int nmx, int nmx0) {
    const int ng = (n + 4095) / 4096;
    for (int g = 0; g < ng; ++g) {
        const int lo = g * 4096, hi = std::min(n, lo + 4096);
        for (int col = 0; col < ncol; ++col) {
            float m = 0.0f;
            for (int p = lo; p < hi; ++p) m = std::max(m, std::fabs(C[static_cast<size_t>(col) * stride + p]));
            int ex = 0;
            if (m > 0.0f) std::frexp(m, &ex);
            const int e = m > 0.0f ? 50 - ex : 0;
            long long acc = 0;
            for (int p = lo; p < hi; ++p)
                acc += static_cast<long long>(std::nearbyint(std::ldexp(C[static_cast<size_t>(col) * stride + p], e)));
            sums[ncol * g + col] = acc;
            exps[ncol * g + col] = e;
        }
        for (int col = 0; col < nmx; ++col) {
            float m = col < nmx0 ? 0.0f : -HUGE_VALF;
            for (int p = lo; p < hi; ++p) m = std::max(m, Mx[static_cast<size_t>(col) * stride + p]);
            mxo[nmx * g + col] = m;
        }
    }
}

}  // namespace f32cpu

}  // namespace avas
