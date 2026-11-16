"""Beam input of the linear model: beam.txt / .dst moments and the initial second moments."""
import math
import os
import struct

import numpy as np

from avas.model.common import C_LIGHT, PreviewError


def read_beam(input_dir):
    """Beam of ``<input_dir>/beam.txt`` for :func:`linear_preview`.

    Returns ``{"mass", "charge", "energy", "current", "frequency", "dc",
    "twiss": {"x"|"y"|"z": (alpha, beta, emit_n)}, "source", "warnings"}``.
    When the beam comes from a TraceWin ``.dst`` file (``use_dst``) its
    moments are used instead of the Twiss keywords.
    """
    path = os.path.join(input_dir, "beam.txt")
    if not os.path.isfile(path):
        raise PreviewError(f"beam.txt not found in {input_dir}")
    values = {}
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            tokens = line.split("!")[0].split()
            if tokens:
                values[tokens[0].lower()] = tokens[1:]

    def num(key, index=0, default=None):
        try:
            return float(values[key][index])
        except (KeyError, IndexError, ValueError):
            return default

    beam = {
        "mass": num("particlerestmass"), "charge": num("numofcharge", default=1.0),
        "energy": num("kneticenergy"), "current": num("current", default=0.0),
        "frequency": num("frequency", default=0.0),
        "dc": (values.get("beamtype") or ["notdc"])[0].lower() == "dc",
        "twiss": {}, "source": "twiss", "warnings": [],
    }
    for plane in "xyz":
        twiss = [num("twiss" + plane, k) for k in range(3)]
        if all(v is not None for v in twiss):
            beam["twiss"][plane] = tuple(twiss)

    dst = (values.get("readparticledistribution") or [""])[0]
    if dst and dst.lower() != "unknown" and num("use_dst", default=1) != 0:
        dst_path = dst if os.path.isabs(dst) else os.path.join(input_dir, dst)
        if dst_path.lower().endswith(".dst") and os.path.isfile(dst_path):
            try:
                # the reference energy is beam.txt kneticenergy when it is positive, else the mean of the
                # file (the new kernel's default mode, M10 D1)
                ref = num("kneticenergy")
                beam.update(_beam_from_dst(dst_path, ref if ref is not None and ref > 0 else None))
                beam["source"] = "dst"
                note = reference_energy_note(beam["energy"], beam["meanEnergy"]) if ref is not None and ref > 0 else None
                if note:
                    beam["warnings"].append(note)
            except (OSError, ValueError, struct.error) as exc:
                beam["warnings"].append((f"Particle file '{dst}' could not be read ({exc}); Twiss keywords used.",
                                         f"无法读取粒子文件“{dst}”（{exc}），改用 Twiss 参数。"))
        else:
            beam["warnings"].append((f"Particle file '{dst}' is not a readable .dst file; Twiss keywords used.",
                                     f"粒子文件“{dst}”不是可读取的 .dst 文件，改用 Twiss 参数。"))
    return beam


REFERENCE_ENERGY_TOLERANCE = 0.01   # relative difference of kneticenergy and the mean energy that is warned about


def reference_energy_note(reference, mean):
    """Warning (en, zh) when the reference energy (beam.txt kneticenergy) differs from the mean energy of the
    particle file by more than 1 %, else None."""
    if not (mean and mean > 0) or abs(reference - mean) <= REFERENCE_ENERGY_TOLERANCE * mean:
        return None
    return (f"beam.txt kneticenergy {reference:.6g} MeV differs from the mean energy {mean:.6g} MeV of the particle file "
            f"by {abs(reference - mean) / mean * 100:.2g} %; the new kernel (default mode) starts the synchronous particle "
            "with kneticenergy, the original kernel and the compatibility mode with the mean. Write a negative "
            "kneticenergy to use the mean.",
            f"beam.txt 的 kneticenergy {reference:.6g} MeV 与粒子文件的平均能量 {mean:.6g} MeV 相差 "
            f"{abs(reference - mean) / mean * 100:.2g} %；新内核（默认模式）以 kneticenergy 作为同步粒子能量，"
            "原内核和兼容模式用平均能量。kneticenergy 写负值则使用平均能量。")


def dst_energies(input_dir):
    """``(kneticenergy, mean energy, file name)`` when beam.txt reads a .dst particle file (main beam), else None.
    The mean is weighted with the .dstb file under input.txt ``indweight 1``, as in the kernel."""
    def keys(name):
        out = {}
        try:
            with open(os.path.join(input_dir, name), encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    tokens = line.split("!")[0].split()
                    if tokens:
                        out[tokens[0].lower()] = tokens[1:]
        except OSError:
            pass
        return out
    beam, inp = keys("beam.txt"), keys("input.txt")
    dst = (beam.get("readparticledistribution") or [""])[0]
    if not dst.lower().endswith(".dst") or (inp.get("secondarybeam") or ["0"])[0] == "1":
        return None
    try:
        ref = float(beam["kneticenergy"][0])
    except (KeyError, IndexError, ValueError):
        ref = -1.0
    path = dst if os.path.isabs(dst) else os.path.join(input_dir, dst)
    try:
        with open(path, "rb") as fh:
            raw = fh.read()
        n = struct.unpack_from("<i", raw, 2)[0]
        if n < 1 or len(raw) < 23 + 48 * n + 8:
            return None
        w = np.frombuffer(raw, dtype="<f8", count=6 * n, offset=23).reshape(n, 6)[:, 5]
        weights = None
        if (inp.get("indweight") or ["0"])[0] == "1" and os.path.isfile(path + "b"):
            with open(path + "b", "rb") as fh:
                rb = fh.read()
            if len(rb) >= 4 + 8 * n and struct.unpack_from("<i", rb, 0)[0] == n:
                weights = np.frombuffer(rb, dtype="<f8", count=n, offset=4)
    except (OSError, struct.error, ValueError):
        return None
    mean = float(np.average(w, weights=weights)) if weights is not None else float(w.mean())
    return ref, mean, dst


def _beam_from_dst(path, reference=None):
    """Energy, current, frequency, mass and rms Twiss of a TraceWin .dst file.  *reference*: the reference
    energy (MeV, beam.txt kneticenergy); None: the mean energy of the particles.  ``meanEnergy`` is the mean."""
    with open(path, "rb") as fh:
        raw = fh.read()
    n = struct.unpack_from("<i", raw, 2)[0]
    current, freq_mhz = struct.unpack_from("<dd", raw, 6)
    if n < 2 or len(raw) < 23 + 48 * n + 8:
        raise ValueError("truncated file")
    parts = np.frombuffer(raw, dtype="<f8", count=6 * n, offset=23).reshape(n, 6)
    mass = struct.unpack_from("<d", raw, 23 + 48 * n)[0]
    mean = float(parts[:, 5].mean())
    energy = mean if reference is None else float(reference)
    gamma = 1.0 + energy / mass
    bg = math.sqrt(gamma * gamma - 1.0)
    beta = bg / gamma
    freq = freq_mhz * 1e6
    wavelength = C_LIGHT / freq if freq > 0 else 1.0
    z = -(parts[:, 4] - parts[:, 4].mean()) * beta * wavelength / (2 * math.pi)
    delta = (parts[:, 5] - mean) / (beta * beta * gamma * mass)   # spread about the mean (rms moments)
    twiss = {}
    for plane, (u, up) in (("x", (parts[:, 0] * 0.01, parts[:, 1])), ("y", (parts[:, 2] * 0.01, parts[:, 3])),
                           ("z", (z, delta))):
        cov = np.cov(u, up)
        emit = math.sqrt(max(cov[0, 0] * cov[1, 1] - cov[0, 1] ** 2, 0.0))
        if emit <= 0:
            twiss[plane] = (0.0, 1.0, 0.0)
        else:
            twiss[plane] = (-cov[0, 1] / emit, cov[0, 0] / emit, emit * bg * 1e6)
    return {"mass": mass, "energy": energy, "meanEnergy": mean, "current": current, "frequency": freq, "twiss": twiss}


def _check_beam(beam):
    if not isinstance(beam, dict):
        raise PreviewError("No beam data.")
    try:
        mass, energy = float(beam["mass"]), float(beam["energy"])
        charge = float(beam.get("charge") or 0.0)
    except (KeyError, TypeError, ValueError):
        raise PreviewError("Beam data needs mass and energy.") from None
    if not (mass > 0 and energy > 0 and charge != 0 and math.isfinite(mass * energy * charge)):
        raise PreviewError("Beam mass, energy and charge must be positive / non-zero numbers.")
    twiss = beam.get("twiss") or {}
    for plane in "xy":
        t = twiss.get(plane)
        if t is None or len(t) < 3 or not float(t[1]) > 0 or not float(t[2]) >= 0:
            raise PreviewError(f"Beam Twiss parameters of plane {plane} are missing or invalid.")
    return mass, energy, charge


def _sigma0(beam, bg):
    """Initial 6×6 second-moment matrix in m and rad."""
    sigma = np.zeros((6, 6))
    for k, plane in enumerate("xyz"):
        t = (beam.get("twiss") or {}).get(plane)
        if t is None:
            continue
        alpha, beta, emit_n = (float(v) for v in t[:3])
        if not beta > 0 or not emit_n >= 0:
            continue
        emit = emit_n / bg * 1e-6                          # geometric, m·rad
        i = 2 * k
        sigma[i, i] = beta * emit                          # β [mm/mrad] = β [m/rad]
        sigma[i, i + 1] = sigma[i + 1, i] = -alpha * emit
        sigma[i + 1, i + 1] = (1 + alpha * alpha) / beta * emit
    return sigma
