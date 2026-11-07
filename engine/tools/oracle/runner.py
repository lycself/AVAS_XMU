"""Run original-kernel experiment cases (phase 1 of the engine rebuild).

usage: python runner.py cases.json WORKDIR REPO
Each case copies REPO/examples/hwr010/InputFile, applies edits, runs
`python -m avas run --mode basic` with a timeout and records log + result.json.
"""
import json
import os
import shutil
import subprocess
import sys
import time


def set_keys(path, changes):
    lines = open(path, encoding="utf-8").read().splitlines() if os.path.exists(path) else []
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
    py = sys.executable
    cases = json.load(open(cases_file, encoding="utf-8"))
    only = set(sys.argv[4].split(",")) if len(sys.argv) > 4 else None
    for case in cases:
        name = case["name"]
        if only and name not in only:
            continue
        d = os.path.join(work, name)
        shutil.rmtree(d, ignore_errors=True)
        inp = os.path.join(d, "InputFile")
        shutil.copytree(os.path.join(repo, "examples", "hwr010", "InputFile"), inp)
        set_keys(os.path.join(inp, "beam.txt"), case.get("beam", {}))
        set_keys(os.path.join(inp, "input.txt"), case.get("input", {}))
        if "lattice" in case:
            open(os.path.join(inp, "lattice_mulp.txt"), "w").write("\n".join(case["lattice"]) + "\n")
        for fname, text in case.get("files", {}).items():
            open(os.path.join(inp, fname), "w").write(text)
        for fname, src in case.get("copy", {}).items():
            shutil.copy(src.replace("{HERE}", os.path.dirname(os.path.abspath(cases_file))), os.path.join(inp, fname))
        env = dict(os.environ, **case.get("env", {}))
        cmd = [py, "-m", "avas", "run", "--input", inp, "--output", os.path.join(d, "OutputFile"),
               "--mode", "basic", "--lattice", "lattice_mulp.txt"]
        t0 = time.time()
        timed_out = False
        with open(os.path.join(d, "log.txt"), "w", encoding="utf-8", errors="replace") as log:
            try:
                rc = subprocess.run(cmd, cwd=repo, stdout=log, stderr=subprocess.STDOUT,
                                    timeout=case.get("timeout", 300), env=env).returncode
            except subprocess.TimeoutExpired:
                rc, timed_out = None, True
        res = {"rc": rc, "timeout": timed_out, "seconds": round(time.time() - t0, 2)}
        json.dump(res, open(os.path.join(d, "result.json"), "w"))
        shutil.rmtree(inp, ignore_errors=True)          # field maps are large; inputs are re-creatable
        print(name, res, flush=True)


if __name__ == "__main__":
    main()
