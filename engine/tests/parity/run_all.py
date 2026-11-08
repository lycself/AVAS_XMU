"""One-command M1 / M2 parity check of the new engine against the AVAS.dll references.

usage: python engine/tests/parity/run_all.py [--lib PATH] [--work DIR] [--strict]

1. runs every case with compat=dll and compares with refs/ (written by AVAS.dll on Windows);
2. runs every case with the default (corrected) mode and reports the synchronous phase error of
   the V3 = 0 cavities and the differences to AVAS.dll.
OMP_NUM_THREADS is set to the thread count of the reference machine so that the particle blocks
(and with them the summation order of the beam statistics) are the same as in AVAS.dll.
Exit code 0 when compat=dll agrees with the references (byte-identical with --strict, otherwise
within 1e-10 relative, the size of libm differences between platforms).
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


def run(lib, compat, work, threads):
    env = dict(os.environ, OMP_NUM_THREADS=str(threads))
    subprocess.run([sys.executable, os.path.join(HERE, "run_engine.py"), "--lib", lib, "--compat", compat,
                    "--work", work], check=True, env=env, stdout=subprocess.DEVNULL)


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
    default_report(doc, os.path.join(work, "none"))
    bad = energy_check(doc, os.path.join(work, "none"))
    bad += segment_drift_check(doc, os.path.join(work, "none"))
    bad += distribution_check(doc, os.path.join(work, "none"))
    bad += indata_sha_check(doc, os.path.join(work, "none"))
    bad += error_study_check(doc, os.path.join(work, "none"))
    bad += loop_feature_check(doc, os.path.join(work, "none"))
    bad += secondary_steerer_check(work)
    bad += libpic_keyword_check(work)
    bad += thread_check(lib, work, ["SC1_spicnic_S1", "SC2_fft_S1", "SC5_spicnic_Z1", "PN1_picnic_S1", "PN2_picnic_Z1", "D_GS_GS",
                                    "ER3_quad_dyn_r2_step31_dst", "ER16_quad_dyn_spicnic_dst",
                                    "LP2_lpboundary_fft_dst", "LB2_boundary_mixed_dst"])
    if bad:
        print(f"DEFAULT-MODE CHECKS FAILED: {bad}")
        return 1
    if identical:
        return 0
    if a.strict:
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
