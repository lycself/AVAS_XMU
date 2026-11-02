"""Linear model against engine, seed ensembles and engine response matrices."""
import numpy as np

from avas.dynamics.response import Knob, Response
from avas.research.variant import Variant

QUANTITIES = ("rms_x", "rms_y", "rms_z", "cx", "cy", "energy")


def common_z(runs, points=400):
    """A z grid inside every run."""
    end = min(float(r.z[-1]) for r in runs if r is not None and r.ok)
    start = max(float(r.z[0]) for r in runs if r is not None and r.ok)
    return np.linspace(start, end, points)


def on_grid(run, quantity, z):
    """*quantity* of *run* interpolated on *z* (NaN rows of the run left out)."""
    values = np.asarray(run.curve(quantity), dtype=float)
    keep = np.isfinite(values)
    if not keep.any():
        return np.full(len(z), np.nan)
    return np.interp(z, run.z[keep], values[keep])


def compare(study, variant=None, points=400, quantities=QUANTITIES, **engine_kwargs):
    """Linear model and engine for one variant on a common z grid.

    Returns ``z``, ``linear`` / ``engine`` (the runs), per quantity ``{"linear", "engine", "diff"}``
    curves (diff = linear − engine) and ``summary``: rows with the final and the largest values of both
    and their relative differences (sizes in mm, energy in MeV).
    """
    variant = variant or Variant()
    lin = study.linear(variant)
    eng = study.engine(variant, **engine_kwargs)
    if not eng.ok:
        raise RuntimeError(f"The engine run failed: {eng.record.get('error')}")
    z = common_z([lin, eng], points)
    out = {"z": z, "linear": lin, "engine": eng, "summary": []}
    for q in quantities:
        a, b = on_grid(lin, q, z), on_grid(eng, q, z)
        out[q] = {"linear": a, "engine": b, "diff": a - b}
        scale = 1.0 if q == "energy" else 1e3
        for what, fa, fb in (("end", a[-1], b[-1]), ("max", np.nanmax(np.abs(a)), np.nanmax(np.abs(b)))):
            out["summary"].append({"quantity": q, "at": what, "linear": fa * scale, "engine": fb * scale,
                                   "diff_rel": (fa - fb) / fb if fb else np.nan})
    return out


def ensemble(study, variant=None, seeds=(1, 2, 3, 4, 5), points=400, quantities=QUANTITIES, **batch_kwargs):
    """Engine runs of *variant* with each seed: mean and standard deviation of curves and metrics."""
    variant = variant or Variant()
    runs = study.run_many([variant.set("seed", s) for s in seeds], "engine", **batch_kwargs)
    good = [r for r in runs if r is not None and r.ok]
    if not good:
        raise RuntimeError("No engine run of the ensemble finished.")
    z = common_z(good, points)
    out = {"z": z, "runs": runs, "curves": {}, "metrics": {}}
    for q in quantities:
        stack = np.array([on_grid(r, q, z) for r in good])
        out["curves"][q] = {"mean": np.nanmean(stack, axis=0), "std": np.nanstd(stack, axis=0, ddof=1 if len(good) > 1 else 0)}
    keys = sorted({k for r in good for k, v in r.metrics.items() if isinstance(v, (int, float))})
    for k in keys:
        vals = np.array([r.metrics.get(k) for r in good if isinstance(r.metrics.get(k), (int, float))], dtype=float)
        out["metrics"][k] = {"mean": float(np.mean(vals)), "std": float(np.std(vals, ddof=1)) if len(vals) > 1 else 0.0}
    return out


def _knob_variant(base, knob, value):
    if knob.kind == "lattice":
        return base.set((("lattice", knob.target, knob.param)), value)
    if knob.kind == "beam":
        return base.set(f"beam.{knob.param}", value)
    raise ValueError(f"Knob {knob.label}: the engine has no deterministic {knob.kind} knobs (use the linear model).")


def engine_response(study, knobs, diagnostics, variant=None, central=False, **batch_kwargs):
    """:class:`~avas.dynamics.Response` of *diagnostics* to *knobs* from engine runs (the study's fixed seed
    makes the runs comparable; one-sided steps unless *central*).  Knob steps as in
    :mod:`avas.dynamics.response`."""
    from avas.dynamics.response import _step, base_value
    variant = variant or Variant()
    knobs = [k if isinstance(k, Knob) else Knob(*k) for k in knobs]
    u0 = np.array([base_value(study.model, k) for k in knobs])
    steps = [_step(k, u) for k, u in zip(knobs, u0)]
    plus = [_knob_variant(variant, k, u + h) for k, u, h in zip(knobs, u0, steps)]
    minus = [_knob_variant(variant, k, u - h) for k, u, h in zip(knobs, u0, steps)] if central else []
    runs = study.run_many([variant] + plus + minus, "engine", **batch_kwargs)
    if any(r is None or not r.ok for r in runs):
        bad = [r.record.get("error") for r in runs if r is not None and not r.ok]
        raise RuntimeError(f"Engine runs failed: {bad}")
    y0 = diagnostics.read(runs[0])
    matrix = np.empty((len(y0), len(knobs)))
    for j, h in enumerate(steps):
        up = diagnostics.read(runs[1 + j])
        down = diagnostics.read(runs[1 + len(knobs) + j]) if central else y0
        matrix[:, j] = (up - down) / (2 * h if central else h)
    return Response(matrix, y0, u0, knobs, diagnostics.labels(), runs[0])
