"""Run the M1 parity cases with one engine library and summarise its outputs.

usage:
  python run_engine.py --lib PATH [--compat dll|none] --work DIR [--only a,b] [--repo REPO]

Works with the original kernel (AVAS.dll / libAVAS.so) and with the new engine
(avas_engine.dll / libavas_engine.so / libavas_engine.dylib).  Each case runs in its own
subprocess (the original kernel may exit() or crash).  For every case DIR/<case>/summary/
holds the text files that are compared against engine/tests/parity/refs/<case>/.
"""
import argparse
import hashlib
import json
import math
import os
import shutil
import struct
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
WINDOWS = os.name == "nt"

CHILD = r'''
import ctypes, os, sys
lib_path, inp, out, field, compat = sys.argv[1:6]
if os.name == "nt" and hasattr(os, "add_dll_directory"):
    os.add_dll_directory(os.path.dirname(os.path.abspath(lib_path)))
lib = ctypes.CDLL(lib_path)
text = ctypes.c_wchar_p if os.name == "nt" else ctypes.c_char_p
lib.path.argtypes = [text, text, text]
lib.path.restype = ctypes.c_int
lib.main_agent.argtypes = [ctypes.POINTER(ctypes.c_int)]
lib.main_agent.restype = ctypes.c_int
if compat != "-" and hasattr(lib, "avas_set_option"):
    lib.avas_set_option.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    lib.avas_set_option.restype = ctypes.c_int
    if lib.avas_set_option(b"compat", compat.encode()) != 0:
        print("[parity] avas_set_option failed", flush=True)
args = (inp, out, field) if os.name == "nt" else tuple(a.encode("utf-8") for a in (inp, out, field))
lib.path(*args)
r = lib.main_agent(ctypes.byref(ctypes.c_int(1)))
sys.stdout.flush()
print("\n[parity] main_agent -> %d" % r, flush=True)
sys.exit(100 + r)
'''

TEXT_FILES = ["synData.txt", "scanPhaseData.txt", "Phase.txt", "SingleParticle.txt", "DataSet.txt",
              "LongitudinalBeamLoss.txt", "SpaceChargeMesh.txt", "StaticErrorData.txt", "DynamicErrorData.txt",
              "SeSynParticle.txt", "ParticleTracking.txt", "collisionData.txt"]
BINARY_FILES = ["BeamSet.plt", "PCHistogram.dat", "SpaceChargeSet.plt"]


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
    open(path, "w", encoding="utf-8", newline="\n").write("\n".join(lines) + "\n")


def phi_s3a(sources):
    """Entry RF phase of the first cavity of S3a (column 7 of synData row 0)."""
    for src in sources:
        p = os.path.join(src, "S3a_two_cav_v0_v0", "synData.txt") if src.endswith("refs") else \
            os.path.join(src, "S3a_two_cav_v0_v0", "summary", "synData.txt")
        if os.path.exists(p):
            return open(p).read().splitlines()[0].split()[7]
    raise SystemExit("S3a synData not available yet (run S3a first)")


def summarise_dst(path):
    b = open(path, "rb").read()
    if len(b) < 23:
        return f"short file {len(b)} bytes\n"
    h0, h1, n, ib, f, h2 = struct.unpack("<BBiddB", b[:23])
    body = b[23:23 + 48 * n]
    vals = struct.unpack("<%dd" % (6 * n), body) if len(body) == 48 * n else ()
    tail = b[23 + 48 * n:]
    mc2 = struct.unpack("<d", tail[:8])[0] if len(tail) >= 8 else float("nan")
    out = [f"bytes {len(b)}", f"header {h0} {h1} {h2}", f"N {n}", f"Ib {ib!r}", f"f {f!r}",
           f"mc2 {mc2!r}", "sha256 " + hashlib.sha256(body).hexdigest()]
    if vals:
        names = ["x", "xp", "y", "yp", "phi", "W"]
        for c in range(6):
            col = vals[c::6]
            m = math.fsum(col) / n
            r = math.sqrt(math.fsum((v - m) ** 2 for v in col) / n)
            out.append(f"{names[c]} mean {m!r} rms {r!r} min {min(col)!r} max {max(col)!r}")
        for i in range(min(n, 4)):
            out.append("p%d %s" % (i, " ".join(repr(v) for v in vals[6 * i:6 * i + 6])))
    return "\n".join(out) + "\n"


def summarise_edst(path):
    b = open(path, "rb").read()
    if len(b) < 23:
        return f"short file {len(b)} bytes\n"
    h0, h1, n, ib, f, h2 = struct.unpack("<BBiddB", b[:23])
    m = (len(b) - 31) // 72          # records (the header count is one less)
    body = b[23:23 + 72 * m]
    vals = struct.unpack("<%dd" % (9 * m), body)
    out = [f"bytes {len(b)}", f"header {h0} {h1} {h2}", f"N {n}", f"Ib {ib!r}", f"f {f!r}",
           "sha256 " + hashlib.sha256(body).hexdigest()]
    names = ["x", "xp", "y", "yp", "phi", "W", "Q", "mc2", "w"]
    for c in range(9):
        col = vals[c::9]
        mm = math.fsum(col) / m
        r = math.sqrt(math.fsum((v - mm) ** 2 for v in col) / m)
        out.append(f"{names[c]} mean {mm!r} rms {r!r} min {min(col)!r} max {max(col)!r}")
    return "\n".join(out) + "\n"


def make_binary_maps(field_src, inp, maps):
    """Field maps in the binary format (name ending in b): int32 nz, double zlen, int32 nx, double xmin,
    xmax, int32 ny, double ymin, ymax, double norm, then float32 values (spec 30 C-19)."""
    for new, (old, exts) in maps.items():
        for ext in exts:
            words = open(os.path.join(field_src, old + ext)).read().split()
            nz, zlen, nx, xmin, xmax, ny, ymin, ymax, norm = (int(words[0]), float(words[1]), int(words[2]), float(words[3]),
                                                              float(words[4]), int(words[5]), float(words[6]),
                                                              float(words[7]), float(words[8]))
            vals = [float(v) for v in words[9:9 + (nx + 1) * (ny + 1) * (nz + 1)]]
            with open(os.path.join(inp, new + ext), "wb") as f:
                f.write(struct.pack("<ididdiddd", nz, zlen, nx, xmin, xmax, ny, ymin, ymax, norm))
                f.write(struct.pack("<%df" % len(vals), *vals))


def summarise(out_dir, summary_dir, rc, log, top=True, checks=()):
    os.makedirs(summary_dir, exist_ok=True)
    if top:
        open(os.path.join(summary_dir, "rc.txt"), "w").write(f"{rc}\n")
        # error samples of the kernel's own error study: OutputFile/output_<k>/
        for sub in sorted(os.listdir(out_dir)) if os.path.isdir(out_dir) else []:
            if sub.startswith("output_") and os.path.isdir(os.path.join(out_dir, sub)):
                summarise(os.path.join(out_dir, sub), os.path.join(summary_dir, sub), rc, "", top=False)
    files = sorted(os.listdir(out_dir)) if os.path.isdir(out_dir) else []
    with open(os.path.join(summary_dir, "files.txt"), "w") as fl:
        for f in files:
            p = os.path.join(out_dir, f)
            if os.path.isfile(p):
                size = os.path.getsize(p)
                if f in ("SimulationTime.txt", "ErrorLog.txt", "synParticle.txt"):
                    size = "-" if size else 0       # content depends on time stamps
                fl.write(f"{f} {size}\n")
    for f in TEXT_FILES:
        p = os.path.join(out_dir, f)
        if os.path.exists(p):
            shutil.copy(p, os.path.join(summary_dir, f))
    p = os.path.join(out_dir, "FullParticle.txt")
    if os.path.exists(p):
        b = open(p, "rb").read().replace(b"\r\n", b"\n")
        lines = b.decode("latin-1").splitlines()
        open(os.path.join(summary_dir, "FullParticle.summary.txt"), "w", newline="\n").write(
            "lines %d\nsha256 %s\n" % (len(lines), hashlib.sha256(b).hexdigest()) + "".join(ln + "\n" for ln in lines[:3]))
    p = os.path.join(out_dir, "synParticle.txt")
    if os.path.exists(p):
        lines = open(p, "rb").read().decode("latin-1").splitlines()
        open(os.path.join(summary_dir, "synParticle.txt"), "w", newline="\n").write(
            "".join(ln[21:] + "\n" for ln in lines))
    p = os.path.join(out_dir, "ErrorLog.txt")
    if os.path.exists(p):
        lines = open(p, "rb").read().decode("latin-1").splitlines()
        open(os.path.join(summary_dir, "ErrorLog.txt"), "w", newline="\n").write(
            "".join(ln[19:] + "\n" for ln in lines))
    for f in files:
        if f.startswith("outData_") and f.endswith(".txt"):
            shutil.copy(os.path.join(out_dir, f), os.path.join(summary_dir, f))   # secondary particles
    for f in BINARY_FILES:
        p = os.path.join(out_dir, f)
        if os.path.exists(p):
            raw = open(p, "rb").read()
            open(os.path.join(summary_dir, f + ".sha.txt"), "w").write(
                "bytes %d\nsha256 %s\n" % (len(raw), hashlib.sha256(raw).hexdigest()))
    for f in files:
        if f.endswith(".dst"):
            open(os.path.join(summary_dir, f + ".txt"), "w").write(summarise_dst(os.path.join(out_dir, f)))
        elif f.endswith(".edst"):
            open(os.path.join(summary_dir, f + ".txt"), "w").write(summarise_edst(os.path.join(out_dir, f)))
        elif f.endswith(".dstb"):
            open(os.path.join(summary_dir, f + ".txt"), "w").write(
                "sha256 " + hashlib.sha256(open(os.path.join(out_dir, f), "rb").read()).hexdigest() + "\n")
    if not top:
        return
    # keep the console lines that carry results (no timing values)
    keep = []
    for ln in log.replace("\r", "\n").splitlines():
        s = ln.strip()
        if any(s.startswith(k) for k in ("The ", "Exit bunch energy", "Entry  time", "Export time",
                                         "Lattice initialisation", "A total of", "Note that",
                                         "Error!", "Average beam", "[parity]")):
            keep.append(s)
    open(os.path.join(summary_dir, "console.txt"), "w", newline="\n", encoding="utf-8").write("\n".join(keep) + "\n")
    if checks:
        # console messages of a case (exit paths, warnings): the matching lines in order
        hit = [ln.strip() for ln in log.replace("\r", "\n").splitlines() if any(c in ln for c in checks)]
        open(os.path.join(summary_dir, "console_check.txt"), "w", newline="\n", encoding="utf-8").write("\n".join(hit) + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lib", required=True)
    ap.add_argument("--compat", default="-")
    ap.add_argument("--work", required=True)
    ap.add_argument("--repo", default=os.path.abspath(os.path.join(HERE, "..", "..", "..")))
    ap.add_argument("--only", default="")
    ap.add_argument("--reference", action="store_true", help="also run reference-only cases")
    ap.add_argument("--timeout", type=float, default=600)
    ap.add_argument("--cases", default=os.path.join(HERE, "cases.json"), help="case file (default: cases.json)")
    a = ap.parse_args()
    a.lib = os.path.abspath(a.lib)
    a.work = os.path.abspath(a.work)
    cases_doc = json.load(open(a.cases, encoding="utf-8"))
    only = set(a.only.split(",")) if a.only else None
    base = os.path.join(a.repo, "examples", "hwr010", "InputFile")
    new_engine = "avas_engine" in os.path.basename(a.lib)
    os.makedirs(a.work, exist_ok=True)
    for case in cases_doc["cases"]:
        name = case["name"]
        if only and name not in only:
            continue
        if case.get("reference_only") and not a.reference:
            continue
        if case.get("new_engine_only") and not new_engine:
            continue
        d = os.path.join(a.work, name)
        shutil.rmtree(d, ignore_errors=True)
        inp, out = os.path.join(d, "inputs"), os.path.join(d, "OutputFile")
        os.makedirs(inp)
        os.makedirs(out)
        for f in ("beam.txt", "input.txt"):
            shutil.copy(os.path.join(base, f), inp)
        set_keys(os.path.join(inp, "beam.txt"), case.get("beam", {}))
        set_keys(os.path.join(inp, "input.txt"), dict(cases_doc.get("common_input", {}), **case.get("input", {})))
        lattice = "\n".join(case["lattice"]) + "\n"
        if "{PHI_S3A_CAV1}" in lattice:
            # the stored reference phase, so that every engine runs the same S3c lattice; when the
            # references are being generated, the phase of the S3a run just made
            order = [a.work, os.path.join(HERE, "refs")] if a.reference else [os.path.join(HERE, "refs"), a.work]
            lattice = lattice.replace("{PHI_S3A_CAV1}", phi_s3a(order))
        open(os.path.join(inp, "lattice.txt"), "w", newline="\n").write(lattice)
        for dst_name, src in case.get("copy", {}).items():
            shutil.copy(os.path.join(HERE, src), os.path.join(inp, dst_name))
        for fname, text in case.get("files", {}).items():
            open(os.path.join(inp, fname), "w", newline="\n").write("\n".join(text) + "\n" if isinstance(text, list) else text)
        # field maps are read from the example (read only); cases that generate maps (multipole)
        # use their own input folder
        field = inp if case.get("field_dir") == "inputs" else base
        if case.get("binary_maps"):
            make_binary_maps(base, inp, case["binary_maps"])
            for f in case.get("copy_maps", []):
                shutil.copy(os.path.join(base, f), inp)
        # renamed copies of example maps (e.g. the RF map of hwr010 used as a static electric field)
        for new_name, old_name in case.get("rename_maps", {}).items():
            shutil.copy(os.path.join(base, old_name), os.path.join(inp, new_name))
        t0 = time.time()
        try:
            # AVAS_PARITY_LAUNCHER: optional command prefix, e.g. "mpirun -np 2" (MPI GPU runs)
            launcher = os.environ.get("AVAS_PARITY_LAUNCHER", "").split()
            proc = subprocess.run(launcher + [sys.executable, "-c", CHILD, a.lib, inp + os.sep, out + os.sep, field + os.sep,
                                              a.compat], cwd=d, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                  timeout=case.get("timeout", a.timeout))
            rc = proc.returncode - 100 if proc.returncode >= 100 else f"exit {proc.returncode}"
            log = proc.stdout.decode("latin-1")
        except subprocess.TimeoutExpired as e:
            rc, log = "timeout", (e.stdout or b"").decode("latin-1")
        open(os.path.join(d, "log.txt"), "w", encoding="utf-8").write(log)
        summarise(out, os.path.join(d, "summary"), rc, log, checks=case.get("console_check", ()))
        for f, target in case.get("export", {}).items():
            if os.path.exists(os.path.join(out, f)):
                shutil.copy(os.path.join(out, f), os.path.join(d, os.path.basename(target)))
        print(f"{name}: rc={rc} {time.time() - t0:.1f} s", flush=True)


if __name__ == "__main__":
    main()
