"""Linearised envelope map of a period: δΣ_{n+1} = A·δΣ_n around the matched beam.

The map Σ → Σ' over one period is nonlinear with space charge.  Its Jacobian at the matched
beam (finite differences on the upper triangle of the matched planes' block) is the A of the
control problem δX_{n+1} = A·δX_n + B·u_n for the second moments.  Eigenvalues outside the unit
circle mean an envelope instability (e.g. the 90° stop band of a FODO channel with strong space
charge); three of them are 1 for the emittances, which the map keeps.  At zero current the
eigenvalues are the products λ_i·λ_j of the transfer matrix's.
"""
import math

import numpy as np

from avas.dynamics.moments import plane_indices, set_upper, upper

ENVELOPE_TOL = 1e-4         # |eigenvalue| above 1 + this counts as an envelope instability


def envelope_jacobian(period, matched=None, current=None, step=1e-6):
    """Jacobian of :meth:`Period.map` at the matched beam (computed with :meth:`Period.matched` unless
    given), in the variables Σ_ij / √(Σ_ii·Σ_jj) of the matched planes.

    Returns ``matrix``, ``eigenvalues``, ``max_modulus``, ``stable``, ``mode_tunes_deg`` (phases of the
    eigenvalues in the upper half plane: the envelope oscillation modes per period), ``labels`` (Σ index
    pairs of the variables), ``scale`` and the ``matched`` result used.
    """
    matched = matched or period.matched(current=current)
    sigma, planes, current = matched["sigma"], matched["planes"], matched["current"]
    idx = plane_indices(planes)
    scale = np.sqrt(np.abs(np.outer(np.diag(sigma), np.diag(sigma))))[np.ix_(idx, idx)]
    scale = scale[np.triu_indices(len(idx))]
    x0 = upper(sigma, idx) / scale

    def f(x):
        return upper(period.map(set_upper(sigma, idx, x * scale), current), idx) / scale

    n = len(x0)
    jac = np.empty((n, n))
    for k in range(n):
        h = np.zeros(n)
        h[k] = step
        jac[:, k] = (f(x0 + h) - f(x0 - h)) / (2 * step)
    values = np.linalg.eigvals(jac)
    rows, cols = np.triu_indices(len(idx))
    return {"matrix": jac, "eigenvalues": values, "max_modulus": float(np.abs(values).max()),
            "stable": bool(np.all(np.abs(values) <= 1 + ENVELOPE_TOL)),
            "mode_tunes_deg": sorted(math.degrees(math.atan2(v.imag, v.real)) for v in values if v.imag > 1e-9),
            "labels": [(idx[i], idx[j]) for i, j in zip(rows, cols)], "scale": scale, "matched": matched}
