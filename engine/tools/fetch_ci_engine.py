"""Put the CI-built libraries of the new kernel into avas/engine/new/ (to be committed, like avas/gui/web/).

usage:
  python engine/tools/fetch_ci_engine.py                     latest successful engine.yml run of the current branch
  python engine/tools/fetch_ci_engine.py --run 123456789     a given workflow run
  python engine/tools/fetch_ci_engine.py --branch main
  python engine/tools/fetch_ci_engine.py --from-dir DIR      an artifact already downloaded / unpacked
options: --repo OWNER/NAME (default lycself/AVAS_XMU), --dest DIR (default avas/engine/new),
         --allow-partial (accept an artifact without all three platforms; never for a commit to main)

The artifact "avas-engine-new-<source hash>" of the summary job holds the three libraries and
build-stamp.json.  Nothing is installed unless
  * every library's SHA-256 and size match the stamp, no library is unlisted,
  * the stamp's engine source hash equals the hash of this checkout's engine/ (the libraries were built
    from exactly these sources; commit or stash local engine changes first, or run CI on them),
  * the library of this platform loads and reports the same source hash,
  * Windows, Linux and macOS are all present (unless --allow-partial).
Needs the GitHub CLI (gh) logged in, except with --from-dir.  Afterwards: python -m pytest
tests/test_design_rules.py, then commit avas/engine/new/.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ci_engine  # noqa: E402

REPO_DIR = ci_engine.REPO
DEFAULT_REPO = "lycself/AVAS_XMU"
WORKFLOW = "engine.yml"
CHILD = r'''
import ctypes, json, os, sys
lib = sys.argv[1]
if os.name == "nt" and hasattr(os, "add_dll_directory"):
    os.add_dll_directory(os.path.dirname(lib))
f = ctypes.CDLL(lib).avas_engine_info
f.restype = ctypes.c_char_p
print(json.loads(f().decode())["source_hash"])
'''


def gh(*args):
    try:
        return subprocess.run(["gh", *args], check=True, capture_output=True, text=True).stdout
    except FileNotFoundError:
        raise SystemExit("the GitHub CLI (gh) is needed: https://cli.github.com, then 'gh auth login'")
    except subprocess.CalledProcessError as exc:
        raise SystemExit(f"gh {' '.join(args)} failed:\n{exc.stderr.strip()}")


def current_branch():
    try:
        return subprocess.run(["git", "rev-parse", "--abbrev-ref", "HEAD"], cwd=REPO_DIR, check=True,
                              capture_output=True, text=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return ""


def latest_run(repo, branch):
    out = gh("run", "list", "--repo", repo, "--workflow", WORKFLOW, "--branch", branch, "--status", "success",
             "--limit", "1", "--json", "databaseId,headSha,url,createdAt")
    runs = json.loads(out or "[]")
    if not runs:
        raise SystemExit(f"no successful {WORKFLOW} run on branch {branch!r} of {repo}")
    return runs[0]


def find_artifact(folder):
    """The folder holding build-stamp.json below *folder* (exactly one)."""
    hits = [dp for dp, _, fs in os.walk(folder) if ci_engine.STAMP_NAME in fs]
    if len(hits) != 1:
        raise SystemExit(f"expected one {ci_engine.STAMP_NAME} below {folder}, found {len(hits)}")
    return hits[0]


def native_hash(folder):
    """Source hash reported by this platform's library in *folder*, or None when there is none."""
    name = ci_engine.LIBRARIES.get(ci_engine.native_platform())
    lib = os.path.join(folder, name or "?")
    if not name or not os.path.isfile(lib):
        return None
    out = subprocess.run([sys.executable, "-c", CHILD, os.path.abspath(lib)], capture_output=True, text=True)
    if out.returncode:
        raise SystemExit(f"{lib} does not load: {out.stderr.strip()[-400:]}")
    return out.stdout.strip()


def check(folder, tree_hash, allow_partial, run_id=None):
    need = () if allow_partial else tuple(sorted(ci_engine.LIBRARIES))
    problems = ci_engine.verify_stamp(folder, tree_hash, need)
    stamp = ci_engine.read_stamp(folder)
    if not stamp:
        problems.append(f"no {ci_engine.STAMP_NAME} in {folder}")
    if run_id and str(stamp.get("workflow_run", {}).get("id")) != str(run_id):
        problems.append(f"the stamp names workflow run {stamp.get('workflow_run', {}).get('id')}, not {run_id}")
    if not problems:
        got = native_hash(folder)
        if got is not None and got != tree_hash:
            problems.append(f"the {ci_engine.native_platform()} library reports source hash {got}")
    if problems:
        raise SystemExit("REFUSED, the libraries cannot be verified:\n  " + "\n  ".join(problems))
    return stamp


def install(src, dest):
    os.makedirs(dest, exist_ok=True)
    old = ci_engine.read_stamp(dest) if os.path.isfile(os.path.join(dest, ci_engine.STAMP_NAME)) else {}
    for name in set(old.get("files") or {}) | ci_engine.LIBRARY_NAMES | {ci_engine.STAMP_NAME}:
        p = os.path.join(dest, name)
        if os.path.isfile(p):
            os.remove(p)
    stamp = ci_engine.read_stamp(src)
    for name in list(stamp["files"]) + [ci_engine.STAMP_NAME]:
        shutil.copyfile(os.path.join(src, name), os.path.join(dest, name))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--run", help="workflow run id (default: the latest successful run of --branch)")
    ap.add_argument("--branch", help="default: the current branch")
    ap.add_argument("--repo", default=DEFAULT_REPO)
    ap.add_argument("--from-dir", help="use an artifact folder instead of downloading")
    ap.add_argument("--dest", default=os.path.join(REPO_DIR, "avas", "engine", "new"))
    ap.add_argument("--engine-dir", default=os.path.join(REPO_DIR, "engine"), help=argparse.SUPPRESS)
    ap.add_argument("--allow-partial", action="store_true")
    a = ap.parse_args(argv)
    tree = ci_engine.source_hash(a.engine_dir)
    print(f"engine source hash of this checkout: {tree}")
    with tempfile.TemporaryDirectory(prefix="avas_fetch_engine_") as tmp:
        run_id = a.run
        if a.from_dir:
            src = find_artifact(a.from_dir)
        else:
            if not run_id:
                branch = a.branch or current_branch()
                run = latest_run(a.repo, branch)
                run_id = run["databaseId"]
                print(f"run {run_id} ({run['createdAt']}, {run['headSha'][:10]}): {run['url']}")
            gh("run", "download", str(run_id), "--repo", a.repo, "--pattern", "avas-engine-new-*", "--dir", tmp)
            src = find_artifact(tmp)
        stamp = check(src, tree, a.allow_partial, run_id)
        install(src, a.dest)
    check(a.dest, tree, a.allow_partial)
    wr = stamp.get("workflow_run", {})
    print(f"installed into {a.dest} (workflow run {wr.get('number')} / {wr.get('id')}, commit {stamp.get('commit', '')[:10]}):")
    for name, entry in sorted(stamp["files"].items()):
        print(f"  {name:<22} {entry['platform']:<17} {entry['size']:>11} bytes  {entry['sha256'][:16]}")
    print("next: python -m pytest tests/test_design_rules.py, then commit avas/engine/new/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
