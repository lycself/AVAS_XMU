// Secondary particles (MethodMP::SeRun, spec 70 9-10): every particle is tracked alone through the
// whole lattice with its own time and time step.  Shared by the CPU tracker and the GPU back ends;
// outputs go to a sink (Sink::plane, Sink::hit, and for the synchronous particle Sink::sync_step /
// Sink::sync_iter), which writes files on the CPU and records them on a GPU.
#pragma once
#include "core/kernels.h"

namespace avas {
namespace kern {

// One lattice segment: time steps (events ev[ev0 .. ev0 + nev)) or hard-edge elements.
struct SeSegV {
    int zcode;
    int ev0, nev;  // t segment: events (z, mark, output slot)
    int el0, nel;  // z segment: elements (index, mark, output slot)
};

// Hard-edge element of SeHardEdgeTrans (prepared on the host).
struct SeElemV {
    int kind, hv;
    double strength, radius, z0, L, len;  // len: transfer length (AVAS.dll (z0 + L) - z0)
    double k1, k2;                        // edge
    double arc, h, kx, ky, sx, cx, sy, cy;  // bend (sincos of k * arc of this element)
};

struct SeV {
    const SeSegV* segs;
    int nSeg;
    const double* ev;
    const int* evMark;
    const int* evSlot;
    const int* el;
    const int* elMark;
    const int* elSlot;
    const SeElemV* elems;  // per lattice element
    const double* bendRef;  // reference gamma*beta_z of each bend (synchronous particle)
    int maxSec;
    int errOn;  // element errors on (superposeout search)
    int boundary;
    int compat;
    double L;   // lattice length
};

// SeHardEdgeTrans kernels (their operation order differs from the main-beam HardEdgeTrans).
template <class P>
AVAS_HD inline void se_quad(double* S, double qm, double G, double len) {
    const double uz = S[5], uy = S[3], ux = S[1];
    const double g2 = (uy * uy + uz * uz) + (ux * ux + 1.0);
    const double gb = hm::sqrt((1.0 - 1.0 / g2) * g2);
    const double kq = hm::sqrt(hm::fabs((G * qm) / (gb * k::c)));
    const double iuz = 1.0 / uz;
    const double xa = iuz * ux, ya = iuz * uy;
    const double vz = hm::sqrt((k::c2 - k::c2 / g2) / ((ya * ya + xa * xa) + 1.0));
    const double dt = len / vz;
    S[6] = dt + S[6];
    S[4] = dt;
    const double ph = len * kq, ik = 1.0 / kq;
    double sn, cs;
    P::sincos(ph, sn, cs);
    S[1] = xa;
    const double x = S[0], y = S[2];
    double X, Xp, Y, Yp;
    if (G * qm <= 0.0) {
        const double ch = P::cosh(ph), sh = P::sinh(ph);
        X = sh * xa * ik + x * ch;
        Xp = kq * x * sh + ch * xa;
        const double yp = S[3] / S[5];
        S[3] = yp;
        Y = yp * sn * ik + cs * y;
        Yp = cs * yp - kq * y * sn;
    } else {
        X = xa * sn * ik + cs * x;
        Xp = cs * xa - kq * x * sn;
        const double yp = S[3] / S[5];
        S[3] = yp;
        const double ch = P::cosh(ph), sh = P::sinh(ph);
        Y = sh * yp * ik + ch * y;
        Yp = kq * y * sh + ch * yp;
    }
    S[0] = X;
    S[1] = S[5] * Xp;
    S[2] = Y;
    S[3] = S[5] * Yp;
}

template <class P>
AVAS_HD inline void se_solenoid(double* S, double qm, double B, double len) {
    const double uz = S[5], uy = S[3], ux = S[1];
    const double g2 = (uy * uy + uz * uz) + (ux * ux + 1.0);
    const double gb = hm::sqrt((1.0 - 1.0 / g2) * g2);
    const double K = (k::half_inv_c / gb) * qm * B;
    const double iuz = 1.0 / uz;
    const double xa = iuz * ux, ya = iuz * uy;
    const double vz = hm::sqrt((k::c2 - k::c2 / g2) / ((ya * ya + xa * xa) + 1.0));
    const double dt = len / vz;
    S[6] = dt + S[6];
    S[4] = dt;
    double sn, cs;
    P::sincos(K * len, sn, cs);
    const double C2 = cs * cs, S2 = sn * sn, SC = cs * sn;
    S[1] = xa;
    S[3] = ya;
    const double iK = 1.0 / K;
    const double x = S[0], y = S[2];
    const double X = (S2 * ya + SC * xa) * iK + C2 * x + SC * y;
    const double Xp = ((C2 * xa - K * x * SC) - K * y * S2) + SC * ya;
    const double Y = (C2 * y - (S2 * xa * iK + SC * x)) + SC * ya * iK;
    const double Yp = ((K * x * S2 - SC * xa) - K * y * SC) + C2 * ya;
    S[0] = X;
    S[1] = S[5] * Xp;
    S[2] = Y;
    S[3] = S[5] * Yp;
}

AVAS_HD inline void se_bend(double* S, const SeElemV& e, double ref) {
    const double arc = e.arc, h = e.h, kx = e.kx, ky = e.ky;
    const double sx = e.sx, cx = e.cx, sy = e.sy, cy = e.cy;
    const double uz = S[5], iuz = 1.0 / uz;
    const double ig2 = 1.0 / ((uz * uz + S[3] * S[3]) + (S[1] * S[1] + 1.0));
    const double b2 = 1.0 - ig2;
    const double xa = iuz * S[1], ya = iuz * S[3];
    const double vz = hm::sqrt((b2 * k::c2) / ((xa * xa + ya * ya) + 1.0));
    const double dt = arc / vz;
    const double dl = (uz - ref) / ref;
    S[6] = dt + S[6];
    S[4] = dt;
    S[1] = xa;
    S[3] = ya;
    const double omc = 1.0 - cx, ikx = 1.0 / kx, ikx2 = 1.0 / (kx * kx);
    const double x = S[0], y = S[2];
    double X, Xp, Y, Yp;
    if (e.hv == 0) {
        X = sx * xa * ikx + cx * x + omc * h * dl * ikx2;
        Xp = (cx * xa - kx * x * sx) + h * dl * sx * ikx;
        if (ky == 0.0) {
            Y = ya * arc + y;
            Yp = ya;
        } else {
            Y = (ya * sy) / ky + cy * y;
            Yp = cy * ya - ky * y * sy;
        }
    } else {
        Y = sx * ya * ikx + cx * y + omc * h * dl * ikx2;
        Yp = (cx * ya - kx * y * sx) + h * dl * sx * ikx;
        if (ky == 0.0) {
            X = arc * xa + x;
            Xp = xa;
        } else {
            X = (xa * sy) / ky + cy * x;
            Xp = cy * xa - ky * x * sy;
        }
    }
    S[0] = X;
    S[1] = S[5] * Xp;
    S[2] = Y;
    S[3] = S[5] * Yp;
}

// SeTRun: own-time steps of particle i through t segment sg.  S: crossing record handed to the next
// (hard-edge) segment.  Returns false when the particle stops (lost or past the last plane record).
template <class P, class Sink>
AVAS_HD inline void se_trun(const LatV& L, const PartV& b, const SeV& V, int sg, int i, bool isSync, double* S,
                            Sink& sink) {
    if (b.lost[i] != 0) return;
    const SeSegV& seg = V.segs[sg];
    const double* Pz = V.ev + seg.ev0;
    const int nP = seg.nev;
    const double Lt = V.L;
    int iter = 0;
    // AVAS.dll compares z with a register that holds 0 only in the first step and afterwards the
    // particle's own new z (the comparisons with 0 then always pass); the corrected engine uses 0
    double thr = 0.0;
    while (true) {
        ++iter;
        if (!isSync) {
            if (iter > V.maxSec) {
                b.lost[i] = 1;
                return;
            }
        } else {
            sink.sync_step(i);
        }
        double z = b.z[i];
        int ei = b.elemIndex[i];
        const bool inSp = L.elems[ei].inSuperposeOut != 0;
        double E[3] = {0, 0, 0}, B[3] = {0, 0, 0};
        if ((thr <= z && b.lost[i] == 0 && z <= Lt) || (inSp && b.lost[i] == 0)) {
            if (z >= thr) {
                const int last = L.nElem - 1;
                if (z < L.elemZStart[ei]) {
                    do { --ei; } while (ei > 0 && z < L.elemZStart[ei]);
                } else if (L.elemZEnd[ei] <= z) {
                    do {
                        // AVAS.dll stops at a superposeout block when there are no element errors
                        if (!V.errOn && L.elems[ei].inSuperposeOut != 0) break;
                        ++ei;
                    } while (ei < last && L.elemZEnd[ei] <= z);
                }
                if (ei < 0) ei = 0;
                if (ei > last) ei = last;
                b.elemIndex[i] = ei;
                const RfSrc rf{nullptr, b.t[i]};
                if (L.errOn && L.elems[ei].errUse) element_err_field<P>(L, b, i, ei, rf);
                else element_field<P>(L, b, i, ei, rf);
                E[0] = b.ex[i]; E[1] = b.ey[i]; E[2] = b.ez[i];
                B[0] = b.bx[i]; B[1] = b.by[i]; B[2] = b.bz[i];
            }
        } else {
            if (Lt < z) b.lost[i] = 2;
            if (z < thr) b.lost[i] = 1;
        }
        double* oldp = b.old + 6 * static_cast<long long>(i);
        const double old[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
        if (b.lost[i] != 0) {
            if (isSync) sink.sync_iter(iter);
            return;
        }
        const double qm = b.qOverM[i], dt = b.dt[i];
        double ux = b.ux[i], uy = b.uy[i], uz = b.uz[i], f;
        if (qm == 0.0) {
            const double g = hm::sqrt((uy * uy + uz * uz) + (ux * ux + 1.0));
            f = (k::c / g) * dt;
        } else {
            const double hq = qm * k::inv_c;
            const double h = hq * dt * 0.5;
            ux = h * E[0] + ux;
            uy = h * E[1] + uy;
            uz = h * E[2] + uz;
            const double g = hm::sqrt((uz * uz + uy * uy) + (ux * ux + 1.0));
            const double tau = (dt * qm) / g;
            const double a = tau * B[0], bb = tau * B[1], c = tau * B[2];
            const double bb2 = bb * bb, a2 = a * a, c2 = c * c;
            const double D = 1.0 / ((a2 + bb2 + c2) * 0.25 + 1.0);
            const double ab = bb * a * 0.5, ac = c * a * 0.5, bc = c * bb * 0.5;
            const double h2 = hq * dt * 0.5;
            const double nx = ((((a2 - bb2) - c2) * 0.25 + 1.0) * ux + (ab + c) * uy + (ac - bb) * uz) * D + E[0] * h2;
            const double ny = ((((bb2 - c2) - a2) * 0.25 + 1.0) * uy + (ab - c) * ux + (bc + a) * uz) * D + E[1] * h2;
            const double nz = E[2] * h2 + ((bc - a) * uy + (ac + bb) * ux + (((c2 - bb2) - a2) * 0.25 + 1.0) * uz) * D;
            ux = nx; uy = ny; uz = nz;
            const double g2 = hm::sqrt((uz * uz + uy * uy) + (ux * ux + 1.0));
            f = (k::c / g2) * dt;
        }
        b.ux[i] = ux; b.uy[i] = uy; b.uz[i] = uz;
        b.z[i] = f * uz + b.z[i];
        b.x[i] = f * ux + b.x[i];
        b.y[i] = uy * f + b.y[i];
        b.t[i] = dt + b.t[i];
        z = b.z[i];
        if (V.compat) thr = z;
        // output planes (the original outputs only at the last one of the segment)
        const int kp = b.nextPlane[i];
        if (kp + 1 <= nP && Pz[kp] <= z && L.elems[ei].inSuperposeOut == 0) {
            const double r = (Pz[kp] - old[4]) / (z - old[4]);
            const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
            for (int j = 0; j < 6; ++j) S[j] = (cur[j] - old[j]) * r + old[j];
            S[6] = (r - 1.0) * dt + b.t[i];
            S[7] = 1.0;
            b.nextPlane[i] += 1;
            const int mk = V.evMark[seg.ev0 + kp];
            if (nP <= kp + 1) {
                if (mk == -1) sink.plane(V.evSlot[seg.ev0 + kp], S, isSync, i);
                if (isSync) sink.sync_iter(iter);
                return;
            }
            if (!V.compat && mk == -1) sink.plane(V.evSlot[seg.ev0 + kp], S, isSync, i);
        }
        // superposeout exit
        const int si = b.spOutCount[i];
        if (si < L.spOutCount && L.elems[ei].inSuperposeOut != 0) {
            const SpOutV& row = L.spOut[si];
            const double zs = L.elems[ei].zStart;
            const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
            double lc[6], lp[6];
            sp_local(row, zs, cur, lc);
            sp_local(row, zs, old, lp);
            if (0.0 < lc[4]) {
                const double r = lp[4] / (lp[4] - lc[4]);
                b.x[i] = (lc[0] - lp[0]) * r + lp[0];
                b.y[i] = (lc[2] - lp[2]) * r + lp[2];
                b.z[i] = row.row[6];
                b.ux[i] = (lc[1] - lp[1]) * r + lp[1];
                b.uy[i] = (lc[3] - lp[3]) * r + lp[3];
                b.uz[i] = (lc[5] - lp[5]) * r + lp[5];
                b.t[i] = (r - 1.0) * dt + b.t[i];
                if (row.row[6] == Lt) {
                    S[0] = b.x[i]; S[1] = b.ux[i]; S[2] = b.y[i]; S[3] = b.uy[i];
                    S[4] = b.z[i]; S[5] = b.uz[i]; S[6] = b.t[i]; S[7] = 1.0;
                    if (isSync) sink.sync_iter(iter);
                    return;
                }
                b.spOutZ1[i] = L.spOut[si + 1].row[6];
                b.spOutZ0[i] = row.row[6];
                b.elemIndex[i] = ei + 1;
                ei += 1;
                b.spOutCount[i] = si + 1;
            }
        }
        if (V.boundary) {
            const double zz = b.z[i];
            if (zz < thr || Pz[nP - 1] < zz || b.lost[i] != 0) {
                if (Lt < zz && L.spOutCur == L.spOutSize && b.lost[i] == 0) b.lost[i] = 2;
            } else {
                int gi = b.geomIndex[i];
                for (int j = 0; j < 6; ++j) oldp[j] = old[j];
                double Pp[6], tHit = b.t[i];
                int code = 0;
                if (zz < L.segStart[gi]) {
                    do { --gi; } while (gi > 0 && zz < L.segStart[gi]);
                } else {
                    while (gi + 1 < L.nSeg && L.segEnd[gi] <= zz) {
                        // corrected (M10 D5): a segment crossed completely in this step
                        if (!P::compat) {
                            code = geometry_hit<P>(L.geoms, gi, b, i, b.t[i], dt, 0.0, Pp, tHit);
                            if (code != 0) break;
                        }
                        ++gi;
                    }
                }
                b.geomIndex[i] = gi;
                if (code == 0) code = geometry_hit<P>(L.geoms, gi, b, i, b.t[i], dt, 0.0, Pp, tHit);
                if (code == 1) b.lost[i] = 1;
                if (code != 0) sink.hit(gi, code, Pp, tHit, i);
                if (code < 0) return;
            }
        }
    }
}

// SeZRun / SeHardEdgeTrans: transfer matrices of the hard-edge elements of segment sg.
template <class P, class Sink>
AVAS_HD inline void se_zrun(const LatV& L, const PartV& b, const SeV& V, int sg, int i, bool isSync, double* S,
                            Sink& sink, double* bendRefOut) {
    if (b.lost[i] != 0 || S[7] != 1.0) return;
    const SeSegV& seg = V.segs[sg];
    for (int j = 0; j < seg.nel; ++j) {
        const int idx = V.el[seg.el0 + j];
        const SeElemV& e = V.elems[idx];
        const double qm = b.qOverM[i];
        switch (e.kind) {
            case kQuad: se_quad<P>(S, qm, e.strength, e.len); break;
            case kSolenoid: se_solenoid<P>(S, qm, e.strength, e.len); break;
            case kEdge: edge_row(S, e.k1, e.k2, e.hv == 0 ? 0 : 1); break;
            case kBend:
                if (isSync && bendRefOut) bendRefOut[idx] = S[5];
                se_bend(S, e, isSync && bendRefOut ? bendRefOut[idx] : V.bendRef[idx]);
                break;
            default: break;
        }
        const double r = hm::sqrt(S[2] * S[2] + S[0] * S[0]);
        if (e.radius <= r) {
            b.lost[i] = 1;
            return;
        }
        if (V.elMark[seg.el0 + j] == -2) sink.plane(V.elSlot[seg.el0 + j], S, isSync, i);
    }
    const SeElemV& lastE = V.elems[V.el[seg.el0 + seg.nel - 1]];
    const double zEnd = lastE.L + lastE.z0;
    b.nextPlane[i] = 0;
    if (S[7] == 1.0) {
        b.t[i] = S[6];
        b.x[i] = S[0];
        b.y[i] = S[2];
        b.z[i] = zEnd;
        b.ux[i] = S[1];
        b.uy[i] = S[3];
        b.uz[i] = S[5];
    } else {
        b.lost[i] = 1;
    }
    for (int j = 0; j < 8; ++j) S[j] = 0.0;
}

// The whole lattice for particle i.
template <class P, class Sink>
AVAS_HD inline void se_track(const LatV& L, const PartV& b, const SeV& V, int i, bool isSync, Sink& sink,
                             double* bendRefOut) {
    double S[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int sg = 0; sg < V.nSeg; ++sg) {
        if (V.segs[sg].zcode) se_zrun<P>(L, b, V, sg, i, isSync, S, sink, bendRefOut);
        else se_trun<P>(L, b, V, sg, i, isSync, S, sink);
    }
}

}  // namespace kern
}  // namespace avas
