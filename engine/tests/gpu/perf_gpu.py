"""Timing of the engine on the example hwr010 beam line (M6a performance table).

usage: python engine/tests/gpu/perf_gpu.py --run PATH/avas_engine_run --work DIR [--sizes 1e5,1e6,1e7]
                                           [--sc 0,1] [--label NAME] [--env KEY=VALUE ...] [--json OUT]

The beam line is two periods of the tutorial line of the example project (drift, solenoid "sol",
drift, cavity "hwr010" at -30 deg, drift; 1.72 m), the bunch is generated (GS GS, the Twiss parameters
of the example, randomseed 1), space charge SPICNIC with the default mesh.  The configuration (CPU
threads, device, precision, GPUs) comes from --env, e.g. --env AVAS_ENGINE_DEVICE=cuda
--env AVAS_ENGINE_PRECISION=fp32.  Reported per run: tracking time (SimulationTime.txt, without the
initialisation, with the output files), push + space-charge time (SimulationTime.txt "Exter"), steps (rows
of DataSet.txt), push + space-charge time per step, and the wall time of the process.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
EXAMPLE = os.path.join(REPO, "examples", "hwr010", "InputFile")

LATTICE = """start
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.3 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010
drift 0.1 0.02 0
field 0.35 0.02 0 3 0 0 0 0.3 sol
drift 0.1 0.02 0
field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010
drift 0.1 0.02 0
end
"""


def set_keys(path, changes):
    lines = open(path, encoding="utf-8").read().splitlines()
    for key, value in changes.items():
        idx = [i for i, ln in enumerate(lines) if ln.split()[:1] == [key]]
        if idx:
            lines[idx[0]] = f"{key} {value}"
        else:
            lines.append(f"{key} {value}")
    open(path, "w", encoding="utf-8", newline="\n").write("\n".join(lines) + "\n")


def prepare(d, n, sc):
    shutil.rmtree(d, ignore_errors=True)
    inp = os.path.join(d, "inputs")
    os.makedirs(inp)
    os.makedirs(os.path.join(d, "OutputFile"))
    for f in ("beam.txt", "input.txt"):
        shutil.copy(os.path.join(EXAMPLE, f), inp)
    set_keys(os.path.join(inp, "beam.txt"), {"particlenumber": int(n), "randomseed": 1, "current": 5.0 if sc else 0.0,
                                              "readparticledistribution": "unknown"})
    set_keys(os.path.join(inp, "input.txt"), {"spacecharge": 1 if sc else 0, "scmethod": "SPICNIC"})
    open(os.path.join(inp, "lattice.txt"), "w", newline="\n").write(LATTICE)
    return inp


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--run", required=True, help="avas_engine_run")
    ap.add_argument("--work", required=True)
    ap.add_argument("--sizes", default="1e5,1e6,1e7")
    ap.add_argument("--sc", default="0,1")
    ap.add_argument("--label", default="")
    ap.add_argument("--env", action="append", default=[])
    ap.add_argument("--json", default="")
    ap.add_argument("--keep", action="store_true")
    a = ap.parse_args()
    env = dict(os.environ)
    for kv in a.env:
        k, v = kv.split("=", 1)
        env[k] = v
    results = []
    for sc in [int(s) for s in a.sc.split(",")]:
        for n in [float(s) for s in a.sizes.split(",")]:
            d = os.path.join(a.work, f"perf_{int(n)}_{sc}")
            inp = prepare(d, n, sc)
            out = os.path.join(d, "OutputFile")
            t0 = time.time()
            p = subprocess.run([a.run, inp + os.sep, out + os.sep, EXAMPLE + os.sep], env=env, stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT)
            wall = time.time() - t0
            log = p.stdout.decode("latin-1", "replace")
            open(os.path.join(d, "log.txt"), "w").write(log)
            for ln in log.splitlines():
                if ln.startswith("[gpu profile]"):
                    print("   ", ln)
            dev = next((ln.strip() for ln in log.splitlines() if ln.startswith("Device:")), "Device: CPU")
            steps, track = 0, float("nan")
            ds = os.path.join(out, "DataSet.txt")
            if os.path.exists(ds):
                steps = sum(1 for ln in open(ds, encoding="latin-1") if ln.strip())
            st = os.path.join(out, "SimulationTime.txt")
            push = float("nan")
            if os.path.exists(st):
                w = open(st).read().split()
                track = float(w[0])
                push = float(w[2])  # "Exter:<blocks>:" then the time of the space-charge solves and pushes
            r = {"label": a.label, "particles": int(n), "space_charge": sc, "rc": p.returncode, "steps": steps,
                 "tracking_s": track, "push_s": push, "ms_per_step": 1000.0 * push / steps if steps else float("nan"),
                 "wall_s": wall, "device": dev}
            results.append(r)
            print(f"{a.label:<22} N={int(n):>9} sc={sc} rc={p.returncode} steps={steps:5d} push+sc {push:8.2f} s "
                  f"{r['ms_per_step']:8.2f} ms/step  tracking {track:8.2f} s  wall {wall:7.1f} s  {dev[:60]}", flush=True)
            if p.returncode != 0:
                print(log[-2000:])
            if not a.keep:
                shutil.rmtree(d, ignore_errors=True)
    if a.json:
        json.dump(results, open(a.json, "w"), indent=1)
    return 0


if __name__ == "__main__":
    sys.exit(main())
