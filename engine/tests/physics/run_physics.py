"""Physics checks of the new engine's space charge (default mode), with thresholds.

usage: python engine/tests/physics/run_physics.py [--build DIR]   (after building engine/build or DIR)
1. uniformly charged ellipsoid: SPICNIC converges with mesh / particles to the analytic linear field
   (<= 1.5 % at 40^3), PICNIC too (<= 1 % at 32^3); FFT converges to it as its grounded box grows at fixed cell size (<= 1.5 %);
2. dc KV beam in a 1 m drift: the space-charge growth of the rms converges to the KV envelope of
   avas/dynamics/kv.py (SPICNIC within 2 % at 40^3, PICNIC within 2 % at 24^3);
3. rfq (rfq_check.py): a synthetic RFQ accelerates its synchronous particle to within 3 % of the design energy and
   the captured particles follow it.
Exit code 0 when all pass.
"""
import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.abspath(os.path.join(HERE, "..", "..", "build"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=BUILD)
    build = os.path.abspath(ap.parse_args().build)
    exe = next((p for p in (os.path.join(build, n) for n in
                            ("avas_engine_physics", "avas_engine_physics.exe", os.path.join("Release", "avas_engine_physics.exe")))
                if os.path.exists(p)), None)
    if exe is None:
        raise SystemExit(f"build {build} first (target avas_engine_physics)")
    r1 = subprocess.run([exe], stdout=subprocess.PIPE, text=True)
    print("\n".join(ln for ln in r1.stdout.splitlines() if not ln.startswith("mulitthread") and "0.09" not in ln))
    lib = next((p for p in (os.path.join(build, n) for n in
                            ("libavas_engine.dylib", "libavas_engine.so", "avas_engine.dll", os.path.join("Release", "avas_engine.dll")))
                if os.path.exists(p)), None)
    r2 = subprocess.run([sys.executable, os.path.join(HERE, "dc_kv_envelope.py"), "--check"] + (["--lib", lib] if lib else []))
    r3 = subprocess.run([sys.executable, os.path.join(HERE, "rfq_check.py"), "--check"] + (["--lib", lib] if lib else []))
    ok = r1.returncode == 0 and r2.returncode == 0 and r3.returncode == 0
    print("PHYSICS CHECKS OK" if ok else "PHYSICS CHECKS FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
