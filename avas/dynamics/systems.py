"""The (A, B, C, D) of one period, for the centroid or for the second moments.

``x_{n+1} = A·x_n + B·u_n``, ``y_n = C·x_n + D·u_n``: x at the period entry, u the knobs of the period
(changes from their values in the model), y the readings of the period's monitors.  A is normalised
for acceleration (see :mod:`avas.dynamics.period`).  B and D come from central differences of the
model with each knob, with the effect of the knob on the entry state removed, so knobs upstream of
the period contribute nothing.
"""
import math

import numpy as np

from avas.dynamics.diagnostics import Diagnostics
from avas.dynamics.envelope import envelope_jacobian
from avas.dynamics.moments import PLANES, plane_indices, set_upper, upper
from avas.dynamics.period import Period
from avas.dynamics.response import _step, base_value, settings


def _inside(diagnostics, cell, kind):
    return Diagnostics([m for m in diagnostics.monitors if m.kind == kind and cell.z0 - 1e-9 <= m.z <= cell.z1 + 1e-9])


def orbit_system(model, period, knobs, diagnostics, planes="xy", **evaluate):
    """Centroid system of *period* (number or (z0, z1)): state (x, x', y, y') of *planes*, the period's
    position monitors.  Returns ``A, B, C, D``, ``state_labels``, ``knob_labels``, ``reading_labels``,
    ``period`` (the :class:`Period`)."""
    errors, beam = evaluate.pop("errors", None), evaluate.pop("beam", None)
    res = model.evaluate(errors=errors, beam=beam, **evaluate)
    cell = Period(res, *((period, None) if not isinstance(period, tuple) else (1, period)))
    idx = plane_indices(planes)
    norm = math.sqrt(cell.damping)
    a = cell.transfer()[np.ix_(idx, idx)]
    diag = _inside(diagnostics, cell, "position")
    c_full, reading_labels = diag.output_matrix(res, cell.i0, planes)
    keep = [k for k, (_m, p) in enumerate(diag.channels()) if p in planes]
    c = c_full[:, idx] if len(c_full) else np.zeros((0, len(idx)))
    m_raw = res.transfer(cell.i0, cell.i1)[np.ix_(idx, idx)]
    b = np.zeros((len(idx), len(knobs)))
    d = np.zeros((len(keep), len(knobs)))
    for j, knob in enumerate(knobs):
        h = _step(knob, base_value(model, knob))
        u0 = base_value(model, knob)
        outs = []
        for sign in (1, -1):
            r = model.evaluate(**settings([knob], [u0 + sign * h], errors, beam, **evaluate))
            outs.append((r.centroid[cell.i0, idx], r.centroid[cell.i1, idx],
                         diag.read(r)[keep] if keep else np.zeros(0)))
        d_in = (outs[0][0] - outs[1][0]) / (2 * h)
        d_out = (outs[0][1] - outs[1][1]) / (2 * h)
        b[:, j] = (d_out - m_raw @ d_in) * norm
        if keep:
            d[:, j] = (outs[0][2] - outs[1][2]) / (2 * h) - c @ d_in
    names = "x x' y y' z δ".split()
    return {"A": a, "B": b, "C": c, "D": d, "state_labels": [names[i] for i in idx],
            "knob_labels": [k.label for k in knobs], "reading_labels": reading_labels, "period": cell}


def envelope_system(model, period, knobs, diagnostics, current=None, **evaluate):
    """Second-moment system of *period* around its matched beam: state the scaled upper triangle of the
    matched planes' Σ (:func:`envelope_jacobian`), the period's size monitors (rms, m).  Returns
    ``A, B, C, D`` (D = 0), ``state_labels``, ``knob_labels``, ``reading_labels``, ``period``,
    ``jacobian`` (the :func:`envelope_jacobian` result)."""
    errors, beam = evaluate.pop("errors", None), evaluate.pop("beam", None)
    res = model.evaluate(errors=errors, beam=beam, **evaluate)
    cell = Period(res, *((period, None) if not isinstance(period, tuple) else (1, period)))
    jac = envelope_jacobian(cell, current=current)
    matched = jac["matched"]
    sigma, planes, current = matched["sigma"], matched["planes"], matched["current"]
    idx = plane_indices(planes)
    scale = jac["scale"]
    b = np.zeros((len(scale), len(knobs)))
    for j, knob in enumerate(knobs):
        u0 = base_value(model, knob)
        h = _step(knob, u0)
        outs = []
        for sign in (1, -1):
            r = model.evaluate(**settings([knob], [u0 + sign * h], errors, beam, **evaluate))
            outs.append(upper(Period(r, z=(cell.z0, cell.z1)).map(sigma, current), idx) / scale)
        b[:, j] = (outs[0] - outs[1]) / (2 * h)
    diag = _inside(diagnostics, cell, "size")
    channels = [(m, p) for m, p in diag.channels() if PLANES[p] in idx]
    points = [res.index(m.z, "right") for m, _p in channels]

    def readings(x):
        if not channels:
            return np.zeros(0)
        sig, _eff = res.propagate(set_upper(sigma, idx, x * scale), cell.i0, max(points), space_charge=True,
                                  current=current)
        return np.array([math.sqrt(max(sig[i - cell.i0][PLANES[p], PLANES[p]], 0.0)) for (_m, p), i in
                         zip(channels, points)])

    x0 = upper(sigma, idx) / scale
    c = np.zeros((len(channels), len(x0)))
    for k in range(len(x0)):
        step = np.zeros(len(x0))
        step[k] = 1e-6
        c[:, k] = (readings(x0 + step) - readings(x0 - step)) / 2e-6
    names = "x x' y y' z δ".split()
    return {"A": jac["matrix"], "B": b, "C": c, "D": np.zeros((len(channels), len(knobs))),
            "state_labels": [f"<{names[i]}·{names[k]}>" for i, k in jac["labels"]],
            "knob_labels": [k.label for k in knobs],
            "reading_labels": [f"{m.name}:s{p}" for m, p in channels], "period": cell, "jacobian": jac}
