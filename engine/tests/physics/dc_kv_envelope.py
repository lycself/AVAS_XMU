"""DC KV beam in a 1 m drift with space charge (new engine, default mode) against the KV envelope
equations of avas/dynamics/kv.py.

usage: python engine/tests/physics/dc_kv_envelope.py [--lib PATH] [--check] [--grid "30 30 30"] ...

The dc bunch of the engine is one RF period long (phases uniform over 2 pi); with frequency
32.5 MHz it is 0.53 m long, 65 beam radii, so particles in its middle half (selected by the entrance
phase, followed by index) see the field of an infinitely long beam.  Their entrance moments
(inData.dst) give the KV start (a = 2 rms, a' = 2 <xx'>/rms, full emittance 4 eps_rms) and, drifted
without force, the zero-current reference; the space-charge growth of the exit rms (outData.dst)
is compared with the KV envelope.  --check runs a mesh / particle sweep with thresholds.

--periodic-check (M10 D7): at 1.3 GHz the dc bunch is only 13 mm long (7 beam radii), so its ends matter.  With
lpboundary 0 1 (periodic longitudinal box of one RF period, gamma beta lambda in the rest frame) every particle,
not only the middle half, must follow the KV envelope of the infinite beam (within 2 %); the same bunch with the
open FFT box falls clearly short of it (the end effects the periodic box removes).
"""
import argparse
import math
import os
import shutil
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
sys.path.insert(0, REPO)
sys.path.insert(0, os.path.join(REPO, "engine", "tests", "parity"))
from avas.dynamics import kv  # noqa: E402
import run_engine  # noqa: E402

LENGTH = 1.0


def read_dst(path):
    raw = open(path, "rb").read()
    n = struct.unpack("<i", raw[2:6])[0]
    return [struct.unpack("<6d", raw[23 + 48 * i: 71 + 48 * i]) for i in range(n)]


def moments(q, p):
    n = len(q)
    mq, mp = sum(q) / n, sum(p) / n
    sqq = sum((a - mq) ** 2 for a in q) / n
    spp = sum((b - mp) ** 2 for b in p) / n
    sqp = sum((a - mq) * (b - mp) for a, b in zip(q, p)) / n
    return sqq, sqp, spp


def run_case(lib, current, scmethod, grid, particles, frequency, negligible=None, meshrms=None, extra=None, middle=True):
    work = tempfile.mkdtemp(prefix="avas_dckv_")
    base = os.path.join(REPO, "examples", "hwr010", "InputFile")
    inp, out = os.path.join(work, "inputs"), os.path.join(work, "OutputFile")
    os.makedirs(inp)
    os.makedirs(out)
    for f in ("beam.txt", "input.txt"):
        shutil.copy(os.path.join(base, f), inp)
    run_engine.set_keys(os.path.join(inp, "beam.txt"), {
        "beamtype": "dc", "distribution": "KV GS", "particlenumber": particles, "current": current,
        "randomseed": 1, "frequency": frequency})
    keys = {"spacecharge": 1, "scmethod": scmethod}
    if grid:
        keys["numofgrid"] = grid
    if negligible is not None:
        keys["negligibleratio"] = negligible
    if meshrms:
        keys["meshrms"] = meshrms
    keys.update(extra or {})
    run_engine.set_keys(os.path.join(inp, "input.txt"), keys)
    with open(os.path.join(inp, "lattice.txt"), "w") as fl:
        fl.write("\n".join(["start", f"drift {LENGTH} 0.05 0", "end", ""]))
    try:
        subprocess.run([sys.executable, "-c", run_engine.CHILD, lib, inp + os.sep, out + os.sep, base + os.sep, "-"],
                       check=False, stdout=subprocess.DEVNULL)
        beam = {}
        for ln in open(os.path.join(inp, "beam.txt")):
            w = ln.split("!")[0].split()
            if w:
                beam[w[0].lower()] = w[1:]
        ent = read_dst(os.path.join(out, "inData.dst"))
        ext = read_dst(os.path.join(out, [f for f in os.listdir(out) if f.startswith("outData_")][0]))
    finally:
        shutil.rmtree(work, ignore_errors=True)  # also when the engine failed
    if len(ext) != len(ent):
        raise SystemExit(f"particles lost: {len(ent)} -> {len(ext)}")
    phis = sorted(r[4] for r in ent)
    lo, hi = (phis[len(phis) // 4], phis[3 * len(phis) // 4]) if middle else (phis[0], phis[-1])
    idx = [i for i, r in enumerate(ent) if lo <= r[4] <= hi]
    w0, mc2 = float(beam["kneticenergy"][0]), 938.272029
    perv = kv.perveance(current, w0, mc2)
    res = {}
    start, eps = [], []
    for c in (0, 2):
        q = [ent[i][c] / 100 for i in idx]
        p = [ent[i][c + 1] for i in idx]
        sqq, sqp, spp = moments(q, p)
        e = math.sqrt(sqq * spp - sqp * sqp)
        start += [2 * math.sqrt(sqq), 2 * sqp / math.sqrt(sqq)]
        eps.append(4 * e)
        z0 = math.sqrt(sqq + 2 * sqp * LENGTH + spp * LENGTH * LENGTH)   # zero current: straight lines
        sx = math.sqrt(moments([ext[i][c] / 100 for i in idx], [0.0] * len(idx))[0])
        res[c] = [z0, sx]
    _, u = kv.track([(LENGTH, 0.0, 0.0)], perv, tuple(eps), tuple(start), points=50)
    result = {}
    for c in (0, 2):
        z0, sx = res[c]
        skv = u[-1, c] / 2
        result["xy"[c // 2]] = (sx, skv, z0, (sx - z0) / (skv - z0))
    return perv, len(idx), result


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lib", default=None)
    ap.add_argument("--current", type=float, default=20.0)
    ap.add_argument("--scmethod", default="SPICNIC")
    ap.add_argument("--grid", default="")
    ap.add_argument("--particles", type=int, default=20000)
    ap.add_argument("--frequency", type=float, default=32.5e6)
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--periodic-check", action="store_true", help="lpboundary 0 1 against the open box (M10 D7)")
    ap.add_argument("--negligible", default=None)
    ap.add_argument("--meshrms", default="2.5 2.5 2.0",
                    help="mesh half width in rms; the KV edge is at 2 rms, the dc bunch end at 1.73 rms_z")
    a = ap.parse_args()
    lib = a.lib or next(p for p in (os.path.join(REPO, "engine", "build", n) for n in
                                     ("libavas_engine.dylib", "libavas_engine.so", "avas_engine.dll",
                                      os.path.join("Release", "avas_engine.dll"))) if os.path.exists(p))
    if a.periodic_check:
        f = 1.3e9
        print(f"DC KV beam, {a.current} mA, 1 m drift, bunch = one period at {f / 1e6:g} MHz (13 mm), FFT 32^3, all particles:")
        res = {}
        for label, extra in (("periodic (lpboundary 0 1)", {"lpboundary": "0 1"}), ("open box", {})):
            perv, nall, o = run_case(lib, a.current, "FFT", "32 32 32", 40000, f, None, a.meshrms, extra, middle=False)
            res[label] = (o["x"][3], o["y"][3])
            print(f"  {label}: {nall} particles, K = {perv:.3e}; growth ratio x {o['x'][3]:.4f}, y {o['y'][3]:.4f}")
        per, opn = res["periodic (lpboundary 0 1)"], res["open box"]
        ok = all(abs(g - 1) <= 0.02 for g in per) and all(g <= 0.95 for g in opn)
        print(f"  threshold: periodic within 2 % of KV, open box below 0.95 (end effects): {'ok' if ok else 'FAIL'}")
        print("DC PERIODIC CHECK OK" if ok else "DC PERIODIC CHECK FAILED")
        sys.exit(0 if ok else 1)
    runs = [(a.scmethod, a.grid, a.particles)]
    if a.check:
        runs = [("SPICNIC", "20 20 20", 20000), ("SPICNIC", "30 30 30", 80000), ("SPICNIC", "40 40 40", 160000),
                ("PICNIC", "24 24 24", 80000)]
    print(f"DC KV beam, {a.current} mA, 1 m drift, bunch = one period at {a.frequency / 1e6:g} MHz; "
          "space-charge growth of the exit rms, engine / KV envelope")
    last = {}
    for method, grid, n in runs:
        perv, nmid, o = run_case(lib, a.current, method, grid, n, a.frequency, a.negligible, a.meshrms)
        gx, gy = o["x"][3], o["y"][3]
        print(f"  {method} {grid or 'default'}, {n} particles ({nmid} in the middle half), K = {perv:.3e}: "
              f"rms x {o['x'][0] * 1e3:.4f} mm (KV {o['x'][1] * 1e3:.4f}, 0 mA {o['x'][2] * 1e3:.4f}), "
              f"rms y {o['y'][0] * 1e3:.4f} mm (KV {o['y'][1] * 1e3:.4f}); growth ratio x {gx:.4f}, y {gy:.4f}")
        last[method] = (gx, gy)
    if a.check:
        ok = all(abs(g - 1) <= 0.02 for v in last.values() for g in v)
        print(f"  threshold: growth ratio within 2 % of KV at the finest setting of each solver: {'ok' if ok else 'FAIL'}")
        print("DC KV CHECK OK" if ok else "DC KV CHECK FAILED")
        sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
