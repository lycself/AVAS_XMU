"""Periodic cells of the linear model: Floquet analysis, matched beams with space charge, phase advances.

A period is a ``lattice … lattice_end`` group of the lattice or any z range of a
:class:`avas.model.ModelResult`.  Accelerating cells are not exactly periodic; the
one-period matrix is normalised by √(βγ_out / βγ_in) (adiabatic damping taken out),
and "matched" means Σ_out · βγ_out/βγ_in = Σ_in.  Planes without a stable zero-current
motion (z without RF, for example) are not matched: their block is carried from the
beam at the period entry.
"""
import math

import numpy as np
from scipy.optimize import least_squares

from avas.data.lattice_doc import Group
from avas.dynamics.moments import J6, PLANES, eigen_emittances, mismatch, plane_indices, set_upper, upper
from avas.model.elements import _prefix_products

STABLE_TOL = 1e-3           # |eigenvalue| above 1 + this counts as unstable: the normalised matrix of an
                            # accelerating cell is symplectic only to about 5e-4 (examples/hwr010 sol + HWR)
COUPLED_TOL = 1e-9          # relative size of the x–y blocks above which x and y are matched together


def periods(doc):
    """``[{"index", "title", "line", "z0", "z1"}]`` of the ``lattice … lattice_end`` groups (index from 1)."""
    out = []

    def walk(group):
        for child in group.children:
            if isinstance(child, Group):
                if child.kind == "period":
                    sts = [s for s in child.statements() if s.active and s.is_element and s.z_start is not None]
                    if sts:
                        out.append({"index": len(out) + 1, "title": child.title, "line": child.line_no,
                                    "z0": min(s.z_start for s in sts), "z1": max(s.z_end for s in sts)})
                walk(child)

    walk(doc.root)
    return out


def floquet(m):
    """Floquet analysis of a (normalised) one-period matrix *m*.

    Returns ``eigenvalues``, ``max_modulus``, ``stable``, ``eigen_tunes_deg`` (phases of the
    eigenvalues in the upper half plane, valid with coupling), ``coupling`` (size of the off-diagonal
    plane blocks relative to *m*) and per plane ``mu_deg`` (0–180°, from the trace of the plane's block;
    meaningful without coupling), ``alpha``, ``beta`` of the matched beam and ``stable``.
    """
    values = np.linalg.eigvals(m)
    moduli = np.abs(values)
    planes = {}
    for p, i in PLANES.items():
        b = m[i:i + 2, i:i + 2]
        c = (b[0, 0] + b[1, 1]) / 2
        if abs(c) < 1 - 1e-12 and b[0, 1] != 0:
            mu = math.acos(c)
            s = math.copysign(math.sin(mu), b[0, 1])
            planes[p] = {"mu_deg": math.degrees(mu), "alpha": (b[0, 0] - b[1, 1]) / (2 * s), "beta": b[0, 1] / s,
                         "stable": True}
        else:
            planes[p] = {"mu_deg": math.nan, "alpha": math.nan, "beta": math.nan, "stable": False}
    blocks = np.kron(np.eye(3), np.ones((2, 2))).astype(bool)
    norm = np.linalg.norm(m)
    return {"matrix": m, "eigenvalues": values, "max_modulus": float(moduli.max()),
            "stable": bool(np.all(moduli <= 1 + STABLE_TOL)),
            "eigen_tunes_deg": sorted(math.degrees(math.atan2(v.imag, v.real)) for v in values if v.imag > 1e-12),
            "coupling": float(np.linalg.norm(m[~blocks]) / norm) if norm else 0.0, "planes": planes}


def matched_linear(m, emittances, planes, base):
    """Zero-current matched moments for the one-period matrix *m*: the block of *planes* is
    ``Σ ε_k · Re(v_k v_k†)`` over the eigenvectors (each scaled to its target eigen-emittance), the
    other planes are carried from *base*.  None when the motion of *planes* is not stable."""
    idx = plane_indices(planes)
    sub = m[np.ix_(idx, idx)]
    values, vectors = np.linalg.eig(sub)
    if np.any(np.abs(np.abs(values) - 1) > STABLE_TOL) or np.any(np.abs(values.imag) < 1e-12):
        return None
    j = J6[np.ix_(idx, idx)]
    block = np.zeros((len(idx), len(idx)))
    free = list(planes)
    for k in np.argsort(-values.imag)[:len(planes)]:
        v = vectors[:, k]
        part = np.real(np.outer(v, v.conj()))
        eps = float(np.max(np.abs(np.linalg.eigvals(j @ part).imag)))
        weight = [np.sum(np.abs(v[2 * n:2 * n + 2]) ** 2) for n, p in enumerate(planes)]
        plane = max((p for p in free), key=lambda p: weight[planes.index(p)])
        free.remove(plane)
        block += part * (emittances[plane] / eps)
    sigma = np.array(base, dtype=float)
    others = [i for i in range(6) if i not in idx]
    sigma[np.ix_(idx, others)] = 0.0
    sigma[np.ix_(others, idx)] = 0.0
    sigma[np.ix_(idx, idx)] = block
    return sigma


class Period:
    """One period of an evaluated model (:meth:`avas.model.LinearModel.evaluate`).

    *period*: number of the ``lattice … lattice_end`` group (from 1); or *z* = (z0, z1) in m.
    """

    def __init__(self, result, period=1, z=None):
        if z is None:
            found = periods(result.doc)
            if not found:
                raise ValueError("The lattice has no 'lattice … lattice_end' period; give z=(z0, z1).")
            if not 1 <= int(period) <= len(found):
                raise ValueError(f"Period {period} does not exist; the lattice has {len(found)}.")
            z = (found[int(period) - 1]["z0"], found[int(period) - 1]["z1"])
        self.result = result
        self.z0, self.z1 = float(z[0]), float(z[1])
        self.i0, self.i1 = result.index(self.z0, "right"), result.index(self.z1, "right")
        if self.i1 <= self.i0:
            raise ValueError(f"No steps between z = {self.z0} and {self.z1} m.")
        bg = result.beta_gamma
        self.damping = float(bg[self.i1] / bg[self.i0])        # Σ_out · damping is compared with Σ_in

    @classmethod
    def of(cls, model, period=1, z=None, **evaluate):
        """Period of ``model.evaluate(**evaluate)``."""
        return cls(model.evaluate(**evaluate), period, z)

    @property
    def entry_sigma(self):
        """The beam of the result at the period entry."""
        return self.result.sigma[self.i0]

    def _current(self, current):
        return float(self.result.beam.get("current") or 0.0) if current is None else float(current)

    def transfer(self, sigma=None, current=None):
        """Normalised one-period matrix; with *sigma* the depressed one of that beam (space charge of
        *current* [mA], default the beam's)."""
        if sigma is None:
            m = self.result.transfer(self.i0, self.i1)
        else:
            _s, eff = self.result.propagate(sigma, self.i0, self.i1, space_charge=True,
                                            current=self._current(current))
            m = _prefix_products(eff)[-1]
        return m * math.sqrt(self.damping)

    def map(self, sigma, current=None):
        """Normalised moments after one period with space charge of *current* [mA]."""
        out, _eff = self.result.propagate(sigma, self.i0, self.i1, space_charge=True, current=self._current(current))
        return out[-1] * self.damping

    def zero_current(self):
        """:func:`floquet` of the zero-current period."""
        return floquet(self.transfer())

    def _planes(self, zc, planes):
        if planes is not None:
            return planes
        planes = "".join(p for p in "xyz" if zc["planes"][p]["stable"])
        m = zc["matrix"]
        if np.linalg.norm(m[0:2, 2:4]) + np.linalg.norm(m[2:4, 0:2]) > COUPLED_TOL * np.linalg.norm(m):
            planes = ("xy" if self._stable_block(m, "xy") else "") + planes.replace("x", "").replace("y", "")
        return planes

    @staticmethod
    def _stable_block(m, planes):
        idx = plane_indices(planes)
        values = np.linalg.eigvals(m[np.ix_(idx, idx)])
        return bool(np.all(np.abs(np.abs(values) - 1) < STABLE_TOL) and np.all(np.abs(values.imag) > 1e-12))

    def matched(self, current=None, emittances=None, planes=None):
        """Matched beam of the period.

        *current*: mA (default the beam's; 0 = zero current).  *emittances*: rms eigen-emittances
        ``{plane: m·rad}`` (default those of the beam at the period entry).  *planes*: planes to match
        (default the stable ones; x and y together when coupled).

        Returns ``sigma`` (6×6 at the entry, geometric), ``planes``, ``emittances``, ``current``,
        ``converged``, ``residual``, ``zero_current`` and ``depressed`` (:func:`floquet` of the
        period without and with this beam's space charge), ``tune_depression`` (σ/σ0 per plane) and
        ``mismatch`` of the beam at the entry against the matched one.
        """
        zc = self.zero_current()
        planes = self._planes(zc, planes)
        current = self._current(current)
        base = self.entry_sigma
        if not planes:
            raise ValueError("No plane of this period has a stable motion: nothing to match.")
        targets = dict(eigen_emittances(base, planes), **(emittances or {}))
        sigma = matched_linear(zc["matrix"], targets, planes, base)
        if sigma is None:
            raise ValueError(f"The zero-current motion of planes {planes} is not stable.")
        idx = plane_indices(planes)
        residual, converged = 0.0, True
        if current > 0:
            scale = np.sqrt(np.abs(np.outer(np.diag(sigma), np.diag(sigma))))[np.ix_(idx, idx)]
            scale = scale[np.triu_indices(len(idx))]

            def resid(x):
                trial = set_upper(sigma, idx, x * scale)
                diff = (upper(self.map(trial, current), idx) - x * scale) / scale
                eps = eigen_emittances(trial, planes)
                return np.concatenate([diff, [(eps.get(p, 0.0) - targets[p]) / targets[p] for p in planes]])

            sol = least_squares(resid, upper(sigma, idx) / scale, method="lm", xtol=1e-14, ftol=1e-14)
            sigma = set_upper(sigma, idx, sol.x * scale)
            residual = float(np.max(np.abs(resid(sol.x))))
            converged = bool(residual < 1e-6)
        depressed = floquet(self.transfer(sigma, current)) if current > 0 else zc
        out = {"sigma": sigma, "planes": planes, "emittances": targets, "current": current,
               "converged": converged, "residual": residual, "zero_current": zc, "depressed": depressed}
        out["tune_depression"] = {p: depressed["planes"][p]["mu_deg"] / zc["planes"][p]["mu_deg"] for p in planes}
        out["mismatch"] = {p: mismatch(base, sigma, p) for p in planes}
        return out


def period_table(result):
    """Every ``lattice … lattice_end`` period along *result*: zero-current and depressed (this beam's,
    from the effective matrices) phase advances per plane and the zero-current matched Twiss parameters.
    For accelerating lines, where no two periods are alike, this is the local picture period by period."""
    rows = []
    for p in periods(result.doc):
        cell = Period(result, z=(p["z0"], p["z1"]))
        zc = cell.zero_current()
        eff = _prefix_products(result.effective[cell.i0:cell.i1])[-1] * math.sqrt(cell.damping)
        dep = floquet(eff)
        row = dict(p, energy=float(result.energy[cell.i0]), stable=zc["stable"])
        for plane in PLANES:
            row[f"mu0_{plane}"] = zc["planes"][plane]["mu_deg"]
            row[f"mu_{plane}"] = dep["planes"][plane]["mu_deg"]
            row[f"beta_{plane}"] = zc["planes"][plane]["beta"]
            row[f"alpha_{plane}"] = zc["planes"][plane]["alpha"]
        rows.append(row)
    return rows
