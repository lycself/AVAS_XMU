"""Linear-model analyses of the Study page and the Lattice page's period panel (:mod:`avas.dynamics`).

* ``analysis.periods`` / ``analysis.period`` / ``analysis.periodTable``: periods of the editor's
  lattice text (``lattice … lattice_end``), Floquet analysis, matched beam with space charge,
  envelope stability, mismatch of the project's beam;
* ``analysis.chart*``: a stability chart over two axes in a thread (events ``analysis.chart``,
  ``analysis.chartFinished``);
* ``analysis.diagnostics`` / ``saveDiagnostics``: monitors and knobs of the project
  (``<project>/Research/diagnostics.json``, study.json format; default from the lattice);
* ``analysis.response`` / ``analysis.periodModes`` / ``analysis.feedback``: response matrix,
  correction analysis, per-period mode classes, pulse-to-pulse feedback;
* ``analysis.responseEngine*``: the response matrix from engine runs (research study ``gui``,
  through the sandbox: one simulation at a time, shown on the Run page; event
  ``analysis.responseFinished``).

Nothing here writes input files.
"""
import logging
import math
import os
import threading

import numpy as np

from avas.gui import bridge, context
from avas.gui.bridge import UserError, rpc

log = logging.getLogger("avas.gui")

DIAGNOSTICS_FILE = "diagnostics.json"
_chart = {"thread": None, "stop": None, "result": None}
_engine = {"thread": None, "stop": None, "result": None}


def _num(v):
    try:
        f = float(v)
    except (TypeError, ValueError):
        return None
    return f if math.isfinite(f) else None


def _model(text=None, fieldDirs=None):
    """Linear model of *text* (default: the run lattice file) with the project's beam."""
    from avas.gui.services.lattice import _beam_for_preview, _field_dirs
    from avas.model import LinearModel
    p = context.project().require()
    if text is None:
        with open(p.lattice_path(), encoding="utf-8", errors="replace") as fh:
            text = fh.read()
    return LinearModel(text, _beam_for_preview(p), _field_dirs(fieldDirs))


def _plane_dict(fl):
    return {p: {"mu": _num(v["mu_deg"]), "alpha": _num(v["alpha"]), "beta": _num(v["beta"]), "stable": v["stable"]}
            for p, v in fl["planes"].items()}


# =========================================================================== periods
@rpc("analysis.periods")
def periods(text, fieldDirs=None):
    from avas.data.lattice_doc import LatticeDocument
    from avas.dynamics import periods as find
    return find(LatticeDocument(text or ""))


@rpc("analysis.period")
def period(text, period=1, current=None, fieldDirs=None):
    """Analysis of one period of the editor's lattice text with the project's beam.

    Returns ``z0``, ``z1``, ``energy``, ``current`` [mA], ``zero`` and ``depressed`` (per plane: phase
    advance mu [deg], matched alpha / beta [m/rad = mm/mrad], stable), ``stable`` (zero current),
    ``coupling``, ``eigenTunes`` [deg], ``tuneDepression``, ``mismatch`` (of the project's beam at the
    period entry), ``matched`` (alpha, beta, normalised rms emittance [π·mm·mrad] per matched plane),
    ``envelope`` ({maxModulus, stable}) and ``messages`` (why a part is missing).
    """
    from avas.dynamics import Period, envelope_jacobian
    from avas.dynamics.moments import twiss
    from avas.model import PreviewError
    try:
        res = _model(text, fieldDirs).evaluate(beam={"current": float(current)} if current is not None else None)
        cell = Period(res, int(period))
    except (PreviewError, ValueError) as exc:
        raise UserError(str(exc)) from exc
    zc = cell.zero_current()
    out = {"z0": cell.z0, "z1": cell.z1, "energy": float(res.energy[cell.i0]),
           "current": float(res.beam.get("current") or 0.0), "zero": _plane_dict(zc), "stable": zc["stable"],
           "coupling": zc["coupling"], "eigenTunes": [_num(v) for v in zc["eigen_tunes_deg"]],
           "maxModulus": zc["max_modulus"], "messages": []}
    try:
        m = cell.matched()
    except ValueError as exc:
        out["messages"].append(str(exc))
        return out
    bg = float(res.beta_gamma[cell.i0])
    out["planes"] = m["planes"]
    out["converged"] = m["converged"]
    out["depressed"] = _plane_dict(m["depressed"])
    out["tuneDepression"] = {k: _num(v) for k, v in m["tune_depression"].items()}
    out["mismatch"] = {k: _num(v) for k, v in m["mismatch"].items()}
    out["matched"] = {}
    for plane in m["planes"]:
        a, b = twiss(m["sigma"], plane)
        out["matched"][plane] = {"alpha": _num(a), "beta": _num(b), "emit_n": _num(m["emittances"][plane] * bg * 1e6)}
    try:
        jac = envelope_jacobian(cell, matched=m)
        out["envelope"] = {"maxModulus": jac["max_modulus"], "stable": jac["stable"],
                           "modeTunes": [_num(v) for v in jac["mode_tunes_deg"]]}
    except (ValueError, np.linalg.LinAlgError) as exc:
        out["messages"].append(str(exc))
    if not m["converged"]:
        out["messages"].append("The matched beam with space charge did not converge; its values are approximate.")
    return out


@rpc("analysis.periodTable")
def period_table(text, fieldDirs=None):
    """Every period along the line: z, energy, zero-current and depressed phase advances (blobs per column)."""
    from avas.dynamics import period_table as table
    from avas.model import PreviewError
    try:
        rows = table(_model(text, fieldDirs).evaluate())
    except (PreviewError, ValueError) as exc:
        raise UserError(str(exc)) from exc
    keys = ["z0", "z1", "energy"] + [f"{k}_{p}" for k in ("mu0", "mu") for p in "xyz"]
    return {"count": len(rows), **{k: bridge.blob(np.array([_num(r[k]) if _num(r[k]) is not None else np.nan for r in rows]),
                                                  "float64") for k in keys}}


# =========================================================================== stability chart
def _chart_axis(p, spec):
    """``(label, unit, apply(evaluate kwargs, value))`` of a chart axis: a lattice parameter or the current."""
    if spec.get("kind") == "current":
        return "current", "mA", lambda kw, v: kw.setdefault("beam", {}).update(current=float(v))
    from avas.sim import scan as core
    try:
        target = core.ScanTarget({"kind": "lattice", "target": spec.get("target"), "param": spec.get("param")},
                                 core.read_inputs(p.input_dir, p.lattice_name()), p.field_dirs(), p.lattice_name())
    except core.ScanError as exc:
        raise UserError(str(exc)) from exc
    line, param = target.st.line_no + 1, target.param
    return target.label, target.unit, lambda kw, v: kw.setdefault("knobs", {}).setdefault(line, {}).update({param: float(v)})


def chart_running():
    t = _chart["thread"]
    return t is not None and t.is_alive()


@rpc("analysis.chartStart")
def chart_start(period, x, xs, y, ys):
    """Stability chart of *period* over two axes (``{kind: "lattice", target, param}`` or ``{kind: "current"}``)."""
    from avas.sim import scan as core
    p = context.project().require()
    try:
        xs, ys = core.normalise_values(xs), core.normalise_values(ys)
    except core.ScanError as exc:
        raise UserError(str(exc)) from exc
    if len(xs) * len(ys) > 900:
        raise UserError("At most 900 points per chart.")
    ax, ay = _chart_axis(p, x), _chart_axis(p, y)
    model = _model()
    if chart_running():
        raise UserError("A stability chart is already being computed.")
    stop = threading.Event()
    result = {"x": {"label": ax[0], "unit": ax[1], "values": xs}, "y": {"label": ay[0], "unit": ay[1], "values": ys},
              "period": int(period), "done": 0, "total": len(xs) * len(ys), "status": "running"}
    grid = {k: [[None] * len(xs) for _ in ys] for k in ("sigma0", "sigma", "envelope", "stable", "converged")}
    result["grid"] = grid
    _chart.update(stop=stop, result=result)

    def work():
        from avas.dynamics import Period, envelope_jacobian
        from avas.model import PreviewError
        try:
            for j, yv in enumerate(ys):
                for i, xv in enumerate(xs):
                    if stop.is_set():
                        result["status"] = "stopped"
                        return
                    kw = {}
                    ax[2](kw, xv)
                    ay[2](kw, yv)
                    try:
                        cell = Period(model.evaluate(**kw), result["period"])
                        jac = envelope_jacobian(cell)
                        m = jac["matched"]
                        grid["sigma0"][j][i] = _num(m["zero_current"]["planes"]["x"]["mu_deg"])
                        grid["sigma"][j][i] = _num(m["depressed"]["planes"]["x"]["mu_deg"])
                        grid["envelope"][j][i] = _num(jac["max_modulus"])
                        grid["stable"][j][i] = 1.0 if jac["stable"] else 0.0
                        grid["converged"][j][i] = 1.0 if m["converged"] else 0.0
                    except (PreviewError, ValueError, ArithmeticError, np.linalg.LinAlgError):
                        pass
                    result["done"] += 1
                bridge.emit("analysis.chart", _copy(result))
            result["status"] = "finished"
        except Exception as exc:  # noqa: BLE001 - reported to the page
            result["status"] = "failed"
            result["error"] = f"{type(exc).__name__}: {exc}"
            log.exception("stability chart failed")
        finally:
            bridge.emit("analysis.chartFinished", _copy(result))

    thread = threading.Thread(target=work, name="avas-chart", daemon=True)
    _chart["thread"] = thread
    thread.start()
    return _copy(result)


def _copy(result):
    out = dict(result)
    out["grid"] = {k: [list(row) for row in v] for k, v in result["grid"].items()}
    return out


@rpc("analysis.chartState")
def chart_state():
    r = _chart["result"]
    return {"running": chart_running(), "result": _copy(r) if r else None}


@rpc("analysis.chartStop")
def chart_stop():
    if _chart["stop"] is not None:
        _chart["stop"].set()
    return True


# =========================================================================== monitors and knobs
def _diag_path(p):
    return os.path.join(p.path, "Research", DIAGNOSTICS_FILE)


@rpc("analysis.diagnostics")
def diagnostics():
    """Monitors and knobs of the project: the saved ``Research/diagnostics.json``, else the lattice's
    ``diag_position`` / ``diag_size`` lines and steerers.  ``{monitors, knobs, source}``."""
    from dataclasses import asdict

    from avas.dynamics import Diagnostics, load_study, steerers
    p = context.project().require()
    path = _diag_path(p)
    if os.path.isfile(path):
        try:
            study = load_study(path)
            return {"monitors": [asdict(m) for m in study["diagnostics"].monitors],
                    "knobs": [asdict(k) for k in study["knobs"]], "source": "file", "path": path}
        except (OSError, ValueError, TypeError) as exc:
            raise UserError(f"Cannot read {DIAGNOSTICS_FILE}: {exc}") from exc
    model = _model()
    monitors = [dict(asdict(m), z=round(m.z, 9)) for m in Diagnostics.from_lattice(model).monitors]
    return {"monitors": monitors,
            "knobs": [asdict(k) for k in steerers(model)], "source": "lattice", "path": path}


@rpc("analysis.saveDiagnostics")
def save_diagnostics(monitors, knobs):
    from avas.dynamics import save_study
    p = context.project().require()
    diag, ks = _parse_spec(monitors, knobs)
    os.makedirs(os.path.dirname(_diag_path(p)), exist_ok=True)
    save_study(_diag_path(p), diag, ks)
    return {"path": _diag_path(p)}


def _parse_spec(monitors, knobs):
    from avas.dynamics import Diagnostics, Knob, Monitor
    try:
        diag = Diagnostics(Monitor(name=str(m["name"]), z=float(m["z"]), kind=m.get("kind") or "position",
                                   planes=m.get("planes") or "xy", resolution=float(m.get("resolution") or 0.0),
                                   line=m.get("line")) for m in monitors or [])
        ks = [Knob(target=k["target"], param=str(k["param"]), kind=k.get("kind") or "lattice",
                   step=_num(k.get("step")), lower=_num(k.get("lower")), upper=_num(k.get("upper"))) for k in knobs or []]
    except (KeyError, TypeError, ValueError) as exc:
        raise UserError(f"Invalid monitor or knob: {exc}") from exc
    if not diag.monitors:
        raise UserError("Add at least one monitor.")
    if not ks:
        raise UserError("Add at least one knob.")
    return diag, ks


def _response_json(resp, diag, noise=True):
    from avas.dynamics import static
    st = static(resp.matrix, noise=diag.resolution() if noise else None)
    return {"matrix": resp.matrix.tolist(), "readings": resp.labels, "knobs": [k.label for k in resp.knobs],
            "y0": resp.y0.tolist(), "singular": st["singular_values"].tolist(), "rank": st["rank"],
            "condition": _num(st["condition"]), "knobNoise": st["knob_noise"].tolist(),
            "uncorrectable": int(st["uncorrectable"].shape[1]), "invisible": int(st["invisible"].shape[1])}


@rpc("analysis.response")
def response(monitors, knobs):
    """Linear-model response matrix of the monitors' readings [m] to the knobs, with the correction analysis."""
    from avas.dynamics import response as compute
    from avas.model import ModelError, PreviewError
    diag, ks = _parse_spec(monitors, knobs)
    try:
        resp = compute(_model(), ks, diag)
    except (ModelError, PreviewError, ValueError) as exc:
        raise UserError(str(exc)) from exc
    return _response_json(resp, diag)


@rpc("analysis.periodModes")
def period_modes(monitors, knobs, period=1):
    """Centroid system of one period: modes (eigenvalue, class, controllable, observable) and ranks."""
    from avas.dynamics import Diagnostics, Period, controllability, modes, observability, orbit_system
    from avas.model import ModelError, PreviewError
    diag, ks = _parse_spec(monitors, knobs)
    model = _model()
    try:
        cell = Period(model.evaluate(), int(period))
        inside = [k for k in ks if _knob_z(model, k) is not None and cell.z0 - 1e-9 <= _knob_z(model, k) <= cell.z1 + 1e-9]
        if not inside:
            raise UserError("No knob of this period: put a steerer (or choose a parameter) inside it.")
        sysd = orbit_system(model, int(period), inside, Diagnostics(diag.monitors))
    except (ModelError, PreviewError, ValueError) as exc:
        raise UserError(str(exc)) from exc
    noise = [m.resolution or 1e-6 for m in diag.monitors if m.kind == "position" for _p in m.planes
             if cell.z0 - 1e-9 <= m.z <= cell.z1 + 1e-9]
    ctrl = controllability(sysd["A"], sysd["B"])
    obs = observability(sysd["A"], sysd["C"], noise=noise or None) if len(sysd["C"]) else None
    return {"knobs": sysd["knob_labels"], "readings": sysd["reading_labels"], "states": sysd["state_labels"],
            "controllableRank": ctrl["rank"], "observableRank": obs["rank"] if obs else 0, "n": ctrl["n"],
            "stateError": [_num(v) for v in obs["state_error"]] if obs else [],
            "modes": [{"re": m["eigenvalue"].real, "im": m["eigenvalue"].imag, "modulus": m["modulus"],
                       "tune": m["tune_deg"], "class": m["class"], "controllable": m["controllable"],
                       "observable": m["observable"]} for m in modes(sysd["A"], sysd["B"], sysd["C"] if len(sysd["C"]) else None)]}


def _knob_z(model, knob):
    """Where a knob acts: a steerer's next element, else the element itself (None for beam knobs)."""
    from avas.data import lattice_edit, schema
    from avas.model.model import _address
    if knob.kind == "beam":
        return None
    try:
        st = lattice_edit.resolve(model.doc, _address(knob.target))
    except lattice_edit.LatticeEditError:
        return None
    if st.key == "steerer":
        sts = [s for s in model.doc.statements if s.active]
        k = sts.index(st)
        nxt = next((s for s in sts[k + 1:] if s.is_element and s.key not in ("steerer", "edge")
                    and s.category != schema.DIAG and s.length > 0), None)
        return None if nxt is None else (nxt.z_start + nxt.z_end) / 2
    return None if st.z_start is None else (st.z_start + st.z_end) / 2


@rpc("analysis.feedback")
def feedback(monitors, knobs, gain=0.5, delay=0, pulses=150, drift=None, noise=True, seed=0):
    """Pulse-to-pulse feedback against a random walk ``drift = {target, key, step}`` (step [m] or [rad])."""
    from avas.dynamics import Drift, simulate
    from avas.model import ModelError, PreviewError
    diag, ks = _parse_spec(monitors, knobs)
    pulses = max(2, min(int(pulses), 2000))
    disturbance = None
    if drift and drift.get("target") not in (None, ""):
        target = drift["target"]
        target = int(target) if str(target).isdigit() else target
        disturbance = Drift(errors={target: {drift.get("key") or "dx": float(drift.get("step") or 0.0)}}, seed=int(seed))
    try:
        out = simulate(_model(), ks, diag, pulses=pulses, gain=float(gain), delay=int(delay), disturbance=disturbance,
                       noise=bool(noise), seed=int(seed))
    except (ModelError, PreviewError, ValueError) as exc:
        raise UserError(str(exc)) from exc
    return {"rms": out["rms"].tolist(), "openRms": out["open_rms"].tolist(),
            "predictedModulus": out["predicted_modulus"], "knobs": out["knobs"].tolist(), "knobLabels": out["knob_labels"]}


# =========================================================================== engine response
def engine_running():
    t = _engine["thread"]
    return t is not None and t.is_alive()


@rpc("analysis.responseEngineStart")
def response_engine_start(monitors, knobs, particles=200):
    """Response matrix from engine runs (base + one run per knob, fixed seed) in the background."""
    from avas.ai.sandbox import Sandbox, SandboxError
    from avas.gui.services import runner
    from avas.gui.services.study import STUDY_NAME, _sandbox_runner, _study
    p = context.project().require()
    diag, ks = _parse_spec(monitors, knobs)
    if any(k.kind == "error" for k in ks):
        raise UserError("The engine has no deterministic offset knobs; use the linear model for them.")
    if engine_running() or runner.any_active():
        raise UserError("A simulation is already running; wait for it to finish.")
    stop = threading.Event()
    box = Sandbox(p, "study_response", root=os.path.join(p.path, "Research", STUDY_NAME, ".sandbox", "response"))
    try:
        box.begin("response matrix", len(ks) + 1, stop, source="scan")
    except SandboxError as exc:
        box.cleanup()
        raise UserError(str(exc)) from exc
    _engine.update(stop=stop, result={"status": "running", "runs": len(ks) + 1})

    def work():
        from avas.research import Variant, engine_response
        result = {"status": "finished"}
        try:
            study = _study(p, _sandbox_runner(box))
            resp = engine_response(study, ks, diag, Variant(particles=int(particles)), stop=stop)
            result.update(_response_json(resp, diag))
        except Exception as exc:  # noqa: BLE001 - reported to the page
            result = {"status": "stopped" if stop.is_set() else "failed", "error": str(exc)}
            if not stop.is_set():
                log.exception("engine response matrix failed")
        finally:
            box.end()
            _engine["result"] = result
            bridge.emit("analysis.responseFinished", result)

    thread = threading.Thread(target=work, name="avas-response", daemon=True)
    _engine["thread"] = thread
    thread.start()
    return {"running": True, "runs": len(ks) + 1}


@rpc("analysis.responseEngineStop")
def response_engine_stop():
    if _engine["stop"] is not None:
        _engine["stop"].set()
    from avas.ai import sandbox
    sandbox.stop_all()
    return True


def shutdown():
    chart_stop()
    response_engine_stop()
