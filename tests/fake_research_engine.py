"""Stand-in for ``avas run`` in tests/test_research.py: a deterministic DataSet.txt from the inputs.

    python fake_research_engine.py INPUT OUTPUT LATTICE COUNTER

rms x = 1 mm · (1 + current/100), centroid x = seed·1e-6 m + 1e-3 m/T · (sum of the steerers' By) · z,
energy 1.5 MeV + 0.1 MeV/m · z, macro-particles = particlenumber.  A line ``failme 1`` in input.txt
makes it fail like the engine ("Error: ...").  Every call appends a line to COUNTER.
"""
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


def main():
    inp, out, lattice, counter = sys.argv[1:5]
    with open(counter, "a", encoding="utf-8") as fh:
        fh.write(inp + "\n")
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
            v[16] = 1e-3 * (1 + current / 100)
            v[17], v[18], v[19], v[20] = 1e-3, 1e-3, 1e-3, 2e-3
            v[22], v[24] = 3 * v[16], 3e-3
            v[28] = particles
            fh.write(" ".join(f"{x:.10g}" for x in v) + "\n")
        print("Simulate progress 100.00%. Estimated remaining time 0s. Run time 0s.  1000  1.00m", flush=True)
    with open(os.path.join(out, "big.dst"), "w", encoding="utf-8") as fh:
        fh.write("bulk output that is not kept\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
