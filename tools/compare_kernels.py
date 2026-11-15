"""Compare the original kernel and the new kernel on a project (M8a: before the new kernel becomes the default).

Run from the repository root:

    .venv\\Scripts\\python.exe tools\\compare_kernels.py --input <project or its InputFile> [--out DIR]
        [--particles N] [--seeds 1,2] [--gpu] [--keep-runs]

Every run goes through :class:`avas.research.Study` on a copy of the inputs in ``--out``; the
project's InputFile and OutputFile are never written.  Runs, one after the other (fair timing):

* ``original`` twice with ``randomseed 0`` – the reference and its run-to-run fluctuation.  A
  fixed seed is not used here: the original kernel then generates a distribution with x fully
  correlated with the phase (a known defect fixed in the new kernel).
* ``new`` (the corrected default mode, CPU) with ``--seeds`` (two seeds) – the new kernel and its
  fluctuation;
* with ``--gpu``: ``new`` with ``device auto`` (first seed), for the time and the GPU result;
* ``original`` and ``new --compat dll`` with the same fixed seed – whether compatibility mode
  reproduces the original kernel (byte-identical on Windows; within about 1e-10 on Linux).

Where the original kernel does not run (macOS, ARM), compatibility mode stands in for it as the
reference (it is the original kernel's behaviour) and the byte check is skipped.

The physics table compares the mean of the new runs with the mean of the reference runs on a
common z grid: rms sizes, normalised emittances, energy and transmission.  ``noise`` is the larger
of the two run-to-run differences; a difference more than twice the noise is marked ``beyond``
(caused by the kernels, not by sampling) and the first z where it appears is named with the
lattice element there.  Sizes and emittances are relative to the reference's largest value, energy
in MeV, transmission absolute.  A project that reads its distribution from a file has no sampling
noise; every difference is then the kernels'.

Written to ``--out``: ``report.md``, ``summary.json``, ``curves.csv`` (z and every quantity of
every run).  The run folders (``runs/``) are deleted unless ``--keep-runs``.
"""
import argparse
import csv
import datetime
import json
import os
import shutil
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
    best = None
    for st in doc.elements():
        if st.z_start is None or st.z_end is None:
            continue
        if st.z_start - 1e-9 <= z <= st.z_end + 1e-9:
            best = st  # the innermost / last one listed
    if best is None:
        return ""
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
    both = np.isfinite(da) & np.isfinite(db)
    if (np.isfinite(da) != np.isfinite(db)).any():
        return "different NaN pattern", float("inf")
    den = np.maximum(np.abs(da), np.abs(db))
    rel = np.where(both & (den > 0), np.abs(da - db) / np.where(den > 0, den, 1), 0.0)
    return "different", float(rel.max())


def run_row(name, run):
    eng = (run.record.get("engine") or {})
    act = eng.get("actual") or {}
    return {"run": name, "status": run.record.get("status"), "error": run.record.get("error"),
            "kernel": act.get("kernel") or eng.get("kernel"), "device": act.get("device"),
            "precision": act.get("precision"), "compat": act.get("compat"), "threads": act.get("threads"),
            "seed": run.record.get("seed"), "seconds": run.record.get("seconds"),
            "transmission": run.transmission}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--input", required=True, help="project folder or its InputFile")
    ap.add_argument("--out", help="report folder (default kernel_compare_<time> in the current folder)")
    ap.add_argument("--particles", type=int, help="particlenumber of every run (default: the project's)")
    ap.add_argument("--seeds", default="1,2", help="two randomseed values of the new kernel's runs")
    ap.add_argument("--gpu", action="store_true", help="also run the new kernel with device auto")
    ap.add_argument("--keep-runs", action="store_true", help="keep the run folders in <out>/runs")
    ap.add_argument("--timeout", type=float, default=None, help="seconds per run")
    args = ap.parse_args(argv)

    out = os.path.abspath(args.out or "kernel_compare_" + datetime.datetime.now().strftime("%Y%m%d_%H%M%S"))
    runs_dir = os.path.join(out, "runs")
    os.makedirs(runs_dir, exist_ok=True)
    seeds = [int(s) for s in args.seeds.split(",")][:2]
    if len(seeds) != 2:
        ap.error("--seeds needs two values")
    base = Variant(particles=args.particles) if args.particles else Variant()
    have_original = engines.original_available()

    def study(name, **engine):
        return Study(args.input, name=name, root=os.path.join(runs_dir, name), timeout=args.timeout, **engine)

    ref_kw = dict(engine="original") if have_original else dict(engine="new", compat="dll", device="cpu")
    ref_name = "original" if have_original else "compat"
    plan = [
        (f"{ref_name}_r1", ref_kw, 0, True),
        (f"{ref_name}_r2", ref_kw, 0, True),
        (f"new_s{seeds[0]}", dict(engine="new", device="cpu"), seeds[0], False),
        (f"new_s{seeds[1]}", dict(engine="new", device="cpu"), seeds[1], False),
    ]
    if args.gpu:
        plan.append((f"new_auto_s{seeds[0]}", dict(engine="new", device="auto"), seeds[0], False))
    if have_original:
        plan += [("original_fixed", dict(engine="original"), FIXED_SEED, False),
                 ("compat_fixed", dict(engine="new", compat="dll", device="cpu"), FIXED_SEED, False)]

    runs = {}
    for name, kw, seed, force in plan:
        print(f"[compare_kernels] {name} ...", flush=True)
        st = study(name, **kw)
        runs[name] = st.engine(base.set("seed", seed), force=force)
        r = runs[name]
        print(f"[compare_kernels] {name}: {r.record.get('status')} {r.record.get('seconds')} s "
              f"{r.record.get('error') or ''}", flush=True)

    rows = [run_row(n, r) for n, r in runs.items()]
    ref = [runs[f"{ref_name}_r1"], runs[f"{ref_name}_r2"]]
    new = [runs[f"new_s{seeds[0]}"], runs[f"new_s{seeds[1]}"]]
    summary = {"project": os.path.abspath(args.input), "reference": ref_name, "particles": args.particles,
               "seeds": seeds, "runs": rows, "physics": [], "compat_check": None,
               "platform": engines.platform_label(), "written": datetime.datetime.now().isoformat(timespec="seconds")}

    good = [r for r in ref + new if r.ok]
    if len(good) == 4:
        z = common_z(good, 600)
        st = Study(args.input, name="lattice", root=os.path.join(runs_dir, "_lattice"))
        with open(os.path.join(st.input_dir, st.lattice_name), encoding="utf-8", errors="replace") as fh:
            doc = LatticeDocument(fh.read())
        table = {"z": z}
        for q in QUANTITIES:
            cr = [curve(r, q, z) for r in ref]
            cn = [curve(r, q, z) for r in new]
            for name, r in runs.items():
                if r.ok:
                    table[f"{q}:{name}"] = curve(r, q, z)
            mr, mn = (cr[0] + cr[1]) / 2, (cn[0] + cn[1]) / 2
            s = scale_of(q, mr)
            diff = (mn - mr) / s
            noise = max(np.nanmax(np.abs(cr[0] - cr[1])), np.nanmax(np.abs(cn[0] - cn[1]))) / s
            mx = float(np.nanmax(np.abs(diff)))
            beyond = mx > BEYOND * noise and mx > 1e-12
            first = ""
            if beyond:
                i = int(np.flatnonzero(np.abs(diff) > BEYOND * noise)[0])
                first = f"z = {z[i]:.4f} m, {element_at(doc, z[i])}"
            summary["physics"].append({"quantity": q, "ref_end": float(mr[-1]), "new_end": float(mn[-1]),
                                       "diff_end": float(diff[-1]), "diff_max": mx, "noise": float(noise),
                                       "verdict": "beyond" if beyond else "within", "first": first})
        with open(os.path.join(out, "curves.csv"), "w", newline="", encoding="utf-8") as fh:
            w = csv.writer(fh)
            keys = list(table)
            w.writerow(keys)
            for i in range(len(z)):
                w.writerow([f"{table[k][i]:.10g}" for k in keys])
    if have_original and runs["original_fixed"].ok and runs["compat_fixed"].ok:
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


def report(s):
    unit = {"energy": "MeV", "transmission": "1"}
    lines = [f"# Kernel comparison: {s['project']}", "",
             f"{s['platform']}; reference: **{s['reference']}** (randomseed 0, two runs); new kernel: corrected "
             f"default mode on the CPU, randomseed {s['seeds'][0]} and {s['seeds'][1]}; particles: "
             f"{s['particles'] or 'as in the project'}.", "", "## Runs", "",
             "| run | status | kernel | device | precision | compat | threads | seed | seconds | transmission |",
             "|---|---|---|---|---|---|---|---|---|---|"]
    for r in s["runs"]:
        sec = f"{r['seconds']:.1f}" if isinstance(r["seconds"], (int, float)) else "-"
        tr = f"{r['transmission']:.4f}" if isinstance(r["transmission"], (int, float)) else "-"
        lines.append(f"| {r['run']} | {r['status']}{' – ' + r['error'] if r['error'] else ''} | {r['kernel']} | "
                     f"{r['device'] or '-'} | {r['precision'] or '-'} | {r['compat'] or '-'} | {r['threads'] or '-'} | "
                     f"{r['seed']} | {sec} | {tr} |")
    lines += ["", "## Physics (new − reference)", ""]
    if not s["physics"]:
        lines.append("Not compared: a run failed.")
    else:
        lines += ["Sizes and emittances relative to the reference's largest value; energy in MeV; transmission absolute.",
                  "", "| quantity | reference end | new end | diff end | largest diff | noise | verdict | first beyond |",
                  "|---|---|---|---|---|---|---|---|"]
        for p in s["physics"]:
            q = p["quantity"]
            factor = 1e6 if q.startswith("emit") else 1e3
            fmt = (lambda v: f"{v:.6g}") if q in unit else (lambda v: f"{v * factor:.5g}")
            pct = (lambda v: f"{v:.3g}") if q in unit else (lambda v: f"{v * 100:.3g} %")
            lines.append(f"| {q} [{unit.get(q, 'mm' if q.startswith('rms') else 'mm·mrad')}] | {fmt(p['ref_end'])} | "
                         f"{fmt(p['new_end'])} | {pct(p['diff_end'])} | {pct(p['diff_max'])} | {pct(p['noise'])} | "
                         f"{p['verdict']} | {p['first']} |")
    c = s["compat_check"]
    lines += ["", "## Compatibility mode against the original kernel (same fixed seed)", ""]
    if c is None:
        lines.append("Not checked (the original kernel does not run here, or a run failed).")
    else:
        lines.append(f"DataSet.txt: **{c['verdict']}**" + ("" if c["verdict"] == "identical" else f", largest relative difference {c['max_rel']:.3g}"))
    return "\n".join(lines) + "\n"


if __name__ == "__main__":
    sys.exit(main())
