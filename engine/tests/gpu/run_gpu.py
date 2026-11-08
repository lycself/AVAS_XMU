"""GPU back end checks (M6a): the parity cases in the corrected (default) mode on the CPU and on the GPU.

usage: python engine/tests/gpu/run_gpu.py --lib PATH [--work DIR] [--gpus 0] [--gpus2 0,1]
                                          [--mpi "mpirun -np 2"] [--only a,b] [--fp32] [--stats SEEDS]

fp64 (always):
  * every case that runs in the default mode, once on the CPU (AVAS_ENGINE_DEVICE=cpu) and once on the GPU
    (AVAS_ENGINE_DEVICE=cuda, AVAS_ENGINE_PRECISION=fp64);
  * cases without space charge: every output file (OutputFile and the error samples output_<k>/, time
    stamps removed) must be byte-identical to the CPU;
  * cases with space charge: a second GPU run, a run on the GPUs of --gpus2 and a run with --mpi must be
    byte-identical to the first GPU run; the difference to the CPU is reported (largest relative
    difference of DataSet.txt and of the particle coordinates of outData_*.dst).
--fp32: the cases again in fp32: errors of single particles against the CPU, statistics of multi-particle
  runs; with --stats N also N seeds of a 20000-particle bunch on the CPU and in fp32 (rms sizes,
  emittances, transmission at the exit; the fp32 mean must lie within 3 standard errors of the CPU mean).
Exit code 0 when every required check passes.
"""
import argparse
import json
import math
import os
import re
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
PARITY = os.path.abspath(os.path.join(HERE, "..", "parity"))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
TIMED = {"SimulationTime.txt", "errorData.seed.txt", "inData.seed.txt"}  # timings; seeds taken from the clock
STAMPED = {"synParticle.txt": re.compile(rb"^\d{4}/\d\d/\d\d \d\d:\d\d:\d\d"),
           "ErrorLog.txt": re.compile(rb"^\d{4}/\d\d/\d\d \d\d:\d\d:\d\d")}


def run_cases(lib, work, env_extra, only=None, launcher=None, cases=None):
    env = dict(os.environ, **env_extra)
    if launcher:
        env["AVAS_PARITY_LAUNCHER"] = launcher
    cmd = [sys.executable, os.path.join(PARITY, "run_engine.py"), "--lib", lib, "--compat", "none", "--work", work]
    if only:
        cmd += ["--only", ",".join(only)]
    if cases:
        cmd += ["--cases", cases]
    subprocess.run(cmd, check=True, env=env, stdout=subprocess.DEVNULL)


def norm(name, data):
    pat = STAMPED.get(name)
    if pat is None:
        return data
    return b"\n".join(pat.sub(b"", ln) for ln in data.replace(b"\r\n", b"\n").split(b"\n"))


def output_files(d):
    out = {}
    root = os.path.join(d, "OutputFile")
    for dp, _, fs in os.walk(root):
        for f in fs:
            if f in TIMED:
                continue
            p = os.path.join(dp, f)
            out[os.path.relpath(p, root)] = p
    return out


def identical(a, b):
    fa, fb = output_files(a), output_files(b)
    if set(fa) != set(fb):
        return False, "file lists differ: " + " ".join(sorted(set(fa) ^ set(fb)))[:200]
    bad = [k for k in sorted(fa) if norm(os.path.basename(k), open(fa[k], "rb").read()) !=
           norm(os.path.basename(k), open(fb[k], "rb").read())]
    return not bad, " ".join(bad[:6])


def to_float(t):
    try:
        return float(t)
    except ValueError:
        return None


def dataset_rel(a, b):
    """Largest |difference| of a DataSet.txt column relative to the largest |value| of the column
    (every DataSet.txt of OutputFile and of the error samples)."""
    worst = 0.0
    fb = output_files(b)
    for k, pa in output_files(a).items():
        if os.path.basename(k) != "DataSet.txt":
            continue
        if k not in fb:
            return math.inf
        ra = [ln.split() for ln in open(pa, encoding="latin-1") if ln.strip()]
        rb = [ln.split() for ln in open(fb[k], encoding="latin-1") if ln.strip()]
        if len(ra) != len(rb):
            return math.inf
        for c in range(min(len(ra[0]) if ra else 0, 41)):
            col = [to_float(r[c]) for r in ra]
            scale = max((abs(v) for v in col if v is not None and math.isfinite(v)), default=0.0)
            if not scale:
                continue
            for x, y in zip(ra, rb):
                u, v = to_float(x[c]), to_float(y[c])
                if u is None or v is None or not (math.isfinite(u) and math.isfinite(v)):
                    continue
                worst = max(worst, abs(u - v) / scale)
    return worst


def read_dst(p):
    raw = open(p, "rb").read()
    n = struct.unpack("<i", raw[2:6])[0]
    return [struct.unpack("<6d", raw[23 + 48 * i: 71 + 48 * i]) for i in range(n)]


def dst_rel(a, b):
    """Largest |difference| of the particle coordinates of the outData_*.dst files relative to the rms
    of the coordinate (x, x', y, y', phi, W)."""
    worst = 0.0
    fa = {k: v for k, v in output_files(a).items() if k.endswith(".dst")}
    fb = output_files(b)
    for k, p in fa.items():
        if k not in fb:
            return math.inf
        ra, rb = read_dst(p), read_dst(fb[k])
        if len(ra) != len(rb):
            return math.inf
        for c in range(6):
            col = [r[c] for r in ra]
            if not col:
                continue
            m = sum(col) / len(col)
            rms = math.sqrt(sum((v - m) ** 2 for v in col) / len(col)) or 1.0
            worst = max(worst, max(abs(x[c] - y[c]) for x, y in zip(ra, rb)) / rms)
    return worst


def default_cases(doc):
    names = []
    for c in doc["cases"]:
        if c.get("reference_only"):
            continue
        names.append(c["name"])
    return names


def has_sc(doc, case):
    v = case.get("input", {}).get("spacecharge", doc.get("common_input", {}).get("spacecharge", 1))
    return str(v) == "1"


def device_line(d):
    p = os.path.join(d, "log.txt")
    if not os.path.exists(p):
        return ""
    for ln in open(p, encoding="utf-8", errors="replace"):
        if ln.startswith("Device:"):
            return ln.strip()
    return ""


# ---- fp32 statistics: N seeds of a generated bunch ----

STATS_LATTICE = ["start", "drift 0.1 0.02 0", "field 0.35 0.02 0 3 0 0 0 0.3 sol",
                 "drift 0.1 0.02 0", "field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010", "drift 0.1 0.02 0",
                 "field 0.35 0.02 0 3 0 0 0 0.3 sol", "drift 0.1 0.02 0",
                 "field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010", "drift 0.1 0.02 0", "end"]


def stats_cases(seeds, sc):
    cases = []
    for s in range(1, seeds + 1):
        cases.append({"name": f"STAT_{'sc' if sc else 'nosc'}_{s}",
                      "beam": {"randomseed": 1000 + s, "particlenumber": 20000, "current": 5.0 if sc else 0.0},
                      "input": {"spacecharge": 1 if sc else 0}, "lattice": STATS_LATTICE})
    return {"common_input": {"spacecharge": 0}, "cases": cases}


def exit_stats(d):
    p = os.path.join(d, "OutputFile", "DataSet.txt")
    rows = [ln.split() for ln in open(p, encoding="latin-1") if ln.strip()]
    r = rows[-1]
    return {"rms x": float(r[16]), "rms y": float(r[18]), "rms z": float(r[20]), "eps x": float(r[13]),
            "eps y": float(r[14]), "eps z": float(r[15]), "alive": float(r[28])}


def metal_report(lib, work, names, gpu_env, cpu, g1, by_name):
    g2 = os.path.join(work, "metal_again")
    run_cases(lib, g2, gpu_env, names)
    # the fp32 reference on the CPU: the Metal kernels compiled as C++ (emu, precision fp32)
    e32 = os.path.join(work, "emu_fp32")
    run_cases(lib, e32, {"AVAS_ENGINE_DEVICE": "emu", "AVAS_ENGINE_PRECISION": "fp32", "AVAS_ENGINE_GPUS": ""}, names)
    nref, notref = 0, []
    fails, worst_single, worst_ds, worst_dst = 0, 0.0, 0.0, 0.0
    print("\nMetal (fp32 tier) against the CPU (DataSet: largest column difference / column maximum; dst: / rms)"
          " and a second Metal run (byte-identical required):")
    for n in names:
        same, why = identical(os.path.join(g1, n), os.path.join(g2, n))
        fails += 0 if same else 1
        ref, rwhy = identical(os.path.join(g1, n), os.path.join(e32, n))
        nref += ref
        if not ref:
            notref.append(f"{n}: {rwhy[:60]}")
        single = by_name[n].get("beam", {}).get("particlenumber") in (1, "1")
        ds = dataset_rel(os.path.join(cpu, n), os.path.join(g1, n))
        dd = dst_rel(os.path.join(cpu, n), os.path.join(g1, n))
        if math.isfinite(ds):
            worst_ds = max(worst_ds, ds)
        if math.isfinite(dd):
            if single:
                worst_single = max(worst_single, dd)
            else:
                worst_dst = max(worst_dst, dd)
        print(f"  {n:<32} rerun {'same' if same else 'DIFF ' + why[:40]:<6} DataSet {ds:10.3g}   dst {dd:10.3g}"
              f"{'   (single particle)' if single else ''}")
    print(f"  largest: DataSet {worst_ds:.3g}, dst {worst_dst:.3g} rms, single particles {worst_single:.3g} rms")
    print(f"\nMetal against the fp32 reference on the CPU (emu, precision fp32), byte-identical: {nref}/{len(names)}")
    for line in notref:
        print(f"  differs: {line}")
    print(f"\n{'ALL REQUIRED CHECKS PASSED' if not fails else f'FAILURES: {fails}'}")
    return 1 if fails else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lib", required=True)
    ap.add_argument("--work", default=None)
    ap.add_argument("--gpus", default="0")
    ap.add_argument("--gpus2", default="", help="two GPUs, e.g. 0,1: space-charge cases must not change")
    ap.add_argument("--mpi", default="", help='launcher for an MPI run, e.g. "mpirun -np 2"')
    ap.add_argument("--only", default="")
    ap.add_argument("--fp32", action="store_true")
    ap.add_argument("--stats", type=int, default=0)
    ap.add_argument("--skip-cpu", action="store_true", help="reuse WORK/cpu")
    ap.add_argument("--device", default="cuda", help="cuda, or emu (the device back end on the CPU, for tests)")
    a = ap.parse_args()
    lib = os.path.abspath(a.lib)
    work = os.path.abspath(a.work or tempfile.mkdtemp(prefix="avas_gpu_"))
    if not a.work:
        # a temporary work folder is removed at the end; --work DIR keeps the outputs
        import atexit
        import shutil
        atexit.register(shutil.rmtree, work, True)
    doc = json.load(open(os.path.join(PARITY, "cases.json"), encoding="utf-8"))
    names = default_cases(doc)
    if a.only:
        keep = set(a.only.split(","))
        names = [n for n in names if n in keep]
    by_name = {c["name"]: c for c in doc["cases"]}
    sc_names = [n for n in names if has_sc(doc, by_name[n])]
    print(f"library: {lib}\nwork:    {work}\ncases:   {len(names)} ({len(sc_names)} with space charge)")
    cpu = os.path.join(work, "cpu")
    gpu_env = {"AVAS_ENGINE_DEVICE": a.device, "AVAS_ENGINE_PRECISION": "fp64", "AVAS_ENGINE_GPUS": a.gpus}
    if not a.skip_cpu:
        run_cases(lib, cpu, {"AVAS_ENGINE_DEVICE": "cpu"}, names)
    g1 = os.path.join(work, "gpu64")
    run_cases(lib, g1, gpu_env, names)
    print("device:", device_line(os.path.join(g1, names[0])) if names else "-")
    fails = 0
    if a.device == "metal":
        # Metal (no fp64 on the GPU): the t-code push is the fp32 tier, so every case is compared
        # like --fp32 against the CPU; a second run must be byte-identical
        return metal_report(lib, work, names, gpu_env, cpu, g1, by_name)
    print("\nfp64: cases without space charge, GPU against the CPU (byte-identical required):")
    nsame = 0
    for n in names:
        if n in sc_names:
            continue
        ok, why = identical(os.path.join(cpu, n), os.path.join(g1, n))
        nsame += ok
        if not ok:
            fails += 1
            print(f"  FAIL {n}: {why}")
    print(f"  identical: {nsame}/{len(names) - len(sc_names)}")
    if sc_names:
        g2 = os.path.join(work, "gpu64_again")
        run_cases(lib, g2, gpu_env, sc_names)
        runs = [("second GPU run", g2)]
        if a.gpus2:
            g3 = os.path.join(work, "gpu64_x2")
            run_cases(lib, g3, dict(gpu_env, AVAS_ENGINE_GPUS=a.gpus2), sc_names)
            runs.append((f"GPUs {a.gpus2}", g3))
        if a.mpi:
            g4 = os.path.join(work, "gpu64_mpi")
            run_cases(lib, g4, dict(gpu_env, AVAS_ENGINE_GPUS=""), sc_names, launcher=a.mpi)  # one GPU per rank
            runs.append((a.mpi, g4))
        print("\nfp64: cases with space charge")
        print(f"  {'case':<28} " + " ".join(f"{lbl[:14]:>14}" for lbl, _ in runs) +
              f" {'=CPU':>5} {'DataSet vs CPU':>15} {'dst vs CPU (rms)':>17}")
        for n in sc_names:
            cells = []
            for lbl, d in runs:
                ok, _ = identical(os.path.join(g1, n), os.path.join(d, n))
                cells.append("same" if ok else "DIFF")
                fails += 0 if ok else 1
            same_cpu, _ = identical(os.path.join(cpu, n), os.path.join(g1, n))
            print(f"  {n:<28} " + " ".join(f"{c:>14}" for c in cells) +
                  f" {'yes' if same_cpu else 'no':>5} {dataset_rel(os.path.join(cpu, n), os.path.join(g1, n)):>15.3g}"
                  f" {dst_rel(os.path.join(cpu, n), os.path.join(g1, n)):>17.3g}")
    if a.fp32:
        g32 = os.path.join(work, "gpu32")
        run_cases(lib, g32, dict(gpu_env, AVAS_ENGINE_PRECISION="fp32"), names)
        print("\nfp32 against the CPU (DataSet: largest column difference / column maximum; dst: / rms):")
        worst_single = 0.0
        for n in names:
            single = by_name[n].get("beam", {}).get("particlenumber") in (1, "1")
            ds = dataset_rel(os.path.join(cpu, n), os.path.join(g32, n))
            dd = dst_rel(os.path.join(cpu, n), os.path.join(g32, n))
            if single:
                worst_single = max(worst_single, dd if math.isfinite(dd) else 0.0)
            print(f"  {n:<32} DataSet {ds:10.3g}   dst {dd:10.3g}{'   (single particle)' if single else ''}")
        print(f"  single-particle cases: largest coordinate difference {worst_single:.3g} rms")
    if a.stats:
        for sc in (False, True):
            spec = os.path.join(work, f"stats_{'sc' if sc else 'nosc'}.json")
            json.dump(stats_cases(a.stats, sc), open(spec, "w"))
            sc_cpu = os.path.join(work, f"stats_cpu_{sc}")
            sc_gpu = os.path.join(work, f"stats_gpu32_{sc}")
            run_cases(lib, sc_cpu, {"AVAS_ENGINE_DEVICE": "cpu"}, cases=spec)
            run_cases(lib, sc_gpu, dict(gpu_env, AVAS_ENGINE_PRECISION="fp32"), cases=spec)
            cs = [exit_stats(os.path.join(sc_cpu, c["name"])) for c in stats_cases(a.stats, sc)["cases"]]
            gs = [exit_stats(os.path.join(sc_gpu, c["name"])) for c in stats_cases(a.stats, sc)["cases"]]
            print(f"\nfp32 statistics, {a.stats} seeds, 20000 particles, {'5 mA space charge' if sc else 'no space charge'}:")
            for key in cs[0]:
                cv = [s[key] for s in cs]
                gv = [s[key] for s in gs]
                mc, mg = sum(cv) / len(cv), sum(gv) / len(gv)
                sd = math.sqrt(sum((v - mc) ** 2 for v in cv) / max(len(cv) - 1, 1))
                se = sd / math.sqrt(len(cv))
                dpair = max(abs(x - y) for x, y in zip(cv, gv))
                ok = abs(mg - mc) <= 3 * se + 1e-12 * abs(mc) or key == "alive" and abs(mg - mc) <= 3 * se + 1
                fails += 0 if ok else 1
                print(f"  {key:<6} CPU {mc:.6g}  fp32 {mg:.6g}  seed spread {sd:.3g}  |diff of means| {abs(mg - mc):.3g}"
                      f"  largest same-seed diff {dpair:.3g}  {'ok' if ok else 'FAIL'}")
    print(f"\n{'ALL REQUIRED CHECKS PASSED' if not fails else f'FAILURES: {fails}'}")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
