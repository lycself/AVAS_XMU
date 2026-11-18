"""Particle-motion animations of the primer (``avas-motion`` blocks).

A motion sends a few dozen particles of a beam through a short lattice and returns
their trajectories, so the page can animate them in 2D and 3D while the reader moves
the parameters.  The particles are drawn from the beam's rms matrix (Gaussian, cut
at 3 σ, a fixed seed so that a particle keeps its identity while a slider moves) and
moved with the transfer maps of :class:`avas.model.LinearModel`: the model of the
editor's preview, with its linear space-charge kicks and the centroid shifts of
steerers and element offsets.  The motion is therefore the linear model's, and the
page says so; the multi-particle engine stays the reference.

The default beam is the example project's (protons at 1.527 MeV from the RFQ of
``examples/hwr010``, its Twiss parameters, emittances and current).

Result layout (see :func:`run`)::

    {"lattice", "fieldDirs",
     "s": (nS,) m, "n": particles, "marked": [indices of the highlighted particles], "charge": q,
     "lost": [sample index where each particle hits the bore, −1 if it gets through] (NaN after it),
     "x", "xp", "y", "yp": (n·nS,) float32, particle-major, mm and mrad,
     "z": (n·nS,) float32 mm, the place in the bunch (positive: ahead of its centre; the 3D view spreads
          the particles along the beam by it, as a real bunch is), "dp": (n·nS,) float32 ‰, δ = Δp/p,
     "marks": [[plane, sign] of each highlighted particle, plane "x" | "y" | "z"],
     "centre": draw the beam centre as a line, "rfPhase": (nS,) RF phase (°) the reference particle sees
          inside an RF field map, NaN elsewhere (the field there is Ke·Ez(z)·cos(phase) at that moment),
     "cx", "cy", "rms_x", "rms_y": (nS,) mm, "energy": (nS,) MeV,
     "elements": [{"line", "key", "name", "z0", "z1", "aperture" (mm), "G"? (quad, T/m), "Bx"?, "By"?, "kind"?
                   (steerer, T or V/m), "span"? (steerer: [z0, z1] of the element it acts over), "B"? (solenoid
                   Bz; bend: By on the reference orbit, T), "alpha"?, "rho"?, "N"?, "hv"? (bend: °, m, index,
                   0 horizontal / 1 vertical), "ez"? (RF field map: {"z": m from the element's start,
                   "e": Ke·Ez MV/m}), "f"? (Hz), "phase"? (RF map: (nS,) its RF phase in ° at the moment the
                   reference particle is at each point; the field is Ke·Ez(z)·cos(phase) then)}],
     "monitors": [{"name", "z"}], "ghosts": [{"name": (en, zh), "plane": "x"|"y", "y": (nS,) mm}],
     "values", "plots", "warnings": [(en, zh)], "model": (en, zh), "params"}
"""
import math

import numpy as np

from avas import paths
from avas.data.lattice_doc import to_float
from avas.dynamics.diagnostics import Diagnostics, Monitor
from avas.dynamics.response import response, steerers
from avas.learn import labs as L
from avas.learn.labs import LabError, Param
from avas.model import LinearModel, PreviewError, read_beam
from avas.model.common import C_LIGHT

N_PARTICLES = 60
MAX_POINTS = 400
CUT = 3.0                      # Gaussian cut in rms units
MARK = 1.5                     # the highlighted particles start at 1.5 rms


class Motion:
    def __init__(self, id_, title, params, fn, summary):
        self.id, self.title, self.params, self.fn, self.summary = id_, title, params, fn, summary

    def public(self):
        return {"id": self.id, "title": list(self.title), "summary": list(self.summary),
                "params": [p.public() for p in self.params]}


LINEAR = ("Particles drawn from the example beam's rms matrix and moved by the linear model (the editor's preview, "
          "with linear space charge); the multi-particle engine is the reference.",
          "粒子按示例束流的 rms 矩阵抽样，用线性模型（结构编辑器预览所用的模型，含线性空间电荷）推动；以多粒子内核结果为准。")


# =========================================================================== beam and particles
def example_beam():
    folder = paths.example_input_dir()
    if folder:
        beam = read_beam(folder)
    else:
        beam = L._beam(1.527, L.PROTON, 1, {"x": (-0.658, 0.377, 0.138), "y": (-0.634, 0.373, 0.141)}, current=0.184, dc=False)
    return beam, ([folder] if folder else [])


def _chol2(s):
    """Lower-triangular square root of a 2×2 moment block (a degenerate block gives the diagonal roots)."""
    a = math.sqrt(max(s[0, 0], 0.0))
    if a <= 0:
        return np.array([[0.0, 0.0], [0.0, math.sqrt(max(s[1, 1], 0.0))]])
    b = s[0, 1] / a
    return np.array([[a, 0.0], [b, math.sqrt(max(s[1, 1] - b * b, 0.0))]])


XY_MARKS = (("x", 1), ("y", 1))
_PLANE_INDEX = {"x": 0, "y": 2, "z": 4}


def _normal_cloud(n, seed=7, marks=XY_MARKS):
    """*n* rows of six standard normal numbers cut at ``CUT``: the particles in normalised coordinates.
    The first rows are the highlighted particles, at ``MARK`` rms in the plane and with the sign of *marks*
    (by default one in x and one in y)."""
    rng = np.random.default_rng(seed)
    k = len(marks)
    u = np.empty((0, 6))
    while len(u) < n - k:
        c = rng.standard_normal((2 * n, 6))
        u = np.vstack([u, c[np.all(np.abs(c) < CUT, axis=1)]])
    u = u[:n - k]
    # whiten plane by plane so that the drawn particles have exactly the beam's rms matrix
    u -= u.mean(axis=0)
    for i in (0, 2, 4):
        c = np.cov(u[:, i:i + 2].T)
        u[:, i:i + 2] = u[:, i:i + 2] @ np.linalg.inv(np.linalg.cholesky(c)).T
    marked = np.zeros((k, 6))
    for row, (plane, sign) in enumerate(marks):
        marked[row, _PLANE_INDEX[plane]] = MARK * sign
    return np.vstack([marked, u])


def particles(sigma0, n=N_PARTICLES, seed=7, marks=XY_MARKS):
    """Initial coordinates (n, 6) in m, rad, m, Δp/p: plane by plane ``x = a·u₁``, ``x′ = b·u₁ + c·u₂``,
    so the same particle stays the same while the Twiss parameters change."""
    u = _normal_cloud(n, seed, marks)
    out = np.zeros_like(u)
    for i in (0, 2, 4):
        out[:, i:i + 2] = u[:, i:i + 2] @ _chol2(sigma0[i:i + 2, i:i + 2]).T
    return out


def _evaluate(text, beam, dirs, **kwargs):
    try:
        model = LinearModel(text, beam, dirs)
        with np.errstate(all="ignore"):
            return model, model.evaluate(**kwargs)
    except PreviewError as exc:
        raise LabError(str(exc)) from exc


def trajectories(res, x0):
    """Coordinates (n, points, 6) of the particles *x0* (n, 6) at every point of *res*: the centroid plus the
    product of the step matrices (with the space-charge kicks when the result has them) on the deviation."""
    dev = x0 - res.centroid[0]
    out = np.empty((len(x0), len(res.z), 6))
    out[:, 0] = x0
    if len(res.z) > 1:
        prod = res._products(res.info["space_charge"])
        out[:, 1:] = res.centroid[None, 1:] + np.einsum("kij,nj->nki", prod, dev)
    return out


def _sample_index(n, at_most=MAX_POINTS):
    if n <= at_most:
        return np.arange(n)
    return np.unique(np.round(np.linspace(0, n - 1, at_most)).astype(int))


def _field_profile(dirs, st):
    """On-axis Ke·Ez (MV/m) of an RF field-map statement, z from the element's start (None without the map)."""
    from avas.data import fieldmap
    try:
        comps = fieldmap.components(list(dirs), st.param(8))
        if "edz" not in comps:
            return None
        order = fieldmap.group_order([fieldmap.FieldMap(p) for p in comps.values()])
        z, ez, _peak = fieldmap.FieldMap(comps["edz"]).profiles(order)
    except Exception:  # noqa: BLE001 - only a picture: no profile, no field arrows
        return None
    idx = _sample_index(len(z), 200)
    return {"z": np.asarray(z, dtype=float)[idx] - float(z[0]), "e": np.asarray(ez, dtype=float)[idx] * (to_float(st.param(6)) or 0.0)}


def _elements(res, dirs=()):
    params = {st.line_no: st for st in res.doc.elements()}
    out = []
    for n, e in enumerate(res.elements):
        st = params.get(e["line"])
        item = {"line": e["line"], "key": e["key"], "name": e["name"], "z0": e["z0"], "z1": e["z1"],
                "aperture": e["aperture"] * 1e3 if e["aperture"] else None}
        if st is not None and e["key"] == "quad":
            item["G"] = to_float(st.param(3))
        if st is not None and e["key"] == "steerer":
            item["Bx"], item["By"] = to_float(st.param(3)), to_float(st.param(4))
            item["kind"] = int(to_float(st.param(5)) or 0)
            after = next((f for f in res.elements[n + 1:] if f["z1"] > f["z0"]), None)  # it acts over the next element
            item["span"] = [after["z0"], after["z1"]] if after else None
        if st is not None and e["key"] == "solenoid":
            item["B"] = to_float(st.param(3))
        if st is not None and e["key"] == "bend":
            alpha, rho = to_float(st.param(3)) or 0.0, to_float(st.param(4)) or 0.0
            item.update(alpha=alpha, rho=rho, N=to_float(st.param(5)) or 0.0, hv=int(to_float(st.param(6)) or 0))
            if rho > 0 and e.get("w_in"):
                *_k, brho = L._kin(e["w_in"], res.mass, abs(res.charge) or 1.0)
                # α > 0 bends a positive particle towards −x (the linear model's convention): By > 0
                item["B"] = math.copysign(brho / rho, alpha) * (1 if res.charge >= 0 else -1)
        if st is not None and e["key"] == "field" and to_float(st.param(3)) == 1:   # RF
            item["f"] = to_float(st.param(4))
            item["ez"] = _field_profile(dirs, st)
        out.append(item)
    return out


def _rf_phase(res):
    """RF phase (°) of each RF field map at the moment the reference particle is at each point: the phase it
    sees at the element's entrance plus 360·f·(time since then).  Returns the phase inside the maps (NaN
    elsewhere) and {line: phase all along the line} (the field keeps oscillating while the bunch approaches)."""
    z = np.asarray(res.z, dtype=float)
    gamma = 1.0 + np.asarray(res.energy, dtype=float) / res.mass
    v = np.sqrt(np.maximum(1.0 - 1.0 / gamma ** 2, 1e-30)) * C_LIGHT
    t = np.concatenate([[0.0], np.cumsum(np.diff(z) * 0.5 * (1 / v[1:] + 1 / v[:-1]))])
    out = np.full(len(z), np.nan)
    every = {}
    params = {st.line_no: st for st in res.doc.elements()}
    for e in res.elements:
        st = params.get(e["line"])
        if e["key"] != "field" or e["phase_rf"] is None or st is None or to_float(st.param(3)) != 1:
            continue
        f = to_float(st.param(4)) or 0.0
        inside = (z >= e["z0"] - 1e-12) & (z <= e["z1"] + 1e-12)
        t0 = float(np.interp(e["z0"], z, t))
        every[e["line"]] = e["phase_rf"] + 360.0 * f * (t - t0)
        out[inside] = every[e["line"]][inside]
    return out, every


def package(text, dirs, res, x0, values=(), plots=(), warnings=(), monitors=(), ghosts=(), model=LINEAR,
            marks=XY_MARKS, centre=None):
    """The page's dictionary of a motion: trajectories of *x0* through *res* and the extras."""
    tr = trajectories(res, x0)
    idx = _sample_index(len(res.z))
    tr = tr[:, idx]
    with np.errstate(all="ignore"):
        bad = ~np.isfinite(tr).all(axis=(0, 2))
    if bad.any():                               # the model blew up: keep the part before
        idx, tr = idx[:max(int(np.argmax(bad)), 1)], tr[:, :max(int(np.argmax(bad)), 1)]

    # a particle that reaches the bore of an element is lost there (as the engine counts losses): its
    # coordinates after that point are NaN, and "lost" holds the sample index (−1: it gets through)
    aperture = np.asarray(res.aperture, dtype=float)[idx]
    with np.errstate(all="ignore"):
        hit = (aperture[None] > 0) & (np.hypot(tr[:, :, 0], tr[:, :, 2]) > aperture[None])
    lost = np.where(hit.any(axis=1), np.argmax(hit, axis=1), -1)
    for i, k in enumerate(lost):
        if k >= 0:
            tr[i, k + 1:] = np.nan
    warnings = list(warnings)
    n_lost = int(np.sum(lost >= 0))
    if n_lost:
        warnings.append((f"{n_lost} of the {len(x0)} drawn particles hit the wall of the beam pipe (red crosses). "
                         "The rms envelope of the linear model does not remove lost particles.",
                         f"画出的 {len(x0)} 个粒子中有 {n_lost} 个撞上了管壁（红叉）。线性模型的 rms 包络不扣除丢失的粒子。"))

    def flat(i, scale):
        return (tr[:, :, i] * scale).astype(np.float32).reshape(-1)

    def g(arr):
        return np.asarray(arr, dtype=float)[idx]

    rf_inside, rf_every = _rf_phase(res)
    elements = _elements(res, dirs)
    for item in elements:
        if item["line"] in rf_every:
            item["phase"] = g(rf_every[item["line"]])

    return {
        "lattice": text, "fieldDirs": list(dirs),
        "s": g(res.z), "mass": float(res.mass), "n": int(len(x0)), "marked": list(range(len(marks))), "charge": float(res.charge),
        "lost": [int(k) for k in lost],
        "x": flat(0, 1e3), "xp": flat(1, 1e3), "y": flat(2, 1e3), "yp": flat(3, 1e3), "z": flat(4, 1e3),
        "dp": flat(5, 1e3), "marks": [list(m) for m in marks], "centre": bool(ghosts) if centre is None else centre,
        "rfPhase": g(rf_inside),
        "cx": g(res.centroid[:, 0]) * 1e3, "cy": g(res.centroid[:, 2]) * 1e3,
        "rms_x": g(res.rms("x")) * 1e3, "rms_y": g(res.rms("y")) * 1e3, "energy": g(res.energy),
        "elements": elements, "monitors": list(monitors),
        "ghosts": [{**gh, "y": np.asarray(gh["y"], dtype=float)[idx]} for gh in ghosts],
        "values": [*values, L._value("through", "Drawn particles that get through", "通过的粒子（画出的）", "%",
                                     100.0 * (len(x0) - n_lost) / len(x0))],
        "plots": list(plots), "warnings": [*res.warnings, *warnings], "model": model,
    }


def _ap(aperture=L.APERTURE):
    return f"{aperture:g}"


def _beam_with(beam, twiss=None, current=None, energy=None, emit=None):
    b = dict(beam)
    tw = dict(b.get("twiss") or {})
    for plane, (alpha, beta) in (twiss or {}).items():
        tw[plane] = (alpha, beta, tw.get(plane, (0, 0, 0.14))[2])
    if emit is not None:
        for plane in ("x", "y"):
            a, bt, _e = tw[plane]
            tw[plane] = (a, bt, emit)
    b["twiss"] = tw
    if current is not None:
        b["current"] = current
    if energy is not None:
        b["energy"] = energy
    return b


def _current_param(default=None):
    beam, _dirs = example_beam()
    return Param("I", ("Beam current", "束流流强"), "mA", round(float(beam.get("current") or 0.0), 3) if default is None else default,
                 0.0, 20.0, 0.01, doc=("space charge pushes the particles apart; the example beam carries 0.184 mA",
                                       "空间电荷让粒子互相推开；示例束流是 0.184 mA"))


# =========================================================================== motions
def drift(L_, alpha, emit, I):
    beam, dirs = example_beam()
    beta = beam["twiss"]["x"][1]
    beam = _beam_with(beam, twiss={"x": (alpha, beta), "y": (alpha, beam["twiss"]["y"][1])}, current=I, emit=emit)
    text = f"start\ndrift {L_:g} {_ap()} 0\nend\n"
    _m, res = _evaluate(text, beam, dirs)
    x0 = particles(res.sigma[0])
    rx = res.rms("x") * 1e3
    i_min = int(np.argmin(rx))
    values = [
        L._value("rms0", "rms x at the entrance", "入口 rms x", "mm", rx[0]),
        L._value("rms1", "rms x at the exit", "出口 rms x", "mm", rx[-1]),
        L._value("waist", "Narrowest point (waist) at", "最窄处（束腰）位置", "m", res.z[i_min] if 0 < i_min < len(rx) - 1 else None),
        L._value("div", "rms angle x′", "rms 角度 x′", "mrad", res.rms_divergence("x")[0] * 1e3),
        L._value("ratio", "Exit size / entrance size", "出口尺寸 / 入口尺寸", "", rx[-1] / rx[0] if rx[0] > 0 else None),
    ]
    return package(text, dirs, res, x0, values=values)


def quad(G, Lq, Ld, W, I):
    beam, dirs = example_beam()
    beam = _beam_with(beam, current=I, energy=W)
    text = f"start\ndrift 0.05 {_ap()} 0\nquad {Lq:g} {_ap()} 0 {G:g}\ndrift {Ld:g} {_ap()} 0\nend\n"
    _m, res = _evaluate(text, beam, dirs)
    x0 = particles(res.sigma[0])
    gamma, _bg, _b, _p, brho = L._kin(W, beam["mass"], beam["charge"])
    k = G / brho
    f = 1.0 / (k * Lq) if k else None
    values = [
        L._value("brho", "Magnetic rigidity Bρ (how hard the particle is to bend)", "磁刚度 Bρ（粒子有多难被偏转）", "T·m", brho),
        L._value("k", "Focusing strength k = G/(Bρ)", "聚焦强度 k = G/(Bρ)", "1/m²", k),
        L._value("f", "Focal length in x (thin lens, 1/(k·L))", "x 方向焦距（薄透镜 1/(k·L)）", "m", f),
        L._value("rx", "rms x at the exit", "出口 rms x", "mm", res.rms("x")[-1] * 1e3),
        L._value("ry", "rms y at the exit", "出口 rms y", "mm", res.rms("y")[-1] * 1e3),
    ]
    return package(text, dirs, res, x0, values=values)


def fodo(G, Lq, Ld, cells, I, mismatch):
    beam, dirs = example_beam()
    gamma, _bg, _b, _p, brho = L._kin(beam["energy"], beam["mass"], beam["charge"])
    _k, _segs, _cell, px, py = L._fodo_optics(Lq, G, Ld, gamma, brho)
    warnings = []
    if px and py:
        twiss = {"x": (px[0], px[1] * mismatch ** 2), "y": (py[0], py[1] * mismatch ** 2)}
    else:
        twiss = None
        warnings.append(("The cell is unstable: there is no matched beam, the beam starts as the example beam and grows.",
                         "该周期不稳定，不存在匹配束；束流以示例束流出发并不断变粗。"))
    beam = _beam_with(beam, twiss=twiss, current=I)
    text = L._fodo_lattice(Lq, G, Ld, cells)
    _m, res = _evaluate(text, beam, dirs)
    x0 = particles(res.sigma[0])
    mu_x = res.phase_advance("x")
    values = [
        L._value("mux", "Phase advance per cell μx", "每周期相移 μx", "°", math.degrees(px[2]) if px else None),
        L._value("muy", "Phase advance per cell μy", "每周期相移 μy", "°", math.degrees(py[2]) if py else None),
        L._value("turns", "Oscillations of a particle in x along the channel", "粒子在 x 方向沿通道振荡的圈数", "",
                 float(mu_x[-1]) / (2 * math.pi) if len(mu_x) else None),
        L._value("maxrms", "Largest rms size", "最大 rms 尺寸", "mm", float(max(np.max(res.rms("x")), np.max(res.rms("y")))) * 1e3),
    ]
    return package(text, dirs, res, x0, values=values, warnings=warnings)


# ---- orbit correction: a FODO channel with misaligned quadrupoles, a monitor (BPM) after every
# quadrupole and a steerer acting over every quadrupole; SVD of the response matrix
ORBIT_CELLS, ORBIT_G, ORBIT_LQ, ORBIT_LD = 6, 6.0, 0.1, 0.25
ORBIT_MAX_KICK = 5e-3          # T, the steerers' maximum field


def _orbit_lattice():
    lines = ["start"]
    for c in range(1, ORBIT_CELLS + 1):
        for tag, g in (("F", ORBIT_G), ("D", -ORBIT_G)):
            lines.append(f"S{tag}{c} : steerer 0 {_ap()} 0 0 0 0 0")
            lines.append(f"Q{tag}{c} : quad {ORBIT_LQ:g} {_ap()} 0 {g:g}")
            lines.append(f"drift {ORBIT_LD:g} {_ap()} 0")
    lines.append("end")
    return "\n".join(lines) + "\n"


_ORBIT_CACHE = {}


def _orbit_setup(beam, dirs):
    """Model, monitors, steerer knobs and the response matrix of the orbit-correction channel (cached:
    the lattice is fixed and the centroid is linear in the steerers whatever the errors)."""
    text = _orbit_lattice()
    key = (text, beam["energy"], beam["mass"], beam["charge"])
    if key not in _ORBIT_CACHE:
        model = LinearModel(text, beam, dirs)
        base = model.evaluate(space_charge=False)
        quads = [e for e in base.elements if e["key"] == "quad"]
        monitors = Diagnostics([Monitor(f"BPM{i + 1}", float(e["z1"]) + 0.05) for i, e in enumerate(quads)])
        knobs = steerers(model)
        with np.errstate(all="ignore"):
            r = response(model, knobs, monitors, space_charge=False)
        _ORBIT_CACHE.clear()
        _ORBIT_CACHE[key] = (text, model, monitors, knobs, r.matrix, [e["name"] for e in quads])
    return _ORBIT_CACHE[key]


def _svd_correct(matrix, readings, keep):
    """Steerer changes ``−V_k Σ_k⁻¹ U_kᵀ y`` that best cancel *readings* with the *keep* largest singular values."""
    u, s, vt = np.linalg.svd(matrix, full_matrices=False)
    keep = min(int(keep), len(s))
    if keep <= 0:
        return np.zeros(matrix.shape[1]), s
    return -(vt[:keep].T @ ((u[:, :keep].T @ readings) / s[:keep])), s


def orbit(err, noise, svs, seed, I):
    beam, dirs = example_beam()
    beam = _beam_with(beam, current=I)
    text, model, monitors, knobs, matrix, quads = _orbit_setup(beam, dirs)
    rng = np.random.default_rng(int(seed))
    offsets = rng.normal(0.0, err * 1e-3, (len(quads), 2))
    errors = {name: {"dx": float(dx), "dy": float(dy)} for name, (dx, dy) in zip(quads, offsets)}
    try:
        with np.errstate(all="ignore"):
            before = model.evaluate(errors=errors)
    except PreviewError as exc:
        raise LabError(str(exc)) from exc
    readings = monitors.read(before) + rng.normal(0.0, noise * 1e-6, len(monitors.channels()))
    chans = monitors.channels()
    rows = {p: [i for i, (_m, q) in enumerate(chans) if q == p] for p in "xy"}
    cols = {"x": [j for j, k in enumerate(knobs) if k.param == "By"], "y": [j for j, k in enumerate(knobs) if k.param == "Bx"]}
    du = np.zeros(len(knobs))
    sing = {}
    for p in "xy":
        d, sing[p] = _svd_correct(matrix[np.ix_(rows[p], cols[p])], readings[rows[p]], svs)
        du[cols[p]] = d
    warnings = []
    if np.max(np.abs(du), initial=0.0) > ORBIT_MAX_KICK:
        warnings.append((f"Some steerers would need more than their maximum of {ORBIT_MAX_KICK * 1e3:g} mT and are capped: "
                         "the small singular values amplify the BPM noise.",
                         f"部分校正铁需要超过最大值 {ORBIT_MAX_KICK * 1e3:g} mT，已按最大值限幅：小奇异值把 BPM 噪声放大了。"))
        du = np.clip(du, -ORBIT_MAX_KICK, ORBIT_MAX_KICK)
    lattice = {}
    for k, v in zip(knobs, du):
        lattice.setdefault(k.target, {})[k.param] = float(v)
    with np.errstate(all="ignore"):
        after = model.evaluate(errors=errors, knobs=lattice) if svs > 0 else before
    res = after
    x0 = particles(res.sigma[0])
    true_before, true_after = monitors.read(before), monitors.read(after)
    zs = np.array([m.z for m in monitors.monitors])

    def rms(v):
        return float(np.sqrt(np.mean(v * v))) * 1e3

    bpm_x = rows["x"]
    values = [
        L._value("bx", "rms orbit x at the BPMs, before", "校正前 BPM 处 rms 轨道 x", "mm", rms(true_before[bpm_x])),
        L._value("ax", "rms orbit x at the BPMs, after", "校正后 BPM 处 rms 轨道 x", "mm", rms(true_after[bpm_x])),
        L._value("by", "rms orbit y at the BPMs, before", "校正前 BPM 处 rms 轨道 y", "mm", rms(true_before[rows["y"]])),
        L._value("ay", "rms orbit y at the BPMs, after", "校正后 BPM 处 rms 轨道 y", "mm", rms(true_after[rows["y"]])),
        L._value("maxx", "Largest |x| of the beam centre along the line, after", "校正后束流中心沿线最大 |x|", "mm",
                 float(np.max(np.abs(after.centroid[:, 0]))) * 1e3),
        L._value("kick", "Strongest steerer field", "最强校正铁磁场", "mT", float(np.max(np.abs(du))) * 1e3),
    ]
    sv = sing["x"]
    plots = [
        L._plot("bpm", ("Beam centre x at the BPMs", "BPM 处的束流中心 x"), L._axis("z", "z", "m"), L._axis("x", "x", "mm"), [
            L._series("before correction", "校正前", zs, true_before[bpm_x] * 1e3, dash=True),
            L._series("after correction", "校正后", zs, true_after[bpm_x] * 1e3),
            L._series("BPM reading (with noise)", "BPM 读数（含噪声）", zs, readings[bpm_x] * 1e3, marker=True),
        ]),
        L._plot("sv", ("Singular values of the x response matrix (filled: used)", "x 方向响应矩阵的奇异值（实心：参与校正）"),
                L._axis("number", "序号"), L._axis("singular value", "奇异值", "m/T", log=True), [
                    L._series("all", "全部", np.arange(1, len(sv) + 1), sv, dash=True),
                    L._series("used", "参与校正", np.arange(1, min(svs, len(sv)) + 1), sv[:min(svs, len(sv))], marker=True),
                ]),
        L._plot("kicks", ("Steerer fields By (x plane)", "校正铁磁场 By（x 方向）"), L._axis("steerer", "校正铁"),
                L._axis("By", "By", "mT"), [L._series("By", "By", np.arange(1, len(cols["x"]) + 1), du[cols["x"]] * 1e3, marker=True)]),
    ]
    ghosts = []
    if svs > 0:
        ghosts = [{"name": ("beam centre before correction", "校正前的束流中心"), "plane": p, "y": before.centroid[:, i] * 1e3}
                  for p, i in (("x", 0), ("y", 2))]
    mons = [{"name": m.name, "z": m.z} for m in monitors.monitors]
    model_note = ("Linear model: quadrupole offsets move the beam centre, the response matrix is exact for steerers; "
                  "particles drawn from the example beam.",
                  "线性模型：四极铁错位推动束流中心，校正铁的响应矩阵是精确的；粒子按示例束流抽样。")
    return package(text, dirs, res, x0, values=values, plots=plots, monitors=mons, ghosts=ghosts, model=model_note,
                   warnings=warnings)


def solenoid(B, Ls, Ld, W, I):
    beam, dirs = example_beam()
    beam = _beam_with(beam, current=I, energy=W)
    text = f"start\ndrift 0.05 {_ap()} 0\nsolenoid {Ls:g} {_ap()} 0 {B:g}\ndrift {Ld:g} {_ap()} 0\nend\n"
    _m, res = _evaluate(text, beam, dirs)
    x0 = particles(res.sigma[0])
    _g, _bg, _b, _p, brho = L._kin(W, beam["mass"], beam["charge"])
    k = B / (2 * brho)
    kl = k * Ls
    values = [
        L._value("brho", "Magnetic rigidity Bρ", "磁刚度 Bρ", "T·m", brho),
        L._value("k", "k = B/(2Bρ)", "k = B/(2Bρ)", "1/m", k),
        L._value("theta", "Rotation of the beam (Larmor angle kL)", "束流转过的角度（拉莫尔角 kL）", "°", math.degrees(kl)),
        L._value("f", "Focal length 1/(k·sin kL)", "焦距 1/(k·sin kL)", "m",
                 1 / (k * math.sin(kl)) if abs(k * math.sin(kl)) > 1e-12 else None),
        L._value("rx", "rms x at the exit", "出口 rms x", "mm", res.rms("x")[-1] * 1e3),
        L._value("ry", "rms y at the exit", "出口 rms y", "mm", res.rms("y")[-1] * 1e3),
    ]
    return package(text, dirs, res, x0, values=values)


STEERER_LENGTH = 0.1           # m, the drift a steerer acts over in the animation (its magnet)


def steerer(By, Bx, W, Ld):
    beam, dirs = example_beam()
    beam = _beam_with(beam, current=0.0, energy=W)
    text = (f"start\ndrift 0.1 {_ap()} 0\nsteerer 0 {_ap()} 0 {Bx * 1e-3:g} {By * 1e-3:g} 0 0\n"
            f"drift {STEERER_LENGTH:g} {_ap()} 0\ndrift {Ld:g} {_ap()} 0\nend\n")
    _m, res = _evaluate(text, beam, dirs)
    x0 = particles(res.sigma[0])
    _g, _bg, _b, _p, brho = L._kin(W, beam["mass"], beam["charge"])
    values = [
        L._value("brho", "Magnetic rigidity Bρ", "磁刚度 Bρ", "T·m", brho),
        L._value("tx", "Kick angle in x: −By·L/(Bρ)", "x 方向偏转角：−By·L/(Bρ)", "mrad", -By * 1e-3 * STEERER_LENGTH / brho * 1e3),
        L._value("ty", "Kick angle in y: +Bx·L/(Bρ)", "y 方向偏转角：+Bx·L/(Bρ)", "mrad", Bx * 1e-3 * STEERER_LENGTH / brho * 1e3),
        L._value("cx", "Beam centre x at the end", "末端束流中心 x", "mm", res.centroid[-1, 0] * 1e3),
        L._value("cy", "Beam centre y at the end", "末端束流中心 y", "mm", res.centroid[-1, 2] * 1e3),
    ]
    model = ("Linear model: the steerer's uniform field acts over the 0.1 m element after it and moves the beam centre; "
             "particles drawn from the example beam (no space charge).",
             "线性模型：校正铁的均匀磁场作用在它后面 0.1 m 的元件上，推动束流中心；粒子按示例束流抽样（不含空间电荷）。")
    return package(text, dirs, res, x0, values=values, model=model, centre=True)


def dipole(alpha, rho, N, edge, W):
    if not rho > 0:
        raise LabError("ρ must be positive.")
    beam, dirs = example_beam()
    beam = _beam_with(beam, current=0.0, energy=W)
    text = (f"start\ndrift 0.15 {_ap(0.03)} 0\nedge 0 {_ap(0.03)} 0 {edge:g} {rho:g} 0 0 0 0\n"
            f"bend 0 {_ap(0.03)} 0 {alpha:g} {rho:g} {N:g} 0\nedge 0 {_ap(0.03)} 0 {edge:g} {rho:g} 0 0 0 0\n"
            f"drift 0.5 {_ap(0.03)} 0\nend\n")
    model, res = _evaluate(text, beam, dirs)
    x0 = particles(res.sigma[0])
    _g, _bg, _b, _p, brho = L._kin(W, beam["mass"], beam["charge"])
    ghosts = []
    for d in (0.01, -0.01):
        with np.errstate(all="ignore"):
            off = model.evaluate(space_charge=False, centroid0=np.array([0, 0, 0, 0, 0, d]))
        ghosts.append({"name": (f"centre of a beam with Δp/p = {d * 100:+g} %", f"Δp/p = {d * 100:+g} % 的束流中心"),
                       "plane": "x", "y": off.centroid[:, 0] * 1e3})
    bend_el = next(e for e in res.elements if e["key"] == "bend")
    r16 = (ghosts[0]["y"][-1] / 1e3) / 0.01
    values = [
        L._value("B", "Dipole field B = Bρ/ρ", "二极场 B = Bρ/ρ", "T", brho / rho),
        L._value("arc", "Arc length ρ·α", "弧长 ρ·α", "m", bend_el["z1"] - bend_el["z0"]),
        L._value("D", "Offset per 1 % momentum at the end (dispersion)", "末端每 1 % 动量偏差的偏移（色散）", "mm", r16 * 1e-2 * 1e3),
        L._value("rx", "rms x at the end", "末端 rms x", "mm", res.rms("x")[-1] * 1e3),
        L._value("ry", "rms y at the end", "末端 rms y", "mm", res.rms("y")[-1] * 1e3),
    ]
    note = ("Linear model, sector dipole with pole-face angles; x is measured from the curved reference orbit, positive "
            "towards the outside of the bend. Particles drawn from the example beam (no space charge).",
            "线性模型，扇形二极铁加极面旋转角；x 从弯曲的参考轨道量起，正方向指向弯转外侧。粒子按示例束流抽样（不含空间电荷）。")
    return package(text, dirs, res, x0, values=values, ghosts=ghosts, model=note)


Z_MARKS = (("z", 1), ("z", -1))


def cavity(phase, Ke, W, I):
    folder = paths.example_input_dir()
    if folder is None:
        raise LabError("The example field maps (examples/hwr010) are not installed.")
    beam, dirs = example_beam()
    beam = _beam_with(beam, current=I, energy=W)
    text = L._cavity_text(phase, Ke)
    _m, res = _evaluate(text, beam, dirs)
    x0 = particles(res.sigma[0], marks=Z_MARKS)
    cav = next((e for e in res.elements if e["key"] == "field"), None)
    w_in = float(res.energy[0])
    values = [
        L._value("win", "Entrance energy", "入口能量", "MeV", w_in),
        L._value("gain", "Energy gain ΔW", "能量增益 ΔW", "MeV", float(res.energy[-1]) - w_in),
        L._value("phis", "Synchronous phase φs", "同步相位 φs", "°", cav["phase_s"] if cav else None),
        L._value("entry", "RF phase at the entrance φ0", "入口射频相位 φ0", "°", cav["phase_rf"] if cav else None),
        L._value("z0", "Bunch length (rms z), entrance", "束团长度（rms z），入口", "mm", res.rms("z")[0] * 1e3),
        L._value("z1", "Bunch length (rms z), exit", "束团长度（rms z），出口", "mm", res.rms("z")[-1] * 1e3),
        L._value("dp1", "Momentum spread (rms δ), exit", "动量分散（rms δ），出口", "‰", res.rms_divergence("z")[-1] * 1e3),
    ]
    note = ("Linear model of the example HWR field map (162.5 MHz, Kb = −Ke as in the example); the highlighted particles "
            "start 1.5 rms ahead of and behind the bunch centre. The multi-particle engine is the reference.",
            "示例 HWR 场图的线性模型（162.5 MHz，与示例一样取 Kb = −Ke）；两个跟踪粒子分别从束团中心前方和后方 1.5 rms 处出发。"
            "以多粒子内核结果为准。")
    return package(text, dirs, res, x0, values=values, model=note, marks=Z_MARKS)


MOTIONS = {m.id: m for m in [
    Motion("drift", ("A beam in a drift", "束流在漂移段中"),
           [Param("L", ("Drift length", "漂移段长度"), "m", 1.5, 0.1, 3.0, 0.01),
            Param("alpha", ("Converging (+) or diverging (−) at the entrance: Twiss α", "入口处汇聚（+）或发散（−）：Twiss α"), "",
                  1.0, -3.0, 3.0, 0.01, doc=("the example beam has α = −0.66 (it diverges)", "示例束流 α = −0.66（正在发散）")),
            Param("emit", ("Emittance ε (spread of angles)", "发射度 ε（角度的分散程度）"), "π·mm·mrad", 0.138, 0.01, 1.0, 0.001),
            _current_param()],
           lambda L, alpha, emit, I: drift(L, alpha, emit, I),
           ("Every particle flies straight on; the beam grows because the angles differ.",
            "每个粒子都走直线；束流变粗是因为各粒子的角度不同。")),
    Motion("quad", ("Through one quadrupole", "穿过一块四极铁"),
           [Param("G", ("Gradient G", "梯度 G"), "T/m", 10.0, -30.0, 30.0, 0.1,
                  doc=("G > 0 focuses x and defocuses y (positive particles)", "G > 0 时 x 聚焦、y 散焦（正电荷）")),
            Param("Lq", ("Quadrupole length", "四极铁长度"), "m", 0.1, 0.02, 0.4, 0.005),
            Param("Ld", ("Drift after it", "后面的漂移段"), "m", 0.5, 0.1, 1.5, 0.01),
            Param("W", ("Kinetic energy W", "动能 W"), "MeV", 1.527, 0.1, 200.0, 0.001, log=True,
                  doc=("faster particles are harder to bend", "越快的粒子越难偏转")),
            _current_param()],
           quad, ("Off-axis particles are pushed back in x and pushed out in y.", "离轴粒子在 x 方向被推回、在 y 方向被推开。")),
    Motion("fodo", ("Through a FODO channel", "穿过 FODO 通道"),
           [*L._fodo_params(G=6.0), _current_param(),
            Param("mismatch", ("Mismatch: initial size factor", "失配：初始尺寸倍数"), "", 1.0, 0.5, 2.0, 0.01)],
           fodo, ("Alternating quadrupoles keep the particles oscillating about the axis.", "交替排列的四极铁让粒子绕轴振荡而不散开。")),
    Motion("orbit", ("Orbit correction", "轨道校正"),
           [Param("err", ("Quadrupole offsets (rms)", "四极铁错位（rms）"), "mm", 0.4, 0.0, 1.0, 0.01),
            Param("noise", ("BPM noise (rms)", "BPM 噪声（rms）"), "µm", 10.0, 0.0, 200.0, 1.0),
            Param("svs", ("Singular values used (0 = no correction)", "参与校正的奇异值个数（0 表示不校正）"), "", 0, 0, 2 * ORBIT_CELLS, 1,
                  integer=True),
            Param("seed", ("Error set (random seed)", "误差样本（随机种子）"), "", 1, 1, 50, 1, integer=True),
            _current_param()],
           orbit, ("Misaligned quadrupoles push the beam off axis; BPMs and steerers bring it back.",
                   "错位的四极铁把束流推离轴线；BPM 与校正铁把它拉回来。")),
    Motion("solenoid", ("Through a solenoid", "穿过一个螺线管"),
           [Param("B", ("Field B", "磁场 B"), "T", 1.5, -5.0, 5.0, 0.01,
                  doc=("along the beam inside the coil; the sign sets the sense of rotation", "线圈内沿束流方向；正负号决定旋转方向")),
            Param("Ls", ("Solenoid length", "螺线管长度"), "m", 0.2, 0.05, 0.6, 0.005),
            Param("Ld", ("Drift after it", "后面的漂移段"), "m", 0.5, 0.1, 1.5, 0.01),
            Param("W", ("Kinetic energy W", "动能 W"), "MeV", 1.527, 0.1, 200.0, 0.001, log=True),
            _current_param()],
           solenoid, ("The particles turn about the axis and are focused in both planes at once.",
                      "粒子绕轴旋转，同时在两个方向上被聚焦。")),
    Motion("steerer", ("A steerer bends the beam", "校正铁让束流转向"),
           [Param("By", ("Vertical field By", "竖直磁场 By"), "mT", 3.0, -10.0, 10.0, 0.1,
                  doc=("By > 0 pushes a positive particle towards −x", "By > 0 把正电荷粒子推向 −x")),
            Param("Bx", ("Horizontal field Bx", "水平磁场 Bx"), "mT", 0.0, -10.0, 10.0, 0.1,
                  doc=("Bx > 0 pushes a positive particle towards +y", "Bx > 0 把正电荷粒子推向 +y")),
            Param("W", ("Kinetic energy W", "动能 W"), "MeV", 1.527, 0.1, 200.0, 0.001, log=True),
            Param("Ld", ("Drift after it", "后面的漂移段"), "m", 1.0, 0.2, 2.0, 0.01)],
           steerer, ("A weak uniform field tilts the whole beam by a small angle.", "一个很弱的均匀磁场让整束粒子偏转一个小角度。")),
    Motion("dipole", ("Through a bending magnet", "穿过一块二极铁"),
           [Param("alpha", ("Bend angle α", "偏转角 α"), "°", 30.0, -90.0, 90.0, 0.5,
                  doc=("α > 0 bends a positive particle towards −x", "α > 0 让正电荷粒子转向 −x")),
            Param("rho", ("Bend radius ρ", "曲率半径 ρ"), "m", 0.5, 0.2, 3.0, 0.01),
            Param("N", ("Field index N", "场梯度指数 N"), "", 0.0, -2.0, 2.0, 0.01),
            Param("edge", ("Pole-face angle β", "极面旋转角 β"), "°", 0.0, -30.0, 30.0, 0.5),
            Param("W", ("Kinetic energy W", "动能 W"), "MeV", 1.527, 0.1, 200.0, 0.001, log=True)],
           dipole, ("The beam follows a circular arc; particles with more momentum take a wider arc.",
                    "束流沿圆弧转弯；动量大的粒子走更大的圆弧。")),
    Motion("cavity", ("Through an RF cavity", "穿过一个射频腔"),
           [Param("phase", ("Synchronous phase φs", "同步相位 φs"), "°", -30.0, -90.0, 90.0, 1.0,
                  doc=("negative: the bunch rides the rising side of the wave and stays together", "负值：束团骑在波的上升沿，保持聚拢")),
            Param("Ke", ("Field level Ke", "场强系数 Ke"), "", 1.36, 0.0, 2.5, 0.01),
            Param("W", ("Kinetic energy W", "动能 W"), "MeV", 1.527, 0.4, 3.0, 0.001,
                  doc=("the example HWR cavity is built for protons around 1.5 MeV", "示例 HWR 腔是为 1.5 MeV 左右的质子设计的")),
            _current_param()],
           cavity, ("The RF field pushes the bunch forward; particles behind the centre get a little more.",
                    "射频电场把束团向前推；落在中心后面的粒子多得一点能量。")),
]}


def definitions():
    return [m.public() for m in MOTIONS.values()]


def run(motion_id, params=None):
    """Evaluate motion *motion_id* with *params* (missing ones take their defaults, values are clipped)."""
    m = MOTIONS.get(motion_id)
    if m is None:
        raise LabError(f"Unknown motion: {motion_id}")
    args = L.clip_params(m.params, params)
    with np.errstate(all="ignore"):
        out = m.fn(**args)
    out["params"] = args
    return out
