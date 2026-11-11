"""Stand-in for ``avas run`` in tests/test_research.py: a deterministic DataSet.txt from the inputs.

    python fake_research_engine.py INPUT OUTPUT LATTICE COUNTER

rms x = 1 mm · (1 + current/100), centroid x = seed·1e-6 m + 1e-3 m/T · (sum of the steerers' By) · z,
energy 1.5 MeV + 0.1 MeV/m · z, macro-particles = particlenumber.  A line ``failme 1`` in input.txt
makes it fail like the engine ("Error: ...").  Every call appends a line to COUNTER (the input folder
and the GPUs of ini.ini [engine]).

Kernel: ini.ini ``[engine]`` as the study wrote it (the choice of ``avas run``); avas_run.json gets an
``engine`` entry like the real one.  Device auto is resolved to ``fakeauto DEVICE PRECISION`` of
input.txt (default cpu fp64); fp32 runs have rms x 0.1 % larger (another result tier).
"""
import configparser
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)


def keyword(path, key, default):
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            parts = line.split("!", 1)[0].split()
            if len(parts) >= 2 and parts[0].lower() == key:
                return float(parts[1])
    return default


def words(path, key):
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            parts = line.split("!", 1)[0].split()
            if parts and parts[0].lower() == key:
                return parts[1:]
    return None


def engine_entry(inp):
    cfg = configparser.ConfigParser()
    ini = os.path.join(inp, "ini.ini")
    if os.path.isfile(ini):
        cfg.read(ini, encoding="utf-8")
    sec = dict(cfg.items("engine")) if cfg.has_section("engine") else {}
    if sec.get("kernel", "original") != "new":
        return {"kernel": "original", "file": "fake", "sha1": "fake", "tier": "original"}
    device, precision, compat = sec.get("device", "auto"), sec.get("precision", "auto"), sec.get("compat", "none")
    if device == "auto":
        device, auto_precision = ((words(os.path.join(inp, "input.txt"), "fakeauto") or ["cpu"]) + ["auto"])[:2]
        precision = auto_precision if precision == "auto" else precision
    if precision == "auto":
        precision = "fp32" if device == "metal" else "fp64"
    tier = ("new-compat-dll" if compat == "dll" else "new-fp32" if precision == "fp32"
            else "new-cpu-fp64" if device == "cpu" else "new-cuda-fp64")
    gpus = sec.get("gpus", "all")
    return {"kernel": "new", "file": "fake", "sha1": "fake", "version": "fakeversion0001", "device": device,
            "precision": precision, "gpus": [] if device != "cuda" or gpus == "all" else [int(g) for g in gpus.split(",")],
            "compat": compat, "threads": int(sec.get("threads", 0)) or 8, "space_charge": True, "tier": tier,
            "ini_gpus": gpus}


def main():
    inp, out, lattice, counter = sys.argv[1:5]
    entry = engine_entry(inp)
    with open(counter, "a", encoding="utf-8") as fh:
        fh.write(inp + " gpus=" + str(entry.get("ini_gpus")) + "\n")
    if keyword(os.path.join(inp, "input.txt"), "failme", 0):
        print("Error: the fake engine was told to fail", flush=True)
        return 1
    from avas.data.lattice_doc import LatticeDocument, to_float
    with open(os.path.join(inp, lattice), encoding="utf-8") as fh:
        doc = LatticeDocument(fh.read())
    by = sum(to_float(st.param(4)) for st in doc.statements if st.active and st.key == "steerer")
    current = keyword(os.path.join(inp, "beam.txt"), "current", 0.0)
    particles = keyword(os.path.join(inp, "beam.txt"), "particlenumber", 1000)
    seed = keyword(os.path.join(inp, "input.txt"), "randomseed", 0)
    length = doc.total_length or 1.0
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "DataSet.txt"), "w", encoding="utf-8") as fh:
        for i in range(51):
            z = length * i / 50
            v = [0.0] * 41
            v[0] = 1.5 + 0.1 * z
            v[1] = seed * 1e-6 + 1e-3 * by * z
            v[5] = z
            v[13], v[14], v[15] = 2e-7, 2e-7, 3e-7
            v[16] = 1e-3 * (1 + current / 100) * (1.001 if entry.get("precision") == "fp32" else 1.0)
            v[17], v[18], v[19], v[20] = 1e-3, 1e-3, 1e-3, 2e-3
            v[22], v[24] = 3 * v[16], 3e-3
            v[28] = particles
            fh.write(" ".join(f"{x:.10g}" for x in v) + "\n")
        print("Simulate progress 100.00%. Estimated remaining time 0s. Run time 0s.  1000  1.00m", flush=True)
    with open(os.path.join(out, "avas_run.json"), "w", encoding="utf-8") as fh:
        json.dump({"status": "finished", "engine": entry}, fh)
    with open(os.path.join(out, "big.dst"), "w", encoding="utf-8") as fh:
        fh.write("bulk output that is not kept\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
