"""The simulation kernels in the GUI: the original kernel (AVAS.dll / libAVAS.so) and the new one (engine/).

* ``engine.info``: both kernels and this computer's devices (About window, Simulation settings).  The new
  library is never loaded in the GUI process: a child runs ``avas info --engine-json``
  (:func:`avas.engines.query_subprocess`, cached per library file and choice until ``refresh``).
* ``engine.preview``: what a choice of the Simulation settings form would run on for this project.
* ``engine.check`` / :func:`require_ready`: the check before every run (run page, segment runs, the Study
  page's engine runs, the assistant's sandbox) -- a clear message instead of a failing child process.
* :func:`summary`: the ``engine`` entry of a run record (avas_run.json) shortened for the labels of the
  Run / Results pages; old records without it give ``None``.
* :func:`settings_form` / :func:`save_settings`: the "Computing kernel" group of the Simulation settings
  page (ini.ini ``[engine]``, written only when it changed).
"""
import json
import os

from avas import engines
from avas.engines import EngineChoice, EngineChoiceError
from avas.gui import context
from avas.gui.bridge import UserError, rpc
from avas.gui.project import RUN_INFO_FILE
from avas.utils.iniconfig import read_engine_settings, write_engine_settings

DEVICE_NAMES = {"cpu": "CPU", "cuda": "CUDA", "metal": "Metal", "emu": "emu", "auto": "auto"}


def _query(choice):
    """``(describe result, error text)`` of *choice* from the child process."""
    try:
        return engines.query_subprocess(choice), None
    except EngineChoiceError as exc:
        return None, str(exc)


def _plan(plan):
    plan = plan or {}
    return {"device": plan.get("device"), "precision": plan.get("precision"), "gpus": plan.get("gpus") or [],
            "reason": plan.get("reason"), "error": plan.get("error")}


@rpc("engine.info")
def info(refresh=False):
    """Both kernels, the new kernel's build and this computer's devices; *refresh* queries again."""
    if refresh:
        engines.clear_query_cache()
    data, error = _query(EngineChoice(kernel="new"))
    orig_path = engines.original_library_path()
    out = {"platform": engines.platform_label(), "originalAvailable": engines.original_available(),
           "defaultKernel": engines.default_kernel(),
           "original": {"file": os.path.basename(orig_path), "exists": os.path.isfile(orig_path),
                        "sha1": engines.file_sha1(orig_path)},
           "new": {"available": False, "path": None, "sha1": None, "version": None, "sourceHash": None, "build": None,
                   "compiler": None, "error": error},
           "devices": None, "auto": None, "error": error}
    if data is None:
        return out
    new = data.get("new") or {}
    lib_info = new.get("info") or {}
    build = lib_info.get("build") or {}
    out["new"].update(path=new.get("path"), sha1=new.get("sha1"), version=lib_info.get("version"),
                      sourceHash=lib_info.get("source_hash"), compiler=lib_info.get("compiler"),
                      error=new.get("error"),
                      build={"cuda": bool(build.get("cuda")), "metal": bool(build.get("metal")), "mpi": bool(build.get("mpi")),
                             "cudaArchs": build.get("cuda_archs") or [], "buildType": build.get("build_type"),
                             "platform": lib_info.get("platform")} if lib_info else None)
    out["new"]["available"] = bool(new.get("path") and lib_info and not new.get("error"))
    devices = new.get("devices") or {}
    if devices:
        cuda = devices.get("cuda") or {}
        metal = devices.get("metal") or {}
        out["devices"] = {
            "cuda": [{"index": d.get("index"), "name": d.get("name"), "compute": d.get("compute"),
                      "fp64Full": bool(d.get("fp64_full")), "memoryGb": d.get("memory_gb")} for d in cuda.get("devices") or []],
            "cudaCompiled": bool(cuda.get("compiled")), "cudaNote": cuda.get("note"),
            "metal": [{"name": d.get("name"), "usable": bool(d.get("usable"))} for d in metal.get("devices") or []],
            "metalCompiled": bool(metal.get("compiled")), "metalNote": metal.get("note"),
            "cpuThreads": (devices.get("cpu") or {}).get("threads"),
        }
        auto = devices.get("auto") or {}
        out["auto"] = {"sc": _plan(auto.get("sc")), "nosc": _plan(auto.get("nosc"))}
    return out


def choice_from_form(form):
    """The :class:`EngineChoice` of a settings form (the environment and ini.ini do not take part)."""
    form = form or {}
    cli = {k: form.get(k) for k in engines.KEYS if form.get(k) not in (None, "")}
    return engines.resolve_choice(cli=cli, input_dir=None, env={})


def _space_charge(spaceCharge=None):
    p = context.project()
    if not p.is_open:
        return True if spaceCharge is None else bool(spaceCharge)
    return engines.input_space_charge(p.input_dir, spaceCharge)


@rpc("engine.preview")
def preview(choice, spaceCharge=None):
    """What a run of this project would use with the form's *choice*: ``{kernel, spaceCharge, plan, other,
    error}`` (*plan* for this project's space charge, *other* for the opposite case; *spaceCharge* is the
    form's value when given)."""
    try:
        ch = choice_from_form(choice)
    except EngineChoiceError as exc:
        return {"kernel": (choice or {}).get("kernel"), "error": str(exc), "plan": None, "other": None}
    if not ch.is_new:
        return {"kernel": "original", "error": None, "plan": None, "other": None}
    sc = _space_charge(spaceCharge)
    data, error = _query(ch)
    if data is None or (data.get("new") or {}).get("error"):
        return {"kernel": "new", "spaceCharge": sc, "error": error or data["new"]["error"], "plan": None, "other": None}
    resolved = data.get("resolved") or {}
    return {"kernel": "new", "spaceCharge": sc, "error": None,
            "plan": _plan(resolved.get("sc" if sc else "nosc")), "other": _plan(resolved.get("nosc" if sc else "sc"))}


def require_ready(input_dir):
    """Check the kernel a run of *input_dir* will use (ini.ini ``[engine]`` under the environment, as the
    child process resolves it); :class:`UserError` with a message the user can act on.  Returns the
    expected engine (:func:`summary` form, ``predicted`` True)."""
    try:
        choice = engines.resolve_choice(input_dir=input_dir)
    except EngineChoiceError as exc:
        raise UserError("The computing kernel settings are invalid: {detail}", detail=str(exc)) from exc
    if not choice.is_new:
        if not engines.original_available():
            raise UserError("The original kernel does not run on {platform}. Choose the new kernel in Simulation settings → Computing kernel.", platform=engines.platform_label())
        if engines.input_keywords(input_dir, "input.txt").get("scmethod", "").upper() == "PICNIC":
            raise UserError("The PICNIC space-charge solver is only supported by the new kernel. Choose FFT or SPICNIC, or switch to the new kernel in Simulation settings → Computing kernel.")
        return {"kernel": "original", "predicted": True}
    data, error = _query(choice)
    if data is None:
        raise UserError("The new kernel could not be queried: {detail}", detail=error)
    new = data.get("new") or {}
    if new.get("error"):
        raise UserError("The new kernel's library is not available: {detail}", detail=new["error"])
    sc = engines.input_space_charge(input_dir)
    plan = (data.get("resolved") or {}).get("sc" if sc else "nosc") or {}
    if plan.get("error"):
        raise UserError("The new kernel cannot use the device chosen in Simulation settings ({device}) on this computer: {detail}. Choose Auto or CPU.", device=DEVICE_NAMES.get(choice.device, choice.device), detail=plan["error"])
    if not plan.get("device"):
        raise UserError("The new kernel did not report its device; rebuild it or update AVAS.")
    return {"kernel": "new", "device": plan["device"], "precision": plan.get("precision"), "gpus": plan.get("gpus") or [],
            "compat": choice.compat, "tier": engines.result_tier("new", plan["device"], plan.get("precision"), choice.compat),
            "requestedDevice": choice.device, "reason": plan.get("reason") if choice.device == "auto" else None,
            "version": (new.get("info") or {}).get("version"), "predicted": True}


@rpc("engine.check")
def check():
    """:func:`require_ready` for the open project."""
    return require_ready(context.project().require().input_dir)


# --------------------------------------------------------------------------- labels of run records
def summary(rec):
    """The kernel of a run record (the ``engine`` entry of avas_run.json) for labels, or None for records
    written before it existed (or anything else that is not a record)."""
    if not isinstance(rec, dict) or rec.get("kernel") not in engines.KERNELS:
        return None
    if rec["kernel"] == "original":
        return {"kernel": "original", "legacyGpu": rec.get("device") == "gpu"}
    requested = rec.get("requested") if isinstance(rec.get("requested"), dict) else {}
    return {"kernel": "new", "device": rec.get("device"), "precision": rec.get("precision"),
            "gpus": rec.get("gpus") or [], "compat": rec.get("compat") or requested.get("compat"),
            "tier": rec.get("tier"), "requestedDevice": requested.get("device"), "reason": rec.get("reason"),
            "version": rec.get("version")}


def read_summary(folder):
    """:func:`summary` of the avas_run.json in a results folder (None when there is none)."""
    try:
        with open(os.path.join(folder, RUN_INFO_FILE), encoding="utf-8") as fh:
            return summary(json.load(fh).get("engine"))
    except (OSError, ValueError, AttributeError):
        return None


# --------------------------------------------------------------------------- settings page
def _ini_path(input_dir):
    return os.path.join(input_dir, "ini.ini")


def settings_form(input_dir):
    """``{form, invalid, explicitKernel, env, originalAvailable, defaultKernel, platform}``: the ``[engine]``
    values of the project with the defaults filled in (*form* values are strings for the page), entries that
    are not valid, whether the kernel is written, environment variables that override the file for runs
    started here, and whether this platform has the original kernel."""
    raw = {str(k).strip().lower(): v for k, v in read_engine_settings(_ini_path(input_dir)).items()}
    form, invalid = {}, []
    for key in engines.KEYS:
        default = engines.default_kernel() if key == "kernel" else engines.DEFAULTS[key]
        value = raw.get(key)
        if value is None or str(value).strip() == "":
            form[key] = default
            continue
        try:
            form[key] = engines.parse_value(key, value, f"[engine] {key}")
        except EngineChoiceError:
            invalid.append(f"{key} = {value}")
            form[key] = default
    form["threads"] = str(form["threads"])
    env = [name for name in engines.ENV.values() if str(os.environ.get(name, "")).strip()]
    return {"form": form, "invalid": invalid, "explicitKernel": "kernel" in raw, "env": env,
            "originalAvailable": engines.original_available(), "defaultKernel": engines.default_kernel(),
            "platform": engines.platform_label()}


def _normalized(form):
    out = {}
    for key in engines.KEYS:
        default = engines.default_kernel() if key == "kernel" else engines.DEFAULTS[key]
        value = (form or {}).get(key)
        out[key] = default if value in (None, "") else engines.parse_value(key, value, key)
    return out


def validate_form(form):
    """The normalized values of a kernel form; :class:`UserError` for invalid values or combinations."""
    try:
        new = _normalized(form)
        if new["kernel"] == "new":
            engines.validate(EngineChoice(**new))
    except EngineChoiceError as exc:
        raise UserError("The computing kernel settings are invalid: {detail}", detail=str(exc)) from exc
    return new


def save_settings(input_dir, form):
    """Write the page's kernel form to ini.ini ``[engine]`` when it differs from the file (the kernel is
    written explicitly, the other keys only when they are not the default).  Returns whether it wrote.
    The caller holds the run lock check (settings.save)."""
    if form is None:
        return False
    new = validate_form(form)
    current = settings_form(input_dir)
    if new == _normalized(current["form"]) and not current["invalid"]:
        return False
    values = {"kernel": new["kernel"]}
    for key in engines.KEYS[1:]:
        values[key] = None if new[key] == engines.DEFAULTS[key] else new[key]
    write_engine_settings(_ini_path(input_dir), values)
    return True
