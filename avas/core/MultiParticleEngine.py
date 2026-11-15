"""ctypes wrapper of the simulation kernel.

The original kernel is ``AVAS.dll`` (Windows) / ``libAVAS.so`` (Linux) in ``avas/engine``; the new
kernel (``engine/``, same C interface plus options and queries) is chosen with
:class:`avas.engines.EngineChoice` (``avas run --engine new``, ``AVAS_ENGINE=new`` or ini.ini
``[engine] kernel = new``; see :mod:`avas.engines` for the priority and the library lookup).
"""
import ctypes
import json
import ntpath
import os
import platform
import sys
from ctypes import POINTER, c_char_p, c_int, c_wchar_p

from avas import engines
from avas.paths import ENGINE_DIR

_WINDOWS = platform.system() == "Windows"
_UCRT = "ucrtbase.dll"   # a name, not a ctypes.CDLL("...") literal: PyInstaller would bundle that file


def _unbuffer_c_stdout():
    """Make the C runtime's ``stdout`` unbuffered (Windows, shared UCRT).

    AVAS.dll reports progress with ``printf``/``std::cout``.  When the GUI runs
    the simulation in a child process those writes go to a pipe, and the MSVC
    runtime then buffers them in 4 KiB blocks - the progress would arrive in
    bursts many seconds apart.  The DLL links the dynamic UCRT (see its
    ``api-ms-win-crt-stdio`` import), so switching the shared ``stdout`` FILE
    to ``_IONBF`` here makes every line show up immediately.  Harmless on a
    console and silently skipped when anything is missing.

    Every UCRT already loaded in this process gets it, and none is loaded by name.  The packaged
    program has two: System32's, which the kernels' ``api-ms-win-crt`` imports resolve to, and
    ``_internal\\ucrtbase.dll``, which PyInstaller's bootloader loads with python311.dll but
    nothing imports from (XPS, 2026-11-15).  Which one ``GetModuleHandleW`` or a name lookup
    returns depends on the load order, so all of them are switched.
    """
    if not _WINDOWS or sys.stdout is None or sys.stdout.isatty():
        return
    for handle in _loaded_ucrts():
        try:
            ucrt = ctypes.CDLL(_UCRT, handle=handle)
            ucrt.__acrt_iob_func.restype = ctypes.c_void_p
            ucrt.setvbuf.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, ctypes.c_size_t]
            _IONBF = 4
            ucrt.setvbuf(ucrt.__acrt_iob_func(1), None, _IONBF, 0)
        except (OSError, AttributeError):
            pass


def _loaded_ucrts():
    """Handles of every module named ucrtbase.dll in this process (Windows), [] on any failure."""
    try:
        kernel32 = ctypes.WinDLL("kernel32")
        process = kernel32.GetCurrentProcess
        process.restype = ctypes.c_void_p
        enum = kernel32.K32EnumProcessModules
        enum.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_void_p), ctypes.c_ulong, ctypes.POINTER(ctypes.c_ulong)]
        name_of = kernel32.GetModuleFileNameW
        name_of.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p, ctypes.c_ulong]
        me = process()
        count = 1024
        while True:
            mods = (ctypes.c_void_p * count)()
            needed = ctypes.c_ulong(0)
            if not enum(me, mods, ctypes.sizeof(mods), ctypes.byref(needed)):
                return []
            if needed.value <= ctypes.sizeof(mods):
                break
            count = needed.value // ctypes.sizeof(ctypes.c_void_p) + 16
        found = []
        buf = ctypes.create_unicode_buffer(32768)
        for handle in mods[:needed.value // ctypes.sizeof(ctypes.c_void_p)]:
            if handle and name_of(handle, buf, len(buf)) and ntpath.basename(buf.value).lower() == _UCRT:
                found.append(handle)
        return found
    except (OSError, AttributeError):
        return []


def _load(path):
    try:
        if _WINDOWS and hasattr(os, "add_dll_directory"):
            # make libfftw3*.dll / MSVCP140.dll next to AVAS.dll resolvable; the new kernel in
            # avas/engine/new links the same dynamic MSVC runtime, so it also gets avas/engine
            # (MSVCP140.dll) for machines without the VC++ redistributable
            folder = os.path.dirname(os.path.abspath(path))
            os.add_dll_directory(folder)
            if os.path.normcase(folder) != os.path.normcase(os.path.abspath(ENGINE_DIR)) and os.path.isdir(ENGINE_DIR):
                os.add_dll_directory(ENGINE_DIR)
        return ctypes.CDLL(path)
    except OSError as e:
        raise ValueError(f"Failed to load the simulation engine '{path}'. Reason: {e}") from e


class NewLibrary:
    """Options and JSON queries of a loaded new-kernel library (no simulation)."""

    QUERIES = ("avas_engine_version", "avas_engine_info", "avas_engine_devices", "avas_engine_last_run")

    def __init__(self, library, path):
        self.library = library
        self.path = path
        if hasattr(library, "avas_set_option"):
            library.avas_set_option.argtypes = [c_char_p, c_char_p]
            library.avas_set_option.restype = c_int
        for name in self.QUERIES:
            if hasattr(library, name):
                getattr(library, name).restype = c_char_p

    def set_options(self, options):
        """``avas_set_option`` for every key; ValueError when the library rejects one."""
        if not hasattr(self.library, "avas_set_option"):
            raise ValueError(f"{self.path} has no avas_set_option (not the new kernel?)")
        for key, value in options.items():
            rc = self.library.avas_set_option(key.encode(), str(value).encode())
            if rc != 0:
                raise ValueError(f"the new kernel rejected option {key}={value} (code {rc})")

    def _query(self, name):
        if not hasattr(self.library, name):
            return None
        text = getattr(self.library, name)()
        return json.loads(text.decode("utf-8")) if text else None

    def version(self):
        if not hasattr(self.library, "avas_engine_version"):
            return None
        return self.library.avas_engine_version().decode("utf-8", "replace")

    def info(self):
        return self._query("avas_engine_info")

    def devices(self):
        return self._query("avas_engine_devices")

    def last_run(self):
        return self._query("avas_engine_last_run")


def load_new_library(path):
    """:class:`NewLibrary` of the new kernel's library at *path* (queries only)."""
    return NewLibrary(_load(path), path)


class MultiParticleEngine:
    """Loads the kernel once; ``get_path`` sets the folders, ``main_agent`` runs (1) or stops (2) it.

    *choice*: :class:`avas.engines.EngineChoice` (default: the one of the running ``avas run``, else
    from the environment).  The loaded library is ``self.library`` on every platform.  The engine's
    ``path`` takes three wide strings on Windows and three UTF-8 byte strings elsewhere;
    ``main_agent`` takes a pointer to an int and returns its status.
    """

    _device_checked = set()

    def __init__(self, choice=None):
        _unbuffer_c_stdout()
        self.choice = choice or engines.active_choice()
        self.kernel = self.choice.kernel
        if self.kernel == "new":
            self.dll_path = engines.new_library_path()
        else:
            self.dll_path = os.path.join(ENGINE_DIR, "AVAS.dll" if _WINDOWS else "libAVAS.so")
        self.dll_dir = os.path.dirname(os.path.abspath(self.dll_path))
        self.so_path = self.dll_path                     # old name, kept for callers that print it
        self.library = _load(self.dll_path)
        text = c_wchar_p if _WINDOWS else c_char_p
        self.library.path.argtypes = [text, text, text]
        self.library.path.restype = c_int
        self.library.main_agent.argtypes = [POINTER(c_int)]
        self.library.main_agent.restype = c_int
        self.new = NewLibrary(self.library, self.dll_path) if self.kernel == "new" else None
        if self.new is not None:
            self.new.set_options(self.choice.engine_options())
        engines.note_engine(self)

    # ------------------------------------------------------------------ new kernel
    def info(self):
        return self.new.info() if self.new else None

    def devices(self):
        return self.new.devices() if self.new else None

    def last_run(self):
        return self.new.last_run() if self.new else None

    def check_device(self):
        """A device the choice requires (cuda, metal, emu) that this machine or build cannot give:
        :class:`avas.engines.EngineChoiceError` before the run, never a silent fallback to the CPU."""
        if self.new is None or self.choice.device in ("auto", "cpu"):
            return
        key = (self.dll_path, self.choice)
        if key in self._device_checked:
            return
        devices = self.devices()
        if devices is None:
            raise engines.EngineChoiceError(f"{self.dll_path} cannot report its devices (library too old?); "
                                            + engines.BUILD_HINT)
        plan = (devices.get("selected") or {}).get("sc") or {}
        if plan.get("error"):
            raise engines.EngineChoiceError(engines.device_error(self.choice, plan["error"]))
        self._device_checked.add(key)

    # ------------------------------------------------------------------ C interface
    def get_path(self, inputfilepath, outputfilePath, fieldfilePath):
        """Tell the engine where to read the inputs / field maps and where to write; returns its status."""
        args = (str(inputfilepath), str(outputfilePath), str(fieldfilePath))
        if not _WINDOWS:
            args = tuple(a.encode("utf-8") for a in args)
        res = self.library.path(*args)
        if res not in (0, None):
            raise RuntimeError(f"the engine rejected the folders (path() returned {res}): "
                               f"input={inputfilepath}, output={outputfilePath}, field={fieldfilePath}")
        return res

    def main_agent(self, value):
        """``value`` 1 = run, 2 = stop.  Returns the engine status (0 ok; 1 / 2 = error, see ErrorLog.txt)."""
        if int(value) == 1 and self.new is not None:
            self.new.set_options(self.choice.engine_options())     # Python's choice wins over the environment
            self.check_device()
        return self.library.main_agent(ctypes.byref(c_int(int(value))))
