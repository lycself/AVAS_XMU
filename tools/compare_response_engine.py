"""Compare the orbit response matrix of the linear model (avas.dynamics) with the engine.

Run from the repository root, on a machine where the engine runs (Windows):

    .venv\\Scripts\\python.exe tools\\compare_response_engine.py [--particles 200] [--work DIR]

The line is a copy of ``examples/hwr010`` with two steerers, the ``sol`` map and the HWR:

    drift · S1 · drift 0.1 · sol · drift 0.1 · S2 · drift 0.05 · HWR · drift 0.4

and four position monitors (at the end of the 0.1 m drift after ``sol``, after the HWR, 0.2 m
and 0.4 m further).  The engine runs once without kicks and once per knob (S1 By, S1 Bx, S2 By,
S2 Bx; one-sided steps, the centroid is linear in the kicks) with *particles* macro-particles of
a beam whose emittances are 1000 times smaller than the example's and without space charge, so
the mean position is the centroid without the instability of single-particle runs.  Every run
uses ``randomseed 12345`` (input.txt), so all runs start from the same particles: with a new
sample per run the mean of 200 particles moved by about 3.5 µm, 0.0035 mm/mT per 1 mT step
(2026-11-03).  The base case runs twice; the difference of the two is the noise floor.  The
table gives every element of both matrices (mm per mT) and their difference relative to the
largest element; written as ``response.md`` / ``response.csv`` in the work folder.
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

from avas import dynamics as dyn  # noqa: E402
from avas import paths  # noqa: E402
from avas.model import LinearModel  # noqa: E402
from compare_centroid_engine import _edit  # noqa: E402
from compare_preview_engine import _copy_example, engine_available, run_engine  # noqa: E402

STEP = 1e-3                    # T per knob
SEED = 12345                   # fixed engine seed: the same particles in every run
LATTICE = """start
drift 0.05 0.03 0
S1: steerer 0 0.03 0 0 0 0 0.01
drift 0.1 0.03 0
field 0.35 0.02 0 3 0 0 0 0.3 sol
drift 0.1 0.03 0
S2: steerer 0 0.03 0 0 0 0 0.01
drift 0.05 0.03 0
field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010
drift 0.4 0.03 0
end
"""
MONITORS = [("M1", 0.60), ("M2", 0.86), ("M3", 1.06), ("M4", 1.26)]


def make_input(folder, particles):
    inp = _copy_example(folder)
    with open(os.path.join(inp, paths.DEFAULT_LATTICE), "w", newline="\n") as fh:
        fh.write(LATTICE)
    beam = os.path.join(inp, "beam.txt")
    with open(beam) as fh:
        lines = {ln.split()[0].lower(): ln.split()[1:] for ln in fh if ln.split()}
    twiss = {p: [float(v) for v in lines[f"twiss{p}"]] for p in "xyz"}
    _edit(beam, {"particlenumber": particles, "current": 0,
                 **{f"twiss{p}": [t[0], t[1], t[2] * 1e-3] for p, t in twiss.items()}})
    _edit(os.path.join(inp, "input.txt"), {"spacecharge": 0, "randomseed": SEED})
    return inp


def engine_readings(inp, out, monitors):
    env = run_engine(inp, out)
    z, cx, cy = (np.asarray(env[k], dtype=float) for k in ("z", "cx", "cy"))
    keep = np.isfinite(cx) & np.isfinite(cy)
    if not keep.any():
        raise RuntimeError("the engine gave no centroid")
    z, cx, cy = z[keep], cx[keep], cy[keep]
    return np.array([v for _n, zm in monitors for v in (np.interp(zm, z, cx), np.interp(zm, z, cy))]) * 1e-3


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--work", help="work folder (default: a new temporary folder)")
    ap.add_argument("--particles", type=int, default=200, help="macro-particles per engine run (default 200)")
    ap.add_argument("--no-engine", action="store_true", help="model only")
    args = ap.parse_args(argv)
    work = os.path.abspath(args.work) if args.work else tempfile.mkdtemp(prefix="avas_response_compare_")
    os.makedirs(work, exist_ok=True)
    inp = make_input(os.path.join(work, "case"), args.particles)
    model = LinearModel.from_project(inp)
    diag = dyn.Diagnostics([dyn.Monitor(n, z) for n, z in MONITORS])
    knobs = [dyn.Knob(t, p, step=STEP) for t in ("S1", "S2") for p in ("By", "Bx")]
    resp = dyn.response(model, knobs, diag, space_charge=False)
    engine = None
    if not args.no_engine:
        reason = engine_available()
        if reason:
            print(f"ENGINE NOT AVAILABLE ({reason}); model only.")
        else:
            base = engine_readings(inp, os.path.join(work, "case", "OutputFile_base"), MONITORS)
            again = engine_readings(inp, os.path.join(work, "case", "OutputFile_base2"), MONITORS)
            floor = float(np.abs(again - base).max() / STEP)
            print(f"  noise floor (two base runs): {floor:.3g} mm/mT")
            engine = np.zeros_like(resp.matrix)
            for j, knob in enumerate(knobs):
                folder = os.path.join(work, f"knob_{knob.target}_{knob.param}")
                kin = make_input(folder, args.particles)
                with open(os.path.join(kin, paths.DEFAULT_LATTICE), "w", newline="\n") as fh:
                    fh.write(model.with_knobs({knob.target: {knob.param: STEP}}))
                engine[:, j] = (engine_readings(kin, os.path.join(folder, "OutputFile"), MONITORS) - base) / STEP
                print(f"  {knob.label}: done")
    scale = np.abs(resp.matrix).max()
    rows = []
    for i, label in enumerate(resp.labels):
        for j, knob in enumerate(knobs):
            row = {"reading": label, "knob": knob.label, "model_mm_per_mT": resp.matrix[i, j]}
            if engine is not None:
                row["engine_mm_per_mT"] = engine[i, j]
                row["diff_rel_to_max"] = (resp.matrix[i, j] - engine[i, j]) / scale
            rows.append(row)
    keys = ["reading", "knob", "model_mm_per_mT", "engine_mm_per_mT", "diff_rel_to_max"]
    with open(os.path.join(work, "response.csv"), "w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=keys)
        writer.writeheader()
        writer.writerows({k: r.get(k) for k in keys} for r in rows)
    with open(os.path.join(work, "response.md"), "w", encoding="utf-8") as fh:
        fh.write("# Orbit response: linear model vs engine (mm per mT = m per T)\n\n| " + " | ".join(keys) + " |\n|"
                 + "---|" * len(keys) + "\n")
        for r in rows:
            fh.write("| " + " | ".join(f"{r[k]:.4g}" if isinstance(r.get(k), float) else str(r.get(k, "")) for k in keys)
                     + " |\n")
        if engine is not None:
            worst = np.abs(resp.matrix - engine).max() / scale
            fh.write(f"\nLargest difference: {worst:.3g} of the largest element.\n")
            fh.write(f"Noise floor (two base runs, same seed): {floor:.3g} mm/mT = {floor / scale:.3g} of the largest "
                     "element.\n")
            print(f"largest difference {worst:.3g} of the largest element")
    print(f"written: {os.path.join(work, 'response.md')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
