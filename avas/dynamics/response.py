"""Response matrices: how the readings of :class:`~avas.dynamics.diagnostics.Diagnostics` change with knobs.

A knob is a lattice parameter (a steerer's Bx / By, a gradient, a field amplitude, a phase), an
element error (dx, dy, roll; the reference trajectory stays) or a beam value (current, energy).
Derivatives are central differences of :meth:`avas.model.LinearModel.evaluate`; for the centroid
against steerers and offsets the model is linear and they are exact.
"""
from dataclasses import dataclass

import numpy as np

from avas.data import lattice_edit
from avas.data.lattice_doc import to_float
from avas.model.model import ERROR_KEYS, ModelError, _address

DEFAULT_STEPS = {"Bx": 1e-4, "By": 1e-4, "dx": 1e-4, "dy": 1e-4, "roll": 1e-4}   # T (or V/m ×1e4), m, rad
ELECTRIC_STEP = 1e3                                                               # V/m


@dataclass
class Knob:
    """*kind* "lattice" (*param* of the element *target*), "error" (*param* in dx, dy, roll) or "beam"
    (*param* a beam value such as "current"; *target* unused).  *step*: difference step (default by
    parameter).  *lower* / *upper*: limits used by the feedback."""
    target: object
    param: str
    kind: str = "lattice"
    step: float = None
    lower: float = None
    upper: float = None

    @property
    def label(self):
        return f"beam.{self.param}" if self.kind == "beam" else f"{self.target}.{self.param}"


def steerers(model, planes="xy"):
    """Knobs for every steerer of the lattice: By (moves x) and / or Bx (moves y); electric steerers
    get a step in V/m.  Limits are ± the steerer's maximum (7th parameter) when it is set."""
    out = []
    for st in model.doc.statements:
        if not st.active or st.key != "steerer":
            continue
        target = st.name or st.line_no + 1
        electric = to_float(st.param(5)) == 1
        limit = abs(to_float(st.param(6))) or None
        for plane, param in (("x", "By"), ("y", "Bx")):
            if plane in planes:
                out.append(Knob(target, param, step=ELECTRIC_STEP if electric else None,
                                lower=-limit if limit else None, upper=limit))
    return out


def base_value(model, knob):
    """Value of *knob* in the model as it is (errors start at 0)."""
    if knob.kind == "error":
        return 0.0
    if knob.kind == "beam":
        return float(model.beam.get(knob.param) or 0.0)
    try:
        st = lattice_edit.resolve(model.doc, _address(knob.target))
        k = lattice_edit.param_index(st, knob.param)
    except lattice_edit.LatticeEditError as exc:
        raise ModelError(str(exc)) from None
    return to_float(st.param(k))


def settings(knobs, values, errors=None, beam=None, **evaluate):
    """Keyword arguments of :meth:`LinearModel.evaluate` with *knobs* set to *values* (on top of
    *errors* and *beam*, which are not changed)."""
    lat, err, bm = {}, {t: dict(e) for t, e in (errors or {}).items()}, dict(beam or {})
    for knob, value in zip(knobs, values):
        if knob.kind == "lattice":
            lat.setdefault(knob.target, {})[knob.param] = float(value)
        elif knob.kind == "error":
            if knob.param not in ERROR_KEYS:
                raise ModelError(f"Unknown error '{knob.param}'; use {', '.join(ERROR_KEYS)}.")
            err.setdefault(knob.target, {})[knob.param] = err.get(knob.target, {}).get(knob.param, 0.0) + float(value)
        elif knob.kind == "beam":
            bm[knob.param] = float(value)
        else:
            raise ModelError(f"Unknown knob kind '{knob.kind}'.")
    return dict(evaluate, knobs=lat or None, errors=err or None, beam=bm or None)


def _step(knob, value):
    if knob.step:
        return float(knob.step)
    if knob.param in DEFAULT_STEPS:
        return DEFAULT_STEPS[knob.param]
    return 1e-3 * abs(value) if value else 1e-3


@dataclass
class Response:
    """``matrix[channel, knob]`` = ∂reading/∂knob; ``y0`` readings and ``u0`` knob values of the base."""
    matrix: np.ndarray
    y0: np.ndarray
    u0: np.ndarray
    knobs: list
    labels: list
    base: object = None

    def svd(self):
        return np.linalg.svd(self.matrix, full_matrices=False)


def response(model, knobs, diagnostics, **evaluate):
    """:class:`Response` of *diagnostics* to *knobs* around ``model.evaluate(**evaluate)``.

    *evaluate* may hold space_charge, errors, centroid0, beam … of the operating point; the knobs
    are changed on top of them.
    """
    knobs = list(knobs)
    u0 = np.array([base_value(model, k) for k in knobs])
    errors, beam = evaluate.pop("errors", None), evaluate.pop("beam", None)
    base = model.evaluate(errors=errors, beam=beam, **evaluate)
    y0 = diagnostics.read(base)
    matrix = np.empty((len(y0), len(knobs)))
    for j, knob in enumerate(knobs):
        h = _step(knob, u0[j])
        ys = [diagnostics.read(model.evaluate(**settings([knob], [u0[j] + sign * h], errors, beam, **evaluate)))
              for sign in (1, -1)]
        matrix[:, j] = (ys[0] - ys[1]) / (2 * h)
    return Response(matrix, y0, u0, knobs, diagnostics.labels(), base)
