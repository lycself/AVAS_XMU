"""Choice of the simulation kernel: the original one (AVAS.dll / libAVAS.so) or the new one (engine/).

Priority of every setting, highest first: the command line (``avas run --engine … --device …``), the
environment (``AVAS_ENGINE``, ``AVAS_ENGINE_DEVICE``, ``AVAS_ENGINE_PRECISION``, ``AVAS_ENGINE_GPUS``,
``AVAS_ENGINE_THREADS``, ``AVAS_ENGINE_COMPAT``), the project's ``ini.ini`` section ``[engine]``
(keys ``kernel device precision gpus threads compat``), for the kernel only the GUI's global "default
computing kernel" preference (gui.json, :func:`default_choice_kernel`; "original" where the original kernel
does not exist falls back to the built-in default with a note), the defaults (kernel new on every
platform since M8b, 2026-11-15: :func:`default_kernel`; the original kernel stays selectable on Windows and
Linux x86-64; for the new kernel device auto, precision auto, gpus all, threads 0 = automatic, compat none).  With the
original kernel only ``threads`` applies (as ``OMP_NUM_THREADS`` of the process that loads it,
:func:`original_thread_env`); the other keys are ignored; choosing it explicitly on a platform without it is an
error before the run (:func:`check_platform`).

input.txt ``multithreading`` does not choose the thread count: both kernels default to ``multithreading 1``
(absent = 1; 0, 2, 3 lose every particle at the start), so the CPU threads are only ``threads``.  input.txt is never changed (the original kernel reads the same
project).

The new kernel's library is found at ``AVAS_ENGINE_LIB``, then in ``avas/engine/new/`` (the prebuilt
libraries), then in the source tree's ``engine/build`` (``Release``, ``RelWithDebInfo``).

Results of the new kernel fall into *result tiers*: runs of one tier and one engine version give the
same numbers whatever the thread count or the GPU model (``result_tier``).  The research cache keys
on them.

The engine library itself is only loaded in ``avas run`` / ``avas info`` processes (an exception in
C++ ends the process that loaded it); other processes ask a child (:func:`query_subprocess`).
"""
import configparser
import hashlib
import json
import os
import platform
import subprocess
import sys
from dataclasses import dataclass, field, replace

from avas import paths

KERNELS = ("original", "new")
DEVICES = ("auto", "cpu", "cuda", "metal")
DEV_DEVICES = ("emu",)                    # development only: the device back end on the CPU
PRECISIONS = ("auto", "fp64", "fp32")
COMPATS = ("none", "dll")
KEYS = ("kernel", "device", "precision", "gpus", "threads", "compat")
INI_SECTION = "engine"
ENV = {"kernel": "AVAS_ENGINE", "device": "AVAS_ENGINE_DEVICE", "precision": "AVAS_ENGINE_PRECISION",
       "gpus": "AVAS_ENGINE_GPUS", "threads": "AVAS_ENGINE_THREADS", "compat": "AVAS_ENGINE_COMPAT"}
LIB_ENV = "AVAS_ENGINE_LIB"
DEFAULTS = {"kernel": "new", "device": "auto", "precision": "auto", "gpus": "all", "threads": 0, "compat": "none"}
NEW_LIB_NAMES = {"Windows": "avas_engine.dll", "Darwin": "libavas_engine.dylib"}
BUILD_HINT = ("build it from the source tree (cmake -S engine -B engine/build && cmake --build engine/build; "
              "on macOS with Apple's clang for Metal) or set AVAS_ENGINE_LIB to the library file")

# result tiers
TIER_ORIGINAL = "original"
TIER_CPU = "new-cpu-fp64"
TIER_CUDA64 = "new-cuda-fp64"
TIER_FP32 = "new-fp32"
TIER_DLL = "new-compat-dll"
TIERS = (TIER_ORIGINAL, TIER_CPU, TIER_CUDA64, TIER_FP32, TIER_DLL)


ORIGINAL_SYSTEMS = ("Windows", "Linux")
ORIGINAL_MACHINES = ("x86_64", "amd64", "x64")


def original_available(system=None, machine=None):
    """Whether the original kernel (AVAS.dll / libAVAS.so, x86-64 binaries) can run on this platform."""
    system = platform.system() if system is None else system
    machine = (platform.machine() if machine is None else machine).lower()
    return system in ORIGINAL_SYSTEMS and machine in ORIGINAL_MACHINES


def default_kernel():
    """The kernel used when nothing selects one: the new kernel on every platform (M8b, 2026-11-15; before,
    the original kernel where it exists).  The original kernel is still chosen explicitly or with the global
    preference where it runs."""
    return "new"


def platform_label(system=None, machine=None):
    system = platform.system() if system is None else system
    machine = platform.machine() if machine is None else machine
    name = {"Darwin": "macOS"}.get(system, system)
    return f"{name} {machine}".strip()


class EngineChoiceError(ValueError):
    """An invalid kernel / device setting, a missing library or a device this machine does not have
    (a message for the user)."""


# --------------------------------------------------------------------------- the choice
@dataclass(frozen=True)
class EngineChoice:
    """Kernel and, for the new kernel, its options.  *sources* tells where each value came from
    (``cli``, ``env``, ``ini``, ``default``; not compared)."""
    kernel: str = "new"
    device: str = "auto"
    precision: str = "auto"
    gpus: str = "all"
    threads: int = 0
    compat: str = "none"
    sources: dict = field(default_factory=dict, compare=False, hash=False, repr=False)

    @property
    def is_new(self):
        return self.kernel == "new"

    def engine_options(self):
        """``avas_set_option`` key / value pairs of the new kernel (Python's values take precedence over
        the environment the library would read itself)."""
        return {"compat": self.compat, "threads": str(int(self.threads)), "device": self.device,
                "precision": self.precision, "gpus": self.gpus}

    def to_dict(self):
        if not self.is_new:
            return {"kernel": "original", **({"threads": int(self.threads)} if self.threads else {})}
        return {"kernel": "new", "device": self.device, "precision": self.precision, "gpus": self.gpus,
                "threads": int(self.threads), "compat": self.compat}

    def cli_args(self):
        """``avas run`` arguments that select exactly this choice."""
        if not self.is_new:
            return ["--engine", "original"] + (["--threads", str(int(self.threads))] if self.threads else [])
        return ["--engine", "new", "--device", self.device, "--precision", self.precision, "--gpus", self.gpus,
                "--threads", str(int(self.threads)), "--compat", self.compat]

    def label(self):
        if not self.is_new:
            return "original kernel" + (f" ({int(self.threads)} threads)" if self.threads else "")
        parts = [self.device, self.precision] + (["compat=dll"] if self.compat == "dll" else [])
        return "new kernel (" + ", ".join(parts) + ")"


def _norm_gpus(value, origin):
    if isinstance(value, int) and not isinstance(value, bool):
        value = str(value)
    if isinstance(value, (list, tuple)):
        value = ",".join(str(int(v)) for v in value)
    text = str(value).strip().lower().replace(" ", "")
    if text in ("", "all"):
        return "all"
    out = []
    for tok in text.split(","):
        if not tok:
            continue
        if not tok.isdigit():
            raise EngineChoiceError(f"{origin}: invalid gpus '{value}' (expected all or GPU numbers such as 0,1)")
        if int(tok) not in out:
            out.append(int(tok))
    if not out:
        raise EngineChoiceError(f"{origin}: invalid gpus '{value}' (expected all or GPU numbers such as 0,1)")
    return ",".join(str(v) for v in out)


def parse_value(key, value, origin="setting"):
    """Normalized value of *key* (raises :class:`EngineChoiceError` for an invalid one)."""
    if key == "gpus":
        return _norm_gpus(value, origin)
    if key == "threads":
        try:
            n = int(str(value).strip())
        except ValueError:
            n = -1
        if n < 0:
            raise EngineChoiceError(f"{origin}: invalid threads '{value}' (expected 0 = automatic or a positive number)")
        return n
    text = str(value).strip().lower()
    allowed = {"kernel": KERNELS, "device": DEVICES + DEV_DEVICES, "precision": PRECISIONS, "compat": COMPATS}[key]
    if key == "compat" and text == "default":
        text = "none"
    if text not in allowed:
        shown = DEVICES if key == "device" else allowed
        raise EngineChoiceError(f"{origin}: invalid {key} '{value}' (expected {' | '.join(shown)})")
    return text


def read_ini(input_dir):
    """Raw ``[engine]`` values of ``<input_dir>/ini.ini`` (empty when there is none)."""
    path = os.path.join(input_dir, "ini.ini") if input_dir else ""
    if not path or not os.path.isfile(path):
        return {}
    cfg = configparser.ConfigParser()
    cfg.optionxform = str
    try:
        cfg.read(path, encoding="utf-8")
    except configparser.Error:
        return {}
    if not cfg.has_section(INI_SECTION):
        return {}
    return {k.strip().lower(): v for k, v in cfg.items(INI_SECTION) if k.strip().lower() in KEYS and str(v).strip() != ""}


# --------------------------------------------------------------------------- global preference
# The GUI's "default computing kernel" (Settings menu), stored in the GUI preferences file gui.json under
# this key: "original", "new", or absent / "" = the built-in default.  Every process reads the same file:
# AVAS_GUI_SETTINGS when set (``avas serve --settings`` sets it, and children started by the GUI inherit
# it), else the user's gui.json in the user data folder (the same rule as avas.gui.settings.default_path).
PREFERENCE_KEY = "engine/defaultKernel"
PREFERENCE_ENV = "AVAS_GUI_SETTINGS"


def preference_path():
    """The GUI preferences file the global default kernel is read from."""
    override = os.environ.get(PREFERENCE_ENV, "").strip()
    return override or os.path.join(paths.USER_DATA_DIR, "gui.json")


def read_preference(path=None):
    """The global default kernel of the GUI preferences ("original", "new" or "" = not set; an unreadable
    file or an unknown value counts as not set)."""
    try:
        with open(path or preference_path(), encoding="utf-8-sig") as fh:
            data = json.load(fh)
    except (OSError, ValueError):
        return ""
    value = str(data.get(PREFERENCE_KEY) or "").strip().lower() if isinstance(data, dict) else ""
    return value if value in KERNELS else ""


def default_choice_kernel(preference=None):
    """``(kernel, source, note)`` used when neither the command line, the environment nor ini.ini choose a
    kernel: the global preference (source ``preference``), else the built-in default (``default``).  The
    preference "original" on a platform without the original kernel falls back to the built-in default with
    a *note* (not an error: the preference is shared by every project).  *preference* None reads the
    preferences file; a string is used as the preference ("" = not set)."""
    pref = read_preference() if preference is None else str(preference).strip().lower()
    if pref == "original" and not original_available():
        return default_kernel(), "default", ("the default kernel preference 'original' is ignored: the original "
                                             f"kernel does not run on {platform_label()}")
    if pref in KERNELS:
        return pref, "preference", None
    return default_kernel(), "default", None


def resolve_choice(cli=None, input_dir=None, env=None, preference=None):
    """The :class:`EngineChoice` from *cli* (dict, None values ignored), the environment *env* (default
    ``os.environ``), ``[engine]`` of the project's ini.ini in *input_dir*, then the global default kernel
    of the GUI preferences (:func:`default_choice_kernel`, *preference* as there) and the built-in default.
    ``sources["kernel"]`` is ``cli``, ``env``, ``ini``, ``preference`` or ``default``; ``sources["note"]``
    explains an ignored preference."""
    cli = {k: v for k, v in (cli or {}).items() if v is not None and k in KEYS}
    env = os.environ if env is None else env
    ini = read_ini(input_dir)
    origins = {"cli": "command line", "env": "environment variable", "ini": "ini.ini [engine]"}
    note = None

    def raw(key):
        nonlocal note
        if key in cli:
            return cli[key], "cli", f"{origins['cli']} --{'engine' if key == 'kernel' else key}"
        value = env.get(ENV[key], "")
        if str(value).strip() != "":
            return value, "env", f"{origins['env']} {ENV[key]}"
        if key in ini:
            return ini[key], "ini", f"{origins['ini']} {key}"
        if key == "kernel":
            kernel, src, note = default_choice_kernel(preference)
            return kernel, src, "default kernel preference" if src == "preference" else "default"
        return DEFAULTS[key], "default", "default"

    value, src, origin = raw("kernel")
    kernel = parse_value("kernel", value, origin)
    sources = {"kernel": src}
    if note:
        sources["note"] = note
    if kernel == "original":
        value, src, origin = raw("threads")
        sources["threads"] = src
        return EngineChoice(kernel="original", threads=parse_value("threads", value, origin), sources=sources)
    values = {}
    for key in KEYS[1:]:
        value, src, origin = raw(key)
        values[key] = parse_value(key, value, origin)
        sources[key] = src
    choice = EngineChoice(kernel="new", sources=sources, **values)
    validate(choice)
    return choice


def validate(choice):
    """Combinations the new kernel cannot run (a clear error before the run)."""
    if not choice.is_new:
        return
    if choice.compat == "dll" and choice.device not in ("auto", "cpu"):
        raise EngineChoiceError(f"compat dll reproduces AVAS.dll on the CPU only; device '{choice.device}' cannot be "
                                "used with it (choose device auto or cpu)")
    if choice.device == "metal" and choice.precision == "fp64":
        raise EngineChoiceError("Metal computes in fp32 only; choose precision auto or fp32 (or device cpu for fp64)")
    if choice.device == "cpu" and choice.precision == "fp32":
        raise EngineChoiceError("the CPU computes in fp64 only; choose precision auto or fp64 (fp32 runs on a GPU)")


def original_thread_env(choice, environ=None):
    """Set ``OMP_NUM_THREADS`` in *environ* (default ``os.environ``) for the original kernel with a thread
    count (``threads`` > 0); must run before the library is loaded -- the OpenMP runtime (libgomp, vcomp)
    reads it once.  AVAS.dll / libAVAS.so cap every entry of their task table at ``omp_get_max_threads()``,
    so 1 runs single-threaded, which also avoids AVAS.dll's data race in the space charge of matrix elements
    (docs/engine/90-experiments.md).  Returns the value set, or None (threads 0 = the runtime's default)."""
    if choice is None or choice.is_new or not choice.threads:
        return None
    environ = os.environ if environ is None else environ
    environ["OMP_NUM_THREADS"] = str(int(choice.threads))
    return environ["OMP_NUM_THREADS"]


def check_platform(choice):
    """The original kernel chosen on a platform that does not have it: a clear error before the run."""
    if not choice.is_new and not original_available():
        how = {"ini": " (ini.ini [engine] kernel = original)", "cli": " (--engine original)",
               "env": " (environment variable AVAS_ENGINE)"}.get(choice.sources.get("kernel"), "")
        raise EngineChoiceError(f"the original kernel (AVAS.dll / libAVAS.so) does not run on {platform_label()}; it was "
                                f"chosen explicitly{how}. Use the new kernel: kernel = new in ini.ini [engine], "
                                "--engine new, or the Simulation settings page")


# the choice of the running `avas run` (set by the CLI; MultiParticleEngine uses it)
_active = None


def set_active(choice):
    """Make *choice* the one :class:`~avas.core.MultiParticleEngine.MultiParticleEngine` loads; returns
    the previous one (``None``: resolve from the environment)."""
    global _active
    previous, _active = _active, choice
    return previous


def active_choice():
    return _active if _active is not None else resolve_choice()


# --------------------------------------------------------------------------- libraries
def new_library_name():
    return NEW_LIB_NAMES.get(platform.system(), "libavas_engine.so")


def original_library_path():
    return paths.engine_file("AVAS.dll" if platform.system() == "Windows" else "libAVAS.so")


def new_library_candidates():
    name = new_library_name()
    return [os.path.join(d, name) for d in [paths.NEW_ENGINE_DIR] + paths.new_engine_build_dirs()]


def new_library_path(required=True):
    """The new kernel's library: ``AVAS_ENGINE_LIB``, ``avas/engine/new/``, then ``engine/build``.
    None (or :class:`EngineChoiceError` when *required*) when there is none."""
    explicit = os.environ.get(LIB_ENV, "").strip()
    if explicit:
        if os.path.isfile(explicit) or not required:
            return explicit if os.path.isfile(explicit) else None
        raise EngineChoiceError(f"{LIB_ENV}={explicit} does not exist")
    for candidate in new_library_candidates():
        if os.path.isfile(candidate):
            return candidate
    if required:
        raise EngineChoiceError(f"the new kernel's library {new_library_name()} was not found (looked in "
                                f"{paths.NEW_ENGINE_DIR} and {paths.new_engine_build_dirs()[0]}); {BUILD_HINT}")
    return None


_sha1_cache = {}


def file_sha1(path):
    """SHA-1 of a file (cached per path, size and modification time); None when it does not exist."""
    try:
        st = os.stat(path)
    except OSError:
        return None
    key = (os.path.abspath(path), st.st_size, st.st_mtime_ns)
    if key not in _sha1_cache:
        h = hashlib.sha1()
        with open(path, "rb") as fh:
            for block in iter(lambda: fh.read(1 << 20), b""):
                h.update(block)
        _sha1_cache[key] = h.hexdigest()
    return _sha1_cache[key]


# --------------------------------------------------------------------------- engine source hash
THIRD_PARTY_SOURCE_EXT = (".h", ".hpp", ".c", ".cc", ".cpp", ".cu", ".cuh", ".mm", ".metal", ".inc")


def engine_source_files(root):
    """Relative paths ("/" separated, sorted) hashed by :func:`engine_source_hash` (the same files as
    engine/cmake/source_hash.cmake)."""
    rel = ["CMakeLists.txt"]
    for sub in ("include", "src", "third_party"):
        base = os.path.join(root, sub)
        for dirpath, _dirs, files in os.walk(base):
            for name in files:
                r = os.path.relpath(os.path.join(dirpath, name), root).replace(os.sep, "/")
                if sub == "third_party" and os.path.splitext(name)[1].lower() not in THIRD_PARTY_SOURCE_EXT:
                    continue
                rel.append(r)
    rel = [r for r in rel if not any(part.startswith(".") for part in r.split("/"))]
    return sorted(rel, key=lambda r: r.encode("utf-8"))


def engine_source_hash(root=None):
    """Engine source hash (64 hex digits) of the engine folder *root* (default: the source tree's
    engine/): SHA-256 of "<path>\\n<SHA-256 of the file with CRLF -> LF>\\n" over the sorted files.
    The library reports the first 16 digits as its version."""
    root = root or os.path.join(paths.SOURCE_ROOT, "engine")
    acc = hashlib.sha256()
    for rel in engine_source_files(root):
        with open(os.path.join(root, *rel.split("/")), "rb") as fh:
            data = fh.read().replace(b"\r\n", b"\n")
        acc.update(f"{rel}\n{hashlib.sha256(data).hexdigest()}\n".encode("utf-8"))
    return acc.hexdigest()


# --------------------------------------------------------------------------- result tiers
def result_tier(kernel, device=None, precision=None, compat=None):
    """Result tier of a run: original, new-compat-dll, new-fp32 (CUDA, Metal and emu fp32 are byte
    identical), new-cpu-fp64, new-cuda-fp64 (the device back end in fp64: CUDA, and emu which
    reproduces it on the CPU).  None when the device is unknown (the run tracked no particles)."""
    if kernel != "new":
        return TIER_ORIGINAL
    if compat == "dll":
        return TIER_DLL
    if not device:
        return None
    if precision == "fp32" or device == "metal":
        return TIER_FP32
    if device == "cpu":
        return TIER_CPU
    return TIER_CUDA64


def input_keywords(input_dir, name):
    """First value of every keyword of a text input file (``{keyword: value}``, keywords lower case)."""
    out = {}
    try:
        with open(os.path.join(input_dir, name), encoding="utf-8", errors="replace") as fh:
            for line in fh:
                parts = line.split("!", 1)[0].split()
                if len(parts) >= 2:
                    out[parts[0].lower()] = parts[1]
    except OSError:
        pass
    return out


def input_space_charge(input_dir, spacecharge=None):
    """Whether a run of the inputs in *input_dir* has space charge (as the engine decides: input.txt
    ``spacecharge``, ``secondarybeam``; beam.txt current 0 or one particle switch it off).  *spacecharge*
    (bool) replaces the value of input.txt (a form that is not saved yet)."""
    def num(d, key, default):
        try:
            return float(d.get(key, default))
        except ValueError:
            return default

    inp, beam = input_keywords(input_dir, "input.txt"), input_keywords(input_dir, "beam.txt")
    sc_on = num(inp, "spacecharge", 1) == 1 if spacecharge is None else bool(spacecharge)
    if not sc_on or num(inp, "secondarybeam", 0) == 1:
        return False
    if num(beam, "current", 1) == 0 or num(beam, "particlenumber", 2) == 1:
        return False
    return True


def space_charge_size(input_dir):
    """``(particles, mesh cells)`` of a run of the inputs in *input_dir* as the new kernel counts them
    (numofgrid, else its defaults: FFT 64^3, SPICNIC / PICNIC by particle number); None when the particles
    come from a distribution file (beam.txt ``readparticledistribution`` other than ``unknown``)."""
    beam, inp = input_keywords(input_dir, "beam.txt"), input_keywords(input_dir, "input.txt")
    if beam.get("readparticledistribution", "unknown") != "unknown":
        return None
    try:
        n = int(float(beam["particlenumber"]))
    except (KeyError, ValueError):
        return None
    grid = None
    try:
        with open(os.path.join(input_dir, "input.txt"), encoding="utf-8", errors="replace") as fh:
            for line in fh:
                parts = line.split("!", 1)[0].split()
                if len(parts) >= 4 and parts[0].lower() == "numofgrid":
                    grid = [int(float(v)) for v in parts[1:4]]
    except (OSError, ValueError):
        grid = None
    if not grid or min(grid) < 1:
        if inp.get("scmethod", "SPICNIC").upper() == "FFT":     # the kernel's default solver is SPICNIC
            g = 64
        else:
            g = next(v for lim, v in ((3e4, 14), (8e4, 18), (3e5, 20), (8e5, 26), (3e6, 30), (8e6, 40), (3e7, 44),
                                      (8e7, 56), (float("inf"), 64)) if n <= lim)
        grid = [g, g, g]
    return n, grid[0] * grid[1] * grid[2]


def lattice_has_rfq(input_dir):
    """Whether the lattice run from *input_dir* (ini.ini [lattice] source) has an active rfq element."""
    try:
        with open(paths.lattice_source_path(input_dir), encoding="utf-8", errors="replace") as fh:
            text = fh.read()
    except OSError:
        return False
    from avas.data.lattice_doc import LatticeDocument
    return any(st.key == "rfq" for st in LatticeDocument(text).elements())


def boundary_superpose(input_dir):
    """input.txt ``boundary 1`` and a ``superpose`` block in boundary.txt (after its ``start`` line).  AVAS.dll 2026-04-02
    and the new kernel's mode compat=dll stop with an exception (stoi) while reading such a block, before any output
    (spec 10 2.9.2; cases EX_A47, EX_A47b); only the new kernel's default mode runs it (M10 D5)."""
    if input_keywords(input_dir, "input.txt").get("boundary", "0") != "1":
        return False
    try:
        with open(os.path.join(input_dir, "boundary.txt"), encoding="utf-8", errors="replace") as fh:
            lines = fh.read().splitlines()
    except OSError:
        return False
    started = False
    for line in lines:
        words = line.split()          # also drops the \r of a CRLF line
        if not words:
            continue
        if not started:
            started = words[0] == "start"
        elif words[0] == "superpose":
            return True
    return False


def rf_at_superpose_start(input_dir):
    """Line numbers (from 1, as the editor shows them) of RF cavities (field type 1) at the start of a superpose block in the lattice run from *input_dir*
    (the superpose line before the cavity has z0 = 0; "case A" of spec 10 2.6.5.2).  AVAS.dll 2026-04-02 and compat=dll
    load no field for such a block: the phase scan fails ("Scan phase error") or phi_s is NaN and the beam is not
    accelerated (M10-notes 6.5); the new kernel's default mode builds the block correctly."""
    try:
        with open(paths.lattice_source_path(input_dir), encoding="utf-8", errors="replace") as fh:
            text = fh.read()
    except OSError:
        return []
    from avas.data.lattice_doc import LatticeDocument, to_float
    out, in_block, z0 = [], False, None
    for st in LatticeDocument(text).statements:
        if not st.active:
            continue
        if st.key == "superpose":
            in_block, z0 = True, to_float(st.param(0))
        elif st.key in ("superposeend", "superposeout"):
            in_block, z0 = False, None
        elif st.is_element:
            if in_block and z0 == 0.0 and st.field_type() == "1":
                out.append(st.line_no + 1)            # line_no counts from 0; the editor shows line_no + 1
            z0 = None
    return out


def original_kernel_rules(choice):
    """Whether *choice* runs with the original kernel's behaviour (the original kernel, or the new one with compat=dll)."""
    return not choice.is_new or choice.compat == "dll"


# one literal: tests/test_design_rules.py finds the translated text (zh_CN.json) in the sources
BOUNDARY_SUPERPOSE_ERROR = "boundary.txt contains a superpose block, which the original kernel and the mode “Reproduce AVAS.dll” cannot run (AVAS.dll 2026-04-02 stops with an exception while it reads boundary.txt). Choose the new kernel in its default mode in Simulation settings → Computing kernel, or remove the block from boundary.txt."  # noqa: E501


def rf_superpose_warning(lines):
    """Warning (en, zh) for :func:`rf_at_superpose_start` with the original kernel or compat=dll, None without lines."""
    if not lines:
        return None
    at = ", ".join(str(n) for n in lines)
    return (f"Lattice line {at}: an RF cavity at the start of a superpose block gets no field with the original kernel "
            "and in the mode “Reproduce AVAS.dll” (AVAS.dll 2026-04-02): the phase scan fails or phi_s is NaN, and the "
            "beam is not accelerated. The new kernel's default mode computes this block correctly.",
            f"lattice 第 {at} 行：位于 superpose 块开头的射频腔在原内核和“复现 AVAS.dll”模式（AVAS.dll 2026-04-02）中"
            "没有场：扫相位失败或 φs 为 NaN，束流不加速。新内核的默认模式能正确计算这种叠加块。")


# one literal (translated in zh_CN.json); {lines}: boundary.txt line numbers
BOUNDARY_SHAPE_ERROR = "boundary.txt line {lines}: shapes 3 (rectangular pipe) and 4 (elliptical pipe) are only supported by the new kernel's default mode; the original kernel and the mode “Reproduce AVAS.dll” stop with “Undefined shape exists, check boundary.txt”. Choose the new kernel in its default mode in Simulation settings → Computing kernel."  # noqa: E501


def _keyword_lines(path):
    """``[(line number, [words])]`` of a text input file (comments after ``!`` dropped, blank lines skipped)."""
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            lines = fh.read().splitlines()
    except OSError:
        return []
    out = []
    for number, line in enumerate(lines, 1):
        words = line.split("!", 1)[0].split()
        if words:
            out.append((number, words))
    return out


def boundary_new_shapes(input_dir):
    """Line numbers of boundary.txt shapes 3 and 4 (rectangular and elliptical pipes, M10 D5) after its ``start`` line,
    with input.txt ``boundary 1``.  AVAS.dll 2026-04-02 and compat=dll know shapes 1 and 2 only and stop with
    "Undefined shape exists, check boundary.txt" while reading boundary.txt (ErrorLog of case M10_D5_rect_ellipse_dst)."""
    if input_keywords(input_dir, "input.txt").get("boundary", "0") != "1":
        return []
    out, started = [], False
    for number, words in _keyword_lines(os.path.join(input_dir, "boundary.txt")):
        if not started:
            started = words[0] == "start"
        elif words[0] in ("3", "4"):
            out.append(number)
    return out


def new_only_inputs(input_dir):
    """Inputs only the new kernel's default mode reads (schema ``new_only``, M10), which the original kernel and
    compat=dll silently ignore or handle differently: warnings ``[(en, zh)]`` for a run with :func:`original_kernel_rules`.
    Boundary shapes 3 and 4 are refused instead (:func:`boundary_new_shapes`)."""
    out = []
    beam = sorted({words[0].lower() for _, words in _keyword_lines(os.path.join(input_dir, "beam.txt"))}
                  & {"initpos", "initmom"})
    if beam:
        names = " / ".join(beam)
        out.append((f"beam.txt has {names}, which the original kernel and the mode “Reproduce AVAS.dll” ignore: they "
                    "read the single-particle offset from displacepos / displacedpos only. Saving the Beam page writes "
                    "the offset as displacepos / displacedpos.",
                    f"beam.txt 写了 {names}，原内核和“复现 AVAS.dll”模式会忽略它们，只从 displacepos／displacedpos "
                    "读取单粒子偏移。在束流页保存后，偏移会写成 displacepos／displacedpos。"))
    for number, words in _keyword_lines(os.path.join(input_dir, "input.txt")):
        if words[0].lower() == "lpboundary" and len(words) > 2 and words[2] == "1":
            out.append(("input.txt: lpboundary V2 = 1 (periodic space-charge box) is ignored by the original kernel and "
                        "the mode “Reproduce AVAS.dll”, which read V1 (fold into one RF period) only.",
                        "input.txt：lpboundary V2 = 1（周期空间电荷盒）会被原内核和“复现 AVAS.dll”模式忽略，"
                        "它们只读 V1（折叠到一个射频周期）。"))
            break
    try:
        with open(paths.lattice_source_path(input_dir), encoding="utf-8", errors="replace") as fh:
            text = fh.read()
    except OSError:
        text = ""
    if text:
        from avas.data.lattice_doc import LatticeDocument
        named, rf_off = [], []
        for st in LatticeDocument(text).statements:
            if not st.active:
                continue
            if st.key == "outputplane" and len(st.params) > 1 and str(st.params[1]).strip():
                named.append(st.line_no + 1)
            elif st.key == "field" and st.rf_off():
                rf_off.append(st.line_no + 1)
        if named:
            at = ", ".join(str(n) for n in named)
            out.append((f"Lattice line {at}: the original kernel and the mode “Reproduce AVAS.dll” ignore the name of "
                        "an output plane and write outData_<z>.dst.",
                        f"lattice 第 {at} 行：原内核和“复现 AVAS.dll”模式忽略输出面的名称，文件仍为 outData_<z>.dst。"))
        if rf_off:
            at = ", ".join(str(n) for n in rf_off)
            out.append((f"Lattice line {at}: an RF field with Ke = 0 is a drift for the new kernel's default mode only; "
                        "the original kernel and the mode “Reproduce AVAS.dll” still load the field map and scan its "
                        "phase, which usually fails.",
                        f"lattice 第 {at} 行：Ke = 0 的射频场只有新内核默认模式当作漂移段；原内核和“复现 AVAS.dll”"
                        "模式仍读取场图并扫描相位，通常会失败。"))
    return out


RFQ_CPU_PLAN = {"device": "cpu", "precision": "fp64", "gpus": [], "error": None, "reason_code": "rfq_cpu_only",
                "reason": "auto: the rfq element is computed on the CPU only"}


def size_adjusted_plan(plan, input_dir):
    """*plan* (a device plan of :func:`describe` ``resolved``) for the run size of *input_dir*: device auto
    reports Metal with ``cpu_below_particles`` and ``cpu_below_particles_per_cell``, CUDA with
    ``cpu_below_particles``, and the kernel then uses the CPU for a run with fewer particles, or fewer per
    space-charge mesh cell (reason code ``few_particles``, the same text as the kernel)."""
    limit = (plan or {}).get("cpu_below_particles_per_cell")
    least = (plan or {}).get("cpu_below_particles") or 0
    size = space_charge_size(input_dir) if (limit or least) and input_dir else None
    if not size:
        return plan
    n, cells = size
    gpu = "Metal" if plan.get("device") == "metal" else "CUDA"
    if n < least:
        reason = f"auto: {n} particles (fewer than {least}): the CPU is faster than {gpu}"
    elif limit and n < limit * cells:
        reason = (f"auto: {n} particles for {cells} space-charge mesh cells (fewer than {limit:g} per cell): "
                  f"the CPU is faster than {gpu}")
    else:
        return plan
    return {"device": "cpu", "precision": "fp64", "gpus": [], "error": None, "reason_code": "few_particles", "reason": reason}


# --------------------------------------------------------------------------- queries
def describe(choice=None, input_dir=None, load_new=True):
    """Both kernels for ``avas info`` / ``avas doctor`` (this process loads the new library):
    ``{"original": {file, path, exists, sha1}, "new": {path, sha1, info, devices, error}, "choice", "resolved"}``.
    *resolved* is what a run of the choice gets with and without space charge (new kernel only)."""
    orig = original_library_path()
    out = {"original": {"file": os.path.basename(orig), "path": orig, "exists": os.path.isfile(orig),
                        "sha1": file_sha1(orig), "available": original_available()},
           "platform": {"label": platform_label(), "system": platform.system(), "machine": platform.machine(),
                        "default_kernel": default_kernel()},
           "new": {"path": None, "candidates": new_library_candidates(), "error": None},
           "choice": choice.to_dict() if choice else None, "resolved": None}
    try:
        lib = new_library_path()
    except EngineChoiceError as exc:
        out["new"]["error"] = str(exc)
        lib = None
    if lib:
        out["new"].update(path=lib, sha1=file_sha1(lib))
    if lib and load_new:
        from avas.core.MultiParticleEngine import load_new_library
        try:
            handle = load_new_library(lib)
            out["new"]["info"] = handle.info()
            opts = (choice if choice and choice.is_new else EngineChoice(kernel="new")).engine_options()
            handle.set_options(opts)
            devices = handle.devices()
            out["new"]["devices"] = devices
            if choice and choice.is_new and devices:
                out["resolved"] = devices.get("selected")
        except (OSError, ValueError) as exc:
            out["new"]["error"] = f"cannot load {lib}: {exc}"
    return out


_query_cache = {}


def clear_query_cache():
    """Forget the results of :func:`query_subprocess` (devices may have changed: driver, eGPU, new build)."""
    _query_cache.clear()


def query_command(choice):
    """``avas info --engine-json`` of the installed program or of this source tree."""
    args = ["info", "--engine-json"] + choice.cli_args()
    if getattr(sys, "frozen", False):
        console = os.path.join(os.path.dirname(sys.executable), "AVAS.exe")
        return [console if os.path.isfile(console) else sys.executable] + args
    return [sys.executable, "-m", "avas"] + args


def query_subprocess(choice, timeout=120):
    """:func:`describe` of *choice* computed in a child process (the library is never loaded here),
    cached per library file (size, time) and choice."""
    lib = new_library_path(required=False) if choice.is_new else None
    try:
        st = os.stat(lib) if lib else None
    except OSError:
        st = None
    key = (lib, st.st_size if st else None, st.st_mtime_ns if st else None, choice)
    if key in _query_cache:
        return _query_cache[key]
    env = {k: v for k, v in os.environ.items() if k not in ENV.values()}
    root = os.path.dirname(paths.PACKAGE_DIR)
    env["PYTHONPATH"] = root + (os.pathsep + env["PYTHONPATH"] if env.get("PYTHONPATH") else "")
    env.pop("AVAS_LATTICE", None)
    flags = subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0
    try:
        res = subprocess.run(query_command(choice), capture_output=True, text=True, env=env, timeout=timeout,
                             stdin=subprocess.DEVNULL, creationflags=flags)
    except (OSError, subprocess.SubprocessError) as exc:
        raise EngineChoiceError(f"cannot query the new kernel: {exc}") from exc
    text = res.stdout.strip()
    start = text.find("{")
    try:
        data = json.loads(text[start:]) if start >= 0 else None
    except ValueError:
        data = None
    if res.returncode != 0 or not isinstance(data, dict):
        tail = (res.stderr.strip().splitlines() or res.stdout.strip().splitlines() or ["no output"])[-1]
        raise EngineChoiceError(f"cannot query the new kernel: {tail}")
    _query_cache[key] = data
    return data


def predicted_profile(choice, space_charge, query=None):
    """``{"tier", "version", "device", "precision", "gpus"}`` a run of *choice* will have, or
    :class:`EngineChoiceError` when such a run would fail (missing library, device).  *query*:
    function choice -> :func:`describe` result (default :func:`query_subprocess`)."""
    if not choice.is_new:
        return {"tier": TIER_ORIGINAL, "version": None}
    data = (query or query_subprocess)(choice)
    new = data.get("new") or {}
    if new.get("error"):
        raise EngineChoiceError(new["error"])
    resolved = (data.get("resolved") or {}).get("sc" if space_charge else "nosc")
    if not resolved:
        raise EngineChoiceError("the new kernel did not report its device choice (library too old?)")
    if resolved.get("error"):
        raise EngineChoiceError(device_error(choice, resolved["error"]))
    return {"tier": result_tier("new", resolved["device"], resolved["precision"], choice.compat),
            "version": (new.get("info") or {}).get("version"), "device": resolved["device"],
            "precision": resolved["precision"], "gpus": resolved.get("gpus") or [],
            "lib_sha1": new.get("sha1")}


def device_error(choice, reason):
    """The message for a device the new kernel cannot use (never silently replaced by the CPU)."""
    prefix = f"device {choice.device}: "
    if reason.startswith(prefix):
        reason = reason[len(prefix):]
    return (f"the new kernel cannot use device '{choice.device}': {reason}. "
            "Choose device auto (the best available) or cpu")


def gpu_count(data):
    """Usable CUDA GPUs reported by a :func:`describe` result."""
    devices = ((data.get("new") or {}).get("devices") or {}).get("cuda") or {}
    return [d["index"] for d in devices.get("devices") or []]


# --------------------------------------------------------------------------- run record
_last_engine = None


def note_engine(engine):
    """Remember the engine object of this process (its last run goes into avas_run.json)."""
    global _last_engine
    _last_engine = engine


def run_record(choice=None, legacy_device=None):
    """The ``engine`` entry of avas_run.json: ``{"kernel": "original", "file", "sha1"}`` or, for the
    new kernel, also ``version``, the ``device``, ``precision``, ``gpus``, ``compat``, ``threads`` and
    ``space_charge`` of the last run (``avas_engine_last_run()``), the ``tier`` and the request."""
    choice = choice or active_choice()
    if legacy_device == "gpu" and not choice.is_new:
        lib = paths.engine_file("libPIC.so")
        return {"kernel": "original", "device": "gpu", "file": "libPIC.so", "sha1": file_sha1(lib)}
    eng = _last_engine
    if not choice.is_new:
        path = eng.dll_path if eng is not None and getattr(eng, "kernel", "original") == "original" else original_library_path()
        return {"kernel": "original", "file": os.path.basename(path), "sha1": file_sha1(path), "tier": TIER_ORIGINAL}
    rec = {"kernel": "new", "file": None, "sha1": None, "version": None, "device": None, "precision": None,
           "gpus": None, "compat": choice.compat, "threads": None, "space_charge": None, "tier": None,
           "requested": choice.to_dict()}
    if eng is None or getattr(eng, "kernel", None) != "new":
        return rec
    rec.update(file=eng.dll_path, sha1=file_sha1(eng.dll_path))
    info = eng.info() or {}
    rec["version"] = info.get("version")
    last = eng.last_run() or {}
    if last.get("ran"):
        rec.update(device=last.get("device"), precision=last.get("precision"), gpus=last.get("gpus"),
                   compat=last.get("compat"), threads=last.get("threads"), space_charge=last.get("space_charge"))
        rec["tier"] = result_tier("new", rec["device"], rec["precision"], rec["compat"])
        if choice.device == "auto" and rec["device"]:
            if "reason_code" in last:     # the reason of the run itself (libraries since 2026-11-14)
                rec["reason"], rec["reason_code"] = last.get("reason"), last.get("reason_code")
            else:
                rec["reason"] = _auto_reason(eng, rec["device"], rec["space_charge"])
    return rec


def _auto_reason(eng, device, space_charge):
    """Why device auto ran on *device*, for libraries without ``reason_code`` in avas_engine_last_run():
    the engine's plan (a device that failed at run time is named)."""
    try:
        plans = (eng.devices() or {}).get("selected") or {}
    except (OSError, ValueError):
        return None
    plan = plans.get("nosc" if space_charge is False else "sc") or {}
    if plan.get("device") and plan["device"] != device:
        return f"auto: {plan['device']} could not be used at run time (see the log)"
    return plan.get("reason")


def with_overrides(choice, **kwargs):
    """*choice* with some values replaced (validated)."""
    values = {k: parse_value(k, v, "override") for k, v in kwargs.items() if v is not None}
    out = replace(choice, **values)
    validate(out)
    return out
