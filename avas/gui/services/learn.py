"""Primer page: interactive labs and the guided tutorial (:mod:`avas.learn`).

Labs and the tutorial's preview are linear-optics calculations and need no
project.  The tutorial's engine run happens in a sandbox in the user's data
folder (``<user data>/learn/runs/<job>``, the last few runs are kept), with
the example project's text inputs copied and its field maps read in place, so
no project file and no project result is touched.  It is an activity like the
assistant's studies (source ``learn``): the Run page, tool bar and status bar
show it and can pause or stop it, and no other simulation can start meanwhile.

Events: ``learn.tutorial`` ({status: running | done | failed | stopped, step,
percent, result, error}).
"""
import logging
import os
import shutil
import threading
import time
import uuid

import numpy as np

from avas import paths
from avas.gui import bridge
from avas.gui.bridge import UserError, rpc
from avas.learn import labs as lab_core
from avas.learn import tutorial as tut

log = logging.getLogger("avas.gui")

KEEP_RUNS = 3
_lock = threading.Lock()
_current = {"thread": None, "stop": None, "state": {"status": "idle"}}


def work_root():
    return os.path.join(paths.USER_DATA_DIR, "learn", "runs")


def _wire(value):
    """Arrays → blobs, tuples → lists, for the page."""
    if isinstance(value, np.ndarray):
        return bridge.blob(value.astype(float), "float64")
    if isinstance(value, dict):
        return {k: _wire(v) for k, v in value.items()}
    if isinstance(value, (list, tuple)):
        return [_wire(v) for v in value]
    if isinstance(value, np.generic):
        return value.item()
    return value


def _user_error(exc):
    return UserError(str(exc))


@rpc("learn.labs")
def labs():
    return lab_core.definitions()


@rpc("learn.lab")
def lab(id, params=None):  # noqa: A002 - the page's name for it
    try:
        return _wire(lab_core.run(id, params))
    except lab_core.LabError as exc:
        raise _user_error(exc) from exc


@rpc("learn.beamline")
def beamline(text):
    """A chapter's beam-line snippet with the linear preview of the example beam (2D / 3D view)."""
    return _wire(lab_core.beamline(text))


@rpc("learn.examples")
def examples():
    """Where the example field maps are (the chapters' field pictures read them by name)."""
    folder = paths.example_input_dir()
    return {"available": folder is not None, "fieldDirs": [folder] if folder else [],
            "maps": {"hwr010": "HWR cavity (162.5 MHz, 0.21 m)", "sol": "solenoid (0.35 m)"}}


# =========================================================================== tutorial
@rpc("learn.tutorial.steps")
def tutorial_steps():
    return {"steps": tut.definitions(), "available": paths.example_input_dir() is not None}


@rpc("learn.tutorial.preview")
def tutorial_preview(step, params=None):
    from avas.sim import linear_optics as lo
    try:
        text = tut.lattice(step, params)
        res = lo.linear_preview(text, tut.preview_beam(step, params), [tut.example_dir()], max_points=1500)
    except (lab_core.LabError, lo.PreviewError) as exc:
        raise _user_error(exc) from exc
    out = {k: res[k] for k in ("z", "rms_x", "rms_y", "rms_z", "energy")}
    out.update(elements=res["elements"], warnings=[list(w) for w in res["warnings"]], lattice=text,
               params=tut.values(step, params)[1])
    return _wire(out)


class _Workspace:
    """What :class:`avas.ai.sandbox.Sandbox` needs of a project: the example's inputs and field maps."""

    def __init__(self, root, input_dir):
        self.path = root
        self.input_dir = input_dir

    def field_dirs(self):
        return [self.input_dir]

    def lattice_name(self):
        return tut.LATTICE_NAME


def _prune(root, keep):
    if not os.path.isdir(root):
        return
    runs = sorted((os.path.join(root, n) for n in os.listdir(root)), key=os.path.getmtime, reverse=True)
    for old in runs[keep:]:
        shutil.rmtree(old, ignore_errors=True)


def _publish(state):
    """Keep *state*; the event carries everything but the curves (the page fetches those with
    ``learn.tutorial.state``, which sends them as blobs)."""
    _current["state"] = state
    event = {k: v for k, v in state.items() if k != "result"}
    event["hasResult"] = "result" in state
    bridge.emit("learn.tutorial", _wire(event))


def running():
    t = _current["thread"]
    return t is not None and t.is_alive()


@rpc("learn.tutorial.state")
def tutorial_state():
    return _wire(dict(_current["state"], running=running()))


@rpc("learn.tutorial.run")
def tutorial_run(step, params=None):
    """Run the engine on the step's lattice in the sandbox; the result arrives with ``learn.tutorial``."""
    from avas.ai import sandbox
    from avas.data.dataset_stream import dataset_envelope
    from avas.gui.services import runner
    try:
        _step, values = tut.values(step, params)
        text = tut.lattice(step, values)
        beam = tut.beam_text(step, values)
        folder = tut.example_dir()
    except lab_core.LabError as exc:
        raise _user_error(exc) from exc
    if runner.any_active():
        raise UserError("A simulation is already running; wait for it to finish.")
    with _lock:
        if running():
            raise UserError("The tutorial is already running a simulation.")
        root = work_root()
        os.makedirs(root, exist_ok=True)
        _prune(root, KEEP_RUNS - 1)
        job = time.strftime("%Y%m%d-%H%M%S") + "-" + uuid.uuid4().hex[:6]
        stop = threading.Event()
        try:
            box = sandbox.Sandbox(_Workspace(root, folder), job, root=os.path.join(root, job))
            box.begin("Primer tutorial", total=1, stop_event=stop, source="learn")   # reserves the runner now
        except sandbox.SandboxError as exc:
            raise UserError(str(exc)) from exc
        base = {"step": step, "params": values, "lattice": text, "started": time.time()}
        _current["stop"] = stop
        _publish({**base, "status": "running", "percent": 0.0})

        def progress(pct):
            _publish({**base, "status": "running", "percent": pct})

        def work():
            state = {**base, "status": "failed"}
            try:
                out, seconds = box.run(lattice_text=text, files={"beam.txt": beam}, stop_event=stop, progress=progress)
                env = dataset_envelope(out)
                if env is None:
                    state["error"] = "The engine wrote no DataSet.txt."
                else:
                    state.update(status="done", seconds=seconds, output=out,
                                 result={k: env[k] for k in ("z", "rmsX", "rmsY", "rmsZ", "energy", "alive")})
            except sandbox.SandboxError as exc:
                state.update(status="stopped" if str(exc) == "stopped" else "failed", error=str(exc))
            except Exception as exc:  # noqa: BLE001 - reported to the page
                log.exception("primer tutorial run failed")
                state["error"] = f"{type(exc).__name__}: {exc}"
            finally:
                box.end()
            _publish(state)

        thread = threading.Thread(target=work, name="avas-learn", daemon=True)
        _current["thread"] = thread
        thread.start()
    log.info("primer tutorial: engine run of step %s started", step)
    return tutorial_state()


@rpc("learn.tutorial.stop")
def tutorial_stop():
    stop = _current["stop"]
    if stop is not None:
        stop.set()
    from avas.ai import sandbox
    sandbox.stop_all()
    return True


@rpc("learn.tutorial.export")
def tutorial_export(folder, step, params=None):
    """Save the step as a new project in *folder*; the page opens it afterwards."""
    if not folder:
        raise UserError("No folder selected.")
    try:
        path = tut.export(folder, step, params)
    except lab_core.LabError as exc:
        raise _user_error(exc) from exc
    except OSError as exc:
        raise UserError(f"Could not write the project: {exc}") from exc
    log.info("primer tutorial step %s saved as project %s", step, path)
    return {"path": path}
