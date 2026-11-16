"""One-command M1 / M2 parity check of the new engine against the AVAS.dll references.

usage: python engine/tests/parity/run_all.py [--lib PATH] [--work DIR] [--strict [--rerun-original AVAS.dll]]

1. runs every case with compat=dll and compares with refs/ (written by AVAS.dll on Windows);
2. runs every case with the default (corrected) mode and reports the synchronous phase error of
   the V3 = 0 cavities and the differences to AVAS.dll.
OMP_NUM_THREADS is set to the thread count of the reference machine so that the particle blocks
(and with them the summation order of the beam statistics) are the same as in AVAS.dll.
Exit code 0 when compat=dll agrees with the references (byte-identical with --strict, otherwise
within 1e-10 relative, the size of libm differences between platforms).

--rerun-original (with --strict, Windows): compat=dll calls the math functions of the dynamic UCRT
like AVAS.dll does, and the references were written on Windows 11; another Windows (the CI runner is
Windows Server 2022) can round a few of them differently.  The cases that differ are then run again
with the original AVAS.dll on this machine: the check passes when the new engine is byte-identical to
that run, and fails when AVAS.dll still reproduces the references here (a real difference).
"""
import argparse
import json
import math
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
import compare  # noqa: E402


def default_lib():
    for name in ("libavas_engine.dylib", "libavas_engine.so", "avas_engine.dll"):
        for sub in ("build", os.path.join("build", "Release")):
            p = os.path.join(ENGINE, sub, name)
            if os.path.exists(p):
                return p
    raise SystemExit("engine library not found; build it first: cmake -S engine -B engine/build && cmake --build engine/build")


def run(lib, compat, work, threads, only=None):
    env = dict(os.environ, OMP_NUM_THREADS=str(threads))
    subprocess.run([sys.executable, os.path.join(HERE, "run_engine.py"), "--lib", lib, "--compat", compat,
                    "--work", work] + (["--only", ",".join(sorted(only))] if only else []),
                   check=True, env=env, stdout=subprocess.DEVNULL)


def rerun_original(original, work, threads, rows):
    """--rerun-original: the cases of *rows* that differ from refs/, run with AVAS.dll on this machine.
    Returns 0 when the new engine equals AVAS.dll here in every one of them."""
    cases = {r[0] for r in rows if r[2] in ("DIFF", "MISSING")}
    print(f"\nstrict differences in {len(cases)} cases: running them with the original {original} on this machine")
    orig = os.path.join(work, "original")
    # a new-engine library given here (a self-test of this check) runs as compat=dll
    run(os.path.abspath(original), "dll" if "avas_engine" in os.path.basename(original) else "-", orig, threads, cases)
    # the fresh AVAS.dll outputs in the layout of refs/ (<case>/...)
    refs_here = os.path.join(work, "original_refs")
    shutil.rmtree(refs_here, ignore_errors=True)
    for case in cases:
        if os.path.isdir(os.path.join(orig, case, "summary")):
            shutil.copytree(os.path.join(orig, case, "summary"), os.path.join(refs_here, case))
    same_here, rows_here = compare.compare_dirs(os.path.join(work, "dll"), refs_here, cases)
    _, rows_orig = compare.compare_dirs(orig, os.path.join(HERE, "refs"), cases)
    orig_diff = {(r[0], r[1]) for r in rows_orig if r[2] in ("DIFF", "MISSING")}
    bad = 0
    for r in sorted(r for r in rows if r[2] in ("DIFF", "MISSING")):
        here = next((h for h in rows_here if h[:2] == r[:2]), None)
        new_eq_orig = here is not None and here[2] in ("identical", "close")
        orig_moved = (r[0], r[1]) in orig_diff
        if new_eq_orig and orig_moved:
            verdict = "environment: AVAS.dll on this machine gives the same bytes as the new engine"
        elif not orig_moved:
            verdict = "REAL DIFFERENCE: AVAS.dll on this machine still reproduces the reference"
            bad += 1
        else:
            verdict = "DIFFERENT FROM BOTH the reference and AVAS.dll on this machine"
            bad += 1
        print(f"  {r[0]:<36} {r[1]:<28} {verdict}")
    if not same_here:
        others = [h for h in rows_here if h[2] in ("DIFF", "MISSING") and (h[0], h[1]) not in {(r[0], r[1]) for r in rows}]
        for h in others:
            print(f"  {h[0]:<36} {h[1]:<28} differs from AVAS.dll on this machine although it matched the reference")
            bad += 1
    if bad:
        print(f"STRICT PARITY FAILED: {bad} differences not explained by the machine")
        return 1
    print("STRICT PARITY: byte-identical to AVAS.dll on this machine (the references differ only by the UCRT of the machine that wrote them)")
    return 0


ELEMENTS = ("drift", "field", "quad", "solenoid", "steerer", "bend", "edge", "multipole", "rfq")


def lattice_elements(case):
    """Element rows in lattice order; None for lattices with superpose blocks (rows are synthesised)."""
    rows = []
    for ln in case["lattice"]:
        w = ln.split()
        if w and w[0].startswith("superpose"):
            return None
        if w and w[0] in ELEMENTS:
            rows.append(w)
    return rows


def num(tok):
    v = compare.to_float(tok)
    return math.nan if v is None else v


def read_rows(path):
    if not os.path.exists(path):
        return []
    return [ln.split() for ln in open(path, encoding="latin-1").read().replace("\r", "").splitlines() if ln.strip()]


def default_report(doc, work):
    print("\nDefault mode (defects corrected) against AVAS.dll.  DataSet max dev = largest difference of a")
    print("column relative to the largest value of that column; exit W rel = relative change of the exit")
    print("kinetic energy of the synchronous particle.")
    print(f"{'case':<28} {'max |phi_s - target| (deg)':>27} {'phi_s shift vs DLL (deg)':>25} {'DataSet max dev':>16} {'exit W rel':>11}")
    for case in doc["cases"]:
        name = case["name"]
        if case.get("reference_only") or name.startswith(("S5", "E")) or name.startswith("RFQ"):
            continue
        if case.get("new_engine_only"):
            continue
        sub = "output_0" if os.path.isdir(os.path.join(work, name, "summary", "output_0")) else ""
        mine = read_rows(os.path.join(work, name, "summary", sub, "synData.txt"))
        ref = read_rows(os.path.join(HERE, "refs", name, sub, "synData.txt"))
        elems = lattice_elements(case)
        dev, shift = [], []
        for r, q in zip(mine, ref):
            if len(r) < 8 or r[1] != "field" or r[6] == "invalid":
                continue
            i = int(r[0])
            w = elems[i] if elems is not None and i < len(elems) else None
            if w and w[3] == "0":
                dev.append(abs(num(r[6]) - float(w[6])))
            if len(q) > 6:
                shift.append(num(r[6]) - num(q[6]))
        ds_new = read_rows(os.path.join(work, name, "summary", sub, "DataSet.txt"))
        ds_ref = read_rows(os.path.join(HERE, "refs", name, sub, "DataSet.txt"))
        cols = range(28, 41) if name in compare.single_particle_cases() else range(41)
        mrel = 0.0  # largest difference relative to the largest |value| of its column
        for c in cols:
            ref_col = [num(r[c]) for r in ds_ref if len(r) > c]
            scale = max((abs(v) for v in ref_col if math.isfinite(v)), default=0.0)
            diff = max((abs(num(a[c]) - num(b[c])) for a, b in zip(ds_ref, ds_new)
                        if len(a) > c and len(b) > c and math.isfinite(num(a[c])) and math.isfinite(num(b[c]))),
                       default=0.0)
            if scale:
                mrel = max(mrel, diff / scale)
        gz = lambda rows: num(rows[-1][5]) if rows and len(rows[-1]) > 5 else math.nan  # exit gamma*beta
        wn, wr = gz(mine), gz(ref)
        wrel = abs(math.hypot(1, wn) - math.hypot(1, wr)) / (math.hypot(1, wr) - 1)
        dv = f"{max(dev):.2e}" if dev else "-"
        sh = ", ".join(f"{s:+.4f}" for s in shift) if shift else "-"
        print(f"{name:<28} {dv:>27} {sh:>25} {mrel:>16.3g} {wrel:>11.2e}")


def dst_mean_w(path):
    """Mean kinetic energy (MeV) of a .dst file (column 5 of every particle)."""
    import struct
    raw = open(path, "rb").read()
    n = struct.unpack("<i", raw[2:6])[0]
    w = [struct.unpack("<d", raw[23 + (6 * i + 5) * 8: 23 + (6 * i + 6) * 8])[0] for i in range(n)]
    return sum(w) / n


def energy_check(doc, work):
    """Default mode, magnetic elements and drifts only: every particle keeps |gamma*beta|, so
    DataSet column 0 (mean kinetic energy, as printed) never changes and the mean W of the
    outData_*.dst files equals the entrance value.  Returns the number of failures."""
    print("\nEnergy conservation in the default mode (cases without electric fields):")
    fails = 0
    for case in doc["cases"]:
        if not case.get("conserves_energy"):
            continue
        name = case["name"]
        sdir = os.path.join(work, name, "summary")
        ds = read_rows(os.path.join(sdir, "DataSet.txt"))
        col0 = sorted({r[0] for r in ds})
        if "readparticledistribution" in case.get("beam", {}):
            w0 = dst_mean_w(os.path.join(HERE, list(case["copy"].values())[0]))
            tol = 1e-12
        else:
            sp = [r for r in read_rows(os.path.join(sdir, "SingleParticle.txt")) if len(r) >= 8]
            w0 = num(sp[0][6]) if sp else math.nan  # first tracked row (after the 3 header lines)
            tol = 1e-11  # SingleParticle.txt has 12 decimals
        wdst = []
        for f in sorted(os.listdir(sdir)):
            if f.startswith("outData_") and f.endswith(".dst.txt"):
                for ln in open(os.path.join(sdir, f), encoding="latin-1"):
                    if ln.startswith("W mean"):
                        wdst.append(float(ln.split()[2]))
        dev = max((abs(w - w0) / w0 for w in wdst), default=math.inf)
        ok = len(col0) == 1 and dev <= tol
        fails += 0 if ok else 1
        print(f"{name:<28} {'ok  ' if ok else 'FAIL'} DataSet col 0: {' '.join(col0[:4])}   dst W rel dev {dev:.2e}")
    return fails


def read_dst(path):
    import struct
    raw = open(path, "rb").read()
    n = struct.unpack("<i", raw[2:6])[0]
    f = struct.unpack("<d", raw[14:22])[0] * 1e6
    rows = [struct.unpack("<6d", raw[23 + 48 * i: 71 + 48 * i]) for i in range(n)]
    return rows, f


def beam_keys(path):
    keys = {}
    for ln in open(path, encoding="utf-8", errors="replace"):
        w = ln.split("!")[0].split()
        if w:
            keys[w[0].lower()] = w[1:]
    return keys


def ks(values, cdf):
    v = sorted(values)
    n = len(v)
    return max(max(abs((i + 1) / n - cdf(x)), abs(i / n - cdf(x))) for i, x in enumerate(v))


def distribution_check(doc, work):
    """Default mode: the generated inData.dst against the Twiss input (statistics) and the radial
    laws of KV / WB / PB; the transverse-longitudinal correlation must vanish."""
    print("\nGenerated distributions in the default mode (inData.dst against the beam.txt input):")
    fails = 0
    cdf4 = {"KV": None, "WB": lambda s: s * s, "PB": lambda s: 3 * s * s - 2 * s ** 3}
    cdf2 = {"KV": lambda s: s, "WB": lambda s: 2 * s - s * s, "PB": lambda s: 1 - (1 - s) ** 3}
    rmax = {"KV": 4.0, "WB": 6.0, "PB": 8.0}
    for case in doc["cases"]:
        if not case.get("generation") or case.get("input", {}).get("probe"):
            continue
        name = case["name"]
        p = os.path.join(work, name, "OutputFile", "inData.dst")
        if not os.path.exists(p):
            print(f"{name:<24} FAIL no inData.dst")
            fails += 1
            continue
        rows, f = read_dst(p)
        n = len(rows)
        k = beam_keys(os.path.join(work, name, "inputs", "beam.txt"))
        mc2 = float(k.get("particlerestmass", ["938.272029"])[0])
        w0 = float(k["kneticenergy"][0])
        g0 = w0 / mc2 + 1
        bg0 = math.sqrt(g0 * g0 - 1)
        dist = k.get("distribution", ["GS", "GS"])
        dc = k.get("beamtype", [""])[0] == "dc" or "twissz" not in k
        msgs, ok = [], True
        tol = 4.0 * math.sqrt(2.0 / n) + 0.01   # rms emittance / beta / alpha: ~4 sigma

        def plane(q, qp, tw, label, kind, cdf):
            nonlocal ok
            a, b, e = (float(v) for v in tw)
            eps = e * 1e-6 / bg0 if label != "z" else e * 1e-6 / (bg0 * g0 * g0)
            mq, mp = sum(q) / n, sum(qp) / n
            sqq = sum((x - mq) ** 2 for x in q) / n
            spp = sum((x - mp) ** 2 for x in qp) / n
            sqp = sum((x - mq) * (y - mp) for x, y in zip(q, qp)) / n
            em = math.sqrt(sqq * spp - sqp * sqp)
            be, al = sqq / em, -sqp / em
            dev = max(abs(em / eps - 1), abs(be / b - 1), abs(al - a) / max(1.0, abs(a)))
            if dev > tol:
                ok = False
            msgs.append(f"{label}: eps {em / eps:.4f} beta {be / b:.4f} alpha {al:+.3f}/{a:+.3f}")
            gam = (1 + a * a) / b
            return [(gam * x * x + 2 * a * x * y + b * y * y) / eps for x, y in zip(q, qp)]

        # inData.dst gives each particle where it crosses z = 0; undo that drift (x' z) to get the
        # generated snapshot back (z from the phase)
        zc = []
        for r in rows:
            g = r[5] / mc2 + 1
            pt = math.sqrt(g * g - 1)
            vz = pt / math.sqrt(1 + r[1] ** 2 + r[3] ** 2) / g * 299792458.0
            zc.append(0.0 if dc else -r[4] / (2 * math.pi * f) * vz)
        x = [r[0] / 100 + r[1] * zi for r, zi in zip(rows, zc)]
        xp = [r[1] for r in rows]
        y = [r[2] / 100 + r[3] * zi for r, zi in zip(rows, zc)]
        yp = [r[3] for r in rows]
        jx = plane(x, xp, k["twissx"], "x", dist[0], None)
        jy = plane(y, yp, k["twissy"], "y", dist[0], None)
        if dist[0] in rmax:
            s4 = [(u + v) / rmax[dist[0]] for u, v in zip(jx, jy)]
            if dist[0] == "KV":
                d = max(abs(v - 1) for v in s4)
                msgs.append(f"KV J/4eps max dev {d:.1e}")
                ok &= d < 1e-9
            else:
                d = ks([min(v, 1.0) for v in s4], cdf4[dist[0]])
                msgs.append(f"{dist[0]} radial KS {d:.3f}")
                ok &= d < 1.63 / math.sqrt(n) * 1.5 and max(s4) <= 1 + 1e-3
        if not dc:
            # z, delta from phi, W (delta on the total momentum)
            z, dl = [], []
            for r in rows:
                g = r[5] / mc2 + 1
                pt = math.sqrt(g * g - 1)
                vz = pt / math.sqrt(1 + r[1] ** 2 + r[3] ** 2) / g * 299792458.0
                z.append(-r[4] / (2 * math.pi * f) * vz)
                dl.append(pt / bg0 - 1)
            jz = plane(z, dl, k["twissz"], "z", dist[1], None)
            if dist[1] in rmax:
                sz = [v / rmax[dist[1]] for v in jz]
                d = ks([min(v, 1.0) for v in sz], cdf2[dist[1]])
                msgs.append(f"{dist[1]} long. KS {d:.3f}")
                ok &= d < 1.63 / math.sqrt(n) * 1.5 and max(sz) <= 1 + 1e-3

            def corr(a, b):
                ma, mb = sum(a) / n, sum(b) / n
                c = sum((u - ma) * (v - mb) for u, v in zip(a, b))
                return c / math.sqrt(sum((u - ma) ** 2 for u in a) * sum((v - mb) ** 2 for v in b))
            cx, cp = corr(x, [r[4] for r in rows]), corr(xp, [r[5] for r in rows])
            msgs.append(f"corr(x,phi) {cx:+.3f} corr(x',W) {cp:+.3f}")
            ok &= abs(cx) < 4 / math.sqrt(n) and abs(cp) < 4 / math.sqrt(n)
        fails += 0 if ok else 1
        print(f"{name:<24} {'ok  ' if ok else 'FAIL'} " + "; ".join(msgs))
    return fails


def indata_sha_check(doc, work):
    """Default mode: the inData.dst of a seed must be bit-identical on every platform."""
    import hashlib
    fails, seen = 0, 0
    for case in doc["cases"]:
        want = case.get("default_indata_sha256")
        if not want:
            continue
        seen += 1
        p = os.path.join(work, case["name"], "OutputFile", "inData.dst")
        got = hashlib.sha256(open(p, "rb").read()).hexdigest() if os.path.exists(p) else "missing"
        if got != want:
            fails += 1
            print(f"{case['name']:<24} FAIL inData.dst sha256 {got[:16]} expected {want[:16]}")
    print(f"\ninData.dst of the default mode bit-identical to the recorded value: {seen - fails}/{seen}")
    return fails


def segment_drift_check(doc, work):
    """Default mode: particles past the end of a time-step segment drift without field, so in the
    first segment (a drift) the rms slopes x', y' (DataSet columns 17, 19) do not change."""
    fails = 0
    print("\nField-free drift past the segment end (default mode):")
    for case in doc["cases"]:
        if not case.get("segment_drift_check"):
            continue
        ds = read_rows(os.path.join(work, case["name"], "summary", "DataSet.txt"))
        first = []
        for r in ds:
            if first and int(r[39]) != int(first[-1][39]) + 1:
                break
            first.append(r)
        cx = {r[17] for r in first}
        cy = {r[19] for r in first}
        ok = len(cx) == 1 and len(cy) == 1 and len(first) > 2
        fails += 0 if ok else 1
        print(f"{case['name']:<24} {'ok  ' if ok else 'FAIL'} {len(first)} rows, rms x' {sorted(cx)[:2]}, rms y' {sorted(cy)[:2]}")
    return fails


def output_digest(out):
    """sha256 of DataSet.txt and outData_*.dst, of OutputFile and of every error sample output_<k>/."""
    import hashlib
    h = hashlib.sha256()
    dirs = [out] + [os.path.join(out, d) for d in sorted(os.listdir(out)) if d.startswith("output_")]
    for d in dirs:
        if not os.path.isdir(d):
            continue
        for f in sorted(os.listdir(d)):
            if f == "DataSet.txt" or f.startswith("outData_"):
                h.update(f.encode())
                h.update(open(os.path.join(d, f), "rb").read())
    return h.hexdigest()


def error_sample_digest(out):
    """sha256 of the error sample files (StaticErrorData.txt, DynamicErrorData.txt) of every output_<k>/."""
    import hashlib
    h = hashlib.sha256()
    for d in sorted(x for x in os.listdir(out) if x.startswith("output_")):
        for f in ("StaticErrorData.txt", "DynamicErrorData.txt"):
            p = os.path.join(out, d, f)
            if os.path.exists(p):
                h.update(f"{d}/{f}".encode())
                h.update(open(p, "rb").read().replace(b"\r\n", b"\n"))
    return h.hexdigest()


def error_study_check(doc, work):
    """Default mode, kernel error studies: (1) every sample equals a run of the nominal lattice with the
    same errors written by hand (no accumulation between samples); (2) the error samples drawn from
    a seed are the same on every platform."""
    fails = 0
    print("\nError samples in the default mode:")
    for case in doc["cases"]:
        m = case.get("default_manual_of")
        if not m:
            continue
        a = os.path.join(work, m["case"], "OutputFile", f"output_{m['sample']}")
        b = os.path.join(work, case["name"], "OutputFile", "output_0")
        skip = {"SimulationTime.txt", "synParticle.txt"}
        if not os.path.isdir(b):   # the hand-made run has no error lines
            b = os.path.join(work, case["name"], "OutputFile")
            skip |= {"StaticErrorData.txt", "DynamicErrorData.txt", "errorData.seed.txt"}
        files = sorted(f for f in os.listdir(a) if f not in skip) if os.path.isdir(a) else []
        if m.get("only") == "dst_transverse":
            # the synchronous particle never sees element errors, so only the absolute transverse
            # particle coordinates of the outData files can be compared with the displaced lattice
            files = [f for f in files if f.startswith("outData_") and f.endswith(".dst")]

            def same(f):
                ra, fa = read_dst(os.path.join(a, f))
                rb, fb = read_dst(os.path.join(b, f))
                return len(ra) == len(rb) and all(x[:4] == y[:4] for x, y in zip(ra, rb))
            bad = [f for f in files if not os.path.exists(os.path.join(b, f)) or not same(f)]
        else:
            bad = [f for f in files if not os.path.exists(os.path.join(b, f)) or
                   open(os.path.join(a, f), "rb").read() != open(os.path.join(b, f), "rb").read()]
        ok = bool(files) and not bad
        fails += 0 if ok else 1
        print(f"{case['name']:<24} {'ok  ' if ok else 'FAIL'} {m['case']} sample {m['sample']} = nominal lattice + "
              f"the same errors ({len(files)} files{', differ: ' + ' '.join(bad) if bad else ''})")
    for case in doc["cases"]:
        want = case.get("default_errdata_sha256")
        if not want:
            continue
        got = error_sample_digest(os.path.join(work, case["name"], "OutputFile"))
        ok = got == want
        fails += 0 if ok else 1
        print(f"{case['name']:<24} {'ok  ' if ok else 'FAIL'} error samples sha256 {got[:16]}"
              f"{'' if ok else ' expected ' + want[:16]}")
    return fails


def loop_feature_check(doc, work):
    """Default mode, M5b features: boundary.txt hits lie on the pipe wall (straight pipe of LB1),
    lpboundary keeps every particle within half an RF period of the synchronous one, longlimits keeps
    the phase window, secondary particles are written at every output plane and read SeParticle.txt."""
    fails = 0
    print("\nBoundary, lpboundary, longlimits and secondary particles (default mode):")
    cd = os.path.join(work, "LB1_boundary_straight_single", "OutputFile", "collisionData.txt")
    rows = read_rows(cd)
    ok = len(rows) == 1 and abs(math.hypot(num(rows[0][0]) - 0.0, num(rows[0][1])) - 0.01) < 2e-6
    fails += 0 if ok else 1
    print(f"{'LB1_boundary_straight_single':<32} {'ok  ' if ok else 'FAIL'} hit radius {math.hypot(num(rows[0][0]), num(rows[0][1])) if rows else math.nan:.6g} m (pipe 0.01 m)")
    for name in ("LP1_lpboundary_dst", "LP2_lpboundary_fft_dst", "LP3_lpboundary_spicnic_dst"):
        ds = read_rows(os.path.join(work, name, "OutputFile", "DataSet.txt"))
        # column 26 = max |dz| (m); half period beta*lambda/2 from the synchronous gamma*beta_z (col 34)
        worst = max((num(r[26]) / (0.5 * 299792458.0 / 162.5e6 * num(r[34]) / math.hypot(1, num(r[34]))) for r in ds[1:]),
                    default=math.inf)
        ok = worst <= 1.0 + 1e-9
        fails += 0 if ok else 1
        print(f"{name:<32} {'ok  ' if ok else 'FAIL'} max |dz| / (beta lambda / 2) = {worst:.6f}")
    ds = read_rows(os.path.join(work, "LL1_longlimits_dst", "OutputFile", "DataSet.txt"))
    worst = max((num(r[26]) / (60.0 / 360.0 / 162.5e6 * 299792458.0 * num(r[34]) / math.hypot(1, num(r[34]))) for r in ds[2:]),
                default=math.inf)
    ok = worst <= 1.0 + 1e-6
    fails += 0 if ok else 1
    print(f"{'LL1_longlimits_dst':<32} {'ok  ' if ok else 'FAIL'} max |dz| / phase window = {worst:.6f}")
    for case in doc["cases"]:
        want = case.get("default_secondary_planes")
        if not want:
            continue
        out = os.path.join(work, case["name"], "OutputFile")
        files = sorted(f for f in os.listdir(out) if f.startswith("outData_") and f.endswith(".txt")) if os.path.isdir(out) else []
        lines = [len(read_rows(os.path.join(out, f))) for f in files]
        ok = len(files) == want and all(n == 3 for n in lines)
        fails += 0 if ok else 1
        print(f"{case['name']:<32} {'ok  ' if ok else 'FAIL'} {len(files)} output planes, particles {lines}")
    return fails


def secondary_steerer_check(work):
    """G-31: a steerer has no effect on secondary particles. The particles written at the output planes
    (outData_*.txt) of SB6 (with a steerer) and SB6n (without) are identical, in the AVAS.dll references,
    in compat=dll and in the default mode."""
    fails = 0
    print("\nSteerer ignored by secondary particles (SB6 against SB6n):")
    for label, base, sub in (("AVAS.dll", os.path.join(HERE, "refs"), ""), ("compat=dll", os.path.join(work, "dll"), "summary"),
                             ("default", os.path.join(work, "none"), "summary")):
        dirs = [os.path.join(base, n, sub) for n in ("SB6_secondary_steerer", "SB6n_secondary_no_steerer")]
        if not all(os.path.isdir(d) for d in dirs):
            print(f"{label:<12} skip (no results)")
            continue
        files = [sorted(f for f in os.listdir(d) if f.startswith("outData_") and f.endswith(".txt")) for d in dirs]
        same = bool(files[0]) and files[0] == files[1] and all(
            open(os.path.join(dirs[0], f), "rb").read() == open(os.path.join(dirs[1], f), "rb").read() for f in files[0])
        fails += 0 if same else 1
        print(f"{label:<12} {'ok  ' if same else 'FAIL'} {len(files[0])} output planes")
    return fails


def libpic_keyword_check(work):
    """Default mode, keywords from libPIC.so (M6b decisions): s2picnic error, scmethod in any case,
    statoutputinterval (rows whose step number is a multiple of N), BeamSet.plt size warning; in
    compat=dll statoutputinterval is an unknown keyword (no effect)."""
    import re
    none, dll = os.path.join(work, "none"), os.path.join(work, "dll")
    fails = 0
    print("\nlibPIC.so keywords in the default mode (scmethod, statoutputinterval, BeamSet.plt estimate):")

    def log(case, root=none):
        p = os.path.join(root, case, "log.txt")
        return open(p, encoding="utf-8", errors="replace").read() if os.path.exists(p) else ""

    def rows(case, root=none):
        p = os.path.join(root, case, "OutputFile", "DataSet.txt")
        return open(p, "rb").read().splitlines() if os.path.exists(p) else None

    def dsts(case, root=none):
        out = os.path.join(root, case, "OutputFile")
        if not os.path.isdir(out):
            return None
        return {f: open(os.path.join(out, f), "rb").read() for f in sorted(os.listdir(out)) if f.startswith("outData_")}

    def report(label, ok, detail=""):
        nonlocal fails
        fails += 0 if ok else 1
        print(f"{label:<34} {'ok  ' if ok else 'FAIL'} {detail}")

    lg = log("PX1_scmethod_s2picnic")
    report("PX1 s2picnic -> error", "use PICNIC" in lg and not dsts("PX1_scmethod_s2picnic"),
           next((ln.strip() for ln in lg.splitlines() if "use PICNIC" in ln), "no message"))
    for case, ref in (("PX2_scmethod_fft_lower", "SC2_fft_S1"), ("PX3_scmethod_spicnic_mixed", "SC1_spicnic_S1"),
                      ("PX4_stat1", "SC1_spicnic_S1")):
        same = output_digest(os.path.join(none, case, "OutputFile")) == output_digest(os.path.join(none, ref, "OutputFile"))
        report(f"{case.split('_')[0]} = {ref}", same, "DataSet.txt and outData_*.dst bytes")
    report("PX4 usingaggregation warned", "usingaggregation of the GPU kernel libPIC.so" in log("PX4_stat1"))
    for case, ref, every in (("PX5_stat7", "SC1_spicnic_S1", 7), ("PX6_stat0", "SC1_spicnic_S1", 0),
                             ("PX7_stat3_Z1", "SC5_spicnic_Z1", 3)):
        mine, full = rows(case), rows(ref)
        want = [r for r in (full or []) if every and int(r.split()[39]) % every == 0]
        ok = mine is not None and full is not None and mine == want and dsts(case) == dsts(ref)
        report(f"{case.split('_')[0]} statoutputinterval {every}", ok, f"{len(mine or [])} of {len(full or [])} rows, dst identical")
    # compat=dll: statoutputinterval is unknown to AVAS.dll (warning only, every row written)
    ok = rows("PX5_stat7", dll) is not None and rows("PX5_stat7", dll) == rows("SC1_spicnic_S1", dll) and \
        "Contains unknown keywords" in log("PX5_stat7", dll)
    report("PX5 compat=dll: keyword unknown", ok)
    lg = log("PX8_beamset_warning")
    m = re.search(r"BeamSet.plt is estimated at ([0-9.eE+-]+) GB", lg)
    bs = os.path.join(none, "PX8_beamset_warning", "OutputFile", "BeamSet.plt")
    if m and os.path.exists(bs):
        ratio = float(m.group(1)) * 1073741824.0 / os.path.getsize(bs)
        report("PX8 BeamSet.plt warning", 0.8 <= ratio <= 1.5, f"estimate / actual size = {ratio:.3f}")
    else:
        report("PX8 BeamSet.plt warning", False, "no warning or no BeamSet.plt")
    return fails


def m10_check(work):
    """Default mode, M10 (features of the 2026-07-22 original): reference energy of a .dst file (D1), initpos /
    initmom (D2), SingleParticlePlane.txt (D3), named and close output planes (D4), boundary shapes 3 / 4 / 0
    and the thin collimator (D5), RF field with Ke = 0 (D6), periodic longitudinal box (D7)."""
    fails = 0
    print("\nM10 features in the default mode:")

    def out(case):
        return os.path.join(work, case, "OutputFile")

    def report(label, ok, detail=""):
        nonlocal fails
        fails += 0 if ok else 1
        print(f"{label:<34} {'ok  ' if ok else 'FAIL'} {detail}")

    def sync_w0(case):  # synchronous kinetic energy of the first DataSet row (gamma*beta_z, column 34)
        r = read_rows(os.path.join(out(case), "DataSet.txt"))
        return (math.hypot(1.0, num(r[0][34])) - 1.0) * 938.272088 if r else math.nan

    wm = dst_mean_w(os.path.join(HERE, "data", "hwr010_seed12345.dst"))
    w = sync_w0("M10_D1_dst_mean")
    report("D1 kneticenergy -1: mean", abs(w - wm) < 2e-5 * wm, f"W_s {w:.6f}, mean {wm:.6f} MeV")
    w = sync_w0("M10_D1_dst_far")
    lg = open(os.path.join(work, "M10_D1_dst_far", "log.txt"), encoding="utf-8", errors="replace").read()
    report("D1 kneticenergy 1.6: reference", abs(w - 1.6) < 2e-5 and "differs from the mean energy" in lg, f"W_s {w:.6f} MeV, warned")
    # D2: the new units give the start of S2a; both names: the new ones win
    a, b = (read_rows(os.path.join(out(c), "DataSet.txt")) for c in ("M10_D2_initpos_single", "S2a_single_offaxis_hwr_sol"))
    dev = max((abs(num(x) - num(y)) / max(abs(num(y)), 1e-30) for ra, rb in zip(a, b) for x, y in zip(ra[29:35], rb[29:35])
               if math.isfinite(num(x)) and math.isfinite(num(y)) and abs(num(y)) > 1e-12), default=math.inf)
    sp = lambda c: open(os.path.join(out(c), "SingleParticle.txt"), "rb").read() if os.path.exists(os.path.join(out(c), "SingleParticle.txt")) else b"?"
    report("D2 initpos / initmom = S2a", len(a) == len(b) > 0 and dev < 1e-9, f"{len(a)} rows, largest relative difference {dev:.2e}")
    report("D2 both names: new ones win", sp("M10_D2_both_single") == sp("M10_D2_initpos_single"))
    # D3: one line per output plane, x and W as in outData_*.dst
    rows = read_rows(os.path.join(out("S2a_single_offaxis_hwr_sol"), "SingleParticlePlane.txt"))
    dsts = sorted(f for f in os.listdir(out("S2a_single_offaxis_hwr_sol")) if f.startswith("outData_") and f.endswith(".dst"))
    ok = len(rows) == len(dsts) and all(len(r) == 7 for r in rows)
    for r, f in zip(rows, dsts):
        p0, _ = read_dst(os.path.join(out("S2a_single_offaxis_hwr_sol"), f))
        ok = ok and abs(float(f[8:-4]) - num(r[0])) < 1e-6 and abs(p0[0][0] / 100 - num(r[2])) < 1e-12 and abs(p0[0][5] - num(r[1])) < 1e-9
    report("D3 SingleParticlePlane.txt", ok, f"{len(rows)} planes")
    # D4
    files = set(os.listdir(out("M10_D4_named_planes")))
    same_end = "EXIT_0.310000.dst" in files and open(os.path.join(out("M10_D4_named_planes"), "EXIT_0.310000.dst"), "rb").read() == \
        open(os.path.join(out("M10_D4_named_planes"), "outData_0.310000.dst"), "rb").read()
    report("D4 named planes", "MID_0.030000.dst" in files and same_end, "MID_0.030000.dst, EXIT = outData at the end")
    for k, f, exact in (("P1", "P1_0.020000.dst", True), ("P2", "P2_0.023000.dst", True), ("P3", "P3_0.023500.dst", False)):
        pa, pb = os.path.join(out("M10_D4_close_planes"), f), os.path.join(out(f"M10_D4_close_{k}"), f)
        if not (os.path.exists(pa) and os.path.exists(pb)):
            report(f"D4 close planes {k}", False, "missing")
            continue
        ra, rb = read_dst(pa)[0], read_dst(pb)[0]
        big = max((abs(x - y) for r, q in zip(ra, rb) for x, y in zip(r, q)), default=math.inf)
        ok = open(pa, "rb").read() == open(pb, "rb").read() if exact else (len(ra) == len(rb) and big < 1e-12)
        report(f"D4 close planes {k}", ok, ("byte-identical to the plane alone" if exact else
                                            f"extrapolated over one step: largest difference {big:.1e}"))
    # D5: hits on the walls
    hits = read_rows(os.path.join(out("M10_D5_rect_ellipse_dst"), "collisionData.txt"))
    worst = 0.0
    faces = 0
    for h in hits:
        x, y, z = num(h[0]), num(h[1]), num(h[2])
        d = max(abs(x) / 0.003, abs(y) / 0.0012) if 0.1 <= z < 0.2 else ((x / 0.003) ** 2 + (y / 0.0012) ** 2 if 0.2 <= z <= 0.3 else 9)
        if min(abs(z - 0.1), abs(z - 0.2)) < 1e-12 and d >= 1.0:   # on the entrance face, outside the aperture
            faces += 1
            continue
        worst = max(worst, abs(d - 1.0))
    # collisionData.txt has 6 significant digits
    report("D5 rectangle / ellipse walls", len(hits) > 100 and worst < 2e-5 and all(len(h) == 12 for h in hits),
           f"{len(hits)} hits ({faces} on an entrance face), largest distance from the wall {worst:.1e} (relative), 12 columns")
    hits = read_rows(os.path.join(out("M10_D5_composite_dst"), "collisionData.txt"))
    worst = 0.0
    for h in hits:
        x, y, z = num(h[0]), num(h[1]), num(h[2])
        dc = math.hypot(x, y) / 0.0025
        dr = max(abs(x) / 0.004, abs(y) / 0.0015) if 0.15 <= z <= 0.2 else 0.0
        if (abs(z - 0.1) < 1e-12 and dc >= 1.0) or (abs(z - 0.15) < 1e-12 and dr >= 1.0):
            continue   # entrance face of a sub-shape, outside its aperture
        worst = max(worst, min(abs(dc - 1), abs(dr - 1)))
    report("D5 composite block", len(hits) > 100 and worst < 2e-5 and 0.1 - 1e-12 <= min(num(h[2]) for h in hits),
           f"{len(hits)} hits on a wall or an entrance face")
    ref = read_dst(os.path.join(out("M10_D5_thin_reference_dst"), "COLL_0.100000.dst"))[0]
    want = sum(1 for r in ref if math.hypot(r[0], r[2]) / 100 >= 0.0015)
    hits = read_rows(os.path.join(out("M10_D5_thin_collimator_dst"), "collisionData.txt"))
    report("D5 thin collimator", abs(len(hits) - want) <= max(2, want // 500) and all(0.1 - 1e-9 <= num(h[2]) <= 0.101 for h in hits),
           f"{len(hits)} hits, {want} particles outside r = 1.5 mm at its entrance")
    # D6
    same = all(open(os.path.join(out("M10_D6_ke0_cavity_dst"), f), "rb").read() == open(os.path.join(out("M10_D6_drift_dst"), f), "rb").read()
               for f in ("DataSet.txt", "outData_0.570000.dst")) if os.path.isdir(out("M10_D6_ke0_cavity_dst")) else False
    report("D6 Ke = 0 cavity = drift", same, "DataSet.txt and outData bytes")
    # D7: runs, no particle off the mesh longitudinally (all deposited)
    ds = read_rows(os.path.join(out("M10_D7_periodic_fft_dst"), "DataSet.txt"))
    report("D7 periodic box (FFT)", len(ds) > 10 and int(ds[-1][28]) > 5000, f"{len(ds)} rows")
    return fails


def thread_check(lib, work, names):
    """Default mode: the same results with 1 thread and with OMP_NUM_THREADS / all cores."""
    one = os.path.join(work, "none_1thread")
    env = dict(os.environ, OMP_NUM_THREADS="1", AVAS_ENGINE_THREADS="1")
    subprocess.run([sys.executable, os.path.join(HERE, "run_engine.py"), "--lib", lib, "--work", one,
                    "--only", ",".join(names)], check=True, env=env, stdout=subprocess.DEVNULL)
    fails = 0
    print("\nDefault mode, 1 thread against the multi-threaded run (DataSet.txt and outData_*.dst bytes):")
    for n in names:
        same = output_digest(os.path.join(one, n, "OutputFile")) == \
            output_digest(os.path.join(work, "none", n, "OutputFile"))
        fails += 0 if same else 1
        print(f"{n:<24} {'ok  ' if same else 'FAIL'}")
    return fails


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lib", default=None)
    ap.add_argument("--work", default=None)
    ap.add_argument("--strict", action="store_true")
    ap.add_argument("--keep", action="store_true", help="keep the temporary work folder (without --work)")
    ap.add_argument("--rerun-original", metavar="AVAS.dll", default=None,
                    help="with --strict: rerun the differing cases with this original kernel on this machine")
    a = ap.parse_args()
    lib = os.path.abspath(a.lib or default_lib())
    work = a.work or tempfile.mkdtemp(prefix="avas_parity_")
    if not a.work and not a.keep:
        # a temporary work folder (about 0.5 GB) is removed at the end; --work DIR keeps the outputs
        import atexit
        atexit.register(shutil.rmtree, work, True)
    doc = json.load(open(os.path.join(HERE, "cases.json"), encoding="utf-8"))
    threads = doc.get("reference_omp_threads", 12)
    print(f"library: {lib}\nwork:    {work}")
    run(lib, "dll", os.path.join(work, "dll"), threads)
    run(lib, "none", os.path.join(work, "none"), threads)
    print("\ncompat=dll against AVAS.dll references (differences only):")
    sys.argv = ["compare.py", os.path.join(work, "dll"), "--quiet"]
    identical = compare.main() == 0
    strict_rows = [] if identical else compare.compare_dirs(os.path.join(work, "dll"), os.path.join(HERE, "refs"))[1]
    default_report(doc, os.path.join(work, "none"))
    bad = energy_check(doc, os.path.join(work, "none"))
    bad += segment_drift_check(doc, os.path.join(work, "none"))
    bad += distribution_check(doc, os.path.join(work, "none"))
    bad += indata_sha_check(doc, os.path.join(work, "none"))
    bad += error_study_check(doc, os.path.join(work, "none"))
    bad += loop_feature_check(doc, os.path.join(work, "none"))
    bad += secondary_steerer_check(work)
    bad += libpic_keyword_check(work)
    bad += m10_check(os.path.join(work, "none"))
    bad += thread_check(lib, work, ["SC1_spicnic_S1", "SC2_fft_S1", "SC5_spicnic_Z1", "PN1_picnic_S1", "PN2_picnic_Z1", "D_GS_GS",
                                    "ER3_quad_dyn_r2_step31_dst", "ER16_quad_dyn_spicnic_dst",
                                    "LP2_lpboundary_fft_dst", "LB2_boundary_mixed_dst", "M10_D4_close_planes",
                                    "M10_D5_composite_dst", "M10_D7_periodic_fft_dst"])
    if bad:
        print(f"DEFAULT-MODE CHECKS FAILED: {bad}")
        return 1
    if identical:
        return 0
    if a.strict:
        if a.rerun_original:
            return rerun_original(a.rerun_original, work, threads, strict_rows)
        return 1
    # tolerate last-bit libm differences on other platforms
    libm_sensitive = {c["name"]: c["libm_sensitive"] for c in doc["cases"] if c.get("libm_sensitive")}
    for name, why in libm_sensitive.items():
        print(f"not compared across platforms: {name} ({why})")
    worst = 0.0
    ignore = {c["name"]: c["compat_ignore_lines"] for c in doc["cases"] if c.get("compat_ignore_lines")}
    # AVAS.dll crashes or lets a C++ exception escape: the return code and the console message are
    # those of Windows (ctypes OSError); other platforms end with a signal instead
    win_end = {c["name"] for c in doc["cases"] if c.get("windows_process_end")}
    worst_at = ""
    for case in sorted(os.listdir(os.path.join(HERE, "refs"))):
        rdir = os.path.join(HERE, "refs", case)
        files = [os.path.relpath(os.path.join(d, f), rdir) for d, _, fs in os.walk(rdir) for f in fs]
        for rel in files:
            f = os.path.basename(rel)
            rp, np_ = os.path.join(rdir, rel), os.path.join(work, "dll", case, "summary", rel)
            if case in compare.expected_diff_cases() or case in libm_sensitive:
                continue
            if case in win_end and f in ("rc.txt", "console_check.txt"):
                continue
            if f in ("console.txt", "files.txt") or f.endswith(".sha.txt") or not os.path.exists(np_):
                continue   # binary outputs are compared bit for bit on Windows only
            if f.endswith((".dst.txt", ".edst.txt", ".dstb.txt")):
                ok, _, mrel, _, _ = compare.compare_dst(compare.read(rp), compare.read(np_))
            else:
                cols = set(range(28, 41)) if (f == "DataSet.txt" and case in compare.single_particle_cases()) else None
                ra, na = compare.read(rp), compare.read(np_)
                drop = ignore.get(case, {}).get(rel.replace(os.sep, "/"))
                if drop:   # lines holding uninitialised memory of AVAS.dll
                    ra = "\n".join(ln for k, ln in enumerate(ra.split("\n")) if k not in drop)
                    na = "\n".join(ln for k, ln in enumerate(na.split("\n")) if k not in drop)
                ok, _, mrel, _, note = compare.compare_text(ra, na, cols)
                if note.startswith("lines"):
                    mrel = math.inf
            if mrel > worst:
                worst, worst_at = mrel, f"{case}/{rel}"
    print(f"\nlargest relative difference to AVAS.dll: {worst:.3g} ({worst_at})")
    return 0 if worst < 1e-10 else 1


if __name__ == "__main__":
    sys.exit(main())
