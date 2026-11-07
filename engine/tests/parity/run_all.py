"""One-command M1 parity check of the new engine against the AVAS.dll references.

usage: python engine/tests/parity/run_all.py [--lib PATH] [--work DIR] [--strict]

1. runs every case with compat=dll and compares with refs/ (written by AVAS.dll on Windows);
2. runs every case with the default (corrected) mode and reports the synchronous phase error of
   the V3 = 0 cavities and the differences to AVAS.dll.
OMP_NUM_THREADS is set to the thread count of the reference machine so that the particle blocks
(and with them the summation order of the beam statistics) are the same as in AVAS.dll.
Exit code 0 when compat=dll agrees with the references (byte-identical with --strict, otherwise
within 1e-12 relative, the size of libm differences between platforms).
"""
import argparse
import json
import math
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
import compare  # noqa: E402


def default_lib():
    for name in ("libavas_engine.dylib", "libavas_engine.so", "avas_engine.dll"):
        for sub in ("build", os.path.join("build", "Release")):
            p = os.path.join(ENGINE, sub, name)
            if os.path.exists(p):
                return p
    raise SystemExit("engine library not found; build it first: cmake -S engine -B engine/build && cmake --build engine/build")


def run(lib, compat, work, threads):
    env = dict(os.environ, OMP_NUM_THREADS=str(threads))
    subprocess.run([sys.executable, os.path.join(HERE, "run_engine.py"), "--lib", lib, "--compat", compat,
                    "--work", work], check=True, env=env, stdout=subprocess.DEVNULL)


def lattice_elements(case):
    rows = []
    for ln in case["lattice"]:
        w = ln.split()
        if w and w[0] in ("drift", "field"):
            rows.append(w)
    return rows


def read_rows(path):
    if not os.path.exists(path):
        return []
    return [ln.split() for ln in open(path, encoding="latin-1").read().replace("\r", "").splitlines() if ln.strip()]


def default_report(doc, work):
    print("\nDefault mode (defects corrected) against AVAS.dll.  DataSet max dev = largest difference of a")
    print("column relative to the largest value of that column; exit W rel = relative change of the exit")
    print("kinetic energy of the synchronous particle.")
    print(f"{'case':<28} {'max |phi_s - target| (deg)':>27} {'phi_s shift vs DLL (deg)':>25} {'DataSet max dev':>16} {'exit W rel':>11}")
    for case in doc["cases"]:
        name = case["name"]
        if case.get("reference_only") or name.startswith("S5"):
            continue
        mine = read_rows(os.path.join(work, name, "summary", "synData.txt"))
        ref = read_rows(os.path.join(HERE, "refs", name, "synData.txt"))
        elems = lattice_elements(case)
        dev, shift = [], []
        for r, q in zip(mine, ref):
            if len(r) < 8 or r[1] != "field" or r[6] == "invalid":
                continue
            i = int(r[0])
            w = elems[i] if i < len(elems) else None
            if w and w[3] == "0":
                dev.append(abs(float(r[6]) - float(w[6])))
            shift.append(float(r[6]) - float(q[6]))
        ds_new = read_rows(os.path.join(work, name, "summary", "DataSet.txt"))
        ds_ref = read_rows(os.path.join(HERE, "refs", name, "DataSet.txt"))
        cols = range(28, 41) if name in compare.single_particle_cases() else range(41)
        mrel = 0.0  # largest difference relative to the largest |value| of its column
        for c in cols:
            ref_col = [float(r[c]) for r in ds_ref]
            scale = max((abs(v) for v in ref_col if math.isfinite(v)), default=0.0)
            diff = max((abs(float(a[c]) - float(b[c])) for a, b in zip(ds_ref, ds_new)
                        if math.isfinite(float(a[c])) and math.isfinite(float(b[c]))), default=0.0)
            if scale:
                mrel = max(mrel, diff / scale)
        gz = lambda rows: float(rows[-1][5]) if rows else float("nan")  # exit gamma*beta
        wn, wr = gz(mine), gz(ref)
        wrel = abs(math.hypot(1, wn) - math.hypot(1, wr)) / (math.hypot(1, wr) - 1)
        dv = f"{max(dev):.2e}" if dev else "-"
        sh = ", ".join(f"{s:+.4f}" for s in shift) if shift else "-"
        print(f"{name:<28} {dv:>27} {sh:>25} {mrel:>16.3g} {wrel:>11.2e}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lib", default=None)
    ap.add_argument("--work", default=None)
    ap.add_argument("--strict", action="store_true")
    a = ap.parse_args()
    lib = os.path.abspath(a.lib or default_lib())
    work = a.work or tempfile.mkdtemp(prefix="avas_parity_")
    doc = json.load(open(os.path.join(HERE, "cases.json"), encoding="utf-8"))
    threads = doc.get("reference_omp_threads", 12)
    print(f"library: {lib}\nwork:    {work}")
    run(lib, "dll", os.path.join(work, "dll"), threads)
    run(lib, "none", os.path.join(work, "none"), threads)
    print("\ncompat=dll against AVAS.dll references (differences only):")
    sys.argv = ["compare.py", os.path.join(work, "dll"), "--quiet"]
    identical = compare.main() == 0
    default_report(doc, os.path.join(work, "none"))
    if identical:
        return 0
    if a.strict:
        return 1
    # tolerate last-bit libm differences on other platforms
    worst = 0.0
    for case in sorted(os.listdir(os.path.join(HERE, "refs"))):
        for f in os.listdir(os.path.join(HERE, "refs", case)):
            rp, np_ = os.path.join(HERE, "refs", case, f), os.path.join(work, "dll", case, "summary", f)
            if f in ("console.txt", "files.txt") or not os.path.exists(np_):
                continue
            if f.endswith(".dst.txt"):
                ok, _, mrel, _, _ = compare.compare_dst(compare.read(rp), compare.read(np_))
            else:
                cols = set(range(28, 41)) if (f == "DataSet.txt" and case in compare.single_particle_cases()) else None
                ok, _, mrel, _, note = compare.compare_text(compare.read(rp), compare.read(np_), cols)
                if note.startswith("lines"):
                    mrel = math.inf
            worst = max(worst, mrel)
    print(f"\nlargest relative difference to AVAS.dll: {worst:.3g}")
    return 0 if worst < 1e-12 else 1


if __name__ == "__main__":
    sys.exit(main())
