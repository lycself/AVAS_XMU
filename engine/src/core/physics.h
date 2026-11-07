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


// ---- z-code (hard-edge matrix) kernels -------------------------------------------------------
// A z-code row is {x, ux, y, uy, dt, uz, t, flag}: the particle at a fixed z plane, u = gamma*beta,
// t the time it reaches the plane, dt the time of the last transfer (MethodMP::HardEdgeTrans).
namespace k {
constexpr double c2 = 8.987551787368176e16;  // c^2 as folded by the DLL
constexpr double inv_c2 = 1.1126500560536185e-17;
}  // namespace k

// Common prologue of the particle workers: returns gamma^2 and the slopes, advances the time.
template <class R>
AVAS_HD inline R zrow_advance(R* r, R len, R& xa, R& ya) {
    const R g2 = (r[5] * r[5] + r[3] * r[3]) + (r[1] * r[1] + R(1));
    const R iuz = R(1) / r[5];
    xa = iuz * r[1];
    ya = iuz * r[3];
    const R vz = std::sqrt((R(k::c2) - R(k::c2) / g2) / ((ya * ya + xa * xa) + R(1)));
    const R dt = len / vz;
    r[6] = dt + r[6];
    r[4] = dt;
    return g2;
}

// Quadrupole, particle version (fn.1-3).  sincos = the static sincos of the DLL.
template <class R, class SinCos>
AVAS_HD inline void quad_row(R* r, R qOverM, R G, R len, SinCos sincos) {
    R xa, ya;
    const R g2 = zrow_advance(r, len, xa, ya);
    const R qG = qOverM * G;
    const R gb = std::sqrt((R(1) - R(1) / g2) * g2);
    const R kq = std::sqrt(std::fabs(qG / (gb * R(k::c))));
    const R x = r[0], y = r[2];
    const R xp = r[1] / r[5];
    const R yp = r[3] / r[5];
    const R ik = R(1) / kq;
    R X, Xp, Y, Yp;
    R s, c;
    sincos(len * kq, s, c);
    const R sh = std::sinh(kq * len), ch = std::cosh(kq * len);
    if (qG > R(0)) {
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

// Quadrupole, synchronous row (whole element, time from the total velocity).
template <class R, class SinCos>
AVAS_HD inline void quad_sync_row(R* r, R qOverM, R G, R len, SinCos sincos) {
    const R g2 = (r[5] * r[5] + r[3] * r[3]) + (r[1] * r[1] + R(1));
    const R beta = std::sqrt(R(1) - R(1) / g2);
    const R gamma = std::sqrt(g2);
    const R qG = G * qOverM;
    const R kq = std::sqrt(std::fabs(qG / ((gamma * R(k::c)) * beta)));
    const R dt = (R(k::inv_c) / beta) * len;
    r[4] = dt;
    r[6] = dt + r[6];
    const R x = r[0], y = r[2];
    const R xp = r[1] / r[5];
    const R yp = r[3] / r[5];
    const R ik = R(1) / kq;
    R s, c;
    sincos(len * kq, s, c);
    const R sh = std::sinh(kq * len), ch = std::cosh(kq * len);
    R X, Xp, Y, Yp;
    if (qG > R(0)) {
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

// Hard-edge solenoid (fn.4-6 and the synchronous row): K = qB/(2p).
template <class R, class SinCos>
AVAS_HD inline void solenoid_row(R* r, R qOverM, R B, R len, SinCos sincos) {
    R xa, ya;
    const R g2 = zrow_advance(r, len, xa, ya);
    const R gb = std::sqrt((R(1) - R(1) / g2) * g2);
    const R K = (R(k::half_inv_c) / gb) * (qOverM * B);
    const R x = r[0], y = r[2];
    R S, C;
    sincos(len * K, S, C);
    const R SC = C * S, C2 = C * C, S2 = S * S, iK = R(1) / K;
    const R X = ((S2 * ya + SC * xa) * iK + C2 * x) + SC * y;
    const R Xp = ((C2 * xa - (K * x) * SC) - (K * y) * S2) + SC * ya;
    const R Y = (C2 * y - ((S2 * xa) * iK + SC * x)) + (SC * ya) * iK;
    const R Yp = (((K * x) * S2 - SC * xa) - (K * y) * SC) + C2 * ya;
    r[0] = X;
    r[1] = r[5] * Xp;
    r[2] = Y;
    r[3] = r[5] * Yp;
}

// Bend parameters prepared by the dispatcher (fn.7-12): s = path length of this transfer.
template <class R>
struct BendStep {
    R s, h, kx, kx2, ky, sx, cx, sy, cy;
    int hv;
};

// Bend, particle version.  uzs = gamma*beta_z of the synchronous row (momentum deviation).
template <class R>
AVAS_HD inline void bend_row(R* r, R uzs, const BendStep<R>& b) {
    const R delta = (r[5] - uzs) / uzs;
    const R g2 = (r[5] * r[5] + r[3] * r[3]) + (r[1] * r[1] + R(1));
    const R iuz = R(1) / r[5];
    const R xa = iuz * r[1];
    const R ya = iuz * r[3];
    const R vz2 = ((R(1) - R(1) / g2) * R(k::c2)) / ((ya * ya + xa * xa) + R(1));
    const R dt = b.s / std::sqrt(vz2);
    r[6] = dt + r[6];
    r[4] = dt;
    // bending plane (q, qa), other plane (o, oa)
    const R q = b.hv == 0 ? r[0] : r[2];
    const R qa = b.hv == 0 ? xa : ya;
    const R o = b.hv == 0 ? r[2] : r[0];
    const R oa = b.hv == 0 ? ya : xa;
    const R dh = delta * b.h;
    const R Q = ((qa * b.sx) / b.kx + b.cx * q) + (dh * (R(1) - b.cx)) / b.kx2;
    const R Qp = (qa * b.cx - (b.kx * q) * b.sx) + (dh * b.sx) / b.kx;
    R O, Op;
    if (b.ky == R(0)) {
        O = b.s * oa + o;
        Op = oa;
    } else {
        O = (b.sy * oa) / b.ky + b.cy * o;
        Op = oa * b.cy - (o * b.ky) * b.sy;
    }
    if (b.hv == 0) {
        r[0] = Q; r[1] = r[5] * Qp; r[2] = O; r[3] = r[5] * Op;
    } else {
        r[2] = Q; r[3] = r[5] * Qp; r[0] = O; r[1] = r[5] * Op;
    }
}

// Bend, synchronous row: transverse momentum cleared, no dispersion term, whole arc.
template <class R>
AVAS_HD inline void bend_sync_row(R* r, const BendStep<R>& b) {
    const R g2 = (r[5] * r[5] + r[3] * r[3]) + (r[1] * r[1] + R(1));
    const R iuz = R(1) / r[5];
    const R xa0 = iuz * r[1];
    const R ya0 = iuz * r[3];
    const R vz = std::sqrt((R(k::c2) - R(k::c2) / g2) / ((ya0 * ya0 + xa0 * xa0) + R(1)));
    const R dt = b.s / vz;
    r[6] = dt + r[6];
    r[4] = dt;
    const R q = b.hv == 0 ? r[0] : r[2];
    const R o = b.hv == 0 ? r[2] : r[0];
    const R qa = R(0), oa = R(0);
    const R Q = (b.sx * qa) * (R(1) / b.kx) + b.cx * q;
    const R Qp = b.cx * qa - (b.kx * q) * b.sx;
    R O, Op;
    if (b.ky == R(0)) {
        O = b.s * oa + o;
        Op = oa;
    } else {
        O = (b.sy * oa) / b.ky + b.cy * o;
        Op = oa * b.cy - (o * b.ky) * b.sy;
    }
    if (b.hv == 0) {
        r[0] = Q; r[1] = r[5] * Qp; r[2] = O; r[3] = r[5] * Op;
    } else {
        r[2] = Q; r[3] = r[5] * Qp; r[0] = O; r[1] = r[5] * Op;
    }
}

// Edge (pole-face) thin lens: k1 in the bending plane, k2 in the other one.
template <class R>
AVAS_HD inline void edge_row(R* r, R k1, R k2, int hv) {
    const R iuz = R(1) / r[5];
    const R xp = iuz * r[1];
    const R yp = iuz * r[3];
    R Xp, Yp;
    if (hv == 0) {
        Xp = k1 * r[0] + xp;
        Yp = r[2] * k2 + yp;
    } else {
        Xp = r[0] * k2 + xp;
        Yp = r[2] * k1 + yp;
    }
    r[1] = r[5] * Xp;
    r[3] = r[5] * Yp;
}

// Restores |gamma*beta| = sqrt(p2) of a z-code row after a magnetic transfer, keeping the new
// slopes x' = ux/uz, y' = uy/uz.  The DLL keeps uz fixed, so every change of slope changes |p|.
template <class R>
AVAS_HD inline void zrow_keep_momentum(R* r, R p2) {
    const R xp = r[1] / r[5];
    const R yp = r[3] / r[5];
    R uz = std::sqrt(p2 / (R(1) + xp * xp + yp * yp));
    if (r[5] < R(0)) uz = -uz;
    r[5] = uz;
    r[1] = uz * xp;
    r[3] = uz * yp;
}

template <class R>
AVAS_HD inline R zrow_p2(const R* r) {
    return r[1] * r[1] + r[3] * r[3] + r[5] * r[5];
}

// Steerer kick on a z-code row.  lq = (length of the following element) * q/m.
// kind 0: magnetic (bx, by in T); kind 1: electric (ex, ey in V/m).
template <class R>
AVAS_HD inline void steerer_kick(R* r, int kind, R wx, R wy, R lq) {
    if (kind == 0) {
        const R dX = (lq * wy) * R(k::inv_c);
        const R dY = (lq * wx) * R(k::inv_c);
        r[1] = r[1] - dX;
        r[3] = dY + r[3];
        return;
    }
    const R dX = (lq * wx) * R(k::inv_c2);
    const R dY = (lq * wy) * R(k::inv_c2);
    if (kind == 1) {
        const R g2 = (r[5] * r[5] + r[3] * r[3]) + (r[1] * r[1] + R(1));
        const R ib = R(1) / std::sqrt(R(1) - R(1) / g2);
        r[1] = dX * ib + r[1];
        r[3] = dY * ib + r[3];
    } else {
        r[1] = r[1] - dX;
        r[3] = dY + r[3];
    }
}

// zTotcode: z-code row -> particle at time tRef (straight line back from the plane).
// fullGamma = false reproduces AVAS.dll, which takes gamma from gamma*beta_z alone.
template <class R>
AVAS_HD inline void ztot_row(const R* r, R zPlane, R tRef, bool fullGamma, R& x, R& ux, R& y, R& uy, R& z,
                             R& uz, bool restartForm = false) {
    const R dt = r[6] - tRef;
    if (!fullGamma) {
        const R g = r[5] * r[5] + R(1);
        const R ig = R(1) / std::sqrt(g);
        x = r[0] - (dt * r[1]) * (ig * R(k::c));
        y = r[2] - (dt * r[3]) * (ig * R(k::c));
        // zTotcode multiplies beta*c first, the superposeout restart of tcodeRun beta*dt first
        z = restartForm ? zPlane - (std::sqrt(R(1) - R(1) / g) * dt) * R(k::c)
                        : zPlane - (std::sqrt(R(1) - R(1) / g) * R(k::c)) * dt;
    } else {
        const R gamma = std::sqrt(r[1] * r[1] + R(1) + r[3] * r[3] + r[5] * r[5]);
        const R beta = std::sqrt(R(1) - (R(1) / gamma) / gamma);
        const R xp = r[1] / r[5], yp = r[3] / r[5];
        const R vz = std::sqrt((beta * R(k::c) * beta * R(k::c)) / (xp * xp + yp * yp + R(1)));
        x = r[0] - ((r[1] * dt) / gamma) * R(k::c);
        y = r[2] - ((r[3] * dt) / gamma) * R(k::c);
        z = -dt * vz + zPlane;
    }
    ux = r[1];
    uy = r[3];
    uz = r[5];
}

}  // namespace avas
