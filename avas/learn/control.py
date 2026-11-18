"""Labs of the primer's "accelerator control" part (``avas-lab`` blocks).

``bpm``            how four button electrodes measure the beam position (Δ/Σ and its nonlinearity)
``feedback``       pulse-to-pulse orbit feedback on the orbit-correction channel: gain, delay, stability
``llrf``           low-level RF: holding a cavity's field against beam loading and microphonics
``controllability`` which orbits a set of steerers can correct and a set of BPMs can see
``henon``          nonlinear betatron motion (a sextupole kick per turn): regular islands, chaos, dynamic aperture
``chaos-control``  OGY control of the logistic map: stabilising an unstable fixed point with tiny nudges

The beam-dynamics labs reuse the linear model (avas.model) and the control layer (avas.dynamics); the
LLRF, Hénon and logistic labs are textbook models, and say so.  Registered in ``labs.LABS``.
"""
import math

import numpy as np

from avas.dynamics.control import integral_loop_modulus
from avas.dynamics.feedback import Drift, simulate
from avas.learn.labkit import Lab, LabError, Param, _axis, _plot, _series, _value


# =========================================================================== BPM
def _buttons(x, y, a, n=4):
    """Image-charge share of each of *n* buttons (at 0°, 90°, … : right, top, left, bottom) of a round pipe
    of radius *a* for a line charge at (x, y): (a² − r²) / (a² + r² − 2 a r cos(θ − φ)) / (2π)."""
    r = math.hypot(x, y)
    phi = math.atan2(y, x)
    th = np.arange(n) * 2 * math.pi / n
    return (a * a - r * r) / (a * a + r * r - 2 * a * r * np.cos(th - phi)) / (2 * math.pi)


def bpm(x, y, a):
    if not a > 0:
        raise LabError("The pipe radius must be positive.")
    if math.hypot(x, y) >= 0.95 * a:
        raise LabError("The beam must stay inside the pipe.")
    s = _buttons(x, y, a)
    R, T, Lf, B = s
    dx = (R - Lf) / (R + Lf)
    dy = (T - B) / (T + B)
    k = a / 2                                       # first-order sensitivity: Δ/Σ ≈ 2x/a
    xs = np.linspace(-0.8 * a, 0.8 * a, 161)
    est = []
    for u in xs:
        b = _buttons(u, 0.0, a)
        est.append(k * (b[0] - b[2]) / (b[0] + b[2]))
    values = [
        _value("right", "Right button signal (share of the beam's image charge)", "右侧电极信号（镜像电荷份额）", "", R),
        _value("left", "Left button signal", "左侧电极信号", "", Lf),
        _value("dsx", "Δ/Σ = (R − L)/(R + L)", "Δ/Σ = (R − L)/(R + L)", "", dx),
        _value("xest", "Position read as (a/2)·Δ/Σ", "按 (a/2)·Δ/Σ 读出的位置", "mm", k * dx),
        _value("err", "Reading minus true x", "读数减真实 x", "mm", k * dx - x),
        _value("yest", "Vertical reading (a/2)·(T − B)/(T + B)", "竖直读数 (a/2)·(T − B)/(T + B)", "mm", k * dy),
    ]
    plots = [
        _plot("buttons", ("Signals of the four buttons", "四个电极的信号"), _axis("button", "电极"), _axis("signal", "信号"),
              [_series("right · top · left · bottom", "右 · 上 · 左 · 下", [1, 2, 3, 4], s, marker=True)]),
        _plot("linearity", ("Reading against the true position (y = 0)", "读数与真实位置（y = 0）"), _axis("true x", "真实 x", "mm"),
              _axis("reading", "读数", "mm"), [_series("(a/2)·Δ/Σ", "(a/2)·Δ/Σ", xs, est),
                                               _series("ideal: reading = x", "理想：读数 = x", xs, xs, dash=True),
                                               _series("this beam", "当前束流", [x], [k * dx], marker=True)], equal=True),
    ]
    return {"values": values, "plots": plots,
            "model": ("A line charge in a round pipe with point-like buttons (the image-charge formula); real BPMs are "
                      "calibrated, and their electronics add noise of a few µm.",
                      "圆管中的线电荷与点状电极（镜像电荷公式）；真实 BPM 要经过标定，电子学还会带来几微米的噪声。")}


# =========================================================================== feedback
FEEDBACK_PULSES = 150


def _gain_limit(delay):
    """The gain at which the integral loop with *delay* pulses becomes unstable (largest |z| = 1), by bisection."""
    lo, hi = 1e-6, 3.0
    for _ in range(50):
        mid = 0.5 * (lo + hi)
        if integral_loop_modulus(mid, delay) < 1:
            lo = mid
        else:
            hi = mid
    return lo


def feedback(gain, delay, drift, noise, I):
    from avas.dynamics.diagnostics import Diagnostics, Monitor
    from avas.learn import motion
    beam, dirs = motion.example_beam()
    beam = motion._beam_with(beam, current=I)
    _text, model, monitors, knobs, _matrix, quads = motion._orbit_setup(beam, dirs)
    mons = Diagnostics([Monitor(m.name, m.z, resolution=noise * 1e-6) for m in monitors.monitors])
    steps = {q: {"dx": drift * 1e-6, "dy": drift * 1e-6} for q in quads}
    with np.errstate(all="ignore"):
        out = simulate(model, knobs, mons, pulses=FEEDBACK_PULSES, gain=gain, delay=int(delay), cut=1e-3,
                       disturbance=Drift(errors=steps, seed=3), noise=noise > 0, seed=5, space_charge=I > 0)
    k = np.arange(FEEDBACK_PULSES)
    tail = slice(FEEDBACK_PULSES // 2, None)
    modulus = integral_loop_modulus(gain, int(delay))
    gains = np.linspace(0.01, 2.5, 120)
    curve = [integral_loop_modulus(g, int(delay)) for g in gains]
    kn = out["knobs"] * 1e3
    j = 0
    values = [
        _value("open", "rms orbit at the BPMs without feedback (second half)", "无反馈时 BPM 处 rms 轨道（后半段）", "mm",
               float(np.sqrt(np.mean(out["open_rms"][tail] ** 2))) * 1e3),
        _value("closed", "rms orbit at the BPMs with feedback (second half)", "有反馈时 BPM 处 rms 轨道（后半段）", "mm",
               float(np.sqrt(np.mean(out["rms"][tail] ** 2))) * 1e3),
        _value("modulus", "Largest |z| of the loop (below 1: stable)", "反馈环路最大 |z|（小于 1 才稳定）", "", modulus),
        _value("limit", "Largest stable gain for this delay", "该延迟下最大稳定增益", "", _gain_limit(int(delay))),
    ]
    plots = [
        _plot("rms", ("rms orbit at the BPMs, pulse by pulse", "BPM 处 rms 轨道，逐个脉冲"), _axis("pulse", "脉冲"),
              _axis("rms orbit", "rms 轨道", "mm", log=True),
              [_series("without feedback", "无反馈", k, out["open_rms"] * 1e3, dash=True),
               _series("with feedback", "有反馈", k, out["rms"] * 1e3)]),
        _plot("knob", ("Field of the first steerer", "第一个校正铁的磁场"), _axis("pulse", "脉冲"),
              _axis(out["knob_labels"][j], out["knob_labels"][j], "mT"), [_series("steerer", "校正铁", k, kn[:, j])]),
        _plot("stability", ("Stability of the loop: largest |z| against the gain", "环路稳定性：最大 |z| 随增益的变化"),
              _axis("gain g", "增益 g"), _axis("largest |z|", "最大 |z|"),
              [_series(f"delay {int(delay)} pulse(s)", f"延迟 {int(delay)} 个脉冲", gains, curve),
               _series("|z| = 1: stability limit", "|z| = 1：稳定边界", gains, np.ones_like(gains), dash=True),
               _series("this gain", "当前增益", [gain], [modulus], marker=True)]),
    ]
    warnings = []
    if modulus >= 1:
        warnings.append(("The loop is unstable at this gain and delay: every correction overshoots and the orbit grows.",
                         "在这个增益和延迟下环路不稳定：每次校正都过头，轨道越来越大。"))
    return {"values": values, "plots": plots, "warnings": warnings,
            "model": ("Linear model of the orbit-correction channel (6 FODO cells, 12 BPMs, 12 steerers); quadrupole offsets "
                      "random-walk from pulse to pulse; integral feedback u ← u − g·R⁺·y with the delayed readings.",
                      "轨道校正通道的线性模型（6 个 FODO 周期、12 个 BPM、12 个校正铁）；四极铁错位逐个脉冲随机游走；"
                      "积分反馈 u ← u − g·R⁺·y，用延迟后的读数。")}


# =========================================================================== LLRF
def llrf(half, gain, delay, micro, loading):
    """Complex envelope V of the cavity field: dV/dt = −ω½(1 − iΔω/ω½)·V + ω½·(u − Ib), set point 1.
    Proportional-integral feedback on the delayed error; open loop: u fixed at the drive that gives V = 1."""
    if not half > 0:
        raise LabError("The half bandwidth must be positive.")
    w = 2 * math.pi * half
    T = 4e-3
    dt = min(0.05e-6, 0.02 / w)
    n = int(T / dt)
    t = np.arange(n) * dt
    det = 2 * math.pi * micro * np.sin(2 * math.pi * 230.0 * t)        # microphonics: detuning at 230 Hz
    ib = np.where(t >= 1e-3, loading / 100.0, 0.0)                      # beam switched on at 1 ms
    lag = max(int(round(delay * 1e-6 / dt)), 0)
    ki = gain * w / 10                                                  # integral corner a decade below the loop crossover

    def run(closed):
        v = np.empty(n, dtype=complex)
        v[0] = 1.0
        acc = 0.0 + 0.0j
        for i in range(n - 1):
            u = 1.0 + 0j
            if closed:
                e = 1.0 - v[max(i - lag, 0)]
                acc += e * dt
                u += gain * e + ki * acc
            dv = -w * (1 - 1j * det[i] / w) * v[i] + w * (u - ib[i])
            v[i + 1] = v[i] + dv * dt
            if not np.isfinite(v[i + 1]) or abs(v[i + 1]) > 1e6:
                v[i + 1:] = np.nan
                break
        return v

    vo, vc = run(False), run(True)
    stride = max(n // 1500, 1)
    ms = t[::stride] * 1e3

    def amp(v):
        return (np.abs(v[::stride]) - 1) * 100

    def ph(v):
        return np.degrees(np.angle(v[::stride]))

    crossover = gain * half                                            # Hz: where the loop gain falls to 1
    margin = 90.0 - 360.0 * crossover * delay * 1e-6
    after = t >= 1.5e-3
    values = [
        _value("open_a", "Amplitude error without feedback (rms, after the beam is on)", "无反馈时幅度误差（rms，束流开启后）", "%",
               float(np.sqrt(np.nanmean((np.abs(vo[after]) - 1) ** 2))) * 100),
        _value("closed_a", "Amplitude error with feedback", "有反馈时幅度误差", "%",
               float(np.sqrt(np.nanmean((np.abs(vc[after]) - 1) ** 2))) * 100),
        _value("open_p", "Phase error without feedback (rms)", "无反馈时相位误差（rms）", "°", float(np.sqrt(np.nanmean(np.angle(vo[after]) ** 2))) * 180 / math.pi),
        _value("closed_p", "Phase error with feedback", "有反馈时相位误差", "°", float(np.sqrt(np.nanmean(np.angle(vc[after]) ** 2))) * 180 / math.pi),
        _value("cross", "Loop crossover frequency gain·f½", "环路穿越频率 增益·f½", "kHz", crossover / 1e3),
        _value("margin", "Phase margin 90° − 360°·f_c·τ", "相位裕度 90° − 360°·f_c·τ", "°", margin),
    ]
    plots = [
        _plot("amp", ("Field amplitude error (beam on at 1 ms)", "腔场幅度误差（1 ms 时束流开启）"), _axis("time", "时间", "ms"),
              _axis("amplitude error", "幅度误差", "%"),
              [_series("without feedback", "无反馈", ms, amp(vo), dash=True), _series("with feedback", "有反馈", ms, amp(vc))]),
        _plot("phase", ("Field phase error", "腔场相位误差"), _axis("time", "时间", "ms"), _axis("phase error", "相位误差", "°"),
              [_series("without feedback", "无反馈", ms, ph(vo), dash=True), _series("with feedback", "有反馈", ms, ph(vc))]),
    ]
    warnings = []
    if margin <= 0 or not np.all(np.isfinite(vc)):
        warnings.append(("The loop is unstable: the delay is too long for this gain, so the correction arrives too late and "
                         "pushes the field the wrong way.",
                         "环路不稳定：对这个增益来说延迟太长，校正来得太晚，反而把场往错误的方向推。"))
    return {"values": values, "plots": plots, "warnings": warnings,
            "model": ("Textbook baseband model of one cavity (first-order low-pass of half bandwidth f½, detuning from "
                      "microphonics, beam loading as a current step) with proportional-integral feedback; AVAS does not "
                      "simulate the LLRF, it takes the cavity fields as given.",
                      "单个腔的教科书式基带模型（半带宽 f½ 的一阶低通、颤噪引起的失谐、以电流阶跃表示的束流负载），"
                      "加比例积分反馈；AVAS 不模拟 LLRF，腔场按给定值使用。")}


# =========================================================================== controllability
def controllability(bpms, steerers, err, seed):
    from avas.learn import motion
    beam, dirs = motion.example_beam()
    beam = motion._beam_with(beam, current=0.0)
    _text, model, monitors, knobs, matrix, quads = motion._orbit_setup(beam, dirs)
    chans = monitors.channels()
    rows = [i for i, (_m, q) in enumerate(chans) if q == "x"]
    cols = [j for j, k in enumerate(knobs) if k.param == "By"]
    nb, ns = len(rows), len(cols)
    use_b = sorted({int(round(v)) for v in np.linspace(0, nb - 1, int(bpms))})
    use_s = sorted({int(round(v)) for v in np.linspace(0, ns - 1, int(steerers))})
    full = matrix[np.ix_(rows, cols)]
    sub = full[np.ix_(use_b, use_s)]
    rng = np.random.default_rng(int(seed))
    offsets = rng.normal(0.0, err * 1e-3, (len(quads), 2))
    errors = {name: {"dx": float(dx), "dy": float(dy)} for name, (dx, dy) in zip(quads, offsets)}
    with np.errstate(all="ignore"):
        before = model.evaluate(errors=errors, space_charge=False)
    y_all = monitors.read(before)[rows]
    u, s, vt = np.linalg.svd(sub, full_matrices=False)
    keep = s > 1e-6 * s[0]
    du_sub = -(vt[keep].T @ ((u[:, keep].T @ y_all[use_b]) / s[keep]))
    du = np.zeros(len(knobs))
    for j, v in zip(use_s, du_sub):
        du[cols[j]] = v
    lattice = {}
    for kn, v in zip(knobs, du):
        lattice.setdefault(kn.target, {})[kn.param] = float(v)
    with np.errstate(all="ignore"):
        after = model.evaluate(errors=errors, knobs=lattice, space_charge=False)
    y_after = monitors.read(after)[rows]
    zs = np.array([m.z for m in monitors.monitors])

    def rms(v):
        return float(np.sqrt(np.mean(np.asarray(v) ** 2))) * 1e3

    values = [
        _value("rank", "Rank of the response matrix (independent corrections)", "响应矩阵的秩（独立的校正方式数）", "", int(np.sum(keep))),
        _value("used", "rms orbit at the BPMs used, after", "校正后所用 BPM 处 rms 轨道", "mm", rms(y_after[use_b])),
        _value("all", "rms orbit at all 12 BPM places, after", "校正后全部 12 个 BPM 位置的 rms 轨道", "mm", rms(y_after)),
        _value("max", "Largest |x| of the beam centre along the line, after", "校正后束流中心沿线最大 |x|", "mm",
               float(np.nanmax(np.abs(after.centroid[:, 0]))) * 1e3),
        _value("before", "rms orbit at all BPM places, before", "校正前全部 BPM 位置的 rms 轨道", "mm", rms(y_all)),
        _value("cond", "Condition number σmax/σmin", "条件数 σmax/σmin", "", float(s[0] / s[keep][-1])),
    ]
    used_z = zs[use_b]
    plots = [
        _plot("orbit", ("Beam centre x along the line", "束流中心 x 沿线分布"), _axis("z", "z", "m"), _axis("x", "x", "mm"), [
            _series("before correction", "校正前", before.z, before.centroid[:, 0] * 1e3, dash=True),
            _series("after correction", "校正后", after.z, after.centroid[:, 0] * 1e3),
            _series("BPMs used", "使用的 BPM", used_z, y_after[use_b] * 1e3, marker=True),
        ]),
        _plot("sv", ("Singular values of the response matrix used", "所用响应矩阵的奇异值"), _axis("number", "序号"),
              _axis("singular value", "奇异值", "m/T", log=True), [_series("σ", "σ", np.arange(1, len(s) + 1), s, marker=True)]),
    ]
    return {"values": values, "plots": plots,
            "model": ("Linear model of the orbit-correction channel (x plane); the BPMs and steerers used are spread evenly; "
                      "the correction is the least-squares solution with tiny singular values dropped.",
                      "轨道校正通道的线性模型（x 方向）；所用 BPM 与校正铁均匀分布；校正取最小二乘解，去掉极小的奇异值。")}


# =========================================================================== nonlinear motion and chaos
def _henon_orbit(x, p, c, s, turns, limit=10.0):
    """(x, p) after every turn of the Hénon map: a thin sextupole kick p += x², then a rotation by 2πν."""
    xs = np.full(turns, np.nan)
    ps = np.full(turns, np.nan)
    for k in range(turns):
        p = p + x * x
        x, p = c * x + s * p, -s * x + c * p
        if abs(x) > limit or abs(p) > limit:
            return xs, ps, k
        xs[k], ps[k] = x, p
    return xs, ps, turns


def henon(tune, turns, amp):
    c, s = math.cos(2 * math.pi * tune), math.sin(2 * math.pi * tune)
    turns = int(turns)
    starts = np.linspace(0.04, amp, 14)
    kept_x, kept_p, gone_x, gone_p = [], [], [], []
    lost = 0
    for a in starts:
        xs, ps, survived = _henon_orbit(float(a), 0.0, c, s, turns)
        if survived < turns:
            lost += 1
            gone_x.append(xs)
            gone_p.append(ps)
        else:
            kept_x.append(xs)
            kept_p.append(ps)
    series = [_series("orbits that survive (14 starts on the x axis)", "存活的轨道（x 轴上 14 个起点）",
                      np.concatenate(kept_x) if kept_x else [], np.concatenate(kept_p) if kept_p else [], cloud=True)]
    if gone_x:
        series.append(_series("orbits that escape (until they leave)", "逃逸的轨道（直到离开为止）",
                              np.concatenate(gone_x), np.concatenate(gone_p), cloud=True))
    # dynamic aperture: the largest start on the x axis that survives, scanned finely
    da = 0.0
    for a in np.linspace(0.01, 1.2, 120):
        if _henon_orbit(float(a), 0.0, c, s, turns)[2] < turns:
            break
        da = float(a)
    values = [
        _value("da", "Dynamic aperture: largest start that survives (on the x axis)", "动力学孔径：能存活的最大起点（x 轴上）", "", da),
        _value("lost", "Starting points lost of 14", "14 个起点中丢失的个数", "", lost),
        _value("tune", "Tune ν (oscillations per turn)", "工作点 ν（每圈振荡次数）", "", tune),
    ]
    plots = [_plot("portrait", ("Phase space after every turn (normalised x, p)", "每圈之后的相空间（归一化 x、p）"),
                   _axis("x", "x"), _axis("p", "p"), series, equal=True)]
    return {"values": values, "plots": plots,
            "model": ("The Hénon map: a linear turn (rotation by 2πν) plus one thin sextupole kick, in normalised units; "
                      "the standard textbook model of nonlinear betatron motion, not an AVAS lattice.",
                      "Hénon 映射：一圈线性传输（转过 2πν）加一次薄六极铁冲击，归一化单位；"
                      "这是非线性 betatron 运动的标准教科书模型，不是 AVAS 的 lattice。")}


def chaos_control(r, start, dmax, noise, steps):
    if not 3.0 <= r <= 4.0:
        raise LabError("r must lie between 3 and 4.")
    steps = int(steps)
    xs = 1 - 1 / r                                     # fixed point x* = 1 − 1/r
    lam = 2 - r                                        # f′(x*) = r(1 − 2x*) = 2 − r
    g = xs * (1 - xs)                                  # ∂f/∂r at x*
    free = np.empty(steps)
    ctrl = np.empty(steps)
    nudges = np.zeros(steps)
    a = b = 0.3
    caught = None
    rng = np.random.default_rng(11)
    kicks = rng.normal(0.0, noise, steps)            # a little noise every step, the same for both runs
    for n in range(steps):
        free[n], ctrl[n] = a, b
        a = min(max(r * a * (1 - a) + kicks[n], 0.0), 1.0)
        dr = 0.0
        if n >= start:
            want = -lam * (b - xs) / g                 # the nudge that lands the next step on x*
            if abs(want) <= dmax:
                dr = want
                if caught is None:
                    caught = n
        nudges[n] = dr
        b = min(max((r + dr) * b * (1 - b) + kicks[n], 0.0), 1.0)
    n_ = np.arange(steps)
    after = ctrl[caught + 5:] if caught is not None and caught + 5 < steps else np.array([np.nan])
    rr = np.linspace(2.8, 4.0, 400)
    bx, by = [], []
    for ri in rr:
        v = 0.3
        for _ in range(300):
            v = ri * v * (1 - v)
        for _ in range(60):
            v = ri * v * (1 - v)
            bx.append(ri)
            by.append(v)
    values = [
        _value("xstar", "Fixed point x* = 1 − 1/r", "不动点 x* = 1 − 1/r", "", xs),
        _value("slope", "Slope there f′(x*) = 2 − r (|·| > 1: unstable)", "该处斜率 f′(x*) = 2 − r（绝对值大于 1 即不稳定）", "", lam),
        _value("caught", "Step at which the control catches the orbit", "控制捕获轨道的步数", "", caught),
        _value("nudge", "Largest nudge |δr| used", "用到的最大扰动 |δr|", "", float(np.max(np.abs(nudges)))),
        _value("spread", "rms distance from x* after capture", "捕获后到 x* 的 rms 距离", "", float(np.sqrt(np.nanmean((after - xs) ** 2)))),
    ]
    plots = [
        _plot("series", ("x step by step (control switched on at the dashed step)", "x 逐步的变化（虚线处开启控制）"),
              _axis("step n", "步数 n"), _axis("x", "x"), [
                  _series("without control", "不控制", n_, free, dash=True),
                  _series("with control", "控制", n_, ctrl),
                  _series("control on", "开启控制", [start, start], [0, 1], dash=True),
              ]),
        _plot("nudges", ("Nudges δr applied", "施加的扰动 δr"), _axis("step n", "步数 n"), _axis("δr", "δr"),
              [_series("δr", "δr", n_, nudges)]),
        _plot("bifurcation", ("Where the logistic map settles for each r (the dashed line is this r)", "逻辑斯谛映射在各个 r 下最终停留的位置（虚线为当前 r）"),
              _axis("r", "r"), _axis("x", "x"),
              [_series("long-run values", "长期取值", bx, by, cloud=True), _series("this r", "当前 r", [r, r], [0, 1], dash=True)]),
    ]
    return {"values": values, "plots": plots,
            "model": ("The logistic map x → r·x·(1 − x) with OGY control (Ott, Grebogi and Yorke 1990): when the orbit comes "
                      "close to the unstable fixed point, a small change of r puts the next step on it. A textbook model, not "
                      "an accelerator simulation.",
                      "逻辑斯谛映射 x → r·x·(1 − x) 加 OGY 控制（Ott、Grebogi、Yorke，1990）：轨道走近不稳定不动点时，"
                      "稍微改一下 r，让下一步正好落在不动点上。这是教科书模型，不是加速器模拟。")}


def _current(default=0.0):
    return Param("I", ("Beam current", "束流流强"), "mA", default, 0.0, 20.0, 0.01)


LABS = {lab.id: lab for lab in [
    Lab("bpm", ("Beam position monitor", "束流位置监测器（BPM）"),
        [Param("x", ("Beam position x", "束流位置 x"), "mm", 2.0, -15.0, 15.0, 0.1),
         Param("y", ("Beam position y", "束流位置 y"), "mm", 0.0, -15.0, 15.0, 0.1),
         Param("a", ("Pipe radius a", "管道半径 a"), "mm", 20.0, 5.0, 50.0, 0.5)],
        bpm, ("Four buttons see the beam's image charge; the difference over the sum gives its position.",
              "四个电极感应束流的镜像电荷；差除以和给出束流位置。")),
    Lab("feedback", ("Pulse-to-pulse orbit feedback", "逐脉冲轨道反馈"),
        [Param("gain", ("Gain g", "增益 g"), "", 0.5, 0.0, 2.5, 0.01,
               doc=("the share of the measured error corrected each pulse", "每个脉冲校正掉所测误差的比例")),
         Param("delay", ("Delay (pulses)", "延迟（脉冲数）"), "", 1, 0, 3, 1, integer=True),
         Param("drift", ("Quadrupole drift per pulse (rms)", "四极铁每个脉冲的漂移（rms）"), "µm", 2.0, 0.0, 20.0, 0.1),
         Param("noise", ("BPM noise (rms)", "BPM 噪声（rms）"), "µm", 10.0, 0.0, 100.0, 1.0),
         _current()],
        feedback, ("A loop that measures and corrects every pulse: gain, delay and stability.", "每个脉冲测量并校正的环路：增益、延迟与稳定性。")),
    Lab("llrf", ("Low-level RF: holding the cavity field", "低电平射频：稳住腔场"),
        [Param("half", ("Cavity half bandwidth f½", "腔的半带宽 f½"), "Hz", 100.0, 5.0, 5000.0, 1.0, log=True,
               doc=("superconducting cavities: tens to hundreds of Hz", "超导腔：几十到几百 Hz")),
         Param("gain", ("Feedback gain", "反馈增益"), "", 50.0, 0.0, 500.0, 1.0),
         Param("delay", ("Loop delay τ", "环路延迟 τ"), "µs", 2.0, 0.0, 50.0, 0.1),
         Param("micro", ("Microphonics: detuning amplitude", "颤噪：失谐幅度"), "Hz", 20.0, 0.0, 200.0, 1.0),
         Param("loading", ("Beam loading", "束流负载"), "%", 30.0, 0.0, 100.0, 1.0,
               doc=("field the beam would take away without feedback", "无反馈时束流会带走的场"))],
        llrf, ("A feedback loop keeps the cavity's amplitude and phase while the beam and vibrations disturb them.",
               "反馈环路在束流和振动的扰动下保持腔场的幅度与相位。")),
    Lab("controllability", ("What steerers can correct and BPMs can see", "校正铁能校正什么、BPM 能看到什么"),
        [Param("bpms", ("BPMs used", "使用的 BPM 个数"), "", 12, 1, 12, 1, integer=True),
         Param("steerers", ("Steerers used", "使用的校正铁个数"), "", 12, 1, 12, 1, integer=True),
         Param("err", ("Quadrupole offsets (rms)", "四极铁错位（rms）"), "mm", 0.4, 0.0, 1.0, 0.01),
         Param("seed", ("Error set (random seed)", "误差样本（随机种子）"), "", 1, 1, 50, 1, integer=True)],
        controllability, ("Fewer steerers than BPMs leave orbit they cannot remove; fewer BPMs leave orbit nobody sees.",
                          "校正铁比 BPM 少，会留下消不掉的轨道；BPM 少，会留下谁也看不见的轨道。")),
    Lab("henon", ("Nonlinear motion and chaos", "非线性运动与混沌"),
        [Param("tune", ("Tune ν", "工作点 ν"), "", 0.2114, 0.05, 0.45, 0.0001),
         Param("turns", ("Turns", "圈数"), "", 600, 50, 3000, 50, integer=True),
         Param("amp", ("Largest starting amplitude", "最大起始振幅"), "", 0.7, 0.1, 1.2, 0.01)],
        henon, ("A sextupole kick every turn: regular rings, islands, a chaotic sea and the dynamic aperture.",
                "每圈一次六极铁冲击：规则的圈、岛链、混沌海与动力学孔径。")),
    Lab("chaos-control", ("Controlling chaos", "控制混沌"),
        [Param("r", ("Parameter r", "参数 r"), "", 3.7, 3.0, 4.0, 0.001,
               doc=("above about 3.57 the motion is chaotic", "大约 3.57 以上运动是混沌的")),
         Param("start", ("Control switched on at step", "开启控制的步数"), "", 100, 0, 400, 1, integer=True),
         Param("dmax", ("Largest nudge allowed |δr|", "允许的最大扰动 |δr|"), "", 0.1, 0.0, 0.3, 0.001,
               doc=("the control only acts when the orbit comes close enough to need less", "只有轨道走得足够近、所需扰动小于它时才施加控制")),
         Param("noise", ("Noise per step", "每步的噪声"), "", 0.0005, 0.0, 0.01, 0.0001),
         Param("steps", ("Steps", "步数"), "", 400, 100, 1000, 10, integer=True)],
        chaos_control, ("Tiny, well-timed nudges hold a chaotic system on an unstable fixed point.",
                        "微小而时机恰当的扰动，把混沌系统稳定在一个不稳定的不动点上。")),
]}
