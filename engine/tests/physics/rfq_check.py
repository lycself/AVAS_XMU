"""Physics check of the rfq element (corrected engine; no reference exists: AVAS.dll cannot run it).

usage: python engine/tests/physics/rfq_check.py [--lib PATH] [--check]

A synthetic 162.5 MHz proton RFQ from 35 keV: 80 cells of length beta*lambda/2 at the design energy, inter-vane
voltage 70 kV, aperture 6 mm, A0 = 0.95, A2 = 0.01, A1 = 0.1, synchronous phase ramped from -90 to -40 deg over
the first 60 % of the cells; the design energy gain per cell is (pi/4) A1 V cos(phi_s).  Checks:
1. the synchronous particle (scan stage) leaves within 3 % of the design energy and ends near the design phase
   (RFQsynPhaseEG.txt: last cell between -50 and -30 deg);
2. 2000 particles without space charge: some are lost or not captured, but the captured ones (above 90 % of the
   synchronous exit energy) average within 1 % of the synchronous particle.
Exit code 0 when the checks pass (--check).
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
sys.path.insert(0, os.path.join(REPO, "engine", "tests", "parity"))
import run_engine  # noqa: E402

F, V, APERTURE, MC2, W0, CELLS = 162.5e6, 70.0, 0.6, 938.272088159928, 0.035, 80


def write_rfq(folder):
    """rfqinput.txt of the synthetic RFQ; returns (length in m, design exit energy in MeV)."""
    lam = 299792458.0 / F
    w, rows, z = W0, [], 0.0
    for i in range(CELLS):
        phis = -90.0 + 50.0 * min(1.0, i / (0.6 * CELLS))
        g = w / MC2 + 1.0
        beta = math.sqrt(1.0 - 1.0 / g ** 2)
        length = beta * lam / 2.0
        w += math.pi / 4.0 * 0.1 * V * 1e-3 * math.cos(math.radians(phis))
        rows.append(f"{i + 1} {length * 100:.6f} {V:.3f} {APERTURE:.4f} 1.0 0 0.950000 0.100000 0.010000 0 0 0 0 0")
        z += length
    with open(os.path.join(folder, "rfqinput.txt"), "w") as fh:
        fh.write("\n".join(rows) + "\n")
    return z, w


def read_dst_energies(path):
    b = open(path, "rb").read()
    n, = struct.unpack_from("<i", b, 2)
    return [struct.unpack_from("<6d", b, 23 + 48 * i)[5] for i in range(n)]


def run(lib, particles):
    work = tempfile.mkdtemp(prefix="avas_rfq_")
    inp, out = os.path.join(work, "inputs"), os.path.join(work, "OutputFile")
    os.makedirs(inp)
    os.makedirs(out)
    try:
        length, design = write_rfq(inp)
        with open(os.path.join(inp, "beam.txt"), "w") as fh:
            fh.write("\n".join(["readparticledistribution unknown", "numofcharge 1", f"particlerestmass {MC2}", "current 0",
                                f"particlenumber {particles}", f"frequency {F}", f"kneticenergy {W0}", "beamtype notdc",
                                "twissx 1.0 0.05 0.2", "twissy 1.0 0.05 0.2", "twissz 0.0 0.5 0.3", "distribution GS GS",
                                "randomseed 1", ""]))
        with open(os.path.join(inp, "input.txt"), "w") as fh:
            fh.write("sim_type mulp\nspacecharge 0\nsteppercycle 100\ndumpperiodicity 0\nboundary 0\n")
        with open(os.path.join(inp, "lattice.txt"), "w") as fh:
            fh.write("\n".join(["start", "drift 0.02 0.02 0", f"rfq {length + 0.001:.5f} 0.01 0 1 {F} -90 rfqinput.txt",
                                "drift 0.05 0.02 0", "end", ""]))
        subprocess.run([sys.executable, "-c", run_engine.CHILD, lib, inp + os.sep, out + os.sep, inp + os.sep, "-"],
                       check=False, stdout=subprocess.DEVNULL)
        phases = [ln.split() for ln in open(os.path.join(out, "RFQsynPhaseEG.txt")) if ln.strip()]
        res = {"design": design, "sync_exit": float(phases[-1][1]), "last_phase": float(phases[-1][0])}
        outs = [f for f in os.listdir(out) if f.startswith("outData_")]
        if particles > 1 and outs:
            ws = read_dst_energies(os.path.join(out, outs[0]))
            cap = [w for w in ws if w > 0.9 * res["sync_exit"]]
            res.update(arrived=len(ws), captured=len(cap), captured_mean=sum(cap) / len(cap) if cap else float("nan"))
        return res
    finally:
        shutil.rmtree(work, ignore_errors=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lib", default=None)
    ap.add_argument("--check", action="store_true")
    a = ap.parse_args()
    lib = a.lib or next((p for p in (os.path.join(REPO, "engine", "build", n) for n in
                                     ("libavas_engine.dylib", "libavas_engine.so", "avas_engine.dll")) if os.path.exists(p)), None)
    if not lib:
        raise SystemExit("build the engine first or pass --lib")
    r = run(lib, 2000)
    ok1 = abs(r["sync_exit"] / r["design"] - 1.0) < 0.03 and -50.0 < r["last_phase"] < -30.0
    ok2 = r.get("captured", 0) > 0 and abs(r["captured_mean"] / r["sync_exit"] - 1.0) < 0.01
    print(f"rfq: synchronous exit {r['sync_exit']:.5f} MeV (design {r['design']:.5f}), last-cell phase "
          f"{r['last_phase']:.1f} deg  {'ok' if ok1 else 'FAIL'}")
    print(f"rfq: {r.get('arrived')} of 2000 arrive, {r.get('captured')} captured, mean {r.get('captured_mean', float('nan')):.5f} MeV "
          f"{'ok' if ok2 else 'FAIL'}")
    return 0 if (ok1 and ok2) or not a.check else 1


if __name__ == "__main__":
    sys.exit(main())
