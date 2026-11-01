"""Guided tutorial of the primer: build a small superconducting linac step by step.

The beam line is made of the example project's field maps (``examples/hwr010``:
the 162.5 MHz half-wave resonator ``hwr010`` and the solenoid ``sol``) and
drifts only, so it is a valid multi-particle lattice (no matrix element at the
ends).  Every step adds something to the line; the page shows the linear
preview while the reader changes the parameters and can run the real engine on
it in a sandbox outside any project (:mod:`avas.gui.services.learn`).  A step
can be saved as a new project (:func:`export`).
"""
import os
import shutil

from avas import paths
from avas.data.lattice_edit import set_keyword_value
from avas.learn.labs import LabError, Param

SOLENOID_MAP, SOLENOID_LENGTH = "sol", 0.35
SOLENOID_PEAK = 8.3                       # T, smoothed on-axis peak of sol.bsz (single noisy points reach 10 T)
CAVITY_MAP, CAVITY_LENGTH = "hwr010", 0.21
FREQUENCY = 162.5e6
R = 0.02                                  # m, the bore of the example maps
TEXT_INPUTS = ("beam.txt", "input.txt", "ini.ini")
LATTICE_NAME = "lattice_mulp.txt"

_P = {
    "L": Param("L", ("Drift length", "漂移段长度"), "m", 0.8, 0.1, 2.0, 0.01),
    "Kb": Param("Kb", ("Solenoid factor Kb", "螺线管系数 Kb"), "", 0.35, 0.0, 0.8, 0.005,
                doc=("the smoothed on-axis peak of the (noisy) map is about 8.3 T, so Bz,max ≈ 8.3·Kb T",
                     "（有噪声的）场图平滑后轴上峰值约 8.3 T，故 Bz,max ≈ 8.3·Kb T")),
    "gap": Param("gap", ("Drift after the solenoid", "螺线管后漂移段"), "m", 0.6, 0.05, 1.5, 0.01),
    "phase": Param("phase", ("Synchronous phase φs", "同步相位 φs"), "°", -30.0, -90.0, 30.0, 1.0),
    "Ke": Param("Ke", ("Cavity factor Ke", "腔场系数 Ke"), "", 1.36, 0.0, 2.5, 0.01),
    "periods": Param("periods", ("Periods", "周期数"), "", 3, 1, 8, 1, integer=True),
    "I": Param("I", ("Beam current", "束流流强"), "mA", 5.0, 0.0, 30.0, 0.1),
    "particles": Param("particles", ("Macro-particles (engine run)", "宏粒子数（内核运行）"), "", 2000, 200, 20000, 100,
                       integer=True, doc=("fewer particles run faster but are noisier", "粒子越少运行越快，但统计噪声越大")),
}


class Step:
    def __init__(self, id_, title, params):
        self.id, self.title, self.params = id_, title, params

    def public(self):
        return {"id": self.id, "title": list(self.title), "params": [_P[k].public() for k in self.params]}


STEPS = [
    Step("drift", ("1 · A beam in a drift", "1 · 漂移段中的束流"), ["L", "particles"]),
    Step("focus", ("2 · Focusing with a solenoid", "2 · 用螺线管聚焦"), ["Kb", "gap", "particles"]),
    Step("accelerate", ("3 · Accelerating in a cavity", "3 · 在腔中加速"), ["Kb", "phase", "Ke", "particles"]),
    Step("periods", ("4 · A periodic accelerating section", "4 · 周期性加速段"), ["periods", "Kb", "phase", "Ke", "particles"]),
    Step("current", ("5 · Adding beam current", "5 · 加入束流流强"), ["periods", "Kb", "phase", "Ke", "I", "particles"]),
]
STEP_BY_ID = {s.id: s for s in STEPS}


def definitions():
    return [s.public() for s in STEPS]


def example_dir():
    folder = paths.example_input_dir()
    if folder is None:
        raise LabError("The example project (examples/hwr010) is not installed; the tutorial needs its field maps.")
    return folder


def values(step_id, params=None):
    """The step and its parameters, defaults filled in and clipped to the ranges."""
    step = STEP_BY_ID.get(step_id)
    if step is None:
        raise LabError(f"Unknown tutorial step: {step_id}")
    params = params or {}
    out = {}
    for key in step.params:
        p = _P[key]
        try:
            v = float(params.get(key, p.default))
        except (TypeError, ValueError):
            raise LabError(f"{p.label[0]}: not a number") from None
        v = min(max(v, p.min), p.max)
        out[key] = int(round(v)) if p.integer else v
    return step, out


def _solenoid(kb):
    return f"field {SOLENOID_LENGTH:g} {R:g} 0 3 0 0 0 {kb:g} {SOLENOID_MAP}   ! solenoid, Bz peak about {SOLENOID_PEAK * kb:.2f} T"


def _cavity(phase, ke):
    return f"field {CAVITY_LENGTH:g} {R:g} 0 1 {FREQUENCY / 1e6:g}e6 {phase:g} {ke:g} {-ke:g} {CAVITY_MAP}   ! HWR, phis = {phase:g} deg"


def lattice(step_id, params=None):
    """AVAS lattice text of the step (kept ASCII so the engine's parser never meets other text)."""
    step, v = values(step_id, params)
    lines = [f"! AVAS primer tutorial, step '{step.id}'", "start"]
    if step.id == "drift":
        lines.append(f"drift {v['L']:g} {R:g} 0")
    elif step.id == "focus":
        lines += ["drift 0.05 0.02 0", _solenoid(v["Kb"]), f"drift {v['gap']:g} {R:g} 0"]
    else:
        periods = v.get("periods", 1)
        lines.append("drift 0.05 0.02 0")
        for i in range(periods):
            lines += [f"! period {i + 1}", _solenoid(v["Kb"]), "drift 0.1 0.02 0", _cavity(v["phase"], v["Ke"]), "drift 0.1 0.02 0"]
        lines.append("drift 0.2 0.02 0")
    lines.append("end")
    return "\n".join(lines) + "\n"


def beam_text(step_id, params=None):
    """beam.txt of the step: the example beam with the step's current and particle number."""
    _step, v = values(step_id, params)
    with open(os.path.join(example_dir(), "beam.txt"), encoding="utf-8") as fh:
        text = fh.read()
    if "I" in v:
        text = set_keyword_value(text, "current", v["I"])
    return set_keyword_value(text, "particlenumber", v["particles"])


def preview_beam(step_id, params=None):
    """The beam of the step for :func:`avas.sim.linear_optics.linear_preview`."""
    from avas.sim import linear_optics as lo
    _step, v = values(step_id, params)
    beam = lo.read_beam(example_dir())
    if "I" in v:
        beam["current"] = v["I"]
    return beam


def field_maps():
    """The example field-map files the tutorial lattice uses."""
    folder = example_dir()
    from avas.data.fieldmap import components
    files = []
    for base in (SOLENOID_MAP, CAVITY_MAP):
        files += sorted(components([folder], base).values())
    return files


def export(folder, step_id, params=None):
    """Write the step as a new project in *folder* (which must not exist or be empty); returns the folder."""
    folder = os.path.abspath(folder)
    if os.path.exists(folder) and (not os.path.isdir(folder) or os.listdir(folder)):
        raise LabError("Choose a new or empty folder for the project.")
    src = example_dir()
    inp = os.path.join(folder, "InputFile")
    os.makedirs(inp)
    os.makedirs(os.path.join(folder, "OutputFile"))
    for name in TEXT_INPUTS:
        if name != "beam.txt" and os.path.isfile(os.path.join(src, name)):
            shutil.copy2(os.path.join(src, name), inp)
    with open(os.path.join(inp, "beam.txt"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(beam_text(step_id, params))
    with open(os.path.join(inp, LATTICE_NAME), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(lattice(step_id, params))
    for path in field_maps():
        shutil.copy2(path, inp)
    return folder
