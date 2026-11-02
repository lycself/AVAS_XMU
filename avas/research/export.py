"""Runs as arrays for learning: inputs (the variants' values), metrics and curves on a common z grid.

Writes ``<path>.npz`` (numpy) and ``<path>.csv`` (one row per run: inputs and metrics); no
dependency beyond numpy.  The npz holds ``inputs`` (runs × input names), ``input_names``,
``metrics`` / ``metric_names``, ``z`` and one ``runs × points`` array per curve, ``readings`` with
``reading_names`` when *diagnostics* is given, ``fidelity`` and ``ok``.
"""
import csv
import math

import numpy as np

from avas.research.compare import QUANTITIES, common_z, on_grid


def export(path, runs, points=200, quantities=QUANTITIES, diagnostics=None, metrics=None):
    """Write *runs* (both fidelities may be mixed) to ``path.npz`` and ``path.csv``; returns the arrays."""
    runs = [r for r in runs if r is not None]
    good = [r for r in runs if r.ok]
    names = sorted({k for r in runs for k in r.variant.flat()})
    inputs = np.array([[_num(r.variant.flat().get(k)) for k in names] for r in runs], dtype=float).reshape(len(runs), -1)
    metric_names = list(metrics) if metrics else sorted({k for r in good for k, v in r.metrics.items()
                                                         if isinstance(v, (int, float))})
    values = np.array([[_num(r.metrics.get(k)) for k in metric_names] for r in runs], dtype=float).reshape(len(runs), -1)
    z = common_z(good, points) if good else np.zeros(0)
    arrays = {"inputs": inputs, "input_names": np.array(names, dtype=object), "metrics": values,
              "metric_names": np.array(metric_names, dtype=object), "z": z,
              "fidelity": np.array([r.fidelity for r in runs], dtype=object), "ok": np.array([r.ok for r in runs])}
    for q in quantities:
        arrays[q] = np.array([on_grid(r, q, z) if r.ok else np.full(len(z), np.nan) for r in runs]).reshape(len(runs), -1)
    if diagnostics is not None:
        arrays["readings"] = np.array([diagnostics.read(r) if r.ok else np.full(len(diagnostics.labels()), np.nan)
                                       for r in runs])
        arrays["reading_names"] = np.array(diagnostics.labels(), dtype=object)
    np.savez(f"{path}.npz", **arrays)
    with open(f"{path}.csv", "w", newline="", encoding="utf-8") as fh:
        writer = csv.writer(fh)
        writer.writerow(["fidelity", "ok", "label"] + names + metric_names)
        for r, row_in, row_m in zip(runs, inputs, values):
            writer.writerow([r.fidelity, r.ok, r.variant.label] + _cells(row_in) + _cells(row_m))
    return arrays


def _num(v):
    try:
        f = float(v)
    except (TypeError, ValueError):
        return math.nan
    return f


def _cells(row):
    return ["" if not np.isfinite(v) else f"{v:.10g}" for v in row]
