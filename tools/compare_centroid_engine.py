"""Check the centroid of the linear model (avas.model) against the engine with single-particle runs.

Run from the repository root, on a machine where the engine runs (Windows):

    .venv\\Scripts\\python.exe tools\\compare_centroid_engine.py [--work DIR] [--cases ...]

The centroid conventions of :mod:`avas.model.model` (the sign and size of a steerer kick, the
deflection by the on-axis transverse fields of field maps, first-order transport through
solenoid and RF maps) were derived from the envelope model, not from engine runs; this script
compares them.  Every case runs one particle (``particlenumber 1``, ``spacecharge 0``) on a copy
of ``examples/hwr010`` in a temporary work folder.  Cases:

* ``steerer-by`` / ``steerer-bx`` – magnetic steerer (By, then Bx = 1e-3 T) over a 0.2 m drift,
  then 1 m of drift: kick direction and size (expected −x for By > 0, +y for Bx > 0).
* ``steerer-ex``  – electric steerer (``kind 1``, Ex = 1e5 V/m): direction and the V/m unit.
* ``steerer-quad`` – the By steerer in front of a zero-gradient quadrupole instead of a drift
  (in case the engine wants a magnet after a steerer).
* ``offset-quad`` / ``offset-sol`` / ``offset-hwr`` – the particle started off axis
  (``displacepos`` 1 mm, 0.5 mm; read in mm by the engine) through one quadrupole, the ``sol``
  map alone or the HWR map alone: where a difference in ``offset`` comes from.
* ``offset``      – the tutorial period (``sol`` + HWR) off axis: transport, solenoid rotation sense.
* ``map-dipoles`` – the same period on axis: deflection by the maps' own transverse fields
  (the model is shown with and without them).

For each case the table gives the engine's and the model's centroid x / y at the end and their
largest values (mm), the final energy (engine: the particle; model: the reference particle) and
the rms of the model − engine difference along z relative to the largest engine offset.
Written as ``centroid.md`` / ``centroid.csv`` (with the reasons of failed engine runs) and the
curves as ``curves.csv`` (case, source, z, cx, cy) in the work folder.  ``--no-engine`` prints
the model only.
"""
import argparse
import csv
import os
import sys
import tempfile

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, "tools"))

from avas import paths  # noqa: E402
from avas.data.lattice_edit import set_keyword_value  # noqa: E402
from avas.learn import tutorial  # noqa: E402
from avas.model import LinearModel  # noqa: E402
from compare_preview_engine import _copy_example, engine_available, run_engine  # noqa: E402

DRIFTS = "drift 0.2 0.03 0\ndrift 1.0 0.03 0\n"
HWR = "field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010\n"


def _edit(path, values):
    with open(path) as fh:
        text = fh.read()
    for keyword, items in values.items():
        for position, value in enumerate(items if isinstance(items, (list, tuple)) else [items], start=1):
            text = set_keyword_value(text, keyword, value, position)
    with open(path, "w", newline="\n") as fh:
        fh.write(text)


def make_case(work, name, lattice, offset_mm=(0.0, 0.0)):
    """Input folder of one single-particle case."""
    inp = _copy_example(os.path.join(work, name))
    with open(os.path.join(inp, paths.DEFAULT_LATTICE), "w", newline="\n") as fh:
        fh.write(lattice)
    _edit(os.path.join(inp, "beam.txt"), {"particlenumber": 1, "current": 0,
                                          "displacepos": [offset_mm[0], offset_mm[1], 0]})
    _edit(os.path.join(inp, "input.txt"), {"spacecharge": 0})
    return inp


def cases():
    period = tutorial.lattice("periods", {"periods": 1})
    return [
        ("steerer-by", "start\nsteerer 0 0.03 0 0 0.001 0\n" + DRIFTS + "end\n", (0, 0)),
        ("steerer-bx", "start\nsteerer 0 0.03 0 0.001 0 0\n" + DRIFTS + "end\n", (0, 0)),
        ("steerer-ex", "start\nsteerer 0 0.03 0 100000 0 1\n" + DRIFTS + "end\n", (0, 0)),
        ("steerer-quad", "start\nsteerer 0 0.03 0 0 0.001 0\nquad 0.2 0.03 0 0\ndrift 1.0 0.03 0\nend\n", (0, 0)),
        ("offset-quad", "start\ndrift 0.05 0.03 0\nquad 0.1 0.03 0 10\ndrift 0.5 0.03 0\nend\n", (1.0, 0.5)),
        ("offset-sol", "start\ndrift 0.05 0.02 0\nfield 0.35 0.02 0 3 0 0 0 0.3 sol\ndrift 0.6 0.02 0\nend\n",
         (1.0, 0.5)),
        ("offset-hwr", "start\ndrift 0.05 0.02 0\n" + HWR + "drift 0.3 0.02 0\nend\n", (1.0, 0.5)),
        ("offset", period, (1.0, 0.5)),
        ("map-dipoles", period, (0, 0)),
    ]


def model_centroid(input_dir, offset_mm, map_dipoles=True):
    model = LinearModel.from_project(input_dir, map_dipoles=map_dipoles)
    c0 = np.zeros(6)
    c0[0], c0[2] = offset_mm[0] * 1e-3, offset_mm[1] * 1e-3
    res = model.evaluate(space_charge=False, centroid0=c0)
    return res.z, res.centroid[:, 0] * 1e3, res.centroid[:, 2] * 1e3, res.energy


def row(name, source, z, cx, cy, w, ref=None):
    out = {"case": name, "source": source, "cx_end_mm": cx[-1], "cy_end_mm": cy[-1],
           "cx_maxabs_mm": float(np.max(np.abs(cx))), "cy_maxabs_mm": float(np.max(np.abs(cy))),
           "w_end_MeV": float(w[-1])}
    if ref is not None:
        rz, rx, ry, _rw = ref
        scale = max(np.max(np.abs(rx)), np.max(np.abs(ry)), 1e-12)
        dx = np.interp(rz, z, cx) - rx
        dy = np.interp(rz, z, cy) - ry
        out["rms_diff_rel"] = float(np.sqrt(np.mean(dx * dx + dy * dy)) / scale)
    return out


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--work", help="work folder (default: a new temporary folder)")
    ap.add_argument("--cases", help="comma-separated subset of " + ",".join(c[0] for c in cases()))
    ap.add_argument("--no-engine", action="store_true", help="model only")
    args = ap.parse_args(argv)
    work = os.path.abspath(args.work) if args.work else tempfile.mkdtemp(prefix="avas_centroid_compare_")
    os.makedirs(work, exist_ok=True)
    use_engine = not args.no_engine
    if use_engine:
        reason = engine_available()
        if reason:
            print(f"ENGINE NOT AVAILABLE ({reason}); model only.")
            use_engine = False
    wanted = {c.strip() for c in args.cases.split(",")} if args.cases else None
    rows, curves, notes = [], [], []

    def add(name, source, data, ref=None):
        rows.append(row(name, source, *data, ref=ref))
        curves.extend((name, source, f"{a:.6g}", f"{b:.6g}", f"{c:.6g}") for a, b, c in zip(*data[:3]))

    for name, lattice, offset in cases():
        if wanted and name not in wanted:
            continue
        print(f"\n=== {name}")
        inp = make_case(work, name, lattice, offset)
        ref = None
        if use_engine:
            try:
                env = run_engine(inp, os.path.join(os.path.dirname(inp), "OutputFile"))
                ref = (env["z"], env["cx"], env["cy"], env["energy"])
                add(name, "engine", ref)
            except RuntimeError as exc:
                print(f"  engine: {exc}")
                notes.append(f"{name}: engine {exc} (log: {os.path.join(os.path.dirname(inp), 'OutputFile.log')})")
        add(name, "model", model_centroid(inp, offset), ref)
        if name == "map-dipoles":
            add(name, "model without map dipoles", model_centroid(inp, offset, False), ref)
        for r in rows:
            if r["case"] == name:
                print("  " + "  ".join(f"{k}={v:.4g}" if isinstance(v, float) else f"{k}={v}" for k, v in r.items()
                                       if k != "case"))
    keys = ["case", "source", "cx_end_mm", "cy_end_mm", "cx_maxabs_mm", "cy_maxabs_mm", "w_end_MeV", "rms_diff_rel"]
    with open(os.path.join(work, "centroid.csv"), "w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=keys)
        writer.writeheader()
        writer.writerows({k: r.get(k) for k in keys} for r in rows)
    with open(os.path.join(work, "centroid.md"), "w", encoding="utf-8") as fh:
        fh.write("# Linear-model centroid vs engine (single particle)\n\n| " + " | ".join(keys) + " |\n|"
                 + "---|" * len(keys) + "\n")
        for r in rows:
            fh.write("| " + " | ".join(f"{r[k]:.5g}" if isinstance(r.get(k), float) else str(r.get(k, ""))
                                       for k in keys) + " |\n")
        if notes:
            fh.write("\n## Engine runs that failed\n\n" + "\n".join(f"- {n}" for n in notes) + "\n")
    with open(os.path.join(work, "curves.csv"), "w", newline="", encoding="utf-8") as fh:
        writer = csv.writer(fh)
        writer.writerow(["case", "source", "z_m", "cx_mm", "cy_mm"])
        writer.writerows(curves)
    print(f"\nwritten: {os.path.join(work, 'centroid.md')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
