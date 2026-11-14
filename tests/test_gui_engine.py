"""GUI back end of the computing kernels (avas/gui/services/kernels.py and the pages that use it).

The kernel query (``avas info --engine-json`` in a child process) is replaced by canned answers, so these
tests run on any machine and need neither kernel: the Simulation settings page's ``[engine]`` group, the
check before a run, the device query's cache and failures, the run lock and the labels of run records.
"""
import json
import os
import re
import shutil
import sys

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXAMPLE = os.path.join(ROOT, "examples", "hwr010", "InputFile")
sys.path.insert(0, ROOT)
os.environ.setdefault("MPLBACKEND", "Agg")


def rpc(method, **params):
    from avas.gui import bridge
    return json.loads(bridge.dispatch(method, params))


def ok(method, **params):
    reply = rpc(method, **params)
    assert reply["ok"], f"{method}: {reply.get('error')}\n{reply.get('detail', '')}"
    return reply["result"]


@pytest.fixture(scope="module")
def project(tmp_path_factory):
    root = tmp_path_factory.mktemp("enginegui")
    os.environ["AVAS_GUI_SETTINGS"] = str(root / "gui-settings.json")
    inp = root / "proj" / "InputFile"
    from avas.data.inputs import copy_text_inputs
    copy_text_inputs(EXAMPLE, str(inp))
    for name in os.listdir(EXAMPLE):                     # the field maps of the example (run.check parses the lattice)
        if not os.path.exists(inp / name):
            shutil.copy(os.path.join(EXAMPLE, name), inp / name)
    os.makedirs(root / "proj" / "OutputFile")
    from avas.gui import services  # noqa: F401 - registers the handlers
    assert ok("project.open", path=str(root / "proj"))["open"]
    yield root / "proj"
    ok("project.close")


def plan(device, precision, reason="auto", error=None):
    if error:
        return {"device": None, "precision": None, "gpus": [], "reason": None, "error": error}
    return {"device": device, "precision": precision, "gpus": [], "reason": reason, "error": None}


def describe(choice=None, sc=None, nosc=None, lib=True, cuda=0):
    """A canned ``avas info --engine-json`` answer (Mac with Metal unless *cuda* cards are given)."""
    auto = {"sc": plan("metal", "fp32"), "nosc": plan("cpu", "fp64", "auto: no space charge, Metal is used for runs with space charge")}
    new = {"path": "/x/libavas_engine.dylib", "sha1": "3faeb7fb077a2ce7aff4a0c0605543f80c07a00f", "error": None,
           "info": {"version": "5762f2c226c3c359", "source_hash": "5762f2c2" + "0" * 56,
                    "build": {"cuda": cuda > 0, "metal": True, "mpi": False, "cuda_archs": [], "build_type": "Release"},
                    "compiler": "clang 17", "platform": "macos-arm64"},
           "devices": {"cuda": {"compiled": cuda > 0, "note": None if cuda else "this engine was built without CUDA",
                                "devices": [{"index": i, "name": f"NVIDIA L40S {i}", "compute": "8.9", "fp64_full": False,
                                             "memory_gb": 47.7} for i in range(cuda)]},
                       "metal": {"compiled": True, "devices": [{"name": "Apple M2", "usable": True}], "note": None},
                       "cpu": {"threads": 8}, "auto": auto, "selected": auto}}
    if not lib:
        new = {"path": None, "error": "the new kernel's library libavas_engine.dylib was not found", "candidates": []}
    return {"original": {"file": "libAVAS.so", "path": "/x/libAVAS.so", "exists": True, "sha1": "3d6c3e08", "available": False},
            "new": new, "choice": choice.to_dict() if choice else None,
            "resolved": {"sc": sc or auto["sc"], "nosc": nosc or auto["nosc"]} if lib else None}


@pytest.fixture
def fake_query(monkeypatch):
    """Replace the child-process query; ``calls`` lists the choices asked for, ``answer`` builds the reply."""
    from avas import engines
    state = {"calls": [], "answer": lambda choice: describe(choice)}

    def query(choice, timeout=120):
        state["calls"].append(choice)
        result = state["answer"](choice)
        if isinstance(result, Exception):
            raise result
        return result
    monkeypatch.setattr(engines, "query_subprocess", query)
    return state


def set_platform(monkeypatch, original):
    from avas import engines
    monkeypatch.setattr(engines, "original_available", lambda system=None, machine=None: original)


def ini_text(project):
    return (project / "InputFile" / "ini.ini").read_text(encoding="utf-8")


def write_engine(project, values):
    from avas.utils.iniconfig import write_engine_settings
    path = str(project / "InputFile" / "ini.ini")
    from avas.utils.iniconfig import read_engine_settings
    write_engine_settings(path, {k: None for k in read_engine_settings(path)})
    if values:
        write_engine_settings(path, values)


# --------------------------------------------------------------------------- bridge
def test_user_error_with_values_is_translatable():
    from avas.gui import bridge

    @bridge.rpc("test.templated")
    def templated():
        raise bridge.UserError("Cannot use {device}: {detail}", device="CUDA", detail="no driver")

    reply = rpc("test.templated")
    assert reply == {"ok": False, "error": "Cannot use CUDA: no driver", "user": True,
                     "template": "Cannot use {device}: {detail}", "params": {"device": "CUDA", "detail": "no driver"}}
    assert str(bridge.UserError("plain {braces} stay")) == "plain {braces} stay"


# --------------------------------------------------------------------------- engine.info
def test_engine_info_lists_kernels_and_devices(project, fake_query, monkeypatch):
    set_platform(monkeypatch, False)
    data = ok("engine.info")
    assert data["originalAvailable"] is False and data["defaultKernel"] == "new"
    assert data["new"]["available"] and data["new"]["version"] == "5762f2c226c3c359"
    assert data["new"]["build"]["metal"] is True and data["new"]["build"]["cuda"] is False
    assert data["devices"]["metal"] == [{"name": "Apple M2", "usable": True}] and data["devices"]["cpuThreads"] == 8
    assert data["auto"]["sc"]["device"] == "metal" and data["auto"]["nosc"]["device"] == "cpu"
    assert fake_query["calls"][0].kernel == "new"


def test_engine_info_reports_failures_without_raising(project, fake_query):
    from avas.engines import EngineChoiceError
    fake_query["answer"] = lambda choice: EngineChoiceError("cannot query the new kernel: crashed")
    data = ok("engine.info")
    assert data["error"] == "cannot query the new kernel: crashed" and data["new"]["available"] is False and data["devices"] is None
    fake_query["answer"] = lambda choice: describe(choice, lib=False)
    data = ok("engine.info")
    assert data["new"]["available"] is False and "was not found" in data["new"]["error"] and data["error"] is None


def test_device_query_is_cached_until_refresh(project, monkeypatch, tmp_path):
    """The real query_subprocess: one child process per library file and choice, again after refresh."""
    from avas import engines
    lib = tmp_path / "libavas_engine.dylib"
    lib.write_bytes(b"x")
    monkeypatch.setenv("AVAS_ENGINE_LIB", str(lib))
    engines.clear_query_cache()
    runs = []

    class Done:
        returncode = 0
        stderr = ""

        def __init__(self):
            self.stdout = json.dumps(describe())

    def fake_run(cmd, **kwargs):
        runs.append(cmd)
        return Done()
    monkeypatch.setattr(engines.subprocess, "run", fake_run)
    ok("engine.info")
    ok("engine.info")
    assert len(runs) == 1 and "--engine-json" in runs[0]
    ok("engine.info", refresh=True)
    assert len(runs) == 2
    engines.clear_query_cache()


# --------------------------------------------------------------------------- settings page
def test_settings_load_fills_in_the_platform_default(project, monkeypatch):
    write_engine(project, None)
    set_platform(monkeypatch, True)
    eng = ok("settings.load")["engine"]
    assert eng["form"] == {"kernel": "original", "device": "auto", "precision": "auto", "gpus": "all", "threads": "0", "compat": "none"}
    assert eng["explicitKernel"] is False and eng["invalid"] == [] and eng["originalAvailable"] is True
    set_platform(monkeypatch, False)
    eng = ok("settings.load")["engine"]
    assert eng["form"]["kernel"] == "new" and eng["defaultKernel"] == "new" and eng["originalAvailable"] is False


def test_settings_save_writes_engine_only_when_it_changed(project, monkeypatch):
    set_platform(monkeypatch, True)
    write_engine(project, None)
    data = ok("settings.load")
    ok("settings.save", form=data["form"], meta=data["meta"], engine=data["engine"]["form"])
    assert "[engine]" not in ini_text(project)                     # unchanged form: nothing written
    form = dict(data["engine"]["form"], kernel="new", device="cpu", threads="4")
    saved = ok("settings.save", form=data["form"], meta=data["meta"], engine=form)
    text = ini_text(project)
    assert "[engine]" in text and "kernel = new" in text and "device = cpu" in text and "threads = 4" in text
    assert "precision" not in text.split("[engine]")[1]            # defaults are not written
    assert "[lattice]" in text and "[error]" in text                # the rest of ini.ini is kept
    assert saved["engine"]["form"]["kernel"] == "new" and saved["engine"]["explicitKernel"] is True
    form = dict(form, device="auto", threads="0")
    ok("settings.save", form=data["form"], meta=data["meta"], engine=form)
    section = ini_text(project).split("[engine]")[1]
    assert "kernel = new" in section and "device" not in section and "threads" not in section
    ok("settings.save", form=data["form"], meta=data["meta"])        # without the group: [engine] untouched
    assert "kernel = new" in ini_text(project)
    write_engine(project, None)


def test_settings_save_refuses_invalid_combinations_before_writing(project, monkeypatch):
    set_platform(monkeypatch, True)
    write_engine(project, None)
    data = ok("settings.load")
    before = (project / "InputFile" / "input.txt").read_text(encoding="utf-8")
    form = dict(data["engine"]["form"], kernel="new", device="metal", precision="fp64")
    changed = dict(data["form"], steppercycle="77")
    reply = rpc("settings.save", form=changed, meta=data["meta"], engine=form)
    assert reply["ok"] is False and reply["user"] and reply["template"] == "The computing kernel settings are invalid: {detail}"
    assert "Metal computes in fp32 only" in reply["params"]["detail"]
    assert (project / "InputFile" / "input.txt").read_text(encoding="utf-8") == before and "[engine]" not in ini_text(project)
    for bad in ({"device": "cuda", "compat": "dll"}, {"device": "cpu", "precision": "fp32"}, {"threads": "-2"}, {"gpus": "x"}):
        reply = rpc("settings.save", form=data["form"], meta=data["meta"], engine=dict(data["engine"]["form"], kernel="new", **bad))
        assert reply["ok"] is False and reply["user"], bad


def test_settings_load_reports_invalid_entries_and_environment(project, monkeypatch):
    path = project / "InputFile" / "ini.ini"
    path.write_text(ini_text(project) + "\n[engine]\nkernel = new\ndevice = warp\n", encoding="utf-8")
    monkeypatch.setenv("AVAS_ENGINE_DEVICE", "cpu")
    eng = ok("settings.load")["engine"]
    assert eng["form"]["kernel"] == "new" and eng["form"]["device"] == "auto" and eng["invalid"] == ["device = warp"]
    assert eng["env"] == ["AVAS_ENGINE_DEVICE"]
    write_engine(project, None)


def test_settings_save_with_engine_is_refused_while_a_run_is_active(project):
    from avas.gui.services import runner
    data = ok("settings.load")
    r = runner.runner()
    r.job = object()
    try:
        reply = rpc("settings.save", form=data["form"], meta=data["meta"], engine=dict(data["engine"]["form"], kernel="new"))
        assert reply["ok"] is False and reply["user"] and "locked" in reply["error"]
    finally:
        r.job = None
    assert "[engine]" not in ini_text(project)


# --------------------------------------------------------------------------- preview and the check before a run
def test_preview_resolves_auto_for_the_project(project, fake_query):
    form = {"kernel": "new", "device": "auto", "precision": "auto", "gpus": "all", "threads": "0", "compat": "none"}
    res = ok("engine.preview", choice=form)
    assert res["spaceCharge"] is True and res["plan"]["device"] == "metal" and res["other"]["device"] == "cpu"
    res = ok("engine.preview", choice=form, spaceCharge=False)          # the form's unsaved switch
    assert res["spaceCharge"] is False and res["plan"]["device"] == "cpu" and res["plan"]["precision"] == "fp64"
    assert ok("engine.preview", choice=dict(form, kernel="original")) == {"kernel": "original", "error": None, "plan": None, "other": None}
    res = ok("engine.preview", choice=dict(form, device="metal", precision="fp64"))
    assert "fp32 only" in res["error"] and res["plan"] is None
    fake_query["answer"] = lambda choice: describe(choice, lib=False)
    assert "was not found" in ok("engine.preview", choice=form)["error"]


def test_check_before_a_run(project, fake_query, monkeypatch):
    set_platform(monkeypatch, False)
    write_engine(project, None)                                             # macOS default: the new kernel
    expected = ok("engine.check")
    assert expected == {"kernel": "new", "device": "metal", "precision": "fp32", "gpus": [], "compat": "none", "tier": "new-fp32",
                        "requestedDevice": "auto", "reason": "auto", "reasonCode": None, "version": "5762f2c226c3c359",
                        "predicted": True}
    assert ok("run.check")["device"] == "metal"                             # the Run page's check includes it

    write_engine(project, {"kernel": "original"})                          # explicit original kernel on macOS
    reply = rpc("run.check")
    assert reply["ok"] is False and reply["template"].startswith("The original kernel does not run on {platform}")

    set_platform(monkeypatch, True)
    assert ok("engine.check") == {"kernel": "original", "predicted": True}
    inp = project / "InputFile" / "input.txt"
    before = inp.read_text(encoding="utf-8")
    inp.write_text(re.sub(r"(?m)^scmethod\s+\S+", "scmethod PICNIC", before), encoding="utf-8")
    try:
        reply = rpc("engine.check")
        assert reply["ok"] is False and "PICNIC" in reply["error"] and reply["user"]
        write_engine(project, {"kernel": "new"})
        assert ok("engine.check")["kernel"] == "new"                        # the new kernel has PICNIC
    finally:
        inp.write_text(before, encoding="utf-8")

    write_engine(project, {"kernel": "new", "device": "cuda"})
    fake_query["answer"] = lambda choice: describe(choice, sc=plan(None, None, error="device cuda: this engine was built without CUDA"))
    reply = rpc("engine.check")
    assert reply["ok"] is False and reply["params"]["device"] == "CUDA" and "built without CUDA" in reply["params"]["detail"]
    fake_query["answer"] = lambda choice: describe(choice, lib=False)
    reply = rpc("engine.check")
    assert reply["ok"] is False and reply["template"] == "The new kernel's library is not available: {detail}"
    write_engine(project, {"kernel": "new", "device": "metal", "precision": "fp64"})
    reply = rpc("engine.check")
    assert reply["ok"] is False and reply["template"] == "The computing kernel settings are invalid: {detail}"
    write_engine(project, None)


def test_assistant_sandbox_runs_are_checked_too(project, fake_query, monkeypatch):
    from avas.ai import sandbox
    from avas.gui import context
    set_platform(monkeypatch, True)
    write_engine(project, {"kernel": "new", "device": "cuda"})
    fake_query["answer"] = lambda choice: describe(choice, sc=plan(None, None, error="device cuda: no CUDA driver"))
    box = sandbox.Sandbox(context.project(), "enginecheck", root=str(project / ".sandbox"))
    assert "[engine]" in (project / ".sandbox" / "input" / "ini.ini").read_text(encoding="utf-8")   # the copy keeps the choice
    box.begin("check", 1)
    try:
        with pytest.raises(sandbox.SandboxError, match="cannot use the device"):
            box.run_in(box.input_dir, str(project / ".sandbox" / "out"), box.field_dir, box.lattice_name)
    finally:
        box.cleanup()
    write_engine(project, None)


# --------------------------------------------------------------------------- labels of run records
def test_summary_of_run_records():
    from avas.gui.services.kernels import summary
    assert summary(None) is None and summary({}) is None and summary("x") is None
    assert summary({"kernel": "original", "sha1": "abc", "tier": "original"}) == {"kernel": "original", "legacyGpu": False}
    assert summary({"kernel": "original", "device": "gpu"})["legacyGpu"] is True
    rec = {"kernel": "new", "device": "cpu", "precision": "fp64", "gpus": [], "compat": "none", "tier": "new-cpu-fp64",
           "version": "v", "reason": "auto: no CUDA driver", "requested": {"kernel": "new", "device": "auto", "compat": "none"}}
    assert summary(rec) == {"kernel": "new", "device": "cpu", "precision": "fp64", "gpus": [], "compat": "none",
                            "tier": "new-cpu-fp64", "requestedDevice": "auto", "reason": "auto: no CUDA driver",
                            "reasonCode": None, "version": "v"}
    assert summary(dict(rec, reason_code="no_driver"))["reasonCode"] == "no_driver"


def test_result_sources_carry_the_kernel(project, monkeypatch):
    out = project / "OutputFile"
    rec = {"status": "finished", "started": "2026-11-11 10:00:00", "mode": "basic",
           "engine": {"kernel": "new", "device": "metal", "precision": "fp32", "tier": "new-fp32", "requested": {"device": "auto"}}}
    (out / "avas_run.json").write_text(json.dumps(rec), encoding="utf-8")
    old = project / "Runs" / "20260101-000000_old"
    old.mkdir(parents=True)
    (old / "avas_run.json").write_text(json.dumps({"status": "finished", "label": "old"}), encoding="utf-8")
    kept = project / "Runs" / "20260102-000000_kept"
    kept.mkdir(parents=True)
    (kept / "avas_run.json").write_text(json.dumps({"status": "finished", "label": "kept", "engine": {"kernel": "original"}}), encoding="utf-8")
    try:
        sources = {s["outputDir"]: s for s in ok("results.sources")}
        assert sources[str(out)]["engine"]["device"] == "metal" and sources[str(out)]["engine"]["requestedDevice"] == "auto"
        assert sources[str(old)]["engine"] is None                         # a record from before M7: no label
        assert sources[str(kept)]["engine"] == {"kernel": "original", "legacyGpu": False}
    finally:
        shutil.rmtree(project / "Runs")
        (out / "avas_run.json").unlink()


def test_runner_state_carries_the_expected_then_the_actual_kernel(project, tmp_path):
    from avas.gui.services import kernels, runner
    job = runner.Job(object(), [runner.Stage("run", "", lambda: {})], source="segment", output_dir=str(tmp_path))
    job.engine = {"kernel": "new", "device": "metal", "precision": "fp32", "predicted": True}
    r = runner.Runner()
    r.job, r._stage = job, 0
    assert r.state()["engine"]["predicted"] is True
    (tmp_path / "avas_run.json").write_text(json.dumps({"engine": {"kernel": "new", "device": "cpu", "precision": "fp64",
                                                                  "requested": {"device": "auto"}, "reason": "auto: x"}}), encoding="utf-8")
    used = kernels.read_summary(str(tmp_path))
    assert used["device"] == "cpu" and used["requestedDevice"] == "auto" and "predicted" not in used
    assert kernels.read_summary(str(tmp_path / "missing")) is None
