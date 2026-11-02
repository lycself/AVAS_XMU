"""The two-dimensional KV envelope equations, as a theory reference for the full linear model.

    a'' + k_x(s)·a − 2K/(a + b) − ε_x²/a³ = 0,    b'' + k_y(s)·b − 2K/(a + b) − ε_y²/b³ = 0

with the edge radii a = 2σ_x, b = 2σ_y, the full emittances ε = 4·ε_rms and the generalised
perveance K of a continuous (dc) beam.  The lattice is a list of hard-edge segments
``(length [m], k_x [1/m²], k_y [1/m²])``; a solenoid in the Larmor frame focuses both planes
with (B/2Bρ)².  The full model (:mod:`avas.model`) solves the same rms equations through its
σ matrices for a dc beam; this module is short on purpose so the two can check each other.
"""
import math

import numpy as np
from scipy.integrate import solve_ivp
from scipy.optimize import least_squares

from avas.data.lattice_doc import LatticeDocument, to_float
from avas.model.common import C_LIGHT, EPS0, MEV_TM


def perveance(current_ma, energy, mass, charge=1.0):
    """Generalised perveance K = 2·I / (I0·(βγ)³) of a dc beam, I0 = 4πε0·mc³/q (31.3 MA for protons)."""
    gamma = 1 + energy / mass
    bg = math.sqrt(gamma * gamma - 1)
    i0 = 4 * math.pi * EPS0 * C_LIGHT * mass * 1e6 / abs(charge)
    return 2 * current_ma * 1e-3 / (i0 * bg ** 3)


def segments_from_lattice(text, energy, mass, charge=1.0):
    """Hard-edge segments of a lattice made of drifts, quadrupoles and solenoids (others are refused)."""
    gamma = 1 + energy / mass
    p = mass * math.sqrt(gamma * gamma - 1)
    out = []
    for st in LatticeDocument(text).elements():
        if st.key == "drift":
            out.append((st.length, 0.0, 0.0))
        elif st.key == "quad":
            k = to_float(st.param(3)) * MEV_TM * charge / p
            out.append((st.length, k, -k))
        elif st.key == "solenoid":
            k = (to_float(st.param(3)) * MEV_TM * charge / (2 * p)) ** 2
            out.append((st.length, k, k))
        elif st.length > 0:
            raise ValueError(f"Line {st.line_no + 1}: '{st.keyword}' is not a hard-edge KV segment.")
    return out


def _rhs(kx, ky, perv, ex, ey):
    def f(_s, u):
        a, ap, b, bp = u
        sc = 2 * perv / (a + b)
        return [ap, -kx * a + sc + ex * ex / a ** 3, bp, -ky * b + sc + ey * ey / b ** 3]
    return f


def track(segments, perv, eps, start, points=20):
    """Envelopes through *segments* from ``start = (a, a', b, b')``; *eps* = (ε_x, ε_y) full emittances.
    Returns ``(s, u)`` with ``u[:, k]`` = (a, a', b, b') at s."""
    s_all, u_all = [0.0], [np.asarray(start, dtype=float)]
    s0, u = 0.0, np.asarray(start, dtype=float)
    for length, kx, ky in segments:
        if length <= 0:
            continue
        sol = solve_ivp(_rhs(kx, ky, perv, *eps), (s0, s0 + length), u, rtol=1e-11, atol=1e-15,
                        t_eval=np.linspace(s0, s0 + length, points + 1)[1:])
        s_all.extend(sol.t)
        u_all.extend(sol.y.T)
        s0, u = s0 + length, sol.y[:, -1]
    return np.array(s_all), np.array(u_all)


def matched(segments, perv, eps, guess):
    """Periodic solution (a, a', b, b') at the cell entry, from *guess* by least squares."""
    def resid(x):
        _s, u = track(segments, perv, eps, x, points=1)
        return (u[-1] - x) / np.array([guess[0], 1e-3, guess[2], 1e-3])
    sol = least_squares(resid, np.asarray(guess, dtype=float), xtol=1e-15, ftol=1e-15, gtol=1e-15)
    return sol.x
