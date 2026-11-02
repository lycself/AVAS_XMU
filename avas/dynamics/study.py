"""A study file (``study.json``): monitors, knobs and errors of an analysis, kept apart from the lattice.

    {
      "monitors": [{"name": "BPM1", "z": 0.6, "kind": "position", "planes": "xy", "resolution": 2e-5}],
      "lattice_monitors": {"resolution": 2e-5, "size_resolution": 5e-5},
      "knobs": [{"target": "Q3", "param": "G", "step": 0.01}],
      "lattice_steerers": "xy",
      "errors": {"Q3": {"dx": 1e-4}}
    }

``lattice_monitors`` adds the lattice's ``diag_position`` / ``diag_size`` lines, ``lattice_steerers``
a knob for each steerer (see :func:`avas.dynamics.response.steerers`); every entry is optional.
Names in the file refer to the lattice, so a measured data set only has to use the same monitor names.
"""
import json
from dataclasses import asdict

from avas.dynamics.diagnostics import Diagnostics, Monitor
from avas.dynamics.response import Knob, steerers


def load_study(spec, model=None):
    """``{"diagnostics", "knobs", "errors"}`` from a path or a dict; *model* is needed for the
    ``lattice_*`` entries."""
    if not isinstance(spec, dict):
        with open(spec, encoding="utf-8") as fh:
            spec = json.load(fh)
    unknown = set(spec) - {"monitors", "lattice_monitors", "knobs", "lattice_steerers", "errors"}
    if unknown:
        raise ValueError(f"Unknown study entries: {sorted(unknown)}.")
    if model is None and ("lattice_monitors" in spec or "lattice_steerers" in spec):
        raise ValueError("The study takes monitors or steerers from the lattice: pass the model.")
    monitors = [Monitor(**m) for m in spec.get("monitors", [])]
    if "lattice_monitors" in spec:
        monitors += Diagnostics.from_lattice(model, **spec["lattice_monitors"]).monitors
    knobs = [Knob(**k) for k in spec.get("knobs", [])]
    if "lattice_steerers" in spec:
        knobs += steerers(model, spec["lattice_steerers"])
    return {"diagnostics": Diagnostics(monitors), "knobs": knobs, "errors": dict(spec.get("errors", {}))}


def save_study(path, diagnostics=None, knobs=(), errors=None):
    """Write monitors, knobs and errors as an explicit study file (no ``lattice_*`` entries)."""
    spec = {"monitors": [asdict(m) for m in (diagnostics.monitors if diagnostics else [])],
            "knobs": [asdict(k) for k in knobs], "errors": errors or {}}
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(spec, fh, indent=1, ensure_ascii=False)
