"""Helpers on 6×6 second-moment matrices (x, x', y, y', z, δ): symplectic form, eigen-emittances,
Twiss parameters, mismatch, and the flat vector of a symmetric block used by the solvers."""
import math

import numpy as np

PLANES = {"x": 0, "y": 2, "z": 4}
J6 = np.kron(np.eye(3), np.array([[0.0, 1.0], [-1.0, 0.0]]))


def plane_indices(planes):
    """Coordinate indices of *planes* (``"xy"`` → [0, 1, 2, 3])."""
    return [i for p in planes for i in (PLANES[p], PLANES[p] + 1)]


def plane_emittance(sigma, plane):
    i = PLANES[plane]
    return math.sqrt(max(sigma[i, i] * sigma[i + 1, i + 1] - sigma[i, i + 1] ** 2, 0.0))


def eigen_emittances(sigma, planes="xyz"):
    """Eigen-emittances of the block of *planes*, sorted like the planes they are closest to.

    They are the moduli of the eigenvalues ±iε of J·Σ and stay constant under any linear symplectic
    map, coupled or not.  Each is assigned to the plane where its eigenvector lives most.
    """
    idx = plane_indices(planes)
    sub = sigma[np.ix_(idx, idx)]
    j = J6[np.ix_(idx, idx)]
    values, vectors = np.linalg.eig(j @ sub)
    order = np.argsort(-values.imag)[:len(planes)]       # the +iε of each pair
    out = {}
    for k in order:
        weight = [np.sum(np.abs(vectors[2 * n:2 * n + 2, k]) ** 2) for n in range(len(planes))]
        plane = planes[int(np.argmax(weight))]
        if plane in out:                                  # strong coupling: fill the planes in order
            plane = next(p for p in planes if p not in out)
        out[plane] = float(abs(values[k].imag))
    return out


def twiss(sigma, plane):
    """``(alpha, beta)`` of one plane of *sigma* (beta in m/rad)."""
    i = PLANES[plane]
    e = plane_emittance(sigma, plane)
    if not e > 0:
        return math.nan, math.nan
    return -sigma[i, i + 1] / e, sigma[i, i] / e


def mismatch(sigma, reference, plane):
    """Mismatch factor of *sigma* against *reference* in one plane (0 = matched; 0.1 = the beam
    fills about 10 % more phase space than a matched one): ``sqrt((R + sqrt(R² − 4))/2) − 1`` with
    ``R = β·γ_ref + β_ref·γ − 2·α·α_ref``."""
    a, b = twiss(sigma, plane)
    ar, br = twiss(reference, plane)
    if not (b > 0 and br > 0):
        return math.nan
    g, gr = (1 + a * a) / b, (1 + ar * ar) / br
    r = b * gr + br * g - 2 * a * ar
    return math.sqrt(max((r + math.sqrt(max(r * r - 4, 0.0))) / 2, 1.0)) - 1


def upper(sigma, idx):
    """Upper triangle (with diagonal) of the block *idx* of *sigma* as a flat vector."""
    sub = sigma[np.ix_(idx, idx)]
    return sub[np.triu_indices(len(idx))]


def set_upper(sigma, idx, values):
    """Copy of *sigma* with the block *idx* rebuilt from its upper triangle *values*."""
    n = len(idx)
    sub = np.zeros((n, n))
    sub[np.triu_indices(n)] = values
    sub = sub + sub.T - np.diag(np.diag(sub))
    out = np.array(sigma, dtype=float)
    out[np.ix_(idx, idx)] = sub
    return out
