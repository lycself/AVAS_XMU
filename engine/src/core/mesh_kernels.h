// Space-charge mesh of one solve and the per-particle mesh work (CIC cell and weights, field
// interpolation), shared by the CPU engine, the CUDA back end and the Metal back end (compiled as
// MSL with double = float, see hd.h).  Moved out of kernels.h unchanged in arithmetic.
#pragma once
#include "core/hd.h"

namespace avas {
namespace kern {

// ---- space-charge mesh: CIC deposit cell and weights, interpolation (PIC::Weight / Allocate) ----

struct MeshV {
    int nx, ny, nz;
    double dx, dy, dz, vol, gamma, igamma;
    double c0, c2, c4;    // centroid x, y, z
    int lpRel;            // lpboundary: stretch about the synchronous particle
    int lpc;              // lpboundary: out-of-mesh distance from the mesh centre
    int compat;
    int lpWrap;           // corrected engine, lpboundary V1 1: z periodic with period lpLen (nz cells)
    const AVAS_G double* xs;  // node coordinates
    const AVAS_G double* ys;
    const AVAS_G double* zs;
    double Q;             // total mesh charge (for particles outside the mesh)
    double lpLen;         // period of lpWrap (rest frame, = nz dz)
    int rotMode;
    double rs, rc;
    avas_i64 fixScaleExp;  // corrected engine: fixed-point deposit, charge * 2^fixScaleExp
};

// Cell (i, j, k) and the 8 CIC weights of a particle at rel = (x, y, z4); true inside the mesh.
template <class R>
AVAS_HD inline bool mesh_cell_t(AVAS_T const MeshV& M, R x, R y, R z4, AVAS_T int& i, AVAS_T int& j, AVAS_T int& k,
                                AVAS_T R* w) {
    // lpboundary 1: AVAS.dll stretches about the synchronous particle only in the threaded deposit,
    // the serial one about the centroid; the corrected engine always about the synchronous particle
    const R g = R(M.gamma), c4 = R(M.c4);
    R zr = M.lpRel ? (M.compat ? z4 * g + c4 : z4 * g) : (z4 - c4) * g + c4;
    i = static_cast<int>(hm::floor((x - R(M.xs[0])) / R(M.dx)));
    j = static_cast<int>(hm::floor((y - R(M.ys[0])) / R(M.dy)));
    if (M.lpWrap) {
        // periodic in z: zr into [zs0, zs0 + period); the last cell joins node nz - 1 to node 0
        const R per = R(M.lpLen), z0 = R(M.zs[0]);
        zr = zr - per * hm::floor((zr - z0) / per);
        k = static_cast<int>(hm::floor((zr - z0) / R(M.dz)));
        if (k < 0) k = 0;
        if (k > M.nz - 1) k = M.nz - 1;
    } else {
        k = static_cast<int>(hm::floor((zr - R(M.zs[0])) / R(M.dz)));
    }
    if (0 <= i && i < M.nx - 1 && 0 <= j && j < M.ny - 1 && 0 <= k && k < (M.lpWrap ? M.nz : M.nz - 1)) {
        for (int m = 0; m < 8; ++m) {
            const int ii = i + ((m & 1) ^ 1);
            const int jj = j + (((m & 3) == 1 || (m & 3) == 0) ? 1 : 0);
            const int kk = k + (m < 4 ? 1 : 0);
            const R zk = kk == M.nz ? R(M.zs[M.nz - 1]) + R(M.dz) : R(M.zs[kk]);
            w[m] = hm::fabs(x - R(M.xs[ii])) * hm::fabs(y - R(M.ys[jj])) * hm::fabs(zr - zk) / R(M.vol);
        }
        return true;
    }
    for (int m = 0; m < 8; ++m) w[m] = R(0);
    return false;
}

AVAS_HD inline avas_i64 mesh_at(AVAS_T const MeshV& M, int i, int j, int k) {
    return (static_cast<avas_i64>(i) * M.ny + j) * M.nz + k;
}

// Node of corner m of cell (i, j, k), in the order of the deposit.
AVAS_HD inline avas_i64 mesh_corner(AVAS_T const MeshV& M, int i, int j, int k, int m) {
    const int di = m & 1, dj = (m >> 1) & 1, dk = (m >> 2) & 1;
    const int kz = k + dk == M.nz ? 0 : k + dk;  // only with lpWrap (else k + dk < nz)
    return mesh_at(M, i + di, j + dj, kz);
}

// CIC interpolation of one mesh field g in cell (i, j, k) with the weights w (no lambda: MSL has none).
template <class R>
AVAS_HD inline R mesh_trilinear(AVAS_T const MeshV& M, const AVAS_G double* g, int i, int j, int k, AVAS_T const R* w) {
    const int k1 = k + 1 == M.nz ? 0 : k + 1;  // only with lpWrap (else k + 1 < nz)
    return R(g[mesh_at(M, i, j, k)]) * w[0] + R(g[mesh_at(M, i + 1, j, k)]) * w[1] +
           R(g[mesh_at(M, i, j + 1, k)]) * w[2] + R(g[mesh_at(M, i + 1, j + 1, k)]) * w[3] +
           R(g[mesh_at(M, i, j, k1)]) * w[4] + R(g[mesh_at(M, i + 1, j, k1)]) * w[5] +
           R(g[mesh_at(M, i, j + 1, k1)]) * w[6] + R(g[mesh_at(M, i + 1, j + 1, k1)]) * w[7];
}

// Space-charge field of a particle at rel = (x, y, z4) from the mesh field (lab frame, E_perp / gamma).
template <class R>
AVAS_HD inline void mesh_interp_t(AVAS_T const MeshV& M, R x, R y, R z4, const AVAS_G double* ex, const AVAS_G double* ey,
                                  const AVAS_G double* ez, double kE, AVAS_T R* sc) {
    int i, j, k;
    R w[8];
    if (mesh_cell_t<R>(M, x, y, z4, i, j, k, w)) {
        const R Ez = mesh_trilinear<R>(M, ez, i, j, k, w), Ey = mesh_trilinear<R>(M, ey, i, j, k, w),
                Ex = mesh_trilinear<R>(M, ex, i, j, k, w);
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

}  // namespace kern
}  // namespace avas
