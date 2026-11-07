// Per-particle physics kernels (field interpolation, particle pushers).
//
// These functions are allocation free, have no virtual calls and take the number type as a
// template parameter, so the same source can later be compiled for CUDA (AVAS_HD becomes
// __host__ __device__) and Metal.  Arithmetic is written term by term in the order the
// original AVAS.dll evaluates it (it was built with /fp:fast, which re-associated products and
// replaced divisions by 1/c with multiplications); keep the order when editing.
#pragma once
#include <cmath>

#ifndef AVAS_HD
#define AVAS_HD
#endif

namespace avas {

namespace k {
constexpr double c = 299792458.0;
constexpr double inv_c = 3.3356409519815204e-09;       // 1/c as folded by the DLL
constexpr double half_inv_c = 1.6678204759907602e-09;  // 0.5/c
constexpr double two_pi = 6.283185307179586;
constexpr double pi = 3.141592653589793;
constexpr double half_pi = 1.5707963267948966;
constexpr double deg_to_rad = 0.017453292519943295;
constexpr double rad_to_deg = 57.29577951308232;
constexpr double inv_two_pi = 0.15915494309189535;
constexpr double e_charge = 1.60217653e-19;
constexpr double m_proton = 1.67262171e-27;
constexpr double proton_mc2 = 938.271875;
constexpr double inv_proton_mc2 = 0.0010657891669192364;  // 1/938.271875 as folded by the DLL
constexpr double e_over_mp = 95788337.57933226;            // e/m_p as folded by the DLL
}  // namespace k

// Read-only view of one field map (grid, nodes x fastest, then y, then z).
template <class R>
struct MapView {
    const double* data;  // node n, component c at data[n * ncomp + c]
    int ncomp;
    int nx, ny, nz;
    R xmin, xmax, ymin, ymax, zlen, dx, dy, dz;
};

// True when (x, y, zl) lies strictly inside the grid (zl is measured from the map entrance).
template <class R>
AVAS_HD inline bool map_contains(const MapView<R>& m, R x, R y, R zl) {
    return m.xmin < x && x < m.xmax && m.ymin < y && y < m.ymax && R(0) < zl && zl < m.zlen;
}

// Trilinear interpolation: calls acc(w, node_values) for the 8 corners in the original order.
// The weight is the expanded product (1-|a|)(1-|b|)(1-|c|) exactly as the DLL evaluates it.
template <class R, class Acc>
AVAS_HD inline void trilinear(const MapView<R>& m, R x, R y, R zl, Acc&& acc) {
    const R fx = (x - m.xmin) / m.dx;
    const R fy = (y - m.ymin) / m.dy;
    const R fz = zl / m.dz;
    const R cx = std::ceil(fx), cy = std::ceil(fy), cz = std::ceil(fz);
    for (int corner = 0; corner < 8; ++corner) {
        const int ix = static_cast<int>((corner & 1) ? cx - R(1) : cx);
        const int iy = static_cast<int>(((corner & 3) >= 2) ? cy - R(1) : cy);
        const int iz = static_cast<int>((corner >= 4) ? cz - R(1) : cz);
        const R a = fx - R(ix);
        const R b = fy - R(iy);
        const R c = fz - R(iz);
        const R ab = a * b;
        const R w = ((R(1) - std::fabs(c)) - (std::fabs(a) - std::fabs(c * a))) -
                    ((std::fabs(b) - std::fabs(c * b)) - (std::fabs(ab) - std::fabs(c * ab)));
        const int node = (m.nx + 1) * ((m.ny + 1) * iz + iy) + ix;
        acc(w, m.data + static_cast<long long>(node) * m.ncomp);
    }
}

// Boris push of one macro particle (TcodeTrans of the DLL).  u = gamma*beta.
template <class R>
AVAS_HD inline void boris_push(R& x, R& ux, R& y, R& uy, R& z, R& uz, const R E[3], const R B[3], R qOverM,
                               R dt) {
    const R h = qOverM * R(k::half_inv_c) * dt;
    ux = h * E[0] + ux;
    uy = h * E[1] + uy;
    uz = h * E[2] + uz;
    const R g = std::sqrt((uz * uz + uy * uy) + (ux * ux + R(1)));
    const R s = (dt * qOverM) / g;
    const R ty = s * B[1];
    const R tz = s * B[2];
    const R tx = s * B[0];
    const R ty2 = ty * ty, tz2 = tz * tz, tx2 = tx * tx;
    const R txy = ty * tx * R(0.5);
    const R txz = tz * tx * R(0.5);
    const R d = R(1) / ((tx2 + ty2 + tz2) * R(0.25) + R(1));
    const R tyz = tz * ty * R(0.5);
    const R nux = ((((tx2 - ty2) - tz2) * R(0.25) + R(1)) * ux + (txy + tz) * uy + (txz - ty) * uz) * d;
    const R nuy = ((((ty2 - tz2) - tx2) * R(0.25) + R(1)) * uy + (txy - tz) * ux + (tx + tyz) * uz) * d;
    const R nuz = ((txz + ty) * ux + (tyz - tx) * uy + (((tz2 - ty2) - tx2) * R(0.25) + R(1)) * uz) * d;
    ux = h * E[0] + nux;
    uy = h * E[1] + nuy;
    uz = h * E[2] + nuz;
    const R g2 = std::sqrt((uy * uy + uz * uz) + (ux * ux + R(1)));
    const R v = (R(k::c) / g2) * dt;
    x = v * ux + x;
    y = v * uy + y;
    z = v * uz + z;
}

// One step of the synchronous (reference) particle, SynParticle::Update of the DLL.
// coord = {x, ux, y, uy, z, uz}.  fullPush == 0: longitudinal only (transverse cleared).
template <class R>
AVAS_HD inline void sync_push(R coord[6], const R E[3], const R B[3], R qOverM, R dt, bool fullPush) {
    if (!fullPush) {
        coord[0] = coord[1] = coord[2] = coord[3] = R(0);
        const R uz = E[2] * qOverM * (dt * R(k::inv_c)) + coord[5];
        coord[5] = uz;
        const R g = std::sqrt(uz * uz + R(1));
        coord[4] = (R(k::c) / g) * uz * dt + coord[4];
        return;
    }
    if (B[0] == R(0) && B[1] == R(0)) {  // Bz is ignored here, as in the original
        const R k1 = dt * R(k::inv_c);
        const R ux = E[0] * qOverM * k1 + coord[1];
        coord[1] = ux;
        const R uy = E[1] * qOverM * k1 + coord[3];
        coord[3] = uy;
        const R uz = E[2] * qOverM * k1 + coord[5];
        coord[5] = uz;
        const R v = R(k::c) / std::sqrt((uz * uz + uy * uy) + (ux * ux + R(1)));
        coord[0] = v * ux * dt + coord[0];
        coord[2] = v * uy * dt + coord[2];
        coord[4] = v * uz * dt + coord[4];
        return;
    }
    const R kh = dt * R(k::half_inv_c);
    R ux = E[0] * qOverM * kh + coord[1];
    R uy = E[1] * qOverM * kh + coord[3];
    R uz = E[2] * qOverM * kh + coord[5];
    const R ig = R(1) / std::sqrt((uz * uz + uy * uy) + (ux * ux + R(1)));
    const R idt = ig * dt;
    const R tx = idt * (qOverM * B[0]);
    const R ty = (qOverM * B[1]) * idt;
    const R tz = (qOverM * B[2]) * idt;
    const R tx2 = tx * tx, ty2 = ty * ty, tz2 = tz * tz;
    const R d = R(1) / ((ty2 + tx2 + tz2) * R(0.25) + R(1));
    const R txz = tz * tx * R(0.5);
    const R txy = ty * tx * R(0.5);
    const R tyz = ty * tz * R(0.5);
    const R nux = ((((tx2 - ty2) - tz2) * R(0.25) + R(1)) * ux + (tz + txy) * uy + (txz - ty) * uz) * d;
    const R nuy = ((((ty2 - tz2) - tx2) * R(0.25) + R(1)) * uy + (txy - tz) * ux + (tyz + tx) * uz) * d;
    const R nuz = ((tyz - tx) * uy + (txz + ty) * ux + (((tz2 - ty2) - tx2) * R(0.25) + R(1)) * uz) * d;
    ux = E[0] * qOverM * kh + nux;
    uy = E[1] * qOverM * kh + nuy;
    uz = E[2] * qOverM * kh + nuz;
    coord[1] = ux;
    coord[3] = uy;
    coord[5] = uz;
    const R v = R(k::c) / std::sqrt((uz * uz + uy * uy) + (ux * ux + R(1)));
    coord[0] = v * ux * dt + coord[0];
    coord[2] = uy * v * dt + coord[2];
    coord[4] = uz * v * dt + coord[4];
}

}  // namespace avas
