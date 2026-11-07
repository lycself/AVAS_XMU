"""Call the original kernel directly (no Python lattice processing).

usage: python direct.py cases.json WORKDIR REPO
Case fields: beam/input (key edits), lattice_txt (lines written verbatim to the staged
lattice.txt), calls (sequence of main_agent values, default [1]).
Each case runs in its own subprocess so a kernel crash or exit() cannot take the runner down.
"""
import json
import os
import shutil
import subprocess
import sys
import time

CHILD = r'''
import sys, time
sys.path.insert(0, sys.argv[4])
from avas.core.MultiParticleEngine import MultiParticleEngine
e = MultiParticleEngine()
e.get_path(sys.argv[1], sys.argv[2], sys.argv[3])
for v in [int(x) for x in sys.argv[5].split(",")]:
    t = time.time()
    r = e.main_agent(v)
    print(f"[direct] main_agent({v}) -> {r} in {time.time() - t:.2f} s", flush=True)
'''


def set_keys(path, changes):
    lines = open(path, encoding="utf-8").read().splitlines()
    for key, value in changes.items():
        idx = [i for i, ln in enumerate(lines) if ln.split()[:1] == [key]]
        if value is None:
            lines = [ln for i, ln in enumerate(lines) if i not in idx]
        elif idx:
            lines[idx[0]] = f"{key} {value}"
        else:
            lines.append(f"{key} {value}")
    open(path, "w", encoding="utf-8").write("\n".join(lines) + "\n")


def main():
    cases_file, work, repo = sys.argv[1:4]
    base = os.path.join(repo, "examples", "hwr010", "InputFile")
    for case in json.load(open(cases_file, encoding="utf-8")):
        d = os.path.join(work, case["name"])
        shutil.rmtree(d, ignore_errors=True)
        staged, out = os.path.join(d, "inputs"), os.path.join(d, "OutputFile")
        os.makedirs(staged)
        os.makedirs(out)
        for f in ("beam.txt", "input.txt"):
            shutil.copy(os.path.join(base, f), staged)
        set_keys(os.path.join(staged, "beam.txt"), case.get("beam", {}))
        set_keys(os.path.join(staged, "input.txt"), case.get("input", {}))
        open(os.path.join(staged, "lattice.txt"), "w").write("\n".join(case["lattice_txt"]) + "\n")
        calls = ",".join(str(c) for c in case.get("calls", [1]))
        t0 = time.time()
        timed_out = False
        with open(os.path.join(d, "log.txt"), "w", encoding="utf-8", errors="replace") as log:
            try:
                rc = subprocess.run([sys.executable, "-c", CHILD, staged + os.sep, out + os.sep, base + os.sep,
                                     repo, calls], cwd=d, stdout=log, stderr=subprocess.STDOUT,
                                    timeout=case.get("timeout", 300)).returncode
            except subprocess.TimeoutExpired:
                rc, timed_out = None, True
        res = {"rc": rc, "timeout": timed_out, "seconds": round(time.time() - t0, 2)}
        json.dump(res, open(os.path.join(d, "result.json"), "w"))
        print(case["name"], res, flush=True)


if __name__ == "__main__":
    main()
