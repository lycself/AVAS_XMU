"""Compare summaries produced by run_engine.py with the AVAS.dll references.

usage: python compare.py WORK [--refs DIR] [--only a,b] [--quiet]
Prints, per case and file: identical / max relative difference over numeric tokens.
Exit code 0 when every compared file is byte-identical (after CRLF -> LF).
"""
import argparse
import json
import math
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TEXT = ["rc.txt", "synData.txt", "scanPhaseData.txt", "Phase.txt", "SingleParticle.txt", "DataSet.txt",
        "synParticle.txt", "ErrorLog.txt", "LongitudinalBeamLoss.txt", "FullParticle.summary.txt"]
NUM = re.compile(r"^[-+]?(\d+\.?\d*|\.\d+)([eE][-+]?\d+)?$|^[-+]?(inf|nan)(\(ind\))?$")


def read(p):
    text = open(p, "rb").read().decode("latin-1").replace("\r\n", "\n")
    # absolute field-map directory differs between machines
    return re.sub(r"\S*InputFile[\\/]+/", "<FIELD>/", text)


def to_float(tok):
    t = tok.replace("(ind)", "")
    try:
        return float(t)
    except ValueError:
        return None


def rel_diff(a, b):
    if a == b or (math.isnan(a) and math.isnan(b)):
        return 0.0
    if math.isinf(a) or math.isinf(b) or math.isnan(a) or math.isnan(b):
        return math.inf
    scale = max(abs(a), abs(b))
    return abs(a - b) / scale if scale else 0.0


def compare_text(ref, new, columns=None):
    """Returns (identical, n_lines_differing, max_rel, max_abs, note)."""
    rl, nl = ref.splitlines(), new.splitlines()
    if columns is None and ref == new:
        return True, 0, 0.0, 0.0, ""
    note = "" if len(rl) == len(nl) else f"lines {len(nl)} vs ref {len(rl)}"
    ndiff, mrel, mabs = 0, 0.0, 0.0
    for a, b in zip(rl, nl):
        ta, tb = a.split(), b.split()
        if columns is not None:
            ta = [t for i, t in enumerate(ta) if i in columns]
            tb = [t for i, t in enumerate(tb) if i in columns]
        if ta == tb:
            continue
        ndiff += 1
        if len(ta) != len(tb):
            mrel = math.inf
            continue
        for x, y in zip(ta, tb):
            if x == y:
                continue
            fx, fy = to_float(x), to_float(y)
            if fx is None or fy is None:
                mrel = math.inf
                continue
            mrel = max(mrel, rel_diff(fx, fy))
            if not (math.isinf(fx) or math.isinf(fy) or math.isnan(fx) or math.isnan(fy)):
                mabs = max(mabs, abs(fx - fy))
    if len(rl) != len(nl):
        ndiff += abs(len(rl) - len(nl))
    return ndiff == 0 and not note, ndiff, mrel, mabs, note


def compare_dst(ref, new):
    r = dict(ln.split(" ", 1) for ln in ref.splitlines() if " " in ln)
    n = dict(ln.split(" ", 1) for ln in new.splitlines() if " " in ln)
    if r.get("sha256") == n.get("sha256") and r.get("N") == n.get("N"):
        same_head = all(r.get(k) == n.get(k) for k in ("Ib", "f", "mc2", "header"))
        return same_head, 0, 0.0, 0.0, "" if same_head else "header differs"
    mrel = 0.0
    for key in ("x", "xp", "y", "yp", "phi", "W"):
        a, b = r.get(key, "").split(), n.get(key, "").split()
        for i in (1, 3):  # mean, rms
            if i < len(a) and i < len(b):
                mrel = max(mrel, rel_diff(float(a[i]), float(b[i])))
    note = "" if r.get("N") == n.get("N") else f"N {n.get('N')} vs {r.get('N')}"
    return False, 1, mrel, 0.0, note or "moments"


def single_particle_cases():
    doc = json.load(open(os.path.join(HERE, "cases.json"), encoding="utf-8"))
    return {c["name"] for c in doc["cases"] if str(c.get("beam", {}).get("particlenumber", "")) == "1"}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("work")
    ap.add_argument("--refs", default=os.path.join(HERE, "refs"))
    ap.add_argument("--only", default="")
    ap.add_argument("--quiet", action="store_true")
    a = ap.parse_args()
    only = set(a.only.split(",")) if a.only else None
    single = single_particle_cases()
    all_ok = True
    rows = []
    for case in sorted(os.listdir(a.refs)):
        if only and case not in only:
            continue
        rdir, ndir = os.path.join(a.refs, case), os.path.join(a.work, case, "summary")
        if not os.path.isdir(ndir):
            rows.append((case, "-", "MISSING", "", ""))
            all_ok = False
            continue
        files = [f for f in TEXT if os.path.exists(os.path.join(rdir, f)) or os.path.exists(os.path.join(ndir, f))]
        files += sorted(f for f in os.listdir(rdir) if f.endswith(".dst.txt"))
        rf = set(ln.split()[0] for ln in read(os.path.join(rdir, "files.txt")).splitlines() if ln.strip())
        nf = set(ln.split()[0] for ln in read(os.path.join(ndir, "files.txt")).splitlines() if ln.strip())
        if rf != nf:
            rows.append((case, "files.txt", "DIFF", "", f"only ref: {sorted(rf - nf)} only new: {sorted(nf - rf)}"))
            all_ok = False
        for f in files:
            rp, np_ = os.path.join(rdir, f), os.path.join(ndir, f)
            if not os.path.exists(rp) or not os.path.exists(np_):
                rows.append((case, f, "MISSING", "", "ref" if not os.path.exists(rp) else "new"))
                all_ok = False
                continue
            ref, new = read(rp), read(np_)
            if f.endswith(".dst.txt"):
                ok, nd, mrel, mabs, note = compare_dst(ref, new)
            elif f == "DataSet.txt" and case in single:
                # AVAS.dll never initialises the weight of a single macro particle, so the
                # statistics columns 0-27 are garbage there; compare columns 28-40 only.
                ok, nd, mrel, mabs, note = compare_text(ref, new, columns=set(range(28, 41)))
                note = (note + " cols 28-40").strip()
            else:
                ok, nd, mrel, mabs, note = compare_text(ref, new)
            all_ok &= ok
            status = "identical" if ok else "DIFF"
            rows.append((case, f, status, "" if ok else f"{nd} lines, max rel {mrel:.3g}, max abs {mabs:.3g}", note))
    w = max(len(r[0]) for r in rows) if rows else 10
    for r in rows:
        if a.quiet and r[2] == "identical":
            continue
        print(f"{r[0]:<{w}}  {r[1]:<28} {r[2]:<9} {r[3]} {r[4]}")
    print("ALL IDENTICAL" if all_ok else "DIFFERENCES FOUND")
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
