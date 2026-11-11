"""Checks and build records of the new kernel's libraries (used by .github/workflows/engine.yml).

usage:
  python engine/tools/ci_engine.py exports LIB                  exported symbols = the C API of src/exports.map
  python engine/tools/ci_engine.py glibc LIB                    Linux: GLIBC <= 2.17, GLIBCXX <= 3.4.19, CXXABI <= 1.3.7
  python engine/tools/ci_engine.py deps LIB                     the libraries LIB needs (information)
  python engine/tools/ci_engine.py metal LIB                    is a Metal 3 device usable? (prints the reason;
                                                                writes usable=true|false to $GITHUB_OUTPUT)
  python engine/tools/ci_engine.py build-info --lib LIB --platform NAME --out FILE [--check NAME=RESULT ...]
  python engine/tools/ci_engine.py stamp --out DIR [--require P1,P2] ARTIFACT_DIR ...
  python engine/tools/ci_engine.py smoke [--lib-dir avas/engine/new]   short `avas run --engine new` (skipped
                                                                when the folder has no library for this platform)

build-info.json (one per platform job) and build-stamp.json (all platforms, committed with the libraries in
avas/engine/new/) record the engine source hash, the files' SHA-256, the compiler, the CUDA machine code and
the workflow run.  verify_stamp() is the check shared by fetch_ci_engine.py and tests/test_design_rules.py.
Standard library only (runs in the manylinux container and on the CI hosts without installing AVAS).
"""
import argparse
import ctypes
import hashlib
import json
import os
import platform
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from datetime import datetime, timezone

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE = os.path.dirname(HERE)
REPO = os.path.dirname(ENGINE)

STAMP_NAME = "build-stamp.json"
INFO_NAME = "build-info.json"
SCHEMA = 1
# platform -> library file name (the names avas.engines looks for)
LIBRARIES = {"windows-x86_64": "avas_engine.dll", "linux-x86_64": "libavas_engine.so",
             "macos-universal2": "libavas_engine.dylib"}
LIBRARY_NAMES = set(LIBRARIES.values())
# manylinux2014 policy (glibc 2.17, GCC 4.8 libstdc++ ABI)
MAX_VERSIONS = {"GLIBC": (2, 17), "GLIBCXX": (3, 4, 19), "CXXABI": (1, 3, 7)}


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for block in iter(lambda: fh.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def source_hash(engine_dir=ENGINE):
    """Engine source hash of the checkout (avas.engines.engine_source_hash, the CMake algorithm)."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(engine_dir)))
    try:
        from avas.engines import engine_source_hash
    finally:
        sys.path.pop(0)
    return engine_source_hash(os.path.abspath(engine_dir))


def exported_api():
    """Names in the global section of src/exports.map (the C API)."""
    text = open(os.path.join(ENGINE, "src", "exports.map"), encoding="utf-8").read()
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    body = re.search(r"global:(.*?)local:", text, re.S).group(1)
    return {n.strip() for n in body.replace("\n", " ").split(";") if n.strip()}


# ----------------------------------------------------------------------------------------- PE files
def _pe(path):
    data = open(path, "rb").read()
    if data[:2] != b"MZ":
        raise ValueError(f"{path}: not a PE file")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise ValueError(f"{path}: no PE signature")
    nsec, opt_size = struct.unpack_from("<H", data, pe + 6)[0], struct.unpack_from("<H", data, pe + 20)[0]
    opt = pe + 24
    magic = struct.unpack_from("<H", data, opt)[0]
    dirs = opt + (96 if magic == 0x10B else 112)
    sections = []
    for k in range(nsec):
        s = opt + opt_size + 40 * k
        vsize, va, rawsize, raw = struct.unpack_from("<IIII", data, s + 8)
        sections.append((va, max(vsize, rawsize), raw))

    def off(rva):
        for va, size, raw in sections:
            if va <= rva < va + size:
                return rva - va + raw
        raise ValueError(f"{path}: RVA {rva:#x} outside the sections")

    def cstr(rva):
        o = off(rva)
        return data[o:data.index(b"\0", o)].decode("ascii", "replace")

    return data, dirs, off, cstr


def pe_exports(path):
    data, dirs, off, cstr = _pe(path)
    rva, size = struct.unpack_from("<II", data, dirs)
    if not rva:
        return set()
    o = off(rva)
    nnames, names_rva = struct.unpack_from("<I", data, o + 24)[0], struct.unpack_from("<I", data, o + 32)[0]
    no = off(names_rva)
    return {cstr(struct.unpack_from("<I", data, no + 4 * k)[0]) for k in range(nnames)}


def pe_imports(path):
    data, dirs, off, cstr = _pe(path)
    rva, size = struct.unpack_from("<II", data, dirs + 8)
    out = []
    if not rva:
        return out
    o = off(rva)
    while True:
        desc = struct.unpack_from("<IIIII", data, o)
        if not any(desc):
            break
        out.append(cstr(desc[3]))
        o += 20
    return out


# ------------------------------------------------------------------------------------- the commands
def library_exports(lib):
    if lib.lower().endswith(".dll"):
        return pe_exports(lib)
    if lib.endswith(".dylib"):
        out = subprocess.run(["nm", "-gU", lib], check=True, capture_output=True, text=True).stdout
        return {ln.split()[-1][1:] for ln in out.splitlines() if len(ln.split()) == 3}
    out = subprocess.run(["nm", "-D", "--defined-only", lib], check=True, capture_output=True, text=True).stdout
    return {ln.split()[-1].split("@")[0] for ln in out.splitlines() if len(ln.split()) == 3}


# Exports a library may have besides the C API.  The static CUDA runtime on Windows (cudart_static.lib)
# exports NvOptimusEnablementCuda: on laptops with integrated and NVIDIA graphics (Optimus) it asks the
# driver to wake the NVIDIA GPU for this process, which device=cuda needs.
ALLOWED_EXTRA = {".dll": {"NvOptimusEnablementCuda"}}


def cmd_exports(a):
    want, got = exported_api(), library_exports(a.lib)
    allowed = next((v for ext, v in ALLOWED_EXTRA.items() if a.lib.lower().endswith(ext)), set())
    extra, missing = sorted(got - want - allowed), sorted(want - got)
    print(f"{a.lib}: {len(got)} exported symbols")
    for name in sorted(got & want):
        print(f"  {name}")
    for name in sorted(got & allowed):
        print(f"  {name} (allowed: from the static CUDA runtime)")
    if extra:
        print(f"NOT IN THE C API ({len(extra)}): " + " ".join(extra[:40]) + (" ..." if len(extra) > 40 else ""))
    if missing:
        print("MISSING: " + " ".join(missing))
    return 1 if extra or missing else 0


def cmd_glibc(a):
    out = subprocess.run(["objdump", "-T", a.lib], check=True, capture_output=True, text=True).stdout
    bad = 0
    for tag, limit in MAX_VERSIONS.items():
        found = {tuple(int(x) for x in v.split(".")) for v in re.findall(rf"\b{tag}_([0-9.]+[0-9])\b", out)}
        top = max(found) if found else None
        ok = top is None or top <= limit
        bad += 0 if ok else 1
        show = ".".join(map(str, top)) if top else "-"
        print(f"{tag:<8} needs {show:<8} limit {'.'.join(map(str, limit)):<8} {'ok' if ok else 'TOO NEW'}")
    return 1 if bad else 0


def cmd_deps(a):
    if a.lib.lower().endswith(".dll"):
        print("\n".join(pe_imports(a.lib)))
        return 0
    tool = ["otool", "-L", a.lib] if a.lib.endswith(".dylib") else ["readelf", "-d", a.lib]
    out = subprocess.run(tool, check=True, capture_output=True, text=True).stdout
    if a.lib.endswith(".dylib"):
        print(out.rstrip())
    else:
        print("\n".join(ln.strip() for ln in out.splitlines() if "NEEDED" in ln))
    return 0


def load(lib):
    if os.name == "nt" and hasattr(os, "add_dll_directory"):
        os.add_dll_directory(os.path.dirname(os.path.abspath(lib)))
    handle = ctypes.CDLL(os.path.abspath(lib))
    for name in ("avas_engine_info", "avas_engine_devices"):
        getattr(handle, name).restype = ctypes.c_char_p
    return handle


def query(lib, name):
    return json.loads(getattr(load(lib), name)().decode("utf-8"))


def cmd_metal(a):
    metal = query(a.lib, "avas_engine_devices").get("metal") or {}
    usable = [d["name"] for d in metal.get("devices") or [] if d.get("usable")]
    if usable:
        print(f"Metal 3 device: {', '.join(usable)}")
    else:
        print("no usable Metal 3 device (" + (metal.get("note") or "none found") + "): the Metal checks are SKIPPED, "
              "the CPU and emu checks still run")
    if os.environ.get("GITHUB_OUTPUT"):
        with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as fh:
            fh.write(f"usable={'true' if usable else 'false'}\n")
    return 0


def workflow_run():
    """The GitHub Actions run (empty values outside CI)."""
    env = os.environ
    server, repo, run_id = env.get("GITHUB_SERVER_URL", ""), env.get("GITHUB_REPOSITORY", ""), env.get("GITHUB_RUN_ID", "")
    return {"repository": repo, "workflow": env.get("GITHUB_WORKFLOW", ""), "id": run_id,
            "number": env.get("GITHUB_RUN_NUMBER", ""), "attempt": env.get("GITHUB_RUN_ATTEMPT", ""),
            "url": f"{server}/{repo}/actions/runs/{run_id}" if server and repo and run_id else "",
            "commit": env.get("GITHUB_SHA", ""), "ref": env.get("GITHUB_REF_NAME", "")}


def cmd_build_info(a):
    lib = os.path.abspath(a.lib)
    if os.path.basename(lib) != LIBRARIES.get(a.platform):
        raise SystemExit(f"{a.platform} needs {LIBRARIES.get(a.platform)}, got {os.path.basename(lib)}")
    info = query(lib, "avas_engine_info")
    tree = source_hash()
    if info.get("source_hash") != tree:
        raise SystemExit(f"the library reports source hash {info.get('source_hash')}, the checkout is {tree}: stale build")
    checks = dict(c.split("=", 1) for c in a.check)
    runner = " ".join(v for v in (os.environ.get("ImageOS", ""), os.environ.get("ImageVersion", "")) if v)
    doc = {"schema": SCHEMA, "platform": a.platform, "file": os.path.basename(lib), "sha256": sha256(lib),
           "size": os.path.getsize(lib), "source_hash": tree, "version": info.get("version"),
           "build": info.get("build"), "compiler": info.get("compiler"), "cmake_compiler": info.get("cmake_compiler"),
           "cuda_compiler": info.get("cuda_compiler"), "engine_platform": info.get("platform"),
           "runner": runner or platform.platform(), "built": datetime.now(timezone.utc).isoformat(timespec="seconds"),
           "checks": checks, "run": workflow_run()}
    os.makedirs(os.path.dirname(os.path.abspath(a.out)), exist_ok=True)
    with open(a.out, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(doc, fh, indent=1)
        fh.write("\n")
    print(json.dumps(doc, indent=1))
    return 0


def cmd_stamp(a):
    infos = []
    for d in a.dirs:
        p = os.path.join(d, INFO_NAME)
        if not os.path.isfile(p):
            raise SystemExit(f"{d}: no {INFO_NAME}")
        info = json.load(open(p, encoding="utf-8"))
        lib = os.path.join(d, info["file"])
        if not os.path.isfile(lib) or sha256(lib) != info["sha256"]:
            raise SystemExit(f"{lib}: missing or SHA-256 differs from {INFO_NAME}")
        infos.append((info, lib))
    hashes = {i["source_hash"] for i, _ in infos}
    commits = {i["run"]["commit"] for i, _ in infos}
    if len(hashes) != 1 or len(commits) != 1:
        raise SystemExit(f"the platform builds disagree: source hashes {sorted(hashes)}, commits {sorted(commits)}")
    tree = source_hash()
    if hashes != {tree}:
        raise SystemExit(f"the builds are of source hash {hashes.pop()}, the checkout is {tree}")
    plats = sorted(i["platform"] for i, _ in infos)
    want = [p for p in (a.require.split(",") if a.require else []) if p]
    if len(set(plats)) != len(plats) or not set(want) <= set(plats):
        raise SystemExit(f"platforms {plats}, required {want}")
    os.makedirs(a.out, exist_ok=True)
    files = {}
    for info, lib in infos:
        shutil.copy2(lib, os.path.join(a.out, info["file"]))
        files[info["file"]] = {k: info[k] for k in ("platform", "sha256", "size", "version", "build", "compiler",
                                                    "cmake_compiler", "cuda_compiler", "runner", "built", "checks")}
    run = workflow_run()
    stamp = {"schema": SCHEMA, "source_hash": tree, "version": tree[:16], "commit": commits.pop(),
             "ref": run["ref"] or infos[0][0]["run"]["ref"], "repository": run["repository"],
             "workflow_run": {k: run[k] or infos[0][0]["run"][k] for k in ("workflow", "id", "number", "attempt", "url")},
             "created": datetime.now(timezone.utc).isoformat(timespec="seconds"), "files": files}
    with open(os.path.join(a.out, STAMP_NAME), "w", encoding="utf-8", newline="\n") as fh:
        json.dump(stamp, fh, indent=1)
        fh.write("\n")
    problems = verify_stamp(a.out, tree)
    if problems:
        raise SystemExit("\n".join(problems))
    name = f"avas-engine-new-{tree[:16]}"
    print(f"{name}: " + ", ".join(f"{f} ({v['platform']}, {v['size']} bytes)" for f, v in files.items()))
    if os.environ.get("GITHUB_OUTPUT"):
        with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as fh:
            fh.write(f"name={name}\n")
    return 0


def read_stamp(folder):
    """The stamp of *folder*: a dict, {} when the file is absent or empty."""
    p = os.path.join(folder, STAMP_NAME)
    if not os.path.isfile(p) or not open(p, "rb").read().strip():
        return {}
    return json.load(open(p, encoding="utf-8"))


def verify_stamp(folder, expected_source_hash=None, require_platforms=()):
    """Problems (list of text) of a folder of prebuilt libraries and its build-stamp.json: every library
    listed with a matching SHA-256 and size, no unlisted library, the source hash, required platforms."""
    try:
        stamp = read_stamp(folder)
    except ValueError as exc:
        return [f"{STAMP_NAME} is not valid JSON: {exc}"]
    present = sorted(n for n in os.listdir(folder) if n in LIBRARY_NAMES) if os.path.isdir(folder) else []
    if not stamp:
        return [f"libraries without {STAMP_NAME}: {present}"] if present else []
    problems = []
    if stamp.get("schema") != SCHEMA:
        problems.append(f"{STAMP_NAME}: unknown schema {stamp.get('schema')!r}")
    if expected_source_hash and stamp.get("source_hash") != expected_source_hash:
        problems.append(f"the libraries were built from engine source hash {stamp.get('source_hash')}, the sources are now "
                        f"{expected_source_hash}")
    files = stamp.get("files") or {}
    for name, entry in sorted(files.items()):
        p = os.path.join(folder, name)
        if name not in LIBRARY_NAMES or entry.get("platform") not in LIBRARIES or LIBRARIES[entry["platform"]] != name:
            problems.append(f"{STAMP_NAME}: unexpected entry {name} ({entry.get('platform')})")
        elif not os.path.isfile(p):
            problems.append(f"{name} is listed in {STAMP_NAME} but missing")
        elif os.path.getsize(p) != entry.get("size") or sha256(p) != entry.get("sha256"):
            problems.append(f"{name}: SHA-256 or size differs from {STAMP_NAME}")
    for name in present:
        if name not in files:
            problems.append(f"{name} is not listed in {STAMP_NAME}")
    have = {e.get("platform") for e in files.values()}
    for plat in require_platforms:
        if plat not in have:
            problems.append(f"no library for {plat}")
    return problems


def native_platform():
    system = platform.system()
    return {"Windows": "windows-x86_64", "Linux": "linux-x86_64", "Darwin": "macos-universal2"}.get(system)


def cmd_smoke(a):
    """`avas run --engine new` on a shortened copy of the example with the library of *lib_dir*."""
    folder = os.path.abspath(a.lib_dir)
    name = LIBRARIES.get(native_platform())
    lib = os.path.join(folder, name or "?")
    if not name or not os.path.isfile(lib):
        print(f"SKIPPED: {folder} has no {name or 'library for this platform'} (the new kernel is not prebuilt yet)")
        return 0
    problems = verify_stamp(folder, source_hash())
    if problems:
        print("\n".join(problems))
        return 1
    stamp = read_stamp(folder)
    env = {k: v for k, v in os.environ.items() if not k.startswith("AVAS_ENGINE")}
    if os.path.normcase(folder) != os.path.normcase(os.path.join(REPO, "avas", "engine", "new")):
        env["AVAS_ENGINE_LIB"] = lib       # another folder (tests); avas/engine/new is found without it
    with tempfile.TemporaryDirectory(prefix="avas_new_smoke_") as tmp:
        inp, out = os.path.join(tmp, "InputFile"), os.path.join(tmp, "OutputFile")
        shutil.copytree(os.path.join(REPO, "examples", "hwr010", "InputFile"), inp)
        beam = os.path.join(inp, "beam.txt")
        lines = open(beam, encoding="utf-8").read().splitlines()
        lines = ["particlenumber 1000" if ln.split()[:1] == ["particlenumber"] else ln for ln in lines]
        open(beam, "w", encoding="utf-8", newline="\n").write("\n".join(lines) + "\n")
        cmd = [sys.executable, "-m", "avas", "run", "--input", inp, "--output", out, "--engine", "new", "--device", "auto"]
        print(" ".join(cmd), flush=True)
        rc = subprocess.run(cmd, cwd=REPO, env=env).returncode
        record = os.path.join(out, "avas_run.json")
        if rc != 0 or not os.path.isfile(record):
            print(f"FAILED: exit code {rc}, avas_run.json {'present' if os.path.isfile(record) else 'missing'}")
            return 1
        eng = json.load(open(record, encoding="utf-8")).get("engine") or {}
        rows = sum(1 for ln in open(os.path.join(out, "DataSet.txt"), encoding="latin-1") if ln.strip()) \
            if os.path.isfile(os.path.join(out, "DataSet.txt")) else 0
        print("engine:", json.dumps(eng, indent=1))
        checks = {"kernel new": eng.get("kernel") == "new",
                  "library from the prebuilt folder": os.path.normcase(os.path.abspath(eng.get("file") or "")) ==
                  os.path.normcase(lib),
                  "version = stamp": eng.get("version") == stamp.get("version"),
                  "device chosen": eng.get("device") in ("cpu", "cuda", "metal"),
                  "result tier": bool(eng.get("tier")),
                  "DataSet.txt rows": rows > 10}
        for label, ok in checks.items():
            print(f"  {label:<34} {'ok' if ok else 'FAIL'}")
        return 0 if all(checks.values()) else 1


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("exports", "glibc", "deps", "metal"):
        sub.add_parser(name).add_argument("lib")
    p = sub.add_parser("build-info")
    p.add_argument("--lib", required=True)
    p.add_argument("--platform", required=True, choices=sorted(LIBRARIES))
    p.add_argument("--out", required=True)
    p.add_argument("--check", action="append", default=[], help="NAME=RESULT recorded in the file")
    p = sub.add_parser("stamp")
    p.add_argument("--out", required=True)
    p.add_argument("--require", default="")
    p.add_argument("dirs", nargs="+")
    p = sub.add_parser("smoke")
    p.add_argument("--lib-dir", default=os.path.join(REPO, "avas", "engine", "new"))
    a = ap.parse_args(argv)
    return {"exports": cmd_exports, "glibc": cmd_glibc, "deps": cmd_deps, "metal": cmd_metal,
            "build-info": cmd_build_info, "stamp": cmd_stamp, "smoke": cmd_smoke}[a.cmd](a)


if __name__ == "__main__":
    sys.exit(main())
