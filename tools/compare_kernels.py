"""Compare the original kernel and the new kernel on a project (M8a: before the new kernel becomes the default).

Run from the repository root:

    .venv\\Scripts\\python.exe tools\\compare_kernels.py --input <project or its InputFile> [--out DIR]
        [--particles N] [--repeats 2] [--seeds 1,2] [--gpu] [--keep-runs]

Every run goes through :class:`avas.research.Study` on a copy of the inputs in ``--out``; the
project's InputFile and OutputFile are never written.  Runs, one after the other (fair timing):

* ``original`` ``--repeats`` times with ``randomseed 0`` – the reference and its run-to-run
  fluctuation.  A fixed seed is not used here: the original kernel then generates a distribution
  with x fully correlated with the phase (a known defect fixed in the new kernel).
* ``new`` (the corrected default mode, CPU) once per ``--seeds`` value – the new kernel and its
  fluctuation;
* with ``--gpu``: ``new`` with ``device auto`` and the first seed – the time, and the GPU result
  against the CPU run with the same seed;
* on Windows: ``original`` and ``new --compat dll`` with the same fixed seed – whether compatibility
  mode reproduces AVAS.dll byte for byte.  Not on Linux: libAVAS.so generates distributions with
  glibc's ``rand`` (compatibility mode follows AVAS.dll and the Windows UCRT), so the runs start from
  different particles, and the .so also differs from AVAS.dll in places (superpose blocks with a
  cavity, M2-notes); the parity references of ``engine/tests/parity`` are AVAS.dll outputs.

Where the original kernel does not run (macOS, ARM), compatibility mode stands in for it as the
reference (it is AVAS.dll's behaviour) and the byte check is skipped.

The physics table compares the mean of the new runs with the mean of the reference runs on a
common z grid: rms sizes, normalised emittances, energy and transmission.  Emittances are compared
as their change from the first row, so that the sampling of the initial distribution does not
count as a kernel difference; ``start`` gives the difference of the initial values.  ``noise`` is
the larger of the two sides' run-to-run spread (max − min over the runs, largest along z); a
difference more than twice the noise is marked ``beyond`` and the first z where it appears is
named with the lattice element there.  More repeats and seeds give a steadier noise estimate.
Sizes and emittances are relative to the reference's largest value, energy in MeV, transmission
absolute.  A project that reads its distribution from a file, or tracks a single particle, has no
sampling noise: every run of a kernel gives the same result, the verdict is ``systematic`` (or
``identical``) instead of ``beyond`` / ``within``, and every difference is the kernels' (corrected
defects, arithmetic).  Single-particle runs are compared through the synchronous particle's columns of
DataSet.txt (x, y, z, gamma*beta_z) step by step: their rms columns are NaN, and AVAS.dll (and
compatibility mode) also writes NaN energy and centroid columns for one particle.

Times: ``wall`` is the whole ``avas run`` child process, ``kernel`` the engine's own "Simulated
total time" from engine.log (threads from its ``mulitthread:`` line when space charge is on).

Written to ``--out``: ``report.md``, ``summary.json``, ``curves.csv`` (z and every quantity of
every run).  The run folders (``runs/``) are deleted unless ``--keep-runs``.
"""
import argparse
import csv
import datetime
import json
import os
import re
import shutil
import statistics
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from avas import engines  # noqa: E402
from avas.data.lattice_doc import LatticeDocument  # noqa: E402
from avas.research import Study, Variant  # noqa: E402
from avas.research.compare import common_z, on_grid  # noqa: E402

FIXED_SEED = 12345
QUANTITIES = ("rms_x", "rms_y", "rms_z", "emit_x", "emit_y", "emit_z", "energy", "transmission")
BEYOND = 2.0
BEFORE = "initial distribution (before the first element)"


def curve(run, q, z):
    if q == "transmission":
        alive = np.asarray(run.alive, dtype=float)
        if not len(alive) or not alive[0]:
            return np.full(len(z), np.nan)
        return np.interp(z, run.z, alive / alive[0])
    return on_grid(run, q, z)


def scale_of(q, ref):
    if q in ("energy", "transmission"):
        return 1.0
    m = np.nanmax(np.abs(ref))
    return m if m > 0 else 1.0


def element_at(doc, z):
    best, first = None, None
    for st in doc.elements():
        if st.z_start is None or st.z_end is None:
            continue
        first = st.z_start if first is None else min(first, st.z_start)
        if st.z_start - 1e-9 <= z <= st.z_end + 1e-9:
            best = st  # the innermost / last one listed
    if best is None:
        return BEFORE if first is not None and z < first else "(no element)"
    return f"{best.name or best.keyword} (line {best.line_no}, {best.z_start:.4f}–{best.z_end:.4f} m)"


def dataset_identity(a, b):
    """'identical' or the largest relative difference of the DataSet.txt numbers of two runs."""
    pa, pb = (os.path.join(r.output_dir, "DataSet.txt") for r in (a, b))
    with open(pa, "rb") as fa, open(pb, "rb") as fb:
        ta, tb = fa.read().replace(b"\r\n", b"\n"), fb.read().replace(b"\r\n", b"\n")
    if ta == tb:
        return "identical", 0.0
    from avas.post.analysis.run_diagnostics import read_dataset_array
    da, db = read_dataset_array(pa), read_dataset_array(pb)
    if da.shape != db.shape:
        return f"different shape {da.shape} / {db.shape}", float("inf")
    if (np.isfinite(da) != np.isfinite(db)).any():
        return "different NaN pattern", float("inf")
    both = np.isfinite(da)
    den = np.maximum(np.abs(da), np.abs(db))
    rel = np.where(both & (den > 0), np.abs(da - db) / np.where(den > 0, den, 1), 0.0)
    return "different", float(rel.max())


def log_facts(run):
    """(kernel seconds, threads) from the run's engine.log; None where it does not say."""
    secs = threads = None
    try:
        with open(os.path.join(run.folder, "engine.log"), encoding="utf-8", errors="replace") as fh:
            text = fh.read()
    except OSError:
        return secs, threads
    m = re.findall(r"Simulated total time:?\s*([0-9.eE+-]+)", text)
    if m:
        secs = float(m[-1])
    m = re.search(r"mulitthread:\s*(\d+)", text)
    if m:
        threads = int(m.group(1))
    return secs, threads


def run_row(name, group, run):
    eng = (run.record.get("engine") or {})
    act = eng.get("actual") or {}
    kernel_s, log_threads = log_facts(run)
    return {"run": name, "group": group, "status": run.record.get("status"), "error": run.record.get("error"),
            "kernel": act.get("kernel") or eng.get("kernel"), "device": act.get("device"),
            "precision": act.get("precision"), "compat": act.get("compat"),
            "threads": act.get("threads") or log_threads, "seed": run.record.get("seed"),
            "seconds": run.record.get("seconds"), "kernel_seconds": kernel_s, "transmission": run.transmission}


SYNC = (("x_sync", 29, 1e3, "mm"), ("y_sync", 31, 1e3, "mm"), ("z_sync", 33, 1e3, "mm"), ("gbz_sync", 34, 1.0, "1"))


def single_particle(ref, new):
    """Rows comparing the synchronous particle of the first reference run with the first new run, row by row
    (both step in the same time step): largest and final difference of x, y, z and gamma*beta_z."""
    from avas.post.analysis.run_diagnostics import read_dataset_array
    a, b = (read_dataset_array(os.path.join(r.output_dir, "DataSet.txt")) for r in (ref[0], new[0]))
    n = min(len(a), len(b))
    rows = []
    for q, col, factor, unit in SYNC:
        da, db = a[:n, col], b[:n, col]
        d = (db - da) * factor
        rows.append({"quantity": f"{q} [{unit}]", "ref_end": float(da[-1] * factor), "new_end": float(db[-1] * factor),
                     "diff_end": float(d[-1]), "diff_max": float(np.nanmax(np.abs(d))), "rows": [len(a), len(b)]})
    return rows


def physics(ref, new, z, doc, runs, quantities=QUANTITIES):
    """Rows of the physics table, the curves of every run and whether the input is deterministic (every run
    of each kernel identical: a particle file or a single particle)."""
    rows, table = [], {"z": z}
    for name, r in runs.items():
        if r.ok:
            for q in quantities:
                table[f"{q}:{name}"] = curve(r, q, z)
    spread = 0.0
    for q in quantities:
        for side in (ref, new):
            c = np.array([curve(r, q, z) for r in side])
            if np.isfinite(c).any():
                spread = max(spread, float(np.nanmax(np.ptp(c, axis=0))))
    deterministic = spread == 0.0
    for q in quantities:
        cr = np.array([curve(r, q, z) for r in ref])
        cn = np.array([curve(r, q, z) for r in new])
        mr, mn = cr.mean(axis=0), cn.mean(axis=0)
        s = scale_of(q, mr)
        start = float((mn[0] - mr[0]) / s)
        if q.startswith("emit"):  # growth: the initial sampling is not a kernel difference
            cr, cn = cr - cr[:, :1], cn - cn[:, :1]
            mr0, mn0 = cr.mean(axis=0), cn.mean(axis=0)
        else:
            mr0, mn0 = mr, mn
        diff = (mn0 - mr0) / s
        noise = max(float(np.nanmax(np.ptp(cr, axis=0))), float(np.nanmax(np.ptp(cn, axis=0)))) / s
        mx = float(np.nanmax(np.abs(diff)))
        beyond = mx > BEYOND * noise and mx > 1e-12
        first = ""
        if beyond:
            i = int(np.flatnonzero(np.abs(diff) > BEYOND * noise)[0])
            first = f"z = {z[i]:.4f} m, {element_at(doc, z[i])}"
        if deterministic:
            verdict = "systematic" if mx > 1e-12 else "identical"
            first = ""
        else:
            verdict = "beyond" if beyond else "within"
        rows.append({"quantity": q, "ref_end": float(mr[-1]), "new_end": float(mn[-1]), "start": start,
                     "diff_end": float(diff[-1]), "diff_max": mx, "noise": noise,
                     "verdict": verdict, "first": first,
                     "growth": q.startswith("emit")})
    return rows, table, deterministic


def gpu_rows(gpu, cpu, z, quantities=QUANTITIES):
    """Largest difference of the GPU run against the CPU run with the same seed."""
    out = []
    for q in quantities:
        a, b = curve(gpu, q, z), curve(cpu, q, z)
        s = scale_of(q, b)
        out.append({"quantity": q, "diff_max": float(np.nanmax(np.abs(a - b)) / s), "diff_end": float((a[-1] - b[-1]) / s)})
    return out


def timing(rows):
    """Median wall / kernel seconds per group of runs."""
    out = []
    for group in ("reference", "new CPU", "new auto", "original (fixed seed)", "compat (fixed seed)"):
        rs = [r for r in rows if r["group"] == group and r["status"] == "done"]
        if not rs:
            continue
        wall = [r["seconds"] for r in rs if isinstance(r["seconds"], (int, float))]
        kern = [r["kernel_seconds"] for r in rs if isinstance(r["kernel_seconds"], (int, float))]
        out.append({"group": group, "runs": len(rs), "wall": statistics.median(wall) if wall else None,
                    "kernel": statistics.median(kern) if kern else None,
                    "device": rs[0]["device"], "precision": rs[0]["precision"], "threads": rs[0]["threads"]})
    return out


def main(argv=None):
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")  # a GBK console cannot print the report
    except (AttributeError, ValueError):
        pass
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--input", required=True, help="project folder or its InputFile")
    ap.add_argument("--out", help="report folder (default kernel_compare_<time> in the current folder)")
    ap.add_argument("--particles", type=int, help="particlenumber of every run (default: the project's)")
    ap.add_argument("--repeats", type=int, default=2, help="runs of the reference with randomseed 0 (default 2)")
    ap.add_argument("--seeds", default="1,2", help="randomseed values of the new kernel's runs (at least two)")
    ap.add_argument("--gpu", action="store_true", help="also run the new kernel with device auto")
    ap.add_argument("--keep-runs", action="store_true", help="keep the run folders in <out>/runs")
    ap.add_argument("--timeout", type=float, default=None, help="seconds per run")
    args = ap.parse_args(argv)

    out = os.path.abspath(args.out or "kernel_compare_" + datetime.datetime.now().strftime("%Y%m%d_%H%M%S"))
    runs_dir = os.path.join(out, "runs")
    os.makedirs(runs_dir, exist_ok=True)
    seeds = [int(s) for s in args.seeds.split(",") if s.strip()]
    if len(seeds) < 2 or args.repeats < 2:
        ap.error("--seeds needs at least two values and --repeats at least 2")
    base = Variant(particles=args.particles) if args.particles else Variant()
    have_original = engines.original_available()

    ref_kw = dict(engine="original") if have_original else dict(engine="new", compat="dll", device="cpu")
    ref_name = "original" if have_original else "compat"
    plan = [(f"{ref_name}_r{i + 1}", "reference", ref_kw, 0, True) for i in range(args.repeats)]
    plan += [(f"new_s{s}", "new CPU", dict(engine="new", device="cpu"), s, False) for s in seeds]
    if args.gpu:
        plan.append((f"new_auto_s{seeds[0]}", "new auto", dict(engine="new", device="auto"), seeds[0], False))
    if have_original and os.name == "nt":
        plan += [("original_fixed", "original (fixed seed)", dict(engine="original"), FIXED_SEED, False),
                 ("compat_fixed", "compat (fixed seed)", dict(engine="new", compat="dll", device="cpu"), FIXED_SEED, False)]

    runs, groups = {}, {}
    for name, group, kw, seed, force in plan:
        print(f"[compare_kernels] {name} ...", flush=True)
        st = Study(args.input, name=name, root=os.path.join(runs_dir, name), timeout=args.timeout, **kw)
        r = runs[name] = st.engine(base.set("seed", seed), force=force)
        groups[name] = group
        print(f"[compare_kernels] {name}: {r.record.get('status')} {r.record.get('seconds')} s "
              f"{r.record.get('error') or ''}", flush=True)

    rows = [run_row(n, groups[n], r) for n, r in runs.items()]
    # failed runs are listed in the table and left out of the comparison (AVAS.dll crashes at random on some
    # projects); the physics needs at least one finished run on each side
    ref = [r for n, r in runs.items() if groups[n] == "reference" and r.ok]
    new = [r for n, r in runs.items() if groups[n] == "new CPU" and r.ok]
    summary = {"project": os.path.abspath(args.input), "reference": ref_name, "particles": args.particles,
               "repeats": args.repeats, "seeds": seeds, "runs": rows, "timing": timing(rows), "physics": [],
               "gpu": [], "compat_check": None, "platform": engines.platform_label(),
               "written": datetime.datetime.now().isoformat(timespec="seconds")}

    single = bool(ref and new) and all(len(r.alive) and r.alive[0] <= 1 for r in ref + new)    # one macro particle
    if single:
        summary["single"] = single_particle(ref, new)
    elif ref and new:
        z = common_z(ref + new, 600)
        st = Study(args.input, name="lattice", root=os.path.join(runs_dir, "_lattice"))
        with open(os.path.join(st.input_dir, st.lattice_name), encoding="utf-8", errors="replace") as fh:
            doc = LatticeDocument(fh.read())
        qs = QUANTITIES
        summary["physics"], table, summary["deterministic"] = physics(ref, new, z, doc, runs, qs)
        gpu = runs.get(f"new_auto_s{seeds[0]}")
        if gpu is not None and gpu.ok:
            summary["gpu"] = gpu_rows(gpu, runs[f"new_s{seeds[0]}"], z, qs)
        with open(os.path.join(out, "curves.csv"), "w", newline="", encoding="utf-8") as fh:
            w = csv.writer(fh)
            keys = list(table)
            w.writerow(keys)
            for i in range(len(z)):
                w.writerow([f"{table[k][i]:.10g}" for k in keys])
    if "original_fixed" in runs and runs["original_fixed"].ok and runs["compat_fixed"].ok:
        verdict, rel = dataset_identity(runs["original_fixed"], runs["compat_fixed"])
        summary["compat_check"] = {"verdict": verdict, "max_rel": rel}

    with open(os.path.join(out, "summary.json"), "w", encoding="utf-8") as fh:
        json.dump(summary, fh, indent=1, default=str)
    text = report(summary)
    with open(os.path.join(out, "report.md"), "w", encoding="utf-8") as fh:
        fh.write(text)
    print(text)
    if not args.keep_runs:
        shutil.rmtree(runs_dir, ignore_errors=True)
    print(f"[compare_kernels] written to {out}")
    return 0


def _num(v, fmt):
    return format(v, fmt) if isinstance(v, (int, float)) else "-"


def report(s):
    unit = {"energy": "MeV", "transmission": "1"}
    seeds = ", ".join(str(x) for x in s["seeds"])
    lines = [f"# Kernel comparison: {s['project']}", "",
             f"{s['platform']}; reference: **{s['reference']}** (randomseed 0, {s['repeats']} runs); new kernel: "
             f"corrected default mode on the CPU, randomseed {seeds}; particles: {s['particles'] or 'as in the project'}.",
             "", "## Runs", "",
             "| run | status | kernel | device | precision | compat | threads | seed | wall s | kernel s | transmission |",
             "|---|---|---|---|---|---|---|---|---|---|---|"]
    for r in s["runs"]:
        lines.append(f"| {r['run']} | {r['status']}{' - ' + r['error'] if r['error'] else ''} | {r['kernel']} | "
                     f"{r['device'] or '-'} | {r['precision'] or '-'} | {r['compat'] or '-'} | {r['threads'] or '-'} | "
                     f"{r['seed']} | {_num(r['seconds'], '.1f')} | {_num(r['kernel_seconds'], '.2f')} | "
                     f"{_num(r['transmission'], '.4f')} |")
    lines += ["", "## Timing (median)", "", "| runs | n | device | precision | threads | wall s | kernel s |",
              "|---|---|---|---|---|---|---|"]
    for t in s["timing"]:
        lines.append(f"| {t['group']} | {t['runs']} | {t['device'] or '-'} | {t['precision'] or '-'} | {t['threads'] or '-'} | "
                     f"{_num(t['wall'], '.1f')} | {_num(t['kernel'], '.2f')} |")
    lines += ["", "## Physics (new - reference)", ""]
    if s.get("single"):
        lines += ["Single particle: the synchronous particle of the first reference run and of the first new run, compared "
                  "time step by time step (deterministic: every run of a kernel is the same).", "",
                  "| quantity | reference end | new end | diff end | largest diff | rows (ref / new) |", "|---|---|---|---|---|---|"]
        for p in s["single"]:
            lines.append(f"| {p['quantity']} | {p['ref_end']:.6g} | {p['new_end']:.6g} | {p['diff_end']:.3g} | {p['diff_max']:.3g} | "
                         f"{p['rows'][0]} / {p['rows'][1]} |")
    elif not s["physics"]:
        lines.append("Not compared: no finished run on one side.")
    else:
        lines += ["Sizes, centroids and emittances relative to the reference's largest value; energy in MeV; transmission "
                  "absolute. Emittances: change from the first row (start = difference of the initial values).", ""]
        if s.get("deterministic"):
            lines += ["**Deterministic input** (a particle file or a single particle): every run of a kernel gives the same "
                      "result, so there is no statistical noise; the differences are the kernels' (corrected defects, "
                      "arithmetic), not fluctuations.", ""]
        lines += [
                  "| quantity | reference end | new end | start | diff end | largest diff | noise | verdict | first beyond |",
                  "|---|---|---|---|---|---|---|---|---|"]
        for p in s["physics"]:
            q = p["quantity"]
            factor = 1e6 if q.startswith("emit") else 1e3
            val = (lambda v: f"{v:.6g}") if q in unit else (lambda v: f"{v * factor:.5g}")
            pct = (lambda v: f"{v:.3g}") if q in unit else (lambda v: f"{v * 100:.3g} %")
            name = f"{q} [{unit.get(q, 'mm mrad' if q.startswith('emit') else 'mm')}]" + (" (growth)" if p["growth"] else "")
            lines.append(f"| {name} | {val(p['ref_end'])} | {val(p['new_end'])} | {pct(p['start'])} | {pct(p['diff_end'])} | "
                         f"{pct(p['diff_max'])} | {pct(p['noise'])} | {p['verdict']} | {p['first']} |")
    if s["gpu"]:
        lines += ["", f"## New kernel: device auto against the CPU (randomseed {s['seeds'][0]})", "",
                  "| quantity | largest diff | diff end |", "|---|---|---|"]
        for g in s["gpu"]:
            pct = (lambda v: f"{v:.3g}") if g["quantity"] in unit else (lambda v: f"{v * 100:.3g} %")
            lines.append(f"| {g['quantity']} | {pct(g['diff_max'])} | {pct(g['diff_end'])} |")
    c = s["compat_check"]
    lines += ["", "## Compatibility mode against the original kernel (same fixed seed)", ""]
    if c is None:
        lines.append("Not checked (only on Windows, where the original kernel is AVAS.dll; or a run failed).")
    else:
        lines.append(f"DataSet.txt: **{c['verdict']}**" + ("" if c["verdict"] == "identical" else f", largest relative difference {c['max_rel']:.3g}"))
    return "\n".join(lines) + "\n"


if __name__ == "__main__":
    sys.exit(main())
