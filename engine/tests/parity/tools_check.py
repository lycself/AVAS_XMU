"""AVAS.dll-only tools (sampling, meshing, dataprocess): run them with one library and summarise.

usage: python tools_check.py --lib PATH --work DIR [--compat dll|none] [--ref DIR]
Each tool runs in its own subprocess (the original exits the process on errors).  With --ref the
output files are compared byte by byte with an earlier run (for example the AVAS.dll one).
"""
import argparse
import hashlib
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DST = os.path.join(HERE, "data", "hwr010_seed12345.dst")

CHILD = r'''
import ctypes, os, sys
lib_path, compat, tool = sys.argv[1:4]
args = sys.argv[4:]
if os.name == "nt" and hasattr(os, "add_dll_directory"):
    os.add_dll_directory(os.path.dirname(os.path.abspath(lib_path)))
lib = ctypes.CDLL(lib_path)
s = ctypes.c_wchar_p if os.name == "nt" else ctypes.c_char_p
enc = (lambda v: v) if os.name == "nt" else (lambda v: v.encode())
if compat != "-" and hasattr(lib, "avas_set_option"):
    lib.avas_set_option(b"compat", compat.encode())
if tool == "sampling":
    f = lib.sampling
    f.argtypes = [s, s] + [ctypes.c_int] * 6 + [ctypes.c_double] * 2
    r = f(enc(args[0]), enc(args[1]), *[int(v) for v in args[2:8]], float(args[8]), float(args[9]))
elif tool == "meshing":
    f = lib.meshing
    f.argtypes = [s, s, ctypes.c_int, ctypes.c_double]
    r = f(enc(args[0]), enc(args[1]), int(args[2]), float(args[3]))
else:
    f = lib.dataprocess
    f.argtypes = [s, ctypes.c_int, ctypes.c_int, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_int]
    r = f(enc(args[0]), int(args[1]), int(args[2]), float(args[3]), float(args[4]), float(args[5]), int(args[6]))
f.restype = ctypes.c_int
sys.stdout.flush()
print("\n[tool] %s -> %d" % (tool, r), flush=True)
'''


def run(lib, compat, cwd, tool, *args):
    p = subprocess.run([sys.executable, "-c", CHILD, lib, compat, tool] + [str(a) for a in args], cwd=cwd,
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return p.returncode, p.stdout.decode("latin-1")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lib", required=True)
    ap.add_argument("--work", required=True)
    ap.add_argument("--compat", default="-")
    ap.add_argument("--ref", default="")
    a = ap.parse_args()
    lib = os.path.abspath(a.lib)
    w = os.path.abspath(a.work)
    shutil.rmtree(w, ignore_errors=True)
    os.makedirs(w)
    shutil.copy(DST, os.path.join(w, "in.dst"))
    logs = {}
    logs["meshing"] = run(lib, a.compat, w, "meshing", "in.dst", "mesh.dst", 5, 3.0)
    logs["sampling"] = run(lib, a.compat, w, "sampling", "in.dst", "samp.dst", 400, 6, 4, 6, 4, 10, 3.0, 1.5)
    # dataprocess: FiveCube = the meshed distribution (with weights), original = the input
    root = os.path.join(w, "dp")
    for sub in ("FiveCube", "original"):
        os.makedirs(os.path.join(root, "OutputFile_0", sub))
    for z in ("0.100000", "0.250000"):
        shutil.copy(os.path.join(w, "in.dst"), os.path.join(root, "OutputFile_0", "original", f"outData_{z}.dst"))
        for ext in ("", "b"):
            shutil.copy(os.path.join(w, "mesh.dst" + ext), os.path.join(root, "OutputFile_0", "FiveCube", f"outData_{z}.dst" + ext))
    logs["dataprocess"] = run(lib, a.compat, w, "dataprocess", root, 0, 0, 12.0, 16.0, 3.0, 1)
    # H-14: a magnification that leaves a segment empty ends the original with exit(0)
    # (in its own folder: sampling rewrites ./SPTest.txt)
    big = os.path.join(w, "big")
    os.makedirs(big)
    shutil.copy(DST, os.path.join(big, "in.dst"))
    logs["sampling_too_large"] = run(lib, a.compat, big, "sampling", "in.dst", "big.dst", 400, 6, 6, 3000, 3000, 10, 3.0, 1.5)
    outs = ["mesh.dst", "mesh.dstb", "samp.dst", "samp.dstb", "SPTest.txt",
            os.path.join("dp", "preprocessing_dataset", "OutputFile_0.ppj"), os.path.join("big", "SPTest.txt")]
    with open(os.path.join(w, "summary.txt"), "w", newline="\n") as f:
        for name, (rc, log) in logs.items():
            keep = [ln.strip() for ln in log.replace("\r", "\n").splitlines()
                    if ln.strip() and not ln.strip().startswith(("The inFile", "The outFile"))]
            f.write(f"== {name} rc={rc}\n" + "\n".join(keep) + "\n")
        for o in outs:
            p = os.path.join(w, o)
            raw = open(p, "rb").read() if os.path.exists(p) else b""
            f.write(f"{o.replace(os.sep, '/')} {len(raw)} {hashlib.sha256(raw).hexdigest()}\n")
    print(open(os.path.join(w, "summary.txt")).read())
    if a.ref:
        bad = 0
        # return code and console lines of every run (H-14: exit(0) after "The Magnification is too large")
        def sections(path):
            out, cur = {}, None
            for ln in open(path).read().splitlines():
                if ln.startswith("== "):
                    cur = ln.split()[1]
                    out[cur] = [ln]
                elif cur and not ln.startswith(tuple(o.replace(os.sep, "/") + " " for o in outs)):
                    out[cur].append(ln)
            return out
        rs, ns = sections(os.path.join(a.ref, "summary.txt")), sections(os.path.join(w, "summary.txt"))
        for name in logs:
            same = rs.get(name) == ns.get(name)
            bad += 0 if same else 1
            print(f"{'console ' + name:<45} {'identical' if same else 'DIFF'}")
        for o in outs:
            x, y = os.path.join(a.ref, o), os.path.join(w, o)
            same = os.path.exists(x) and os.path.exists(y) and open(x, "rb").read() == open(y, "rb").read()
            bad += 0 if same else 1
            print(f"{o:<45} {'identical' if same else 'DIFF'}")
        return 1 if bad else 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
