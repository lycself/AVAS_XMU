"""Grid scans over any number of addresses and Latin-hypercube samples (training data for surrogates)."""
import itertools
import math

import numpy as np

from avas.research.runs import COMMON_METRICS
from avas.research.variant import Variant


def grid_variants(axes, base=None):
    """Variants of every combination of ``axes = {address: values}`` (C order: the last axis fastest)."""
    base = base or Variant()
    names = list(axes)
    out = []
    for combo in itertools.product(*(list(axes[n]) for n in names)):
        v = base
        for name, value in zip(names, combo):
            v = v.set(name, value)
        out.append(v)
    return out


def latin_hypercube(ranges, n, seed=0, base=None):
    """*n* variants sampled in ``ranges = {address: (low, high)}`` by a Latin hypercube."""
    from scipy.stats import qmc
    names = list(ranges)
    sample = qmc.LatinHypercube(d=len(names), seed=seed).random(n)
    lows = np.array([ranges[k][0] for k in names], dtype=float)
    highs = np.array([ranges[k][1] for k in names], dtype=float)
    values = qmc.scale(sample, lows, highs)
    out = []
    for row in values:
        v = base or Variant()
        for name, value in zip(names, row):
            v = v.set(name, float(value))
        out.append(v)
    return out


def _value(run, metric):
    if callable(metric):
        return metric(run)
    if run is None or not run.ok:
        return math.nan
    if metric == "transmission":
        return math.nan if run.transmission is None else run.transmission
    v = run.metrics.get(metric)
    return math.nan if v is None else float(v)


def grid(study, axes, fidelity="linear", metrics=COMMON_METRICS, base=None, **run_kwargs):
    """Run every combination of *axes* and collect *metrics* (keys of ``run.metrics``, "transmission",
    or functions of a run) as arrays shaped like the axes.

    Returns ``{"axes": [(address, values)], "runs": [...], metric: array, ...}``.
    """
    names = list(axes)
    shape = tuple(len(list(axes[n])) for n in names)
    variants = grid_variants(axes, base)
    runs = study.run_many(variants, fidelity, **run_kwargs)
    out = {"axes": [(n, np.asarray(list(axes[n]))) for n in names], "runs": runs}
    for m in metrics:
        key = m if isinstance(m, str) else getattr(m, "__name__", "metric")
        out[key] = np.array([_value(r, m) for r in runs], dtype=float).reshape(shape)
    return out
