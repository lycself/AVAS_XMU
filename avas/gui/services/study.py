"""Study page, scan tab: one- or two-axis scans by the linear model or the engine (:mod:`avas.research`).

Scans of the GUI are runs of the research study ``<project>/Research/gui/`` (shared with notebooks:
``research.Study(project, name="gui")`` sees the same runs and cache).  Each scan is described by
``Research/gui/scans/<label>_<time>.json`` (axes, values, rows with the metrics and the run folder).
Engine scans run one simulation at a time through the AI sandbox's activity (source ``scan``), so the
Run page shows them and they can be paused and stopped like the old scans; linear scans run in a
thread and take seconds.  Old ``Scans/`` results stay readable through ``scan.*``.

Events: ``study.progress`` (the partial scan after every point), ``study.finished`` ({path, status, error}).
"""
import itertools
import json
import logging
import os
import threading
import time

from avas.gui import bridge, context
from avas.gui.bridge import UserError, rpc
from avas.sim import scan as core

log = logging.getLogger("avas.gui")

STUDY_NAME = "gui"
MAX_POINTS = 2500
_lock = threading.Lock()
_current = {"thread": None, "stop": None, "scan": None}


def running():
    t = _current["thread"]
    return t is not None and t.is_alive()


def _study(p, runner=None):
    from avas.research import Study
    return Study(p.path, name=STUDY_NAME, runner=runner)


def _scans_dir(p):
    return os.path.join(p.path, "Research", STUDY_NAME, "scans")


def _write(path, data):
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        json.dump(data, fh, indent=1, ensure_ascii=False, default=str)
    os.replace(tmp, path)


def _snapshot(scan):
    if scan is None:
        return None
    out = dict(scan)
    out["rows"] = [dict(r) for r in scan.get("rows", [])]
    return out


# --------------------------------------------------------------------------- axes
class Axis:
    """One scan axis: an old-style scan spec resolved against the project and turned into variant changes."""

    def __init__(self, p, spec):
        texts = core.read_inputs(p.input_dir, p.lattice_name())
        try:
            self.target = core.ScanTarget(spec, texts, p.field_dirs(), p.lattice_name())
        except core.ScanError as exc:
            raise UserError(str(exc)) from exc
        self.kind = self.target.kind
        if self.kind == "lattice":
            self.address = ("lattice", self.target.st.line_no + 1, self.target.param)
        else:
            from avas.data import lattice_edit as le
            _line, parts = le.keyword_line(texts[self.target.file], self.target.keyword)
            self.base = list(parts[1:]) if parts else []
            self.address = f"{self.kind}.{self.target.keyword}"

    def apply(self, variant, value):
        if self.kind == "lattice":
            return variant.set(self.address, value)
        position = self.target.position
        items = list(self.base)
        while len(items) < position:
            items.append("0")
        items[position - 1] = value
        return variant.set(self.address, items if len(items) > 1 else value)

    def describe(self):
        return self.target.describe()


def _axes(p, specs):
    if not specs or len(specs) > 2:
        raise UserError("Give one or two scan axes.")
    return [Axis(p, s) for s in specs]


def _values(values):
    try:
        return [core.normalise_values(v) for v in values]
    except core.ScanError as exc:
        raise UserError(str(exc)) from exc


@rpc("study.metrics")
def metrics():
    from avas.research.runs import COMMON_METRICS
    return {"default": ["energy_out", "rms_x_max", "rms_y_max", "rms_x_out", "rms_y_out", "transmission"],
            "all": list(COMMON_METRICS) + ["transmission", "emit_z_growth", "lost"],
            "linear": list(COMMON_METRICS)}


@rpc("study.check")
def check(axes, values=None, fidelity="linear"):
    """Resolve the axes (and values) without running: ``{axes, counts, count, estimate_s}``."""
    p = context.project().require()
    resolved = _axes(p, axes)
    vals = _values(values) if values and all(v not in (None, "", []) for v in values) else []
    for axis, v in zip(resolved, vals):
        try:
            axis.target.apply(v[0])
        except core.ScanError as exc:
            raise UserError(str(exc)) from exc
    counts = [len(v) for v in vals]
    count = 1
    for c in counts:
        count *= c
    est = (p.last_run() or {}).get("elapsed_s") if fidelity == "engine" else None
    return {"axes": [a.describe() for a in resolved], "counts": counts, "count": count if vals else 0,
            "estimate_s": est * count if est and vals else None}


# --------------------------------------------------------------------------- running
def _row_metrics(run, keys):
    out = {}
    for k in keys:
        if k == "transmission":
            out[k] = run.transmission
        else:
            v = run.metrics.get(k)
            out[k] = v if isinstance(v, (int, float)) else None
    return out


@rpc("study.start")
def start(axes, values, fidelity="linear", metrics=None, label=""):
    """Start a scan of every combination of the axes' values; returns :func:`state`."""
    p = context.project().require()
    if fidelity not in ("linear", "engine"):
        raise UserError("Choose the linear model or the engine.")
    resolved = _axes(p, axes)
    vals = _values(values)
    if len(vals) != len(resolved):
        raise UserError("Give the values of every axis.")
    combos = list(itertools.product(*vals))
    if len(combos) > MAX_POINTS:
        raise UserError(f"At most {MAX_POINTS} points per scan.")
    keys = [m for m in (metrics or metrics_default()) if m]
    label = core.safe_label(label) or core.safe_label("_".join(a.target.param for a in resolved))
    folder = _scans_dir(p)
    os.makedirs(folder, exist_ok=True)
    path = os.path.join(folder, f"{label}_{time.strftime('%Y%m%d-%H%M%S')}.json")
    scan = {"label": label, "created": time.strftime("%Y-%m-%d %H:%M:%S"), "status": "running", "path": path,
            "fidelity": fidelity, "axes": [a.describe() for a in resolved], "values": vals, "metrics": keys,
            "rows": [], "shape": [len(v) for v in vals]}
    stop = threading.Event()
    box = None
    from avas.gui.services import runner
    with _lock:
        if running():
            raise UserError("A scan is already running; wait for it to finish.")
        if fidelity == "engine":
            if runner.any_active():
                raise UserError("A simulation is already running; wait for it to finish.")
            from avas.ai.sandbox import Sandbox, SandboxError
            box = Sandbox(p, f"study_{os.path.basename(path)[:-5]}",
                          root=os.path.join(p.path, "Research", STUDY_NAME, ".sandbox", os.path.basename(path)[:-5]))
            try:
                box.begin(f"scan {label}", len(combos), stop, source="scan")      # reserves the runner now
            except SandboxError as exc:
                box.cleanup()
                raise UserError(str(exc)) from exc
        _write(path, scan)
        _current.update(stop=stop, scan=scan)
        thread = threading.Thread(target=_work, args=(p, resolved, combos, scan, stop, box), name="avas-study",
                                  daemon=True)
        _current["thread"] = thread
        thread.start()
    log.info("%s scan started: %s, %d points", fidelity, label, len(combos))
    return state()


def metrics_default():
    return metrics()["default"]


def _sandbox_runner(box):
    from avas.ai.sandbox import SandboxError
    from avas.research.engine import EngineError

    def run(input_dir, output_dir, field_dir, lattice, log_path, stop, progress):
        box.count += 1
        try:
            return box.run_in(input_dir, output_dir, field_dir, lattice, stop, progress, log_path=log_path)
        except SandboxError as exc:
            raise EngineError(str(exc)) from exc
    return run


def _work(p, axes, combos, scan, stop, box):
    from avas.model import PreviewError
    from avas.research import Variant
    error = None
    try:
        study = _study(p, _sandbox_runner(box) if box else None)
        for index, combo in enumerate(combos, 1):
            if stop.is_set():
                scan["status"] = "stopped"
                break
            variant = Variant(label=", ".join(f"{a.target.param}={v:g}" for a, v in zip(axes, combo)))
            for axis, value in zip(axes, combo):
                variant = axis.apply(variant, value)
            row = {"index": index, "values": list(combo), "variant": variant.to_dict()}
            started = time.time()
            try:
                run = study.run(variant, scan["fidelity"], **({"stop": stop} if box else {}))
                if run.fidelity == "engine":
                    row.update(folder=run.folder, output_dir=run.output_dir, seconds=run.record.get("seconds"))
                    if run.record.get("status") == "stopped":
                        scan["status"] = "stopped"
                        row["error"] = "stopped"
                        scan["rows"].append(row)
                        break
                    if not run.ok:
                        row["error"] = run.record.get("error") or "failed"
                else:
                    row["seconds"] = round(time.time() - started, 3)
                if run.ok:
                    row.update(_row_metrics(run, scan["metrics"]))
            except (PreviewError, ValueError) as exc:
                row["error"] = str(exc)
            scan["rows"].append(row)
            _write(scan["path"], scan)
            bridge.emit("study.progress", _snapshot(scan))
        if scan["status"] == "running":
            scan["status"] = "finished"
    except Exception as exc:  # noqa: BLE001 - reported to the page
        error = f"{type(exc).__name__}: {exc}"
        scan["status"] = "failed"
        log.exception("study scan failed")
    finally:
        if box is not None:
            box.end()
        scan["finished"] = time.strftime("%Y-%m-%d %H:%M:%S")
        _write(scan["path"], scan)
        bridge.emit("study.finished", {"path": scan["path"], "status": scan["status"], "error": error})


@rpc("study.state")
def state():
    return {"running": running(), "scan": _snapshot(_current["scan"])}


@rpc("study.stop")
def stop():
    stop_event = _current["stop"]
    if stop_event is not None:
        stop_event.set()
    from avas.ai import sandbox
    sandbox.stop_all()
    return True


# --------------------------------------------------------------------------- records
def _check_path(p, path):
    root = os.path.normcase(os.path.abspath(_scans_dir(p)))
    full = os.path.normcase(os.path.abspath(path or ""))
    if not full.startswith(root + os.sep) or not full.endswith(".json") or not os.path.isfile(path):
        raise UserError("This file is not a scan of the project.")


@rpc("study.list")
def list_scans():
    """New scans (Research/gui/scans) and the old ones (Scans/, ``legacy``), newest first."""
    p = context.project().require()
    out = []
    folder = _scans_dir(p)
    if os.path.isdir(folder):
        for name in os.listdir(folder):
            if not name.endswith(".json"):
                continue
            path = os.path.join(folder, name)
            try:
                with open(path, encoding="utf-8") as fh:
                    s = json.load(fh)
            except (OSError, ValueError):
                continue
            total = 1
            for v in s.get("values") or []:
                total *= len(v)
            out.append({"path": path, "label": s.get("label"), "created": s.get("created"), "finished": s.get("finished"),
                        "status": s.get("status"), "fidelity": s.get("fidelity"), "axes": s.get("axes"),
                        "count": total, "done": len(s.get("rows") or []), "legacy": False})
    for r in core.list_scans(p.path):
        out.append({"path": r["folder"], "label": r.get("label"), "created": r.get("created"),
                    "finished": r.get("finished"), "status": r.get("status"), "fidelity": "engine",
                    "axes": [r.get("target")], "count": len(r.get("values") or []), "done": len(r.get("rows") or []),
                    "legacy": True})
    out.sort(key=lambda r: r.get("created") or "", reverse=True)
    return out


@rpc("study.load")
def load(path):
    p = context.project().require()
    _check_path(p, path)
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


@rpc("study.delete")
def delete(path):
    """Move a scan description to the recycle bin; its engine runs stay in the study's cache."""
    p = context.project().require()
    _check_path(p, path)
    current = _current["scan"]
    if running() and current and os.path.normcase(current["path"]) == os.path.normcase(path):
        raise UserError("This scan is still running; stop it first.")
    from avas.gui.services.segments import _trash
    _trash(path)
    return True


@rpc("study.point")
def point(path, index):
    """Curves of one point of a scan: ``{z, rms_x, rms_y, cx, cy, energy}`` (blobs, m and MeV)."""
    p = context.project().require()
    _check_path(p, path)
    with open(path, encoding="utf-8") as fh:
        scan = json.load(fh)
    row = next((r for r in scan.get("rows", []) if r.get("index") == int(index)), None)
    if row is None or row.get("error"):
        raise UserError("This point has no result.")
    from avas.research import Variant
    study = _study(p)
    if scan.get("fidelity") == "engine":
        if not row.get("folder") or not os.path.isdir(row["folder"]):
            raise UserError("The run of this point no longer exists.")
        run = study.load(row["folder"])
    else:
        run = study.linear(Variant.from_dict(row.get("variant")))
    out = {"fidelity": run.fidelity, "label": run.variant.label}
    for q in ("rms_x", "rms_y", "cx", "cy", "energy"):
        out[q] = bridge.blob(run.curve(q), "float32")
    out["z"] = bridge.blob(run.z, "float32")
    return out


def shutdown():
    stop()
