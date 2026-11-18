"""Command-line interface.

::

    avas run  --input DIR --output DIR [--field DIR] [--mode ...]
              [--engine original|new] [--device auto|cpu|cuda|metal] [--precision auto|fp64|fp32]
              [--gpus all|0,1] [--threads N] [--compat none|dll]
    avas plot TYPE --output DIR [--input DIR] [--save FILE] [--no-show]
    avas plot phase --dst FILE [--plane x-x1 --plane phi-w] [--save FILE]
    avas scan --input DIR --target Q1 --param G --values 10,12,14
    avas gui [--lang en|zh_CN]
    avas serve [--port 8765] [--host 127.0.0.1] [--open]
    avas info [--input DIR] [--engine-json]

``avas --input DIR --output DIR`` (without the word ``run``) is accepted too, so
``python run_avas.py --input ... --output ...`` starts a simulation directly.
"""
import argparse
import json
import os
import sys
import time

from avas import __version__
from avas.installation_lock import acquire

acquire()  # console-script / python -m entry: guard before loading application dependencies

from avas.post.plot.dataset_plots import DATASET_PLOTS, DST_PLOTS, LATTICE_PLOTS   # plot type registry (no matplotlib)

RUN_INFO_FILE = "avas_run.json"

PLOT_TYPES = list(DATASET_PLOTS) + list(LATTICE_PLOTS) + list(DST_PLOTS)

DEFAULT_PLANES = ["x-x1", "y-y1", "phi-w"]
SIM_MODES = ["auto", "basic", "stat", "dyn", "stat_dyn"]


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------
class CliError(Exception):
    pass


def _abs(path):
    return os.path.abspath(os.path.expanduser(path)) if path else path


def resolve_input_dir(path):
    """Accept either the input directory itself or a project directory
    containing ``InputFile/``."""
    path = _abs(path)
    if not os.path.isdir(path):
        raise CliError(f"input directory does not exist: {path}")
    if os.path.isfile(os.path.join(path, "input.txt")):
        return path
    nested = os.path.join(path, "InputFile")
    if os.path.isfile(os.path.join(nested, "input.txt")):
        return nested
    raise CliError(f"no input.txt found in {path} (or {nested})")


def check_input_files(input_dir):
    from avas.paths import lattice_source_path
    missing = [f for f in ("input.txt", "beam.txt") if not os.path.isfile(os.path.join(input_dir, f))]
    lattice = lattice_source_path(input_dir)
    if not os.path.isfile(lattice):
        missing.append(os.path.basename(lattice))
    if missing:
        raise CliError(f"missing input files in {input_dir}: {', '.join(missing)}")


def read_ini(input_dir):
    """Settings the GUI stores in ``ini.ini`` (error type, seed, device...)."""
    ini_path = os.path.join(input_dir, "ini.ini")
    if not os.path.isfile(ini_path):
        return {}
    from avas.utils.iniconfig import IniConfig
    res = IniConfig().create_from_file({"otherPath": ini_path})
    if not res or res.get("code", 0) != 0:
        return {}
    return res["data"]["iniParams"]


def write_run_info(output_dir, info):
    with open(os.path.join(output_dir, RUN_INFO_FILE), "w", encoding="utf-8") as fh:
        json.dump(info, fh, indent=2, ensure_ascii=False)


def read_run_info(output_dir):
    path = os.path.join(output_dir, RUN_INFO_FILE)
    if not os.path.isfile(path):
        return {}
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def guess_input_dir(output_dir, explicit=None):
    """Find the input directory that belongs to *output_dir*."""
    if explicit:
        return resolve_input_dir(explicit)
    info = read_run_info(output_dir)
    cand = info.get("input_dir")
    if cand and os.path.isdir(cand):
        return cand
    sibling = os.path.join(os.path.dirname(output_dir), "InputFile")
    if os.path.isfile(os.path.join(sibling, "input.txt")):
        return sibling
    raise CliError("cannot locate the input directory for this output; pass --input DIR")


# ---------------------------------------------------------------------------
# run
# ---------------------------------------------------------------------------
def cmd_run(args):
    import matplotlib
    matplotlib.use("Agg")  # simulations never need a window

    input_dir = resolve_input_dir(args.input)
    from avas.paths import LATTICE_ENV_VAR
    previous_override = os.environ.get(LATTICE_ENV_VAR)
    if args.lattice:
        os.environ[LATTICE_ENV_VAR] = args.lattice      # read by every module via avas.paths
    try:
        return _run(args, input_dir)
    finally:                                            # do not leak the override into later in-process calls
        if args.lattice:
            if previous_override is None:
                os.environ.pop(LATTICE_ENV_VAR, None)
            else:
                os.environ[LATTICE_ENV_VAR] = previous_override


NEW_ONLY_DEVICES = ("auto", "cuda", "metal", "emu")
RUN_DEVICES = ("auto", "cpu", "cuda", "metal", "gpu", "emu")


def engine_choice_from_args(args, input_dir):
    """``(EngineChoice, legacy device)`` of ``avas run`` / ``avas info``: the options of the command
    line over the environment over ini.ini ``[engine]`` (see :mod:`avas.engines`).  The legacy device
    is ``[input] device`` / ``--device cpu|gpu`` of the original kernel (gpu = libPIC.so, Linux)."""
    from avas import engines
    device = getattr(args, "device", None)
    if device is not None and device not in RUN_DEVICES:
        raise CliError(f"invalid --device '{device}' (expected auto | cpu | cuda | metal; gpu = the original "
                       "kernel's libPIC.so)")
    new_only = {k: getattr(args, k, None) for k in ("precision", "gpus", "threads", "compat")}
    try:
        # --threads applies to both kernels (the original kernel: OMP_NUM_THREADS, engines.original_thread_env)
        first = engines.resolve_choice(cli={"kernel": getattr(args, "engine", None), "threads": new_only["threads"]},
                                       input_dir=input_dir)
        kernel = first.kernel
        if kernel == "new":
            if device == "gpu":
                raise CliError("--device gpu selects the original kernel's libPIC.so; for the new kernel use "
                               "--device cuda (or auto)")
            choice = engines.resolve_choice(cli=dict(new_only, kernel="new", device=device), input_dir=input_dir)
            choice.sources.update({k: v for k, v in first.sources.items() if k in ("kernel", "note")})
            return choice, "cpu"
        given = [f"--{k}" for k, v in new_only.items() if v is not None and k != "threads"]
        if device in NEW_ONLY_DEVICES:
            given.insert(0, f"--device {device}")
        if given:
            raise CliError(f"{', '.join(given)} {'applies' if len(given) == 1 else 'apply'} only to the new kernel; "
                           "add --engine new "
                           "(or set kernel = new in ini.ini [engine])")
        choice = first                                   # keeps where the kernel was chosen (check_platform)
    except engines.EngineChoiceError as exc:
        raise CliError(str(exc)) from exc
    ini = read_ini(input_dir) if input_dir else {}
    return choice, device or ini.get("input", {}).get("device") or "cpu"


def _run(args, input_dir):
    from avas import engines
    from avas.paths import lattice_source_name
    check_input_files(input_dir)
    output_dir = _abs(args.output)

    ini = read_ini(input_dir)
    mode = args.mode
    if mode == "auto":
        mode = ini.get("error", {}).get("error_type") or "basic"
        if mode not in SIM_MODES:
            mode = "basic"
    choice, device = engine_choice_from_args(args, input_dir)
    try:
        engines.check_platform(choice)
    except engines.EngineChoiceError as exc:
        raise CliError(str(exc)) from exc
    if engines.original_kernel_rules(choice):
        # M11: limits of AVAS.dll 2026-04-02 that compat=dll reproduces
        if engines.boundary_superpose(input_dir):
            raise CliError("boundary.txt contains a superpose block, which the original kernel and compat=dll cannot "
                           "run (AVAS.dll 2026-04-02 stops with an exception while it reads boundary.txt); use the new "
                           "kernel's default mode (--engine new --compat none, or kernel = new and compat = none in "
                           "ini.ini [engine]) or remove the block")
        shapes = engines.boundary_new_shapes(input_dir)
        if shapes:
            raise CliError(engines.BOUNDARY_SHAPE_ERROR.format(lines=", ".join(str(n) for n in shapes))
                           .replace("Choose the new kernel in its default mode in Simulation settings → Computing kernel.",
                                    "Use the new kernel's default mode (--engine new --compat none)."))
        warning = engines.rf_superpose_warning(engines.rf_at_superpose_start(input_dir))
        if warning:
            print(f"[avas] warning: {warning[0]}")
        for note in engines.new_only_inputs(input_dir):
            print(f"[avas] warning: {note[0]}")
    if choice.is_new:
        # load the library and check a required device before anything is written
        from avas.core.MultiParticleEngine import MultiParticleEngine
        try:
            MultiParticleEngine(choice).check_device()
        except (engines.EngineChoiceError, ValueError) as exc:
            raise CliError(str(exc)) from exc
    os.makedirs(output_dir, exist_ok=True)
    seed = args.seed if args.seed is not None else int(ini.get("error", {}).get("seed", 50) or 50)
    if_normal = 0 if args.no_normal else int(ini.get("error", {}).get("if_normal", 1))
    field_dir = _abs(args.field) if args.field else (ini.get("project", {}).get("fieldSource") or None)

    item = {
        "project_path": os.path.dirname(input_dir),
        "input_file": input_dir,
        "output_file": output_dir,
        "field_path": field_dir,
        "device": device,
        "seed": seed,
        "if_normal": if_normal,
    }

    info = {
        "avas_version": __version__,
        "input_dir": input_dir,
        "output_dir": output_dir,
        # every mode simulates a staged copy of the text inputs (+ the generated lattice.txt), see api.basic
        "inputs_dir": os.path.join(output_dir, "inputs"),
        "field_dir": field_dir,
        "mode": mode,
        "device": device,
        "engine": engines.run_record(choice, device),
        "seed": seed,
        "lattice": lattice_source_name(input_dir),
        "lattice_sha1": _lattice_fingerprint(input_dir),
        "started": time.strftime("%Y-%m-%d %H:%M:%S"),
        "status": "running",
    }
    write_run_info(output_dir, info)

    print(f"[avas] mode={mode} device={device}" if not choice.is_new else f"[avas] mode={mode}")
    if choice.sources.get("note"):
        print(f"[avas] note: {choice.sources['note']}")
    if choice.is_new:
        print(f"[avas] engine: {choice.label()} {info['engine'].get('file') or ''}".rstrip())
    print(f"[avas] input : {input_dir}")
    print(f"[avas] lattice: {lattice_source_name(input_dir)}")
    print(f"[avas] output: {output_dir}")

    from avas.api import basic as api
    t0 = time.time()
    previous_choice = engines.set_active(choice)
    try:
        if mode == "basic":
            api.basic_mulp(**item)
        elif mode == "stat":
            api.err_stat(**item)
        elif mode == "dyn":
            api.err_dyn(**item)
        elif mode == "stat_dyn":
            api.err_stat_dyn(**item)
    except Exception as exc:
        info.update(status="failed", error=str(exc), finished=time.strftime("%Y-%m-%d %H:%M:%S"),
                    engine=engines.run_record(choice, device))
        write_run_info(output_dir, info)
        if isinstance(exc, engines.EngineChoiceError):
            raise CliError(str(exc)) from exc
        raise
    finally:
        engines.set_active(previous_choice)
    info["engine"] = engines.run_record(choice, device)       # what the (last) run actually used
    if choice.is_new and info["engine"].get("device"):
        e = info["engine"]
        print(f"[avas] engine used: {e['device']} {e['precision']}"
              f"{' GPUs ' + ','.join(str(g) for g in e['gpus']) if e.get('gpus') else ''}, {e['tier']}")
    info.update(status="finished", finished=time.strftime("%Y-%m-%d %H:%M:%S"),
                elapsed_s=round(time.time() - t0, 2))
    diagnostics = _diagnostics(output_dir) if mode == "basic" else None
    if diagnostics:
        info["diagnostics"] = diagnostics
    write_run_info(output_dir, info)
    if os.environ.get("AVAS_GUI_CHILD") != "1":          # the GUI logs them itself, translated
        for m in (diagnostics or {}).get("messages", []):
            print(f"[avas] {m['level']}: {m['text'][0]}")
    print(f"[avas] done in {info['elapsed_s']} s -> {output_dir}")
    return 0


def _lattice_fingerprint(input_dir):
    """Fingerprint of the lattice text this run uses (the GUI tells whether it was edited since)."""
    from avas.gui.textio import read_text, text_fingerprint
    from avas.paths import lattice_source_path
    try:
        return text_fingerprint(read_text(lattice_source_path(input_dir)))
    except OSError:
        return None


def _diagnostics(output_dir):
    """DataSet.txt health check (NaN columns, lost beam); never fails the run."""
    try:
        from avas.post.analysis.run_diagnostics import dataset_diagnostics
        return dataset_diagnostics(output_dir)
    except Exception:  # noqa: BLE001
        return None


# ---------------------------------------------------------------------------
# plot
# ---------------------------------------------------------------------------
def _parse_planes(planes):
    out = []
    for p in planes or DEFAULT_PLANES:
        parts = p.replace(",", "-").split("-")
        if len(parts) != 2:
            raise CliError(f"bad --plane '{p}', expected e.g. x-x1, y-y1, phi-w, z-z1")
        out.append(parts)
    return out


def _parse_ratio(pairs):
    ratio = {}
    for p in pairs or []:
        if "=" not in p:
            raise CliError(f"bad --ratio '{p}', expected NAME=VALUE")
        k, v = p.split("=", 1)
        ratio[k] = float(v)
    return ratio


def cmd_plot(args):
    import matplotlib
    show = not args.no_show and not args.save
    if not show:
        matplotlib.use("Agg")
    from avas.api import plotting as api

    save = _abs(args.save) if args.save else None
    kind = args.type

    if kind in DST_PLOTS:
        if not args.dst:
            raise CliError("plot phase needs --dst FILE (a .dst particle file)")
        dst = _abs(args.dst)
        if not os.path.isfile(dst):
            raise CliError(f"dst file not found: {dst}")
        fig = api.plot_dst({
            "dst_path": dst,
            "picture_type": _parse_planes(args.plane),
            "show_": 1 if show else 0,
            "platform": "qt",
        })
        if save and fig is not None:
            fig.savefig(save, dpi=args.dpi)
            print(f"[avas] saved {save}")
        return 0

    if not args.output:
        raise CliError(f"plot {kind} needs --output DIR (the simulation output directory)")
    output_dir = _abs(args.output)
    if not os.path.isdir(output_dir):
        raise CliError(f"output directory does not exist: {output_dir}")
    input_dir = guess_input_dir(output_dir, args.input)

    if kind in DATASET_PLOTS:
        if not os.path.isfile(os.path.join(output_dir, "DataSet.txt")):
            raise CliError(f"DataSet.txt not found in {output_dir}")
        fig = api.plot_dataset(pictureType=kind, inputDir=input_dir, outputDir=output_dir,
                               show_=1 if show else 0, sampleInterval=args.interval, savePath=None)
    elif kind == "cavity_voltage":
        ratio = _parse_ratio(args.ratio)
        if not ratio:
            raise CliError("plot cavity_voltage needs --ratio FIELD=VALUE (e.g. --ratio efield=1.0)")
        fig = api.plot_cavity_voltage(None, ratio, show_=1 if show else 0, input_dir=input_dir)
    elif kind == "syn_phase":
        fig = api.plot_cavity_syn_phase(inputDir=input_dir, show_=1 if show else 0)
    elif kind == "phase_advance":
        fig = api.plot_phase_advance(pictureType=args.unit, inputDir=input_dir, outputDir=output_dir,
                                     show_=1 if show else 0)
    else:  # pragma: no cover - argparse restricts choices
        raise CliError(f"unknown plot type {kind}")

    if save:
        if fig is None:
            raise CliError("nothing to save (plot returned no figure)")
        fig.savefig(save, dpi=args.dpi, bbox_inches="tight")
        print(f"[avas] saved {save}")
    return 0


# ---------------------------------------------------------------------------
# gui / info
# ---------------------------------------------------------------------------
def cmd_gui(args):
    from avas.gui.app import main as gui_main
    return gui_main(argv=[], language=args.lang)


def cmd_serve(args):
    from avas.gui.serve import main as serve_main
    return serve_main(args=args)


def _info_input_dir(args):
    """The project of ``avas info``: --input, else the current folder when it is one (or None)."""
    if getattr(args, "input", None):
        return resolve_input_dir(args.input)
    cwd = os.getcwd()
    for cand in (cwd, os.path.join(cwd, "InputFile")):
        if os.path.isfile(os.path.join(cand, "input.txt")):
            return cand
    return None


def _plan_text(plan):
    if not plan:
        return "?"
    if plan.get("error"):
        return f"fails: {plan['error']}"
    gpus = plan.get("gpus") or []
    return f"{plan['device']} {plan['precision']}" + (f" (GPUs {','.join(map(str, gpus))})" if gpus else "")


def print_kernels(desc, input_dir=None, choice=None, indent="  "):
    """The kernels, devices and the resolution of auto for ``avas info`` / ``avas doctor``."""
    orig = desc["original"]
    if orig.get("available", True):
        print(f"{indent}original kernel: {orig['path']}"
              + (f"  sha1 {orig['sha1'][:12]}" if orig.get("sha1") else "  (missing)"))
    else:
        plat = (desc.get("platform") or {}).get("label") or "this platform"
        print(f"{indent}original kernel: not available on {plat} (only the new kernel; it is the default here)")
    new = desc["new"]
    if new.get("path") and new.get("info"):
        info = new["info"]
        build = info.get("build") or {}
        opts = [name for name in ("cuda", "metal", "mpi") if build.get(name)]
        archs = ",".join(build.get("cuda_archs") or [])
        print(f"{indent}new kernel     : {new['path']}  version {info.get('version')}  "
              f"(CPU{''.join(' + ' + o.upper() if o != 'metal' else ' + Metal' for o in opts)}"
              f"{'; CUDA ' + archs if archs else ''}; {info.get('compiler')})")
    elif new.get("path"):
        print(f"{indent}new kernel     : {new['path']}  ({new.get('error') or 'no build information'})")
    else:
        print(f"{indent}new kernel     : not found. {new.get('error') or ''}".rstrip())
    devices = new.get("devices")
    if devices:
        cuda = devices.get("cuda") or {}
        cards = cuda.get("devices") or []
        if cards:
            for d in cards:
                print(f"{indent}  CUDA GPU {d['index']}: {d['name']} (sm {d['compute']}, "
                      f"{'full-rate' if d.get('fp64_full') else 'slow'} fp64, {d['memory_gb']:.1f} GB)")
        else:
            print(f"{indent}  CUDA: none ({cuda.get('note') or 'no device'})")
        metal = devices.get("metal") or {}
        if metal.get("devices"):
            for d in metal["devices"]:
                print(f"{indent}  Metal: {d['name']}{'' if d.get('usable') else ' (not usable: ' + str(metal.get('note')) + ')'}")
        else:
            print(f"{indent}  Metal: none ({metal.get('note') or 'no device'})")
        print(f"{indent}  CPU: {(devices.get('cpu') or {}).get('threads')} logical threads")
        auto = devices.get("auto") or {}
        print(f"{indent}  device auto: with space charge -> {_plan_text(auto.get('sc'))}; "
              f"without -> {_plan_text(auto.get('nosc'))}")
    if choice is not None:
        where = f"project {input_dir}" if input_dir else "selected"
        how = {"preference": " (default kernel preference)"}.get(choice.sources.get("kernel"), "")
        print(f"{indent}{where}: {choice.label()}{how}")
        if choice.sources.get("note"):
            print(f"{indent}  note: {choice.sources['note']}")
        if choice.is_new and desc.get("resolved"):
            res = desc["resolved"]
            print(f"{indent}  runs: with space charge -> {_plan_text(res.get('sc'))}; "
                  f"without -> {_plan_text(res.get('nosc'))}")


def cmd_info(args):
    from avas import engines
    input_dir = _info_input_dir(args)
    try:
        choice, _ = engine_choice_from_args(args, input_dir)
    except CliError as exc:
        if getattr(args, "engine_json", False):
            raise
        print(f"warning: {exc}", file=sys.stderr)        # still show the installation
        choice = None
    if getattr(args, "engine_json", False):
        print(json.dumps(engines.describe(choice, input_dir), ensure_ascii=False))
        return 0
    from avas.buildinfo import build_info
    from avas.paths import ENGINE_DIR, LOG_DIR, STATIC_DIR
    b = build_info()
    stamp = ", ".join(x for x in (f"built {b['built']}" if b.get("built") else "",
                                  f"commit {b['commit']}{'+' if b.get('dirty') else ''}" if b.get("commit") else "",
                                  "stand-alone" if b.get("frozen") else "source") if x)
    print(f"AVAS {__version__} ({stamp})")
    print(f"python : {sys.version.split()[0]} ({sys.executable})")
    print(f"engine : {ENGINE_DIR}")
    print(f"static : {STATIC_DIR}")
    print(f"logs   : {LOG_DIR}")
    print("kernels:")
    print_kernels(engines.describe(choice, input_dir), input_dir, choice)
    return 0


# ---------------------------------------------------------------------------
# parser
# ---------------------------------------------------------------------------
def build_parser():
    parser = argparse.ArgumentParser(
        prog="avas",
        description="AVAS - Advanced Virtual Accelerator Software",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "examples:\n"
            '  avas run --input "D:\\proj\\InputFile" --output "D:\\proj\\Results_001"\n'
            '  avas plot emittance_x --output "D:\\proj\\Results_001" --save emit_x.png\n'
            '  avas plot phase --dst "D:\\proj\\Results_001\\outData_0.210000.dst"\n'
            "  avas gui --lang zh_CN\n"
        ),
    )
    parser.add_argument("-V", "--version", action="version", version=f"avas {__version__}")
    sub = parser.add_subparsers(dest="command", metavar="COMMAND")

    # run ------------------------------------------------------------------
    p_run = sub.add_parser("run", help="run a simulation", description="Run a multi-particle simulation.")
    p_run.add_argument("-i", "--input", required=True, metavar="DIR",
                       help="input directory (contains input.txt, beam.txt and the lattice) "
                            "or a project directory containing InputFile/")
    p_run.add_argument("-o", "--output", required=True, metavar="DIR",
                       help="output directory (created if missing)")
    p_run.add_argument("-f", "--field", metavar="DIR", help="field-map directory (default: input dir)")
    p_run.add_argument("-l", "--lattice", metavar="FILE",
                       help="lattice file inside the input dir (default: [lattice] source in ini.ini, "
                            "else lattice_mulp.txt)")
    p_run.add_argument("-m", "--mode", choices=SIM_MODES, default="auto",
                       help="basic | stat | dyn | stat_dyn error study; auto = read ini.ini, else basic")
    p_run.add_argument("-d", "--device", metavar="DEVICE",
                       help="new kernel: auto | cpu | cuda | metal (default: ini.ini [engine] device, else auto; "
                            "cuda / metal fail when the machine has no such GPU); original kernel: cpu | gpu "
                            "(gpu = libPIC.so, Linux; default: ini.ini [input] device)")
    engine_arguments(p_run)
    p_run.add_argument("--seed", type=int, help="random seed for error studies")
    p_run.add_argument("--no-normal", action="store_true", help="error study: skip the error-free reference run")
    p_run.set_defaults(func=cmd_run)

    # plot -----------------------------------------------------------------
    p_plot = sub.add_parser("plot", help="plot simulation results",
                            description="Plot simulation results.\n\nTYPE is one of:\n"
                                        f"  DataSet based : {', '.join(DATASET_PLOTS)}\n"
                                        f"  lattice based : {', '.join(LATTICE_PLOTS)}\n"
                                        "  particle file : phase (needs --dst)",
                            formatter_class=argparse.RawDescriptionHelpFormatter)
    p_plot.add_argument("type", choices=PLOT_TYPES, metavar="TYPE")
    p_plot.add_argument("-o", "--output", metavar="DIR", help="simulation output directory")
    p_plot.add_argument("-i", "--input", metavar="DIR",
                        help="input directory (default: recorded in avas_run.json, or ../InputFile)")
    p_plot.add_argument("--dst", metavar="FILE", help="particle .dst file for TYPE=phase")
    p_plot.add_argument("--plane", action="append", metavar="A-B",
                        help="phase-space plane for TYPE=phase, repeatable "
                             f"(default: {' '.join(DEFAULT_PLANES)})")
    p_plot.add_argument("--ratio", action="append", metavar="FIELD=VALUE",
                        help="cavity_voltage: actual/ke ratio per field name, repeatable")
    p_plot.add_argument("--unit", choices=["period", "meter"], default="period",
                        help="phase_advance unit (default: period)")
    p_plot.add_argument("--interval", type=int, default=1, help="sample interval for DataSet plots")
    p_plot.add_argument("-s", "--save", metavar="FILE", help="save the figure instead of showing it")
    p_plot.add_argument("--dpi", type=int, default=200, help="dpi used with --save (default 200)")
    p_plot.add_argument("--no-show", action="store_true", help="do not open a window")
    p_plot.set_defaults(func=cmd_plot)

    # gui / info ------------------------------------------------------------
    p_gui = sub.add_parser("gui", help="start the graphical interface")
    p_gui.add_argument("--lang", choices=["en", "zh_CN"], help="UI language (default: last used)")
    p_gui.set_defaults(func=cmd_gui)

    from avas.gui.serve import add_arguments as serve_arguments
    p_serve = sub.add_parser("serve", help="serve the interface to a web browser",
                             description="Serve the AVAS interface to a web browser and print its URL (with the access token).")
    serve_arguments(p_serve)
    p_serve.set_defaults(func=cmd_serve)

    p_info = sub.add_parser("info", help="show version, install locations, kernels and devices",
                            description="Show the version, install locations, both kernels and the devices of "
                                        "this machine; in a project (--input or the current folder) also what "
                                        "its runs would use.")
    p_info.add_argument("-i", "--input", metavar="DIR", help="project or input directory (default: the current "
                                                              "folder when it is one)")
    p_info.add_argument("--engine-json", action="store_true", help="print the kernels and devices as JSON")
    p_info.add_argument("-d", "--device", metavar="DEVICE", help="new kernel device: auto | cpu | cuda | metal")
    engine_arguments(p_info)
    p_info.set_defaults(func=cmd_info)

    from avas.cli.scan_cmd import add_parser as add_scan_parser
    add_scan_parser(sub)

    p_doctor = sub.add_parser("doctor", help="check this installation (engine, WebView2, GUI, assistant, preview)")
    p_doctor.set_defaults(func=cmd_doctor)
    return parser


def engine_arguments(p):
    """``--engine --precision --gpus --threads --compat`` (the new kernel's options; ``--device`` is
    added by the command, since ``avas run --device`` also selects the original kernel's libPIC.so)."""
    g = p.add_argument_group("kernel", "Priority: these options > environment variables (AVAS_ENGINE, "
                                       "AVAS_ENGINE_DEVICE, ...) > ini.ini [engine] > defaults.")
    g.add_argument("--engine", choices=["original", "new"],
                   help="simulation kernel (default: ini.ini [engine] kernel, else original; new where the original "
                        "kernel does not run, e.g. macOS)")
    g.add_argument("--precision", choices=["auto", "fp64", "fp32"],
                   help="new kernel: auto = fp64 on full-rate fp64 GPUs and the CPU, fp32 on other GPUs and Metal")
    g.add_argument("--gpus", metavar="LIST", help="new kernel: all (default) or GPU numbers such as 0,1")
    g.add_argument("--threads", type=int, metavar="N", help="CPU threads, 0 = automatic (default); the original kernel gets it as OMP_NUM_THREADS")
    g.add_argument("--compat", choices=["none", "dll"],
                   help="new kernel: none = defects of the original corrected (default); dll = reproduce AVAS.dll "
                        "(CPU only)")


def cmd_doctor(args):
    """Quick self-test of an installation; exit code 1 when something essential fails."""
    import tempfile
    import traceback

    failures = 0

    def check(name, fn, essential=True):
        nonlocal failures
        try:
            detail = fn()
            print(f"  ok    {name}{': ' + str(detail) if detail else ''}")
        except Exception as exc:  # noqa: BLE001 - reported
            if essential:
                failures += 1
            print(f"  {'FAIL' if essential else 'warn'}  {name}: {exc}")
            if os.environ.get("AVAS_DEBUG"):
                traceback.print_exc()

    from avas import engines
    try:
        selected = engines.resolve_choice(input_dir=_info_input_dir(args)).kernel
    except (engines.EngineChoiceError, CliError):
        selected = "original"

    def original_engine():
        from avas.core.MultiParticleEngine import MultiParticleEngine
        return MultiParticleEngine(engines.EngineChoice(kernel="original")).dll_path

    def new_engine():
        desc = engines.describe(engines.EngineChoice(kernel="new"))
        new = desc["new"]
        if not new.get("info"):
            raise RuntimeError(new.get("error") or "the library reports no build information")
        devices = new.get("devices") or {}
        auto = devices.get("auto") or {}
        build = new["info"].get("build") or {}
        extras = "".join(f" + {n}" for n, k in (("CUDA", "cuda"), ("Metal", "metal"), ("MPI", "mpi")) if build.get(k))
        return (f"{new['path']} (version {new['info'].get('version')}, CPU{extras}; auto: space charge -> "
                f"{_plan_text(auto.get('sc'))}, none -> {_plan_text(auto.get('nosc'))})")

    def webview():
        if sys.platform != "win32":
            return "not Windows"
        from avas.gui.webview2 import installed_version
        v = installed_version()
        if not v:
            raise RuntimeError("Microsoft Edge WebView2 runtime is not installed")
        return v

    def gui_services():
        from avas.gui import bridge, services  # noqa: F401
        return f"{len(bridge.handlers())} RPC handlers"

    def assistant():
        from avas.ai import PRESETS
        from avas.ai.avas_tools import build_tools
        from avas.ai.manual import search
        tools = build_tools(object())
        hits = search("superpose")
        if not hits.get("sections"):
            raise RuntimeError("the user manual text is missing")
        return f"{len(tools)} tools, {len(PRESETS)} provider presets, manual ok"

    def preview():
        from avas.sim.linear_optics import linear_preview
        beam = {"mass": 938.272, "charge": 1, "energy": 3.0, "current": 0.0, "frequency": 162.5e6,
                "twiss": {"x": (0.0, 0.5, 0.2), "y": (0.0, 0.5, 0.2), "z": (0.0, 0.5, 0.3)}}
        text = "start\ndrift 0.2 0.02 0\nquad 0.1 0.02 0 10\ndrift 0.2 0.02 0\nquad 0.1 0.02 0 -10\ndrift 0.2 0.02 0\nend\n"
        res = linear_preview(text, beam, [tempfile.gettempdir()], space_charge=False)
        return f"rms_x {res['rms_x'][0]:.3f} -> {res['rms_x'][-1]:.3f} mm in {res['model']['elapsed_ms']:.0f} ms"

    def user_data():
        from avas.paths import USER_DATA_DIR
        os.makedirs(USER_DATA_DIR, exist_ok=True)
        probe = os.path.join(USER_DATA_DIR, ".doctor")
        with open(probe, "w", encoding="utf-8") as fh:
            fh.write("ok")
        os.remove(probe)
        return USER_DATA_DIR

    def secrets():
        from avas.ai import backend_name
        return backend_name()

    from avas.buildinfo import build_info
    b = build_info()
    print(f"AVAS {__version__} ({'stand-alone' if b.get('frozen') else 'source'}"
          f"{', built ' + b['built'] if b.get('built') else ''}{', commit ' + b['commit'] if b.get('commit') else ''})")
    check("simulation engine (original kernel)", original_engine, essential=selected == "original")
    check("simulation engine (new kernel)", new_engine, essential=selected == "new")
    check("WebView2 runtime", webview)
    check("GUI services", gui_services)
    check("AI assistant", assistant)
    check("linear envelope preview", preview)
    check("user data folder", user_data)
    check("API key store", secrets, essential=False)
    print("all checks passed" if not failures else f"{failures} check(s) failed")
    return 1 if failures else 0


def _normalize_argv(argv):
    """``avas --input X --output Y``  ->  ``avas run --input X --output Y``."""
    argv = list(argv)
    known = {"run", "plot", "gui", "serve", "info", "doctor", "-h", "--help", "-V", "--version"}
    if argv and argv[0] not in known and argv[0].startswith("-"):
        argv.insert(0, "run")
    return argv


def main(argv=None):
    from avas.installation_lock import acquire
    acquire()
    argv = sys.argv[1:] if argv is None else list(argv)
    parser = build_parser()
    args = parser.parse_args(_normalize_argv(argv))
    if not getattr(args, "func", None):
        parser.print_help()
        return 1
    from avas.engines import EngineChoiceError
    try:
        return args.func(args) or 0
    except (CliError, EngineChoiceError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
