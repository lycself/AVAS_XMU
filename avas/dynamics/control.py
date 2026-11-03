"""Controllability, observability and closed loops of δX_{n+1} = A·δX_n + B·u_n, y_n = C·δX_n + η.

n counts periods along the line (A a one-period matrix, B the period's knobs) or pulses (A the
pulse-to-pulse map, often the identity for static errors).  For a single pass through the line
without dynamics in n, :func:`static` analyses the response matrix y = R·u instead.

Mode classes follow the PBH test on each eigenvalue λ of A:
"stable" (|λ| < 1, or ≤ 1 for a neutral mode), "stabilizable" (unstable but rank[A − λI, B] = n:
feedback can move it), "uncontrollable" (unstable and invisible to B).
"""
import math

import numpy as np
from scipy.linalg import solve_discrete_are

RANK_TOL = 1e-9             # singular values below this × the largest count as zero


def _rank(m, tol=RANK_TOL):
    s = np.linalg.svd(m, compute_uv=False)
    return int(np.sum(s > tol * s[0])) if s.size and s[0] > 0 else 0


def controllability(a, b, horizon=None):
    """Rank of [B, AB, …, A^{n−1}B], the Gramian W = Σ_{k<N} A^k B Bᵀ A^kᵀ over *horizon* N periods
    (default n) and its spectrum: ``singular_values`` (√ of the Gramian's eigenvalues, the reachable
    size per unit knob along each direction), ``directions`` (columns, strongest first), ``condition``."""
    a, b = np.atleast_2d(a), np.atleast_2d(b)
    n = a.shape[0]
    horizon = horizon or n
    blocks, ak = [], np.eye(n)
    for _ in range(max(n, horizon)):
        blocks.append(ak @ b)
        ak = a @ ak
    gram = sum(blk @ blk.T for blk in blocks[:horizon])
    values, vectors = np.linalg.eigh(gram)
    order = np.argsort(values)[::-1]
    sv = np.sqrt(np.maximum(values[order], 0.0))
    return {"rank": _rank(np.hstack(blocks[:n])), "n": n, "gramian": gram, "singular_values": sv,
            "directions": vectors[:, order], "condition": float(sv[0] / sv[-1]) if sv[-1] > 0 else math.inf}


def observability(a, c, horizon=None, noise=None):
    """Rank of [C; CA; …], the Gramian W = Σ_{k<N} A^kᵀ Cᵀ R⁻¹ C A^k (R = diag(*noise*²), default 1)
    and the Cramér–Rao bound of the state reconstructed from N periods of readings: ``bound`` = W⁻¹
    (covariance) and ``state_error`` = √diag(W⁻¹) (inf where the state is not observable)."""
    a, c = np.atleast_2d(a), np.atleast_2d(c)
    n = a.shape[0]
    horizon = horizon or n
    w = np.ones(c.shape[0]) if noise is None else 1.0 / np.asarray(noise, dtype=float) ** 2
    rows, ak = [], np.eye(n)
    for _ in range(max(n, horizon)):
        rows.append(c @ ak)
        ak = a @ ak
    gram = sum(r.T @ (w[:, None] * r) for r in rows[:horizon])
    rank = _rank(np.vstack(rows[:n]))
    if _rank(gram) == n:
        bound = np.linalg.inv(gram)
        err = np.sqrt(np.maximum(np.diag(bound), 0.0))
    else:
        bound = np.full((n, n), math.inf)
        err = np.full(n, math.inf)
    values = np.linalg.eigvalsh(gram)
    return {"rank": rank, "n": n, "gramian": gram, "bound": bound, "state_error": err,
            "eigenvalues": np.sort(values)[::-1]}


def modes(a, b=None, c=None, tol=1e-7, stable_tol=1e-3):
    """Eigenmodes of *a* with the PBH tests: ``[{"eigenvalue", "modulus", "tune_deg", "controllable",
    "observable", "class"}]``; class "stable" | "stabilizable" | "uncontrollable" (see module doc).
    *tol*: rank tolerance; *stable_tol*: |λ| up to 1 + this is stable (normalised accelerating cells are
    symplectic only to about 5e-4, see :data:`avas.dynamics.period.STABLE_TOL`)."""
    a = np.atleast_2d(a)
    n = a.shape[0]
    out = []
    for lam in np.linalg.eigvals(a):
        shifted = a - lam * np.eye(n)
        ctrl = None if b is None else _rank(np.hstack([shifted, np.atleast_2d(b).astype(complex)]), tol) == n
        obs = None if c is None else _rank(np.vstack([shifted, np.atleast_2d(c).astype(complex)]), tol) == n
        modulus = float(abs(lam))
        if modulus <= 1 + stable_tol:
            cls = "stable"
        else:
            cls = "stabilizable" if ctrl else "uncontrollable"
        out.append({"eigenvalue": complex(lam), "modulus": modulus,
                    "tune_deg": math.degrees(math.atan2(lam.imag, lam.real)),
                    "controllable": ctrl, "observable": obs, "class": cls})
    return out


def closed_loop(a, b, k):
    """Eigenvalues of A + B·K (state feedback u = K·x)."""
    return np.linalg.eigvals(np.atleast_2d(a) + np.atleast_2d(b) @ np.atleast_2d(k))


def _companion(blocks):
    """Block companion matrix of x_{n+1} = Σ_j blocks[j]·x_{n−j}."""
    n, d = blocks[0].shape[0], len(blocks)
    m = np.zeros((n * d, n * d))
    m[:n] = np.hstack(blocks)
    m[n:, :-n] = np.eye(n * (d - 1))
    return m


def delayed_loop(a, b, k, delay=1):
    """Eigenvalues with feedback on a reading *delay* steps old: x_{n+1} = A·x_n + B·K·x_{n−delay}."""
    a, bk = np.atleast_2d(a), np.atleast_2d(b) @ np.atleast_2d(k)
    if delay == 0:
        return np.linalg.eigvals(a + bk)
    blocks = [a] + [np.zeros_like(a)] * (delay - 1) + [bk]
    return np.linalg.eigvals(_companion(blocks))


def difference_loop(a, b, k, r=None):
    """Eigenvalues with the difference feedback u_n = K·(x_n − x_{n−1}) + R·u_{n−1}, which needs no
    operating point (it vanishes on any fixed point): state (x_n, x_{n−1}, u_{n−1})."""
    a, b, k = np.atleast_2d(a), np.atleast_2d(b), np.atleast_2d(k)
    n, m = a.shape[0], b.shape[1]
    r = np.zeros((m, m)) if r is None else np.atleast_2d(r)
    big = np.zeros((2 * n + m, 2 * n + m))
    big[:n, :n] = a + b @ k
    big[:n, n:2 * n] = -b @ k
    big[:n, 2 * n:] = b @ r
    big[n:2 * n, :n] = np.eye(n)
    big[2 * n:, :n] = k
    big[2 * n:, n:2 * n] = -k
    big[2 * n:, 2 * n:] = r
    return np.linalg.eigvals(big)


def lqr(a, b, q=None, r=None):
    """Discrete LQR gain K (u = K·x) minimising Σ xᵀQx + uᵀRu; Q, R default to identities."""
    a, b = np.atleast_2d(a), np.atleast_2d(b)
    q = np.eye(a.shape[0]) if q is None else q
    r = np.eye(b.shape[1]) if r is None else r
    p = solve_discrete_are(a, b, q, r)
    return -np.linalg.solve(r + b.T @ p @ b, b.T @ p @ a)


def static(matrix, noise=None, cut=1e-6):
    """Single-pass correction y = R·u: singular values, rank, condition, the reading patterns no knob
    can correct (``uncorrectable``: left singular vectors beyond the rank) and the knob combinations no
    reading sees (``invisible``), and the knob error caused by reading noise through the pseudo-inverse
    (``knob_noise``: rms per knob for readings of rms *noise*)."""
    r = np.atleast_2d(matrix)
    u, s, vt = np.linalg.svd(r, full_matrices=True)
    rank = int(np.sum(s > cut * s[0])) if s.size and s[0] > 0 else 0
    pinv = (vt[:rank].T / s[:rank]) @ u[:, :rank].T
    noise = np.ones(r.shape[0]) if noise is None else np.broadcast_to(np.asarray(noise, dtype=float), (r.shape[0],))
    knob_noise = np.sqrt(np.sum((pinv * noise[None, :]) ** 2, axis=1))
    return {"singular_values": s, "rank": rank, "condition": float(s[0] / s[rank - 1]) if rank else math.inf,
            "pinv": pinv, "uncorrectable": u[:, rank:], "invisible": vt[rank:].T, "knob_noise": knob_noise}


def integral_loop_modulus(gain, delay=0):
    """Largest |root| of z^{d+1} − z^d + g = 0: each corrected mode of the integral feedback
    u_{k+1} = u_k − g·R⁺·y_{k−d} on a static plant (stable below 1)."""
    coeffs = np.zeros(delay + 2)
    coeffs[0], coeffs[1] = 1.0, -1.0
    coeffs[-1] += gain
    return float(np.max(np.abs(np.roots(coeffs))))
