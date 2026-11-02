"""Engine runs in a child process, as ``avas run`` would do them.

The engine is a C++ library loaded through ctypes; an exception inside it ends the process
that loaded it (seen as Windows error 0xe06d7363).  A notebook must survive that, so every
run is a child process; its output goes to a log file next to the run.
"""
import hashlib
import os
import re
import subprocess
import sys
import threading
import time

from avas import paths
from avas.gui import proctree

PROGRESS = re.compile(r"Simulate progress\s+([\d.]+)\s*%", re.IGNORECASE)
KEEP_OUTPUT = ("DataSet.txt", "avas_run.json", "synData.txt", "Phase.txt")
ENGINE_FILES = ("AVAS.dll", "libAVAS.so")
_identity = {}


class EngineError(RuntimeError):
    """The engine run failed or was stopped (message for the user)."""


def default_command(input_dir, output_dir, field_dir, lattice):
    """``avas run`` of the installed program or of this source tree."""
    args = ["run", "--input", input_dir, "--output", output_dir, "--mode", "basic", "--field", field_dir,
            "--lattice", lattice]
    if getattr(sys, "frozen", False):
        console = os.path.join(os.path.dirname(sys.executable), "AVAS.exe")
        return [console if os.path.isfile(console) else sys.executable] + args
    return [sys.executable, "-u", "-m", "avas"] + args


def identity():
    """``{"file", "sha1"}`` of the engine library (cached per file size and time; None without one)."""
    for name in ENGINE_FILES:
        path = paths.engine_file(name)
        try:
            st = os.stat(path)
        except OSError:
            continue
        key = (path, st.st_size, st.st_mtime_ns)
        if key not in _identity:
            h = hashlib.sha1()
            with open(path, "rb") as fh:
                for block in iter(lambda: fh.read(1 << 20), b""):
                    h.update(block)
            _identity.clear()
            _identity[key] = {"file": name, "sha1": h.hexdigest()}
        return dict(_identity[key])
    return {"file": None, "sha1": None}


def run(command, cwd, log_path, timeout=None, stop=None, progress=None):
    """Run *command*; returns the seconds it took.  *stop*: a ``threading.Event``; *progress* is called
    with the engine's percentage.  Raises :class:`EngineError` on a non-zero exit, a stop or a timeout."""
    env = dict(os.environ)
    env.update(PYTHONUNBUFFERED="1", PYTHONIOENCODING="utf-8", MPLBACKEND="Agg", AVAS_GUI_CHILD="1")
    root = os.path.dirname(paths.PACKAGE_DIR)
    env["PYTHONPATH"] = root + (os.pathsep + env["PYTHONPATH"] if env.get("PYTHONPATH") else "")
    env.pop("AVAS_LATTICE", None)
    flags = subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0
    tail = []
    with open(log_path, "w", encoding="utf-8") as log:
        proc = subprocess.Popen(command, cwd=cwd, env=env, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, creationflags=flags)

        def reader():
            for raw in iter(proc.stdout.readline, b""):
                line = raw.decode("utf-8", "replace").rstrip()
                log.write(line + "\n")
                m = PROGRESS.search(line)
                if m and progress:
                    try:
                        progress(float(m.group(1)))
                    except Exception:  # noqa: BLE001 - progress display only
                        pass
                elif line.strip():
                    tail.append(line.strip())
                    del tail[:-30]

        thread = threading.Thread(target=reader, name="avas-research-reader", daemon=True)
        thread.start()
        started = time.time()
        try:
            while proc.poll() is None:
                if stop is not None and stop.is_set():
                    proctree.kill(proc)
                    raise EngineError("stopped")
                if timeout and time.time() - started > timeout:
                    proctree.kill(proc)
                    raise EngineError(f"timed out after {timeout:.0f} s")
                time.sleep(0.1)
        finally:
            thread.join(5)
    if proc.returncode != 0:
        message = next((ln[6:].strip() for ln in reversed(tail) if ln.lower().startswith("error:")), None)
        raise EngineError(message or (tail[-1] if tail else f"engine exit code {proc.returncode}"))
    return time.time() - started


def trim_output(folder):
    """Remove the bulky outputs (particle distributions …), keeping :data:`KEEP_OUTPUT`."""
    for name in os.listdir(folder):
        path = os.path.join(folder, name)
        if name not in KEEP_OUTPUT and os.path.isfile(path):
            try:
                os.remove(path)
            except OSError:
                pass
