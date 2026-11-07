"""Physics checks of the new engine's space charge (default mode), with thresholds.

usage: python engine/tests/physics/run_physics.py   (after building engine/build)
1. uniformly charged ellipsoid: SPICNIC converges with mesh / particles to the analytic linear field
   (<= 1.5 % at 40^3); FFT converges to it as its grounded box grows at fixed cell size (<= 1.5 %);
2. dc KV beam in a 1 m drift: the space-charge growth of the rms converges to the KV envelope of
   avas/dynamics/kv.py (within 2 % at 40^3).
Exit code 0 when both pass.
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.abspath(os.path.join(HERE, "..", "..", "build"))


def main():
    exe = next((p for p in (os.path.join(BUILD, n) for n in
                            ("avas_engine_physics", "avas_engine_physics.exe", os.path.join("Release", "avas_engine_physics.exe")))
                if os.path.exists(p)), None)
    if exe is None:
        raise SystemExit("build engine/build first (target avas_engine_physics)")
    r1 = subprocess.run([exe], stdout=subprocess.PIPE, text=True)
    print("\n".join(ln for ln in r1.stdout.splitlines() if not ln.startswith("mulitthread") and "0.09" not in ln))
    r2 = subprocess.run([sys.executable, os.path.join(HERE, "dc_kv_envelope.py"), "--check"])
    ok = r1.returncode == 0 and r2.returncode == 0
    print("PHYSICS CHECKS OK" if ok else "PHYSICS CHECKS FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
