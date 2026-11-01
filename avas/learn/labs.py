"""Interactive labs of the accelerator primer.

Every lab is a function of a few physical parameters that returns plain data
for the page: result values, curves and matrices.  The physics is the code of
the linear envelope preview (:mod:`avas.sim.linear_optics`: kinematics,
transfer matrices, :func:`~avas.sim.linear_optics.linear_preview` with its
space-charge model), so a lab and the lattice editor's preview always agree.
The labs are linear models and say so; the multi-particle engine stays the
reference.

Units follow AVAS: lengths in m, Twiss β in mm/mrad (= m/rad), emittances
normalised rms in π·mm·mrad, energies in MeV, gradients in T/m, fields in T.

Result layout (see :func:`run`)::

    {"values":   [{"key", "label": (en, zh), "unit", "value"}],
     "plots":    [{"id", "title", "x": {"label", "unit", "log"?}, "y": {...}, "equal"?,
                   "series": [{"name": (en, zh), "x": array, "y": array, "dash"?, "marker"?}]}],
     "matrices": [{"title", "labels": [...], "rows": [[...]]}],
     "lattice":  AVAS lattice text the lab evaluated (or None),
     "beamline": {"z", "rms_x", "rms_y", "energy", "elements", "fieldDirs"}: the linear preview of
                 that lattice, for the page's 2D / 3D view of it (or None),
     "warnings": [(en, zh)], "model": (en, zh)}
"""
import math

import numpy as np

from avas import paths
from avas.sim import linear_optics as lo

PROTON = 938.272088
ELECTRON = 0.51099895
EXAMPLE_FREQ = 162.5e6            # Hz, the example HWR cavity
CAVITY_MAP, CAVITY_LENGTH = "hwr010", 0.21
APERTURE = 0.02                   # m, the bore of the labs' elements


class LabError(Exception):
    """The parameters cannot be evaluated (a message for the page)."""


class Param:
    __slots__ = ("key", "label", "unit", "default", "min", "max", "step", "log", "integer", "doc")

    def __init__(self, key, label, unit, default, lo_, hi, step, log=False, integer=False, doc=("", "")):
        self.key, self.label, self.unit = key, label, unit
        self.default, self.min, self.max, self.step = default, lo_, hi, step
        self.log, self.integer, self.doc = log, integer, doc

    def public(self):
        return {"key": self.key, "label": list(self.label), "unit": self.unit, "default": self.default,
                "min": self.min, "max": self.max, "step": self.step, "log": self.log, "integer": self.integer,
                "doc": list(self.doc)}


def _mass(default=PROTON):
    return Param("mass", ("Rest mass", "静止质量"), "MeV", default, 0.5, 250000.0, 0.001, log=True,
                 doc=("proton 938.272, electron 0.511", "质子 938.272，电子 0.511"))


def _charge():
    return Param("q", ("Charge state q", "电荷态 q"), "", 1, -10, 92, 1, integer=True,
                 doc=("in units of e; the sign matters for focusing", "以 e 为单位；正负号影响聚焦方向"))


def _energy(default=1.527):
    return Param("W", ("Kinetic energy W", "动能 W"), "MeV", default, 0.01, 2000.0, 0.001, log=True)


def _emit(default=0.14):
    return Param("emit", ("Emittance ε (normalized rms)", "发射度 ε（归一化 rms）"), "π·mm·mrad", default, 0.01, 2.0, 0.01)


def _fodo_params(G=4.0, I=0.0):
    return [
        Param("Lq", ("Quadrupole length", "四极铁长度"), "m", 0.1, 0.02, 0.5, 0.005),
        Param("G", ("Gradient G", "梯度 G"), "T/m", G, 0.0, 60.0, 0.1,
              doc=("QF has +G, QD −G", "QF 取 +G，QD 取 −G")),
        Param("Ld", ("Drift between quadrupoles", "四极铁间漂移段"), "m", 0.3, 0.02, 2.0, 0.01),
        Param("cells", ("Cells", "周期数"), "", 6, 1, 30, 1, integer=True),
    ]


class Lab:
    def __init__(self, id_, title, params, fn, summary):
        self.id, self.title, self.params, self.fn, self.summary = id_, title, params, fn, summary

    def public(self):
        return {"id": self.id, "title": list(self.title), "summary": list(self.summary),
                "params": [p.public() for p in self.params]}


# =========================================================================== helpers
def _kin(W, mass, q):
    if not (mass > 0 and W > 0):
        raise LabError("Mass and kinetic energy must be positive.")
    if q == 0:
        raise LabError("The charge state must not be zero.")
    gamma, bg, beta, p = (float(v) for v in lo._kin(W, mass))
    return gamma, bg, beta, p, p / (lo.MEV_TM * q)


def _value(key, en, zh, unit, value):
    return {"key": key, "label": (en, zh), "unit": unit, "value": None if value is None else float(value)}


def _series(en, zh, x, y, **style):
    return {"name": (en, zh), "x": np.asarray(x, dtype=float), "y": np.asarray(y, dtype=float), **style}


def _plot(id_, title, x, y, series, **opts):
    return {"id": id_, "title": title, "x": x, "y": y, "series": series, **opts}


def _axis(en, zh, unit="", **opts):
    return {"label": (en, zh), "unit": unit, **opts}


def _ellipse(sigma, n=121):
    """Points of the rms ellipse of the 2×2 second-moment matrix *sigma*."""
    a = np.linalg.cholesky(sigma + np.eye(2) * 1e-30)
    t = np.linspace(0.0, 2 * math.pi, n)
    pts = a @ np.vstack([np.cos(t), np.sin(t)])
    return pts[0], pts[1]


def _twiss_from_sigma(s):
    emit = math.sqrt(max(float(np.linalg.det(s)), 0.0))
    if emit <= 0:
        return 0.0, 0.0, 0.0, 0.0
    return -s[0, 1] / emit, s[0, 0] / emit, s[1, 1] / emit, emit


def _periodic(m):
    """``(alpha, beta, mu)`` of the periodic solution of the 2×2 matrix *m*, or None when unstable."""
    half = 0.5 * (m[0, 0] + m[1, 1])
    if not abs(half) < 1.0:
        return None
    mu = math.acos(half)
    if m[0, 1] < 0:                         # β > 0 needs sin μ with the sign of M12
        mu = 2 * math.pi - mu
    s = math.sin(mu)
    return (m[0, 0] - m[1, 1]) / (2 * s), m[0, 1] / s, mu


def _segments(Lq, k, Ld):
    """One FODO cell: QF, drift, QD, drift as ``(kind, length, k)``."""
    return [("quad", Lq, k), ("drift", Ld, 0.0), ("quad", Lq, -k), ("drift", Ld, 0.0)]


def _seg_matrix(kind, length, k, gamma):
    return lo.quad_matrix(length, k, gamma) if kind == "quad" else lo.drift_matrix(length, gamma)


def _cell_matrix(segs, gamma):
    m = np.eye(6)
    for kind, length, k in segs:
        m = _seg_matrix(kind, length, k, gamma) @ m
    return m


def _fodo_lattice(Lq, G, Ld, cells):
    cell = (f"quad {Lq:g} {APERTURE:g} 0 {G:g}\ndrift {Ld:g} {APERTURE:g} 0\n"
            f"quad {Lq:g} {APERTURE:g} 0 {-G:g}\ndrift {Ld:g} {APERTURE:g} 0\n")
    return "start\n" + cell * int(cells) + "end\n"


def _preview(text, beam, field_dirs=(), space_charge=None):
    try:
        return lo.linear_preview(text, beam, list(field_dirs), space_charge=space_charge, max_points=1500)
    except lo.PreviewError as exc:
        raise LabError(str(exc)) from exc


def _beam(W, mass, q, twiss, current=0.0, dc=True, frequency=EXAMPLE_FREQ):
    return {"mass": mass, "charge": q, "energy": W, "current": current, "frequency": frequency, "dc": dc,
            "twiss": twiss, "source": "twiss", "warnings": []}


def _fodo_optics(Lq, G, Ld, gamma, brho):
    k = G / brho
    segs = _segments(Lq, k, Ld)
    m = _cell_matrix(segs, gamma)
    return k, segs, m, _periodic(m[0:2, 0:2]), _periodic(m[2:4, 2:4])


def _beamline(res, field_dirs=()):
    """The preview of the lab's lattice for the page's 2D / 3D beam-line view."""
    out = {k: res[k] for k in ("z", "rms_x", "rms_y", "energy")}
    out["elements"] = res["elements"]
    out["fieldDirs"] = list(field_dirs)
    return out


def _envelope_plot(res, title=("rms envelope", "rms 包络"), extra=()):
    return _plot("envelope", title, _axis("z", "z", "m"), _axis("rms size", "rms 尺寸", "mm"), [
        _series("rms x", "rms x", res["z"], res["rms_x"]),
        _series("rms y", "rms y", res["z"], res["rms_y"]),
        *extra,
    ])


LINEAR = ("Linear model (AVAS linear preview code); the multi-particle engine is the reference.",
          "线性模型（AVAS 线性包络预览的代码）；以多粒子内核结果为准。")


# =========================================================================== labs
def kinematics(mass, q, W, f):
    gamma, bg, beta, p, brho = _kin(W, mass, q)
    lam = lo.C_LIGHT / (f * 1e6) if f > 0 else None
    erho = p * beta / q                                 # pv/q in MV
    values = [
        _value("gamma", "Lorentz factor γ", "洛伦兹因子 γ", "", gamma),
        _value("beta", "Velocity β = v/c", "速度 β = v/c", "", beta),
        _value("bg", "βγ", "βγ", "", bg),
        _value("p", "Momentum p", "动量 p", "MeV/c", p),
        _value("E", "Total energy E = γmc²", "总能量 E = γmc²", "MeV", gamma * mass),
        _value("brho", "Magnetic rigidity Bρ", "磁刚度 Bρ", "T·m", brho),
        _value("erho", "Electric rigidity Eρ", "电刚度 Eρ", "MV", erho),
        _value("lambda", "RF wavelength λ", "射频波长 λ", "m", lam),
        _value("bl", "βλ (distance per RF period)", "βλ（一个射频周期飞行距离）", "mm", None if lam is None else beta * lam * 1e3),
    ]
    w = np.logspace(-3, 4, 300)
    series = []
    for en, zh, m0, dash in (("this particle", "当前粒子", mass, False), ("proton", "质子", PROTON, True),
                             ("electron", "电子", ELECTRON, True)):
        if dash and abs(m0 - mass) / mass < 1e-3:
            continue
        g = 1 + w / m0
        series.append(_series(en, zh, w, np.sqrt(1 - 1 / g ** 2), dash=dash))
    series.append(_series("W", "W", [W], [beta], marker=True))
    g = 1 + w / mass
    pw = mass * np.sqrt(g * g - 1)
    return {
        "values": values,
        "plots": [
            _plot("beta", ("Velocity versus kinetic energy", "速度随动能的变化"), _axis("W", "W", "MeV", log=True),
                  _axis("β", "β"), series),
            _plot("brho", ("Magnetic rigidity versus kinetic energy", "磁刚度随动能的变化"), _axis("W", "W", "MeV", log=True),
                  _axis("Bρ", "Bρ", "T·m", log=True),
                  [_series("Bρ", "Bρ", w, pw / (lo.MEV_TM * abs(q))), _series("W", "W", [W], [abs(brho)], marker=True)]),
        ],
        "model": ("Exact relativistic kinematics.", "严格的相对论运动学。"),
    }


def phase_space(alpha, beta, emit, W, mass, f, L):
    if not beta > 0:
        raise LabError("β must be positive.")
    _gamma, bg, _b, _p, _brho = _kin(W, mass, 1)
    eg = emit / bg                                       # geometric rms, mm·mrad
    gt = (1 + alpha * alpha) / beta
    s0 = eg * np.array([[beta, -alpha], [-alpha, gt]])
    lens = np.array([[1.0, 0.0], [-1.0 / f, 1.0]]) if f else np.eye(2)
    s1 = lens @ s0 @ lens.T
    drift = lo.drift_matrix(L)[0:2, 0:2]
    s2 = drift @ s1 @ drift.T
    a2, b2, g2, _e = _twiss_from_sigma(s2)
    a1, _b1, g1, _e1 = _twiss_from_sigma(s1)
    waist = a1 / g1 if g1 > 0 and a1 > 0 else None
    s = np.linspace(0.0, max(L, 1e-6), 200)
    rms = np.sqrt(s1[0, 0] - 2 * s * (-s1[0, 1]) + s * s * s1[1, 1])
    series = [_series("entrance", "入口", *_ellipse(s0))]
    if f:
        series.append(_series("after the lens", "透镜之后", *_ellipse(s1), dash=True))
    series.append(_series("after the drift", "漂移段之后", *_ellipse(s2)))
    values = [
        _value("eg", "Geometric rms emittance ε/(βγ)", "几何 rms 发射度 ε/(βγ)", "mm·mrad", eg),
        _value("x0", "rms x at the entrance", "入口 rms x", "mm", math.sqrt(s0[0, 0])),
        _value("xp0", "rms x′ at the entrance", "入口 rms x′", "mrad", math.sqrt(s0[1, 1])),
        _value("alpha2", "α after the drift", "漂移段后 α", "", a2),
        _value("beta2", "β after the drift", "漂移段后 β", "mm/mrad", b2),
        _value("gamma2", "γ after the drift", "漂移段后 γ", "mrad/mm", g2),
        _value("x2", "rms x after the drift", "漂移段后 rms x", "mm", math.sqrt(s2[0, 0])),
        _value("waist", "Distance to the waist after the lens", "透镜后到束腰的距离", "m", waist),
        _value("area", "Ellipse area / π (conserved)", "椭圆面积 / π（守恒）", "mm·mrad", math.sqrt(np.linalg.det(s2))),
    ]
    return {
        "values": values,
        "plots": [
            _plot("ellipse", ("rms ellipse in (x, x′)", "(x, x′) 相平面中的 rms 椭圆"), _axis("x", "x", "mm"),
                  _axis("x′", "x′", "mrad"), series, equal=False),
            _plot("size", ("rms size along the drift", "漂移段中的 rms 尺寸"), _axis("s", "s", "m"),
                  _axis("rms x", "rms x", "mm"), [_series("rms x", "rms x", s, rms)]),
        ],
        "model": ("Exact for linear optics: thin lens then drift, σ ↦ M σ Mᵀ.", "线性光学下严格成立：先薄透镜后漂移，σ ↦ M σ Mᵀ。"),
    }


def fodo(Lq, G, Ld, cells, W, mass, q, emit, I, mismatch):
    gamma, bg, _b, _p, brho = _kin(W, mass, q)
    k, segs, m, px, py = _fodo_optics(Lq, G, Ld, gamma, brho)
    warnings = []
    spacing = Lq + Ld
    f_thin = 1.0 / (abs(k) * Lq) if k else None
    cos_thin = 1 - spacing ** 2 / (2 * f_thin ** 2) if f_thin else 1.0
    mu_thin = math.degrees(math.acos(cos_thin)) if abs(cos_thin) < 1 else None
    plots, values = [], []
    if px and py:
        twiss = {"x": (px[0] * 1.0, px[1] * mismatch ** 2, emit), "y": (py[0] * 1.0, py[1] * mismatch ** 2, emit)}
        # β along one cell
        sx = np.array([[px[1], -px[0]], [-px[0], (1 + px[0] ** 2) / px[1]]])
        sy = np.array([[py[1], -py[0]], [-py[0], (1 + py[0] ** 2) / py[1]]])
        z, bx, by = [0.0], [px[1]], [py[1]]
        for kind, length, kk in segs:
            n = 20 if kind == "quad" else 30
            step = _seg_matrix(kind, length / n, kk, gamma)
            for _ in range(n):
                sx = step[0:2, 0:2] @ sx @ step[0:2, 0:2].T
                sy = step[2:4, 2:4] @ sy @ step[2:4, 2:4].T
                z.append(z[-1] + length / n)
                bx.append(sx[0, 0])
                by.append(sy[0, 0])
        plots.append(_plot("beta", ("Periodic β along one cell", "一个周期内的周期 β 函数"), _axis("s", "s", "m"),
                           _axis("β", "β", "m"), [_series("βx", "βx", z, bx), _series("βy", "βy", z, by)]))
    else:
        twiss = {"x": (0.0, 1.0, emit), "y": (0.0, 1.0, emit)}
        warnings.append(("The cell is unstable (|Tr M|/2 ≥ 1): there is no matched beam; the envelope starts from α = 0, β = 1 m and grows.",
                         "该周期不稳定（|Tr M|/2 ≥ 1），不存在匹配束；包络从 α = 0、β = 1 m 出发并不断增大。"))
    text = _fodo_lattice(Lq, G, Ld, cells)
    res = _preview(text, _beam(W, mass, q, twiss, current=I), space_charge=I > 0)
    warnings += list(res["warnings"])
    plots.append(_envelope_plot(res))
    values += [
        _value("k", "Focusing strength k = G/(Bρ)", "聚焦强度 k = G/(Bρ)", "1/m²", k),
        _value("f", "Thin-lens focal length 1/(k·Lq)", "薄透镜焦距 1/(k·Lq)", "m", f_thin),
        _value("cosx", "cos μx = Tr Mx / 2", "cos μx = Tr Mx / 2", "", 0.5 * (m[0, 0] + m[1, 1])),
        _value("mux", "Phase advance μx per cell", "每周期相移 μx", "°", math.degrees(px[2]) if px else None),
        _value("muy", "Phase advance μy per cell", "每周期相移 μy", "°", math.degrees(py[2]) if py else None),
        _value("muthin", "Thin-lens estimate of μ", "薄透镜估计的 μ", "°", mu_thin),
        _value("betax", "Matched βx at the cell entrance", "周期入口匹配 βx", "mm/mrad", px[1] if px else None),
        _value("alphax", "Matched αx at the cell entrance", "周期入口匹配 αx", "", px[0] if px else None),
        _value("maxrms", "Largest rms size", "最大 rms 尺寸", "mm", float(max(np.max(res["rms_x"]), np.max(res["rms_y"])))),
    ]
    return {"values": values, "plots": plots, "lattice": text, "beamline": _beamline(res), "warnings": warnings,
            "matrices": [{"title": ("Cell matrix, x plane", "周期传输矩阵（x 平面）"), "labels": ["x", "x′"],
                          "rows": m[0:2, 0:2].tolist()}],
            "model": LINEAR}


def solenoid(L, B, W, mass, q, emit, d1, d2):
    gamma, _bg, _b, _p, brho = _kin(W, mass, q)
    k = B / (2 * brho)
    kl = k * L
    m4 = lo._solenoid4(np.array([k]), L)[0]
    # one particle that starts 1 mm off axis, parallel to it
    segs = [("drift", d1), ("solenoid", L), ("drift", d2)]
    v = np.array([1.0, 0.0, 0.0, 0.0])
    s, xs, ys = [0.0], [1.0], [0.0]
    for kind, length in segs:
        n = 60
        step = lo._solenoid4(np.array([k if kind == "solenoid" else 0.0]), length / n)[0]
        for _ in range(n):
            v = step @ v
            s.append(s[-1] + length / n)
            xs.append(v[0])
            ys.append(v[2])
    text = (f"start\ndrift {d1:g} {APERTURE:g} 0\nsolenoid {L:g} {APERTURE:g} 0 {B:g}\n"
            f"drift {d2:g} {APERTURE:g} 0\nend\n")
    res = _preview(text, _beam(W, mass, q, {"x": (0.0, 0.4, emit), "y": (0.0, 0.4, emit)}), space_charge=False)
    values = [
        _value("brho", "Magnetic rigidity Bρ", "磁刚度 Bρ", "T·m", brho),
        _value("k", "k = B/(2Bρ)", "k = B/(2Bρ)", "1/m", k),
        _value("theta", "Larmor rotation angle kL", "拉莫尔转角 kL", "°", math.degrees(kl)),
        _value("f", "Focal length 1/(k sin kL) (Larmor frame)", "焦距 1/(k sin kL)（拉莫尔坐标系）", "m",
               1 / (k * math.sin(kl)) if abs(k * math.sin(kl)) > 1e-12 else None),
        _value("fthin", "Thin-lens focal length 1/(k²L)", "薄透镜焦距 1/(k²L)", "m", 1 / (k * k * L) if k else None),
    ]
    return {
        "values": values,
        "plots": [
            _plot("orbit", ("A particle starting at x = 1 mm", "从 x = 1 mm 出发的粒子"), _axis("s", "s", "m"),
                  _axis("position", "位置", "mm"), [_series("x", "x", s, xs), _series("y", "y", s, ys)]),
            _plot("xy", ("Its projection on the (x, y) plane, seen from downstream", "该粒子在 (x, y) 平面上的投影（从下游往上游看）"),
                  _axis("x", "x", "mm"), _axis("y", "y", "mm"), [_series("orbit", "轨迹", xs, ys)], equal=True),
            _envelope_plot(res, extra=()),
        ],
        "matrices": [{"title": ("Hard-edge solenoid matrix (x, x′, y, y′)", "硬边螺线管传输矩阵 (x, x′, y, y′)"),
                      "labels": ["x", "x′", "y", "y′"], "rows": m4.tolist()}],
        "lattice": text, "beamline": _beamline(res), "warnings": list(res["warnings"]),
        "model": ("Hard-edge matrix model; the initial beam is a round waist with β = 0.4 mm/mrad.",
                  "硬边矩阵模型；初始束流为圆形束腰，β = 0.4 mm/mrad。"),
    }


def dipole(angle, rho, N, edge, W, mass, q, delta):
    if not rho > 0:
        raise LabError("ρ must be positive.")
    gamma, _bg, _b, _p, brho = _kin(W, mass, q)
    a = math.radians(angle)
    arc = rho * abs(a)
    e = lo.edge_matrix(edge, rho, 0.0, 0.0, 0.0, 0)
    post = 1.0
    n = 80
    step = lo.bend_matrix(arc / n, a / n, N, 0, gamma)
    total = e.copy()
    s, xp, xm = [0.0], [0.0], [0.0]
    vp = np.zeros(6)
    vp[5] = delta / 100
    vp = e @ vp
    for _ in range(n):
        total = step @ total
        vp = step @ vp
        s.append(s[-1] + arc / n)
        xp.append(vp[0] * 1e3)
    total = e @ total
    vp = e @ vp
    exit_m = total.copy()
    d = lo.drift_matrix(post / 40, gamma)
    for _ in range(40):
        vp = d @ vp
        s.append(s[-1] + post / 40)
        xp.append(vp[0] * 1e3)
    xm = [-v for v in xp]
    text = (f"start\nedge 0 0.03 0 {edge:g} {rho:g} 0 0 0 0\nbend 0 0.03 0 {angle:g} {rho:g} {N:g} 0\n"
            f"edge 0 0.03 0 {edge:g} {rho:g} 0 0 0 0\ndrift {post:g} 0.03 0\nend\n")
    res = _preview(text, _beam(W, mass, q, {"x": (0.0, 1.0, 0.14), "y": (0.0, 1.0, 0.14)}), space_charge=False)
    r = exit_m
    values = [
        _value("B", "Dipole field B = Bρ/ρ", "二极场 B = Bρ/ρ", "T", brho / rho),
        _value("arc", "Arc length ρ·α", "弧长 ρ·α", "m", arc),
        _value("D", "Dispersion D = R16", "色散 D = R16", "m", r[0, 5]),
        _value("Dp", "D′ = R26", "D′ = R26", "rad", r[1, 5]),
        _value("R56", "R56", "R56", "m", r[4, 5]),
        _value("R21", "Horizontal focusing R21", "水平聚焦 R21", "1/m", r[1, 0]),
        _value("R43", "Vertical focusing R43", "垂直聚焦 R43", "1/m", r[3, 2]),
    ]
    labels = ["x", "x′", "y", "y′", "z", "δ"]
    return {
        "values": values,
        "plots": [
            _plot("orbit", ("Off-momentum orbits (bend, then 1 m drift)", "偏离动量粒子的轨道（二极铁后接 1 m 漂移段）"),
                  _axis("s", "s", "m"), _axis("x", "x", "mm"),
                  [_series(f"δ = +{delta:g} %", f"δ = +{delta:g} %", s, xp), _series(f"δ = −{delta:g} %", f"δ = −{delta:g} %", s, xm, dash=True)]),
            _envelope_plot(res),
        ],
        "matrices": [{"title": ("Transfer matrix from the entrance edge to the exit edge", "从入口边缘到出口边缘的传输矩阵"),
                      "labels": labels, "rows": r.tolist()}],
        "lattice": text, "beamline": _beamline(res), "warnings": list(res["warnings"]),
        "model": ("Sector-dipole and pole-face matrices of the linear preview; the envelope has no momentum spread.",
                  "线性预览的扇形二极铁与极面旋转矩阵；包络中不含动量分散。"),
    }


def _cavity_text(phase, Ke, v3=0):
    return (f"start\ndrift 0.05 {APERTURE:g} 0\nfield {CAVITY_LENGTH:g} {APERTURE:g} {v3} 1 {EXAMPLE_FREQ / 1e6:g}e6 {phase:g} "
            f"{Ke:g} {-Ke:g} {CAVITY_MAP}\ndrift 0.2 {APERTURE:g} 0\nend\n")


def rf_cavity(phase, Ke, W):
    folder = paths.example_input_dir()
    if folder is None:
        raise LabError("The example field maps (examples/hwr010) are not installed.")
    beam = lo.read_beam(folder)
    beam["energy"] = W
    dirs = [folder]
    res = _preview(_cavity_text(phase, Ke), beam, dirs)
    cav = next((e for e in res["elements"] if e["key"] == "field"), None)
    entry = cav["phase_rf"] if cav else None
    w_in = float(res["energy"][0])
    gain = float(res["energy"][-1]) - w_in
    phases = np.arange(-180.0, 181.0, 7.5)
    gains = np.array([float(_preview(_cavity_text(p, Ke, v3=1), beam, dirs)["energy"][-1]) - w_in for p in phases])
    top = int(np.argmax(gains))
    from avas.data import fieldmap
    comps = fieldmap.components(dirs, CAVITY_MAP)
    plots = [
        _plot("scan", ("Energy gain versus RF phase at the entrance (V3 = 1)", "能量增益随入口射频相位的变化（V3 = 1）"),
              _axis("entry RF phase φ0", "入口射频相位 φ0", "°"), _axis("ΔW", "ΔW", "MeV"),
              [_series("ΔW(φ0)", "ΔW(φ0)", phases, gains),
               _series("this setting", "当前设置", [entry if entry is not None else 0.0], [gain], marker=True)]),
    ]
    if "edz" in comps:
        maps = [fieldmap.FieldMap(p) for p in comps.values()]
        order = fieldmap.group_order(maps)
        z, ez, _peak = fieldmap.FieldMap(comps["edz"]).profiles(order)
        plots.append(_plot("ez", ("On-axis field Ke·Ez of the cavity", "腔的轴上电场 Ke·Ez"), _axis("z in the cavity", "腔内 z", "m"),
                           _axis("Ez", "Ez", "MV/m"), [_series("Ke·Ez(0, 0, z)", "Ke·Ez(0, 0, z)", z, ez * Ke)]))
    plots.append(_plot("energy", ("Kinetic energy along z", "动能沿 z 的变化"), _axis("z", "z", "m"), _axis("W", "W", "MeV"),
                       [_series("W", "W", res["z"], res["energy"])]))
    plots.append(_envelope_plot(res, extra=(_series("rms z", "rms z", res["z"], res["rms_z"], dash=True),)))
    values = [
        _value("win", "Entrance energy", "入口能量", "MeV", w_in),
        _value("gain", "Energy gain ΔW", "能量增益 ΔW", "MeV", gain),
        _value("phis", "Synchronous phase φs", "同步相位 φs", "°", cav["phase_s"] if cav else None),
        _value("entry", "RF phase at the entrance φ0", "入口射频相位 φ0", "°", entry),
        _value("crest", "Largest gain (on crest, from the scan)", "最大增益（波峰，扫描所得）", "MeV", gains[top]),
        _value("ratio", "ΔW / ΔWmax", "ΔW / ΔWmax", "", gain / gains[top] if gains[top] > 0 else None),
        _value("cos", "cos φs", "cos φs", "", math.cos(math.radians(cav["phase_s"])) if cav and cav["phase_s"] is not None else None),
    ]
    return {"values": values, "plots": plots, "lattice": _cavity_text(phase, Ke), "beamline": _beamline(res, dirs), "warnings": list(res["warnings"]),
            "model": ("Linear preview of the example HWR map (162.5 MHz, protons, Kb = −Ke as in the example); rms values are approximate.",
                      "示例 HWR 场图的线性预览（162.5 MHz，质子，与示例一样取 Kb = −Ke）；rms 数值为近似。")}


def space_charge(I, W, mass, q, emit, Lq, G, Ld, cells):
    gamma, bg, beta, _p, brho = _kin(W, mass, q)
    _k, _segs, _m, px, py = _fodo_optics(Lq, G, Ld, gamma, brho)
    if not (px and py):
        raise LabError("The FODO cell is unstable at zero current; lower the gradient or the drift length.")
    twiss = {"x": (px[0], px[1], emit), "y": (py[0], py[1], emit)}
    text = _fodo_lattice(Lq, G, Ld, cells)
    with_sc = _preview(text, _beam(W, mass, q, twiss, current=I), space_charge=True)
    without = _preview(text, _beam(W, mass, q, twiss, current=I), space_charge=False)

    def perveance(current):
        # K = q I / (2π ε0 m c³ (βγ)³), with m c² = mass [MeV]
        return abs(q) * current * 1e-3 / (2 * math.pi * lo.EPS0 * mass * 1e6 * lo.C_LIGHT * bg ** 3)

    cell = 2 * (Lq + Ld)
    sigma0 = px[2]
    eps_edge = 4 * emit / bg * 1e-6                      # m·rad, KV edge emittance = 4 × rms

    def depressed(current):
        u = perveance(current) / (2 * (sigma0 / cell) * eps_edge)
        return math.sqrt(1 + u * u) - u

    currents = np.linspace(0.0, max(4 * I, 10.0), 120)
    ratio = depressed(I)
    values = [
        _value("K", "Generalized perveance K", "广义导流系数 K", "", perveance(I)),
        _value("sigma0", "Zero-current phase advance σ0", "零流强相移 σ0", "°/cell", math.degrees(sigma0)),
        _value("sigma", "Depressed phase advance σ (smooth approximation)", "降低后的相移 σ（平滑近似）", "°/cell", math.degrees(sigma0) * ratio),
        _value("ratio", "Tune depression σ/σ0", "相移降低 σ/σ0", "", ratio),
        _value("max_sc", "Largest rms size with space charge", "含空间电荷的最大 rms 尺寸", "mm", float(np.max(with_sc["rms_x"]))),
        _value("max_0", "Largest rms size without", "不含空间电荷的最大 rms 尺寸", "mm", float(np.max(without["rms_x"]))),
    ]
    return {
        "values": values,
        "plots": [
            _plot("envelope", ("rms x with and without space charge", "有无空间电荷时的 rms x"), _axis("z", "z", "m"),
                  _axis("rms x", "rms x", "mm"), [
                      _series("with space charge", "含空间电荷", with_sc["z"], with_sc["rms_x"]),
                      _series("zero current", "零流强", without["z"], without["rms_x"], dash=True)]),
            _plot("depression", ("Tune depression versus current", "相移降低随流强的变化"), _axis("I", "I", "mA"),
                  _axis("σ/σ0", "σ/σ0"),
                  [_series("σ/σ0", "σ/σ0", currents, [depressed(c) for c in currents]),
                   _series("I", "I", [I], [ratio], marker=True)]),
        ],
        "lattice": text, "beamline": _beamline(with_sc), "warnings": list(with_sc["warnings"]),
        "model": ("Continuous (DC) beam; envelope from the linear preview's uniform-beam kick, σ from the smooth approximation with KV edge emittance 4ε.",
                  "连续（DC）束；包络来自线性预览的均匀束空间电荷冲量，σ 用平滑近似，KV 边界发射度取 4ε。"),
    }


# the four distributions AVAS generates from Twiss parameters (``distribution`` in beam.txt), defined here
# in the usual way on the 4D transverse phase space; the engine's own generator is not documented, so the lab
# draws the textbook definitions (compare OutputFile/inData.dst for what a run actually used)
DISTRIBUTIONS = [
    ("kv", ("KV", "KV"), ("uniform on the surface of a 4D hyper-ellipsoid", "四维超椭球面上均匀")),
    ("wb", ("Waterbag (WB)", "水袋（WB）"), ("uniform inside the 4D hyper-ellipsoid", "四维超椭球内均匀")),
    ("pb", ("Parabolic (PB)", "抛物线（PB）"), ("density ∝ 1 − r² inside it", "超椭球内密度 ∝ 1 − r²")),
    ("gs", ("Gaussian (GS)", "高斯（GS）"), ("Gaussian, not truncated here", "高斯，此处未截断")),
]
_EDGE_OVER_RMS = {"kv": 2.0, "wb": math.sqrt(6.0), "pb": math.sqrt(8.0)}
# a 4D density ∝ (1 − r²)^k projects onto one axis as (1 − t²)^(k + 3/2); KV is the limit k = −1
_PROJECTION_POWER = {"kv": 0.5, "wb": 1.5, "pb": 2.5}


def _projection(kind, u):
    """Density of the x projection in units of rms x (exact, from the 4D definition)."""
    if kind == "gs":
        return np.exp(-0.5 * u * u) / math.sqrt(2 * math.pi)
    a, p = _EDGE_OVER_RMS[kind], _PROJECTION_POWER[kind]
    norm = a * math.sqrt(math.pi) * math.gamma(p + 1) / math.gamma(p + 1.5)
    return np.clip(1 - (u / a) ** 2, 0.0, None) ** p / norm


def _sample4d(kind, n, rng):
    """*n* points in normalized 4D phase space with an rms of 1 in every coordinate."""
    if kind == "gs":
        return rng.standard_normal((n, 4))
    u = rng.standard_normal((n, 4))
    u /= np.linalg.norm(u, axis=1)[:, None]
    if kind == "kv":
        r = np.ones(n)
    elif kind == "wb":
        r = rng.random(n) ** 0.25                       # radial density ∝ r³ in 4D
    else:                                                # PB: ∝ r³ (1 − r²), by rejection
        r = np.empty(0)
        while r.size < n:
            c = rng.random(2 * n) ** 0.25
            r = np.concatenate([r, c[rng.random(c.size) < 1 - c * c]])
        r = r[:n]
    return u * r[:, None] * _EDGE_OVER_RMS[kind]


def distribution(alpha, beta, emit, W, mass, N):
    if not beta > 0:
        raise LabError("β must be positive.")
    _gamma, bg, _b, _p, _brho = _kin(W, mass, 1)
    eg = emit / bg                                       # geometric rms emittance, mm·mrad
    rng = np.random.default_rng(1)                       # the same cloud while a slider moves
    shown = min(N, 2500)
    sigma = eg * np.array([[beta, -alpha], [-alpha, (1 + alpha * alpha) / beta]])
    ex, exp_ = _ellipse(sigma)
    grid = np.linspace(-4.0, 4.0, 401)
    plots, values, density = [], [], []
    for key, title, doc in DISTRIBUTIONS:
        u = _sample4d(key, N, rng)
        x = math.sqrt(beta * eg) * u[:, 0]
        xp = math.sqrt(eg / beta) * (u[:, 1] - alpha * u[:, 0])
        rms = float(np.std(x))
        cov = np.cov(np.vstack([x, xp]))
        plots.append(_plot(key, (f"{title[0]}: {doc[0]}", f"{title[1]}：{doc[1]}"), _axis("x", "x", "mm"), _axis("x′", "x′", "mrad"), [
            _series("particles", "粒子", x[:shown], xp[:shown], cloud=True),
            _series("rms ellipse", "rms 椭圆", ex, exp_, dash=True),
            _series("4 × rms ellipse", "4 倍 rms 椭圆", 2 * ex, 2 * exp_, dash=True),
        ]))
        density.append(_series(title[0], title[1], grid, _projection(key, grid)))
        values += [
            _value(f"{key}_edge", f"{title[0]}: largest |x| / rms x", f"{title[1]}：最大 |x| / rms x", "", float(np.max(np.abs(x))) / rms),
            _value(f"{key}_tail", f"{title[0]}: share beyond 2 rms", f"{title[1]}：超出 2 倍 rms 的比例", "%", 100 * float(np.mean(np.abs(x) > 2 * rms))),
        ]
        if key == "kv":
            values.insert(0, _value("emit_g", "Geometric rms emittance ε/(βγ), every distribution", "几何 rms 发射度 ε/(βγ)，各分布相同", "mm·mrad",
                                    math.sqrt(max(float(np.linalg.det(cov)), 0.0))))
            values.insert(0, _value("rms_x", "rms x (the same for every distribution)", "rms x（各分布相同）", "mm", rms))
    plots.insert(0, _plot("projection", ("Projection on x: same rms, different shapes (exact, from the definitions)",
                                         "在 x 上的投影：rms 相同，形状不同（由定义严格算出）"),
                          _axis("x / rms x", "x / rms x"), _axis("density", "密度"), density))
    return {"values": values, "plots": plots,
            "model": ("Textbook definitions on 4D transverse phase space, sampled here; how the AVAS engine generates them "
                      "(dimensions, Gaussian cut) is not documented, so check OutputFile/inData.dst of a run.",
                      "按教科书在四维横向相空间中的定义在此抽样；AVAS 内核生成分布的方式（维数、高斯截断）手册未说明，"
                      "以运行输出目录中的 inData.dst 为准。")}


LABS = {lab.id: lab for lab in [
    Lab("kinematics", ("Relativistic kinematics", "相对论运动学"),
        [_mass(), _charge(), _energy(), Param("f", ("RF frequency f", "射频频率 f"), "MHz", 162.5, 1.0, 12000.0, 0.1, log=True)],
        kinematics, ("γ, β, momentum, rigidity and βλ of a particle.", "粒子的 γ、β、动量、刚度与 βλ。")),
    Lab("phase-space", ("Phase-space ellipse", "相空间椭圆"),
        [Param("alpha", ("Twiss α", "Twiss α"), "", 0.0, -5.0, 5.0, 0.01),
         Param("beta", ("Twiss β", "Twiss β"), "mm/mrad", 0.4, 0.01, 10.0, 0.01, log=True),
         _emit(), _energy(), _mass(),
         Param("f", ("Thin-lens focal length f (0 = no lens)", "薄透镜焦距 f（0 表示无透镜）"), "m", 0.5, -5.0, 5.0, 0.01),
         Param("L", ("Drift after the lens", "透镜之后的漂移段"), "m", 0.5, 0.0, 5.0, 0.01)],
        phase_space, ("An rms ellipse through a thin lens and a drift.", "rms 椭圆经过薄透镜和漂移段。")),
    Lab("distribution", ("Particle distributions", "粒子分布"),
        [Param("alpha", ("Twiss α", "Twiss α"), "", -0.658, -5.0, 5.0, 0.01),
         Param("beta", ("Twiss β", "Twiss β"), "mm/mrad", 0.377, 0.01, 10.0, 0.01, log=True),
         _emit(0.138), _energy(), _mass(),
         Param("N", ("Particles", "粒子数"), "", 4000, 200, 20000, 100, integer=True,
               doc=("at most 2500 are drawn; the statistics use all", "最多画 2500 个，统计用全部粒子"))],
        distribution, ("The same Twiss parameters as KV, waterbag, parabolic and Gaussian beams.",
                       "同一组 Twiss 参数下的 KV、水袋、抛物线与高斯分布。")),
    Lab("fodo", ("FODO channel", "FODO 聚焦通道"),
        [*_fodo_params(), _energy(), _mass(), _charge(), _emit(),
         Param("I", ("Beam current (DC)", "束流流强（DC）"), "mA", 0.0, 0.0, 100.0, 0.1),
         Param("mismatch", ("Mismatch: initial size factor", "失配：初始尺寸倍数"), "", 1.0, 0.5, 2.0, 0.01)],
        fodo, ("Phase advance, stability, periodic β and the envelope of a FODO channel.", "FODO 通道的相移、稳定性、周期 β 与包络。")),
    Lab("solenoid", ("Solenoid focusing", "螺线管聚焦"),
        [Param("L", ("Solenoid length", "螺线管长度"), "m", 0.2, 0.02, 1.0, 0.005),
         Param("B", ("Field B", "磁场 B"), "T", 1.0, -10.0, 10.0, 0.01),
         _energy(), _mass(), _charge(), _emit(),
         Param("d1", ("Drift before", "前漂移段"), "m", 0.3, 0.0, 2.0, 0.01),
         Param("d2", ("Drift after", "后漂移段"), "m", 0.8, 0.0, 3.0, 0.01)],
        solenoid, ("Larmor rotation, coupling and focusing of a hard-edge solenoid.", "硬边螺线管的拉莫尔旋转、耦合与聚焦。")),
    Lab("dipole", ("Bending magnet", "二极铁"),
        [Param("angle", ("Bend angle α", "偏转角 α"), "°", 45.0, -180.0, 180.0, 0.5),
         Param("rho", ("Radius ρ", "曲率半径 ρ"), "m", 1.0, 0.1, 20.0, 0.01, log=True),
         Param("N", ("Field index N", "场梯度指数 N"), "", 0.0, -2.0, 2.0, 0.01),
         Param("edge", ("Pole-face angle β (both ends)", "极面旋转角 β（两端）"), "°", 0.0, -45.0, 45.0, 0.5),
         _energy(), _mass(), _charge(),
         Param("delta", ("Momentum offset δ", "动量偏差 δ"), "%", 1.0, 0.0, 10.0, 0.1)],
        dipole, ("Dispersion, weak focusing and edge focusing of a sector dipole.", "扇形二极铁的色散、弱聚焦与边缘聚焦。")),
    Lab("rf-cavity", ("RF cavity: phase and energy gain", "射频腔：相位与能量增益"),
        [Param("phase", ("Synchronous phase φs (V3 = 0)", "同步相位 φs（V3 = 0）"), "°", -30.0, -90.0, 90.0, 1.0),
         Param("Ke", ("Field factor Ke", "场系数 Ke"), "", 1.36, 0.0, 3.0, 0.01),
         _energy()],
        rf_cavity, ("Energy gain of the example HWR cavity against the RF phase.", "示例 HWR 腔的能量增益随射频相位的变化。")),
    Lab("space-charge", ("Space charge in a FODO channel", "FODO 通道中的空间电荷"),
        [Param("I", ("Beam current (DC)", "束流流强（DC）"), "mA", 10.0, 0.0, 200.0, 0.1),
         _energy(), _mass(), _charge(), _emit(), *_fodo_params()],
        space_charge, ("Envelope growth and tune depression caused by space charge.", "空间电荷引起的包络增大与相移降低。")),
]}


def definitions():
    return [lab.public() for lab in LABS.values()]


def _with_ends(text):
    """*text* as a complete lattice: ``start`` / ``end`` are added when the snippet has none."""
    words = [ln.split()[0].lower() for ln in text.splitlines() if ln.split() and not ln.lstrip().startswith("!")]
    body = text.strip("\n") + "\n"
    if "start" not in words:
        body = "start\n" + body
    if "end" not in words:
        body += "end\n"
    return body


def beamline(text):
    """A chapter's beam-line snippet (``avas-beamline`` block) with the linear preview of the example beam.

    The example project's beam (protons, 1.527 MeV) goes through it and its field
    maps (``hwr010``, ``sol``) are on the search path; a snippet the preview cannot
    evaluate still gets its lattice back, with the reason as a warning."""
    lattice = _with_ends(text or "")
    folder = paths.example_input_dir()
    dirs = [folder] if folder else []
    if folder:
        beam = lo.read_beam(folder)
    else:
        beam = _beam(1.527, PROTON, 1, {"x": (0.0, 0.4, 0.14), "y": (0.0, 0.4, 0.14)})
    try:
        with np.errstate(all="ignore"):
            res = _preview(lattice, beam, dirs)
    except LabError as exc:
        return {"lattice": lattice, "fieldDirs": dirs, "beamline": None, "warnings": [(str(exc), str(exc))]}
    return {"lattice": lattice, "fieldDirs": dirs, "beamline": _beamline(res, dirs), "warnings": list(res["warnings"])}


def run(lab_id, params=None):
    """Evaluate lab *lab_id* with *params* (missing ones take their defaults, values are clipped to the ranges)."""
    lab = LABS.get(lab_id)
    if lab is None:
        raise LabError(f"Unknown lab: {lab_id}")
    params = params or {}
    args = {}
    for p in lab.params:
        raw = params.get(p.key, p.default)
        try:
            v = float(raw)
        except (TypeError, ValueError):
            raise LabError(f"{p.label[0]}: not a number") from None
        if not math.isfinite(v):
            raise LabError(f"{p.label[0]}: not a number")
        v = min(max(v, p.min), p.max)
        args[p.key] = int(round(v)) if p.integer else v
    with np.errstate(all="ignore"):
        out = lab.fn(**args)
    out.setdefault("matrices", [])
    out.setdefault("warnings", [])
    out.setdefault("lattice", None)
    out.setdefault("beamline", None)
    out["params"] = args
    return out
