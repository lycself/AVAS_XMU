"""Compare the linear envelope preview (old and new) with the multi-particle engine.

Run from the repository root, on a machine where the engine runs (Windows):

    .venv\\Scripts\\python.exe tools\\compare_preview_engine.py [--repeat 3] [--work DIR]

"Old" is ``avas/sim/linear_optics.py`` of another Git revision (``--old-ref``, default
``main``, else ``origin/main``), loaded as a separate module; "new" is the checked-out
code.  Every case runs on copies in a temporary work folder; the example project and the
user's projects are never touched.  Cases:

* ``example``   – ``examples/hwr010`` as it is (one HWR cavity): nothing may get worse.
* ``periods``   – the primer tutorial's ``periods`` step (``--periods`` × "sol + HWR"),
                  where the solenoid-fringe fix matters most.
* ``v3-<W>``    – one HWR at φs = −30° (V3 = 0), Ke = 1.36, beam energy W MeV
                  (``--energies``); compare the energy gain mainly.  At 0.3 and 1.0 MeV
                  no entry phase gives −30°; the new preview warns and takes the closest φs.

For each case the table gives final and maximum rms x / y / z (mm) and the final energy
(MeV) of the engine (mean and spread over ``--repeat`` runs: the engine is multi-threaded
and varies a little), the old and the new preview, and each preview's relative difference
to the engine.  "Final" is taken at the last z both the engine and the preview reach.
The table is printed and written as ``comparison.md`` / ``comparison.csv`` in the work folder.
``--no-engine`` compares the two previews only (works on any machine).
"""
import argparse
import csv
import importlib.util
import os
import shutil
import subprocess
import sys
import tempfile
import time
import traceback

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)

from avas import paths  # noqa: E402
from avas.data.dataset_stream import dataset_envelope  # noqa: E402
from avas.data.lattice_edit import set_keyword_value  # noqa: E402
from avas.learn import tutorial  # noqa: E402
from avas.sim import linear_optics as new_lo  # noqa: E402

METRICS = [("x_end", "rms x end", "mm"), ("y_end", "rms y end", "mm"), ("z_end", "rms z end", "mm"),
           ("x_max", "rms x max", "mm"), ("y_max", "rms y max", "mm"), ("z_max", "rms z max", "mm"),
           ("w_end", "energy end", "MeV"), ("gain", "energy gain", "MeV")]
V3_LATTICE = ("start\ndrift 0.05 0.02 0\nfield 0.21 0.02 0 1 162.5e6 -30 1.36 -1.36 hwr010\n"
              "drift 0.2 0.02 0\nend\n")


# --------------------------------------------------------------------------- old preview
def load_old_preview(ref, work):
    """``(module, description)`` of ``avas/sim/linear_optics.py`` at Git revision *ref* (or main / origin/main)."""
    refs = [ref] if ref else ["main", "origin/main"]
    for r in refs:
        try:
            sha = subprocess.run(["git", "rev-parse", "--short", r], cwd=ROOT, capture_output=True, text=True,
                                 check=True).stdout.strip()
            text = subprocess.run(["git", "show", f"{r}:avas/sim/linear_optics.py"], cwd=ROOT, capture_output=True,
                                  check=True).stdout
        except (OSError, subprocess.CalledProcessError):
            continue
        path = os.path.join(work, "linear_optics_old.py")
        with open(path, "wb") as fh:
            fh.write(text)
        spec = importlib.util.spec_from_file_location("avas_linear_optics_old", path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        with open(new_lo.__file__, "rb") as fh:
            same = fh.read().replace(b"\r\n", b"\n") == text.replace(b"\r\n", b"\n")
        return module, f"{r} ({sha})" + (" – identical to the checked-out code!" if same else "")
    raise SystemExit("Cannot read avas/sim/linear_optics.py from main or origin/main (git fetch first, or pass --old-ref).")


# --------------------------------------------------------------------------- cases
def _copy_example(dest):
    src = str(paths.example_input_dir())
    inp = os.path.join(dest, "InputFile")
    shutil.copytree(src, inp, ignore=shutil.ignore_patterns("OutputFile", "lattice.txt"))
    return inp


def make_cases(work, periods, energies):
    """``[(name, input_dir, description)]``, built lazily so a failing case does not stop the others."""
    def example():
        return _copy_example(os.path.join(work, "example"))

    def tutorial_periods():
        folder = tutorial.export(os.path.join(work, "periods"), "periods", {"periods": periods})
        return os.path.join(folder, "InputFile")

    def v3(energy):
        def build():
            inp = _copy_example(os.path.join(work, f"v3-{energy:g}"))
            with open(os.path.join(inp, paths.DEFAULT_LATTICE), "w", newline="\n") as fh:
                fh.write(V3_LATTICE)
            with open(os.path.join(inp, "ini.ini")) as fh:
                ini = fh.read()
            if "source = \n" not in ini and "source =\n" not in ini:     # make sure the run uses lattice_mulp.txt
                print(f"  note: ini.ini of the example names another lattice source; v3-{energy:g} may differ")
            beam = os.path.join(inp, "beam.txt")
            with open(beam) as fh:
                text = fh.read()
            with open(beam, "w", newline="\n") as fh:
                fh.write(set_keyword_value(text, "kneticenergy", energy))
            return inp
        return build

    cases = [("example", example, "examples/hwr010 as it is (HWR cavity, phi_s = -33 deg)"),
             ("periods", tutorial_periods, f"primer tutorial 'periods' step, {periods} x (sol + HWR)")]
    cases += [(f"v3-{e:g}", v3(e), f"HWR at V3 = 0, phi_s = -30 deg, Ke = 1.36, W = {e:g} MeV") for e in energies]
    return cases


# --------------------------------------------------------------------------- metrics
def _metrics(z, x, y, rz, w, z_end, w_in):
    z, x, y, rz, w = (np.asarray(a, dtype=float) for a in (z, x, y, rz, w))
    inside = z <= z_end + 1e-9
    return {
        "x_end": float(np.interp(z_end, z, x)), "y_end": float(np.interp(z_end, z, y)),
        "z_end": float(np.interp(z_end, z, rz)),
        "x_max": float(np.nanmax(x[inside])), "y_max": float(np.nanmax(y[inside])), "z_max": float(np.nanmax(rz[inside])),
        "w_end": float(np.interp(z_end, z, w)), "gain": float(np.interp(z_end, z, w)) - w_in,
    }


def preview(module, input_dir):
    """The preview of *module* on the case's input: ``(curves, warnings)``."""
    with open(paths.lattice_source_path(input_dir), encoding="utf-8", errors="replace") as fh:
        text = fh.read()
    res = module.linear_preview(text, module.read_beam(input_dir), [input_dir])
    curves = (res["z"], res["rms_x"], res["rms_y"], res["rms_z"], res["energy"])
    return curves, [en for en, _zh in res["warnings"]]


def engine_available():
    """None when the engine loads in a fresh process, else the reason."""
    code = "from avas.core.MultiParticleEngine import MultiParticleEngine; MultiParticleEngine()"
    res = subprocess.run([sys.executable, "-c", code], cwd=ROOT, capture_output=True, text=True)
    if res.returncode == 0:
        return None
    return (res.stderr.strip().splitlines() or ["unknown error"])[-1]


def run_engine(input_dir, output_dir):
    """Run the engine on *input_dir* into *output_dir*; returns the DataSet envelope (dict of arrays)."""
    started = time.time()
    res = subprocess.run([sys.executable, "-m", "avas", "run", "--input", input_dir, "--output", output_dir],
                         cwd=ROOT, capture_output=True, text=True)
    with open(os.path.join(os.path.dirname(output_dir), os.path.basename(output_dir) + ".log"), "w") as fh:
        fh.write(res.stdout + "\n--- stderr ---\n" + res.stderr)
    env = dataset_envelope(output_dir, max_points=10 ** 9)
    if res.returncode or env is None:
        tail = (res.stderr.strip() or res.stdout.strip()).splitlines()[-3:]
        raise RuntimeError(f"engine run failed (exit {res.returncode}): " + " | ".join(tail))
    env["seconds"] = time.time() - started
    return env


# --------------------------------------------------------------------------- report
def _fmt(v, digits=4):
    return "" if v is None or not np.isfinite(v) else f"{v:.{digits}g}"


def _rel(a, b):
    return (a - b) / b * 100 if b and np.isfinite(a) and np.isfinite(b) else float("nan")


def compare_case(name, build, desc, old, repeat, use_engine, work):
    print(f"\n=== {name}: {desc}")
    input_dir = build()
    w_in = float(new_lo.read_beam(input_dir)["energy"])
    curves = {}
    notes = []
    for label, module in (("old preview", old), ("new preview", new_lo)):
        try:
            curves[label], warns = preview(module, input_dir)
            notes += [f"{label}: {w}" for w in warns]
        except Exception as exc:  # noqa: BLE001 - reported, the other source still runs
            notes.append(f"{label} failed: {exc}")
    runs = []
    if use_engine:
        for k in range(repeat):
            out = os.path.join(os.path.dirname(input_dir), f"OutputFile_{k + 1}")
            env = run_engine(input_dir, out)
            print(f"  engine run {k + 1}/{repeat}: {env['seconds']:.0f} s, {int(env['rows'])} rows, "
                  f"transmission {env['alive'][-1] / env['alive'][0] * 100:.1f} %")
            runs.append(env)
    z_end = min([float(c[0][-1]) for c in curves.values()] + [float(e["z"][-1]) for e in runs])
    values = {label: _metrics(*c, z_end, w_in) for label, c in curves.items()}
    if runs:
        per_run = [_metrics(e["z"], e["rmsX"], e["rmsY"], e["rmsZ"], e["energy"], z_end, w_in) for e in runs]
        values["engine"] = {k: float(np.mean([m[k] for m in per_run])) for k in per_run[0]}
        values["engine spread"] = {k: float(np.max([m[k] for m in per_run]) - np.min([m[k] for m in per_run]))
                                   for k in per_run[0]}
    rows = []
    for key, label, unit in METRICS:
        row = {"case": name, "quantity": f"{label} [{unit}]", "z_end_m": z_end}
        for src in ("engine", "engine spread", "old preview", "new preview"):
            row[src] = values.get(src, {}).get(key, float("nan"))
        for src in ("old preview", "new preview"):
            row[f"{src} vs engine %"] = _rel(row[src], row["engine"])
        row["new vs old %"] = _rel(row["new preview"], row["old preview"])
        rows.append(row)
    _print_rows(rows)
    for n in notes:
        print("  " + n)
    return rows, notes


COLUMNS = ["quantity", "engine", "engine spread", "old preview", "new preview", "old preview vs engine %",
           "new preview vs engine %", "new vs old %"]


def _print_rows(rows):
    widths = [20, 10, 15, 13, 13, 25, 25, 13]
    print("  " + "".join(c.ljust(w) for c, w in zip(COLUMNS, widths)))
    for r in rows:
        cells = [r["quantity"]] + [_fmt(r[c]) if "%" not in c else _fmt(r[c], 3) for c in COLUMNS[1:]]
        print("  " + "".join(str(c).ljust(w) for c, w in zip(cells, widths)))


def write_reports(work, all_rows, notes, header):
    csv_path = os.path.join(work, "comparison.csv")
    with open(csv_path, "w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=["case", "z_end_m"] + COLUMNS)
        writer.writeheader()
        for r in all_rows:
            writer.writerow({k: r.get(k) for k in ["case", "z_end_m"] + COLUMNS})
    md_path = os.path.join(work, "comparison.md")
    with open(md_path, "w", encoding="utf-8") as fh:
        fh.write("# Linear preview vs engine\n\n" + "\n".join(f"- {h}" for h in header) + "\n")
        for case in dict.fromkeys(r["case"] for r in all_rows):
            rows = [r for r in all_rows if r["case"] == case]
            fh.write(f"\n## {case} (final values at z = {rows[0]['z_end_m']:.3f} m)\n\n")
            fh.write("| " + " | ".join(COLUMNS) + " |\n|" + "---|" * len(COLUMNS) + "\n")
            for r in rows:
                fh.write("| " + " | ".join([r["quantity"]] + [_fmt(r[c], 3 if "%" in c else 4) for c in COLUMNS[1:]])
                         + " |\n")
            for n in notes.get(case, ()):
                fh.write(f"\n- {n}")
            fh.write("\n")
        failed = notes.get("_failed", ())
        if failed:
            fh.write("\n## Failed cases\n\n" + "\n".join(f"- {f}" for f in failed) + "\n")
    return csv_path, md_path


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--repeat", type=int, default=1, help="engine runs per case (default 1)")
    ap.add_argument("--periods", type=int, default=8, help="periods of the tutorial case (default 8)")
    ap.add_argument("--energies", default="0.5,0.3,1.0", help="beam energies of the V3 = 0 cases, MeV (default 0.5,0.3,1.0)")
    ap.add_argument("--old-ref", help="Git revision of the old preview (default main, else origin/main)")
    ap.add_argument("--work", help="work folder (default: a new temporary folder)")
    ap.add_argument("--cases", help="comma-separated subset: example,periods,v3-0.5,...")
    ap.add_argument("--no-engine", action="store_true", help="compare the two previews only")
    args = ap.parse_args(argv)

    work = os.path.abspath(args.work) if args.work else tempfile.mkdtemp(prefix="avas_preview_compare_")
    os.makedirs(work, exist_ok=True)
    old, old_desc = load_old_preview(args.old_ref, work)
    head = subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    header = [f"work folder: {work}", f"old preview: {old_desc}", f"new preview: checked-out code ({head or 'unknown'})",
              f"engine runs per case: {0 if args.no_engine else args.repeat}"]
    use_engine = not args.no_engine
    if use_engine:
        reason = engine_available()
        if reason:
            header.append(f"ENGINE NOT AVAILABLE ({reason}); previews only. Run `python -m avas doctor` for details.")
            use_engine = False
    for h in header:
        print(h)
    energies = [float(e) for e in args.energies.split(",") if e.strip()]
    cases = make_cases(work, args.periods, energies)
    if args.cases:
        wanted = {c.strip() for c in args.cases.split(",")}
        cases = [c for c in cases if c[0] in wanted]
    all_rows, notes = [], {"_failed": []}
    for name, build, desc in cases:
        try:
            rows, case_notes = compare_case(name, build, desc, old, max(1, args.repeat), use_engine, work)
            all_rows += rows
            notes[name] = case_notes
        except Exception as exc:  # noqa: BLE001 - one failing case must not stop the others
            print(f"  FAILED: {exc}")
            if os.environ.get("AVAS_DEBUG"):
                traceback.print_exc()
            notes["_failed"].append(f"{name}: {exc}")
    csv_path, md_path = write_reports(work, all_rows, notes, header)
    print(f"\nwritten: {md_path}\n         {csv_path}")
    return 1 if notes["_failed"] else 0


if __name__ == "__main__":
    sys.exit(main())
