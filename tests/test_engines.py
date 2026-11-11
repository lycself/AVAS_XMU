"""Kernel choice (avas.engines): priority, [engine] in ini.ini, result tiers, the engine source hash,
``avas run`` / ``avas info`` options, run records and the research cache per result tier.

The new kernel's library is only needed by the tests marked ``needs_new_lib`` (engine/build of the
source tree, or AVAS_ENGINE_LIB); the research tests use the stand-in engine and a stand-in query.
"""
import json
import os
import shutil
import subprocess
import sys

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXAMPLE_INPUT = os.path.join(ROOT, "examples", "hwr010", "InputFile")
FAKE = os.path.join(ROOT, "tests", "fake_research_engine.py")
sys.path.insert(0, ROOT)

from avas import engines  # noqa: E402
from avas.cli import main as cli  # noqa: E402
from avas.data.inputs import copy_text_inputs  # noqa: E402
from avas.utils.iniconfig import IniConfig, read_engine_settings, write_engine_settings  # noqa: E402

NEW_LIB = engines.new_library_path(required=False)
needs_new_lib = pytest.mark.skipif(NEW_LIB is None, reason="the new kernel is not built (engine/build)")


def _write_ini(folder, text):
    os.makedirs(folder, exist_ok=True)
    with open(os.path.join(folder, "ini.ini"), "w", encoding="utf-8") as fh:
        fh.write(text)


# --------------------------------------------------------------------------- choice and priority
def test_defaults_and_priority_cli_env_ini(tmp_path):
    assert engines.resolve_choice(env={}) == engines.EngineChoice()
    assert engines.resolve_choice(env={}).kernel == "original"
    _write_ini(str(tmp_path), "[engine]\nkernel = new\ndevice = cpu\nthreads = 3\ngpus = 1, 0\nunknown = x\n")
    c = engines.resolve_choice(input_dir=str(tmp_path), env={})
    assert (c.kernel, c.device, c.precision, c.gpus, c.threads, c.compat) == ("new", "cpu", "auto", "1,0", 3, "none")
    assert c.sources == {"kernel": "ini", "device": "ini", "precision": "default", "gpus": "ini", "threads": "ini",
                         "compat": "default"}
    env = {"AVAS_ENGINE_DEVICE": "metal", "AVAS_ENGINE_THREADS": "2", "AVAS_ENGINE_GPUS": ""}   # empty = unset
    c = engines.resolve_choice(input_dir=str(tmp_path), env=env)
    assert (c.device, c.threads, c.gpus) == ("metal", 2, "1,0") and c.sources["device"] == "env"
    c = engines.resolve_choice(cli={"device": "auto", "threads": None}, input_dir=str(tmp_path), env=env)
    assert (c.device, c.threads) == ("auto", 2) and c.sources["device"] == "cli"
    # the original kernel ignores the other keys, even invalid ones
    c = engines.resolve_choice(cli={"kernel": "original"}, input_dir=str(tmp_path), env={"AVAS_ENGINE_DEVICE": "bad"})
    assert c == engines.EngineChoice(kernel="original") and c.to_dict() == {"kernel": "original"}
    assert engines.resolve_choice(env={"AVAS_ENGINE": "NEW"}).device == "auto"           # new-kernel defaults


@pytest.mark.parametrize("cli,env,message", [
    ({"kernel": "neu"}, {}, "invalid kernel 'neu'"),
    ({"kernel": "new", "device": "gpu0"}, {}, "command line --device: invalid device"),
    ({"kernel": "new"}, {"AVAS_ENGINE_PRECISION": "fp16"}, "environment variable AVAS_ENGINE_PRECISION"),
    ({"kernel": "new", "gpus": "0,x"}, {}, "invalid gpus"),
    ({"kernel": "new", "threads": -1}, {}, "invalid threads"),
    ({"kernel": "new", "compat": "dll", "device": "cuda"}, {}, "CPU only"),
    ({"kernel": "new", "device": "metal", "precision": "fp64"}, {}, "Metal computes in fp32 only"),
    ({"kernel": "new", "device": "cpu", "precision": "fp32"}, {}, "fp64 only"),
])
def test_invalid_settings_give_clear_errors(cli, env, message):
    with pytest.raises(engines.EngineChoiceError, match=message):
        engines.resolve_choice(cli=cli, env=env)


def test_invalid_ini_value_names_the_file(tmp_path):
    _write_ini(str(tmp_path), "[engine]\nkernel = new\nprecision = half\n")
    with pytest.raises(engines.EngineChoiceError, match=r"ini.ini \[engine\] precision"):
        engines.resolve_choice(input_dir=str(tmp_path), env={})


def test_choice_options_and_cli_args_round_trip():
    c = engines.resolve_choice(cli={"kernel": "new", "device": "cuda", "gpus": [1, 0, 1], "threads": 4}, env={})
    assert c.engine_options() == {"compat": "none", "threads": "4", "device": "cuda", "precision": "auto", "gpus": "1,0"}
    parser = cli.build_parser()
    args = parser.parse_args(["run", "-i", "x", "-o", "y"] + c.cli_args())
    again, legacy = cli.engine_choice_from_args(args, None)
    assert again == c and legacy == "cpu"
    assert engines.with_overrides(c, gpus="2").gpus == "2"


# --------------------------------------------------------------------------- ini.ini [engine]
def test_ini_engine_section_read_write_keeps_everything_else(tmp_path):
    ini = str(tmp_path / "ini.ini")
    _write_ini(str(tmp_path), "[project]\nfieldSource = maps\n\n[engine]\nkernel = new\nmystery = 7\n")
    assert write_engine_settings(ini, {"device": "CUDA", "gpus": "0, 1", "threads": "2"}) == {
        "kernel": "new", "mystery": "7", "device": "cuda", "gpus": "0,1", "threads": "2"}
    with pytest.raises(ValueError):
        write_engine_settings(ini, {"device": "warp"})
    with pytest.raises(ValueError):
        write_engine_settings(ini, {"colour": "red"})
    write_engine_settings(ini, {"gpus": None, "threads": ""})
    assert read_engine_settings(ini) == {"kernel": "new", "mystery": "7", "device": "cuda"}
    # the GUI's IniConfig round trip keeps [engine]; a project without it gets none
    cfg = IniConfig()
    data = cfg.create_from_file({"otherPath": ini})["data"]["iniParams"]
    assert data["engine"]["kernel"] == "new" and data["project"]["fieldSource"] == "maps"
    cfg.set_param(error={"error_type": "", "seed": 1, "if_normal": 1}, engine={"precision": "fp32", "device": None})
    cfg.write_to_file({"otherPath": ini})
    assert read_engine_settings(ini) == {"kernel": "new", "mystery": "7", "precision": "fp32"}
    bare = str(tmp_path / "bare.ini")
    IniConfig().write_to_file({"otherPath": bare})
    assert read_engine_settings(bare) == {}


# --------------------------------------------------------------------------- tiers, space charge
def test_result_tiers():
    t = engines.result_tier
    assert t("original") == "original"
    assert t("new", "cpu", "fp64", "none") == "new-cpu-fp64"
    assert t("new", "cuda", "fp64", "none") == "new-cuda-fp64"
    assert t("new", "emu", "fp64", "none") == "new-cuda-fp64"         # emu reproduces the device back end
    assert t("new", "cuda", "fp32", "none") == t("new", "metal", "fp32", "none") == t("new", "emu", "fp32") == "new-fp32"
    assert t("new", "cpu", "fp64", "dll") == "new-compat-dll"
    assert t("new", None, None, "none") is None


def test_input_space_charge(tmp_path):
    copy_text_inputs(EXAMPLE_INPUT, str(tmp_path))
    assert engines.input_space_charge(str(tmp_path)) is True

    def setkey(name, key, value):
        p = tmp_path / name
        lines = [ln for ln in p.read_text().splitlines() if not ln.lower().startswith(key)]
        p.write_text("\n".join(lines + [f"{key} {value}"]) + "\n")

    setkey("beam.txt", "current", 0)
    assert engines.input_space_charge(str(tmp_path)) is False
    setkey("beam.txt", "current", 1.0)
    setkey("input.txt", "spacecharge", 0)
    assert engines.input_space_charge(str(tmp_path)) is False


# --------------------------------------------------------------------------- engine source hash
def test_engine_source_hash_rules(tmp_path):
    root = tmp_path / "engine"
    for rel, data in {"CMakeLists.txt": b"project(x)\r\n", "include/a.h": b"int a;\n", "src/core/b.cpp": b"b\r\nc\n",
                      "src/exports.map": b"{};\n", "src/.DS_Store": b"junk", "third_party/p/p.h": b"p\n",
                      "third_party/p/LICENSE.md": b"text\n", "tests/t.cpp": b"not hashed\n"}.items():
        (root / rel).parent.mkdir(parents=True, exist_ok=True)
        (root / rel).write_bytes(data)
    assert engines.engine_source_files(str(root)) == ["CMakeLists.txt", "include/a.h", "src/core/b.cpp",
                                                      "src/exports.map", "third_party/p/p.h"]
    h = engines.engine_source_hash(str(root))
    (root / "src/core/b.cpp").write_bytes(b"b\nc\n")                      # CRLF -> LF: same hash
    (root / "tests/t.cpp").write_bytes(b"changed\n")
    assert engines.engine_source_hash(str(root)) == h
    (root / "src/core/b.cpp").write_bytes(b"b\nd\n")
    assert engines.engine_source_hash(str(root)) != h
    cmake = shutil.which("cmake")
    if cmake:                                                             # the same algorithm as the build
        out = tmp_path / "h.h"
        subprocess.run([cmake, f"-DROOT={root}", f"-DOUT={out}", "-P",
                        os.path.join(ROOT, "engine", "cmake", "source_hash.cmake")], check=True)
        assert engines.engine_source_hash(str(root)) in out.read_text()


@needs_new_lib
def test_library_reports_the_source_hash_of_this_tree():
    res = subprocess.run([sys.executable, "-m", "avas", "info", "--engine-json", "--engine", "new"], cwd=ROOT,
                         capture_output=True, text=True, timeout=120)
    assert res.returncode == 0, res.stderr
    info = json.loads(res.stdout)["new"]["info"]
    assert info["source_hash"] == engines.engine_source_hash(), \
        f"{NEW_LIB} was built from other sources; rebuild it (cmake --build engine/build)"
    assert info["version"] == info["source_hash"][:16]


# --------------------------------------------------------------------------- library lookup
def test_library_lookup_order(tmp_path, monkeypatch):
    lib = tmp_path / engines.new_library_name()
    lib.write_bytes(b"")
    monkeypatch.setenv("AVAS_ENGINE_LIB", str(lib))
    assert engines.new_library_path() == str(lib)
    monkeypatch.setenv("AVAS_ENGINE_LIB", str(tmp_path / "missing.so"))
    with pytest.raises(engines.EngineChoiceError, match="does not exist"):
        engines.new_library_path()
    monkeypatch.delenv("AVAS_ENGINE_LIB")
    prebuilt = tmp_path / "prebuilt"
    prebuilt.mkdir()
    monkeypatch.setattr(engines.paths, "NEW_ENGINE_DIR", str(prebuilt))
    monkeypatch.setattr(engines.paths, "new_engine_build_dirs", lambda: [str(tmp_path / "nobuild")])
    with pytest.raises(engines.EngineChoiceError, match="cmake -S engine -B engine/build"):
        engines.new_library_path()
    assert engines.new_library_path(required=False) is None
    (prebuilt / engines.new_library_name()).write_bytes(b"")
    assert engines.new_library_path() == str(prebuilt / engines.new_library_name())


# --------------------------------------------------------------------------- command line
def _run_cli(*args, env=None, cwd=None):
    full = dict(os.environ)
    for name in engines.ENV.values():
        full.pop(name, None)
    full.update(env or {})
    full["PYTHONPATH"] = ROOT
    return subprocess.run([sys.executable, "-m", "avas"] + list(args), cwd=cwd or ROOT, capture_output=True, text=True,
                          env=full, timeout=600)


@pytest.fixture
def project(tmp_path):
    inp = tmp_path / "proj" / "InputFile"
    shutil.copytree(EXAMPLE_INPUT, str(inp))
    for name, key, value in (("beam.txt", "particlenumber", 300), ("input.txt", "randomseed", 12345)):
        p = inp / name
        lines = [f"{key} {value}" if ln.split() and ln.split()[0] == key else ln for ln in p.read_text().splitlines()]
        p.write_text("\n".join(lines) + "\n")
    return tmp_path / "proj"


def test_run_options_of_the_wrong_kernel_are_refused(project, tmp_path):
    out = tmp_path / "out"
    res = _run_cli("run", "-i", str(project), "-o", str(out), "--device", "metal")
    assert res.returncode == 2 and "applies only to the new kernel" in res.stderr and not out.exists()
    res = _run_cli("run", "-i", str(project), "-o", str(out), "--precision", "fp32", "--threads", "2")
    assert res.returncode == 2 and "--precision, --threads apply only to the new kernel" in res.stderr
    res = _run_cli("run", "-i", str(project), "-o", str(out), "--engine", "new", "--device", "gpu")
    assert res.returncode == 2 and "--device cuda" in res.stderr
    res = _run_cli("run", "-i", str(project), "-o", str(out), "--engine", "new", "--device", "warp")
    assert res.returncode == 2 and "invalid --device" in res.stderr and "Traceback" not in res.stderr


def test_run_record_of_the_original_kernel():
    rec = engines.run_record(engines.EngineChoice())
    assert rec["kernel"] == "original" and rec["tier"] == "original"
    assert rec["file"] == os.path.basename(engines.original_library_path())
    assert rec["sha1"] == engines.file_sha1(engines.original_library_path())
    gpu = engines.run_record(engines.EngineChoice(), legacy_device="gpu")
    assert gpu["file"] == "libPIC.so" and gpu["device"] == "gpu"
    new = engines.run_record(engines.resolve_choice(cli={"kernel": "new", "device": "cpu"}, env={}))
    assert new["kernel"] == "new" and new["requested"]["device"] == "cpu" and new["tier"] is None


def test_info_without_the_new_library_explains_how_to_build(tmp_path):
    res = _run_cli("info", "--engine-json", env={"AVAS_ENGINE_LIB": str(tmp_path / "none.dylib")})
    assert res.returncode == 0, res.stderr
    data = json.loads(res.stdout)
    assert data["original"]["file"] in ("AVAS.dll", "libAVAS.so") and "does not exist" in data["new"]["error"]
    res = _run_cli("info", env={"AVAS_ENGINE_LIB": str(tmp_path / "none.dylib")})
    assert res.returncode == 0 and "new kernel     : not found" in res.stdout and "Traceback" not in res.stderr
    res = _run_cli("doctor", env={"AVAS_ENGINE_LIB": str(tmp_path / "none.dylib")})
    assert "simulation engine (new kernel)" in res.stdout and "Traceback" not in res.stdout + res.stderr


@needs_new_lib
def test_new_kernel_run_records_what_it_used(project, tmp_path):
    out = tmp_path / "cpu"
    res = _run_cli("run", "-i", str(project), "-o", str(out), "--engine", "new", "--device", "cpu", "--threads", "2")
    assert res.returncode == 0, res.stdout[-2000:] + res.stderr
    info = json.loads((out / "avas_run.json").read_text())
    eng = info["engine"]
    assert eng["kernel"] == "new" and eng["device"] == "cpu" and eng["precision"] == "fp64" and eng["threads"] == 2
    assert eng["tier"] == "new-cpu-fp64" and eng["space_charge"] is True and eng["compat"] == "none"
    assert eng["version"] == eng["version"][:16] and eng["requested"]["device"] == "cpu" and eng["sha1"]
    assert info["status"] == "finished"
    # [engine] of ini.ini; the command line wins over it
    write_engine_settings(str(project / "InputFile" / "ini.ini"), {"kernel": "new", "device": "cpu", "compat": "dll"})
    out2 = tmp_path / "ini"
    res = _run_cli("run", "-i", str(project), "-o", str(out2), "--threads", "1")
    assert res.returncode == 0, res.stdout[-2000:] + res.stderr
    eng = json.loads((out2 / "avas_run.json").read_text())["engine"]
    assert eng["tier"] == "new-compat-dll" and eng["threads"] == 1 and eng["requested"]["compat"] == "dll"
    res = _run_cli("info", "-i", str(project))
    assert "new kernel (cpu, auto, compat=dll)" in res.stdout


@needs_new_lib
def test_a_missing_gpu_is_an_error_not_the_cpu(project, tmp_path):
    res = _run_cli("info", "--engine-json", "--engine", "new")
    build = json.loads(res.stdout)["new"]["info"]["build"]
    for device in ("cuda", "metal"):
        if build[device]:
            continue
        out = tmp_path / device
        res = _run_cli("run", "-i", str(project), "-o", str(out), "--engine", "new", "--device", device)
        assert res.returncode == 2 and f"cannot use device '{device}'" in res.stderr and "auto" in res.stderr
        assert not out.exists()


# --------------------------------------------------------------------------- research cache per tier
LATTICE = "start\ndrift 0.1 0.03 0\nS1: steerer 0 0.03 0 0 0 0 0.01\ndrift 0.4 0.03 0\nend\n"


@pytest.fixture
def rproject(tmp_path, monkeypatch):
    for name in engines.ENV.values():              # the studies' own settings, not the caller's environment
        monkeypatch.delenv(name, raising=False)
    inp = tmp_path / "rp" / "InputFile"
    copy_text_inputs(EXAMPLE_INPUT, str(inp))
    (inp / "lattice_mulp.txt").write_text(LATTICE)
    return tmp_path / "rp"


def fake_query(cards=(), metal=True):
    """Stand-in for the child query: CPU build with Metal, or CUDA with *cards*."""
    calls = []

    def query(choice):
        calls.append(choice)

        def plan(sc):
            dev = choice.device
            if dev == "auto":
                dev = "cuda" if cards else ("metal" if metal and sc else "cpu")
            if choice.compat == "dll":
                dev = "cpu"
            prec = choice.precision
            if prec == "auto":
                prec = "fp32" if dev in ("metal",) or (dev == "cuda") else "fp64"
            if dev == "cpu":
                prec = "fp64"
            gpus = list(cards) if dev == "cuda" and choice.gpus == "all" else (
                [int(g) for g in choice.gpus.split(",")] if dev == "cuda" else [])
            return {"device": dev, "precision": prec, "gpus": gpus, "reason": "test", "error": None}

        return {"new": {"path": "fake", "sha1": "f", "info": {"version": "fakeversion0001"},
                        "devices": {"cuda": {"devices": [{"index": c} for c in cards]}}},
                "resolved": {"sc": plan(True), "nosc": plan(False)}}
    query.calls = calls
    return query


def make_study(project, name="t", **kwargs):
    from avas import research
    root = str(project / "Research" / name)
    counter = os.path.join(root, "calls.txt")
    return research.Study(str(project), name=name, engine_command=lambda inp, out, field, lat: [
        sys.executable, FAKE, inp, out, lat, counter], **kwargs)


def ncalls(study):
    path = os.path.join(study.root, "calls.txt")
    return len(open(path).read().splitlines()) if os.path.isfile(path) else 0


def test_research_cache_is_per_result_tier(rproject):
    from avas.research import Variant
    q = fake_query()
    orig = make_study(rproject, engine="original")
    run_o = orig.run(Variant(particles=100), "engine")
    assert run_o.ok and run_o.record["engine"]["tier"] == "original" and ncalls(orig) == 1
    cpu = make_study(rproject, engine="new", device="cpu", engine_query=q)
    run_c = cpu.run(Variant(particles=100), "engine")
    assert run_c.ok and ncalls(cpu) == 2 and run_c.folder != run_o.folder            # not the original's cache
    assert run_c.record["engine"]["tier"] == "new-cpu-fp64"
    assert run_c.record["engine"]["actual"]["tier"] == "new-cpu-fp64" and "warnings" not in run_c.record
    # threads are not part of the fingerprint
    cpu4 = make_study(rproject, engine="new", device="cpu", threads=4, engine_query=q)
    assert cpu4.run(Variant(particles=100), "engine").folder == run_c.folder and ncalls(cpu4) == 2
    # fp32: Metal and emu share the cache entry
    metal = make_study(rproject, engine="new", device="metal", engine_query=q)
    run_m = metal.run(Variant(particles=100), "engine")
    assert run_m.record["engine"]["tier"] == "new-fp32" and ncalls(metal) == 3 and run_m.folder != run_c.folder
    emu = make_study(rproject, engine="new", device="emu", precision="fp32", engine_query=q)
    run_e = emu.run(Variant(particles=100), "engine")
    assert run_e.folder == run_m.folder and ncalls(emu) == 3
    assert run_m.rms("x")[0] == pytest.approx(run_c.rms("x")[0] * 1.001)
    # auto resolves before the run (with space charge: Metal) and lands in the fp32 entry too
    auto = make_study(rproject, engine="new", engine_query=q)
    assert auto.run(Variant(particles=100), "engine").folder == run_m.folder and ncalls(auto) == 3
    # the project's [engine] is the default of a study; the run's ini.ini carries the choice
    write_engine_settings(str(rproject / "InputFile" / "ini.ini"), {"kernel": "new", "device": "cpu"})
    proj = make_study(rproject, engine_query=q)
    assert proj.engine_choice().device == "cpu" and proj.run(Variant(particles=100), "engine").folder == run_c.folder
    with open(os.path.join(run_c.folder, "input", "ini.ini")) as fh:
        assert "[engine]\nkernel = new\ndevice = cpu\n" in fh.read()


def test_research_warns_when_the_run_used_another_tier(rproject):
    from avas.research import Variant
    study = make_study(rproject, engine="new", engine_query=fake_query(metal=False))   # predicted: CPU
    run = study.run(Variant(input={"fakeauto": "metal"}, particles=100), "engine")       # the run: Metal fp32
    assert run.ok and run.record["engine"]["tier"] == "new-cpu-fp64"
    assert run.record["engine"]["actual"]["tier"] == "new-fp32" and "new-fp32" in run.record["warnings"][0]


def test_research_parallel_gpu_runs_bind_one_gpu_each(rproject):
    from avas.research import Variant
    two = make_study(rproject, engine="new", device="cuda", engine_query=fake_query(cards=(0, 1)))
    assert two.parallel_plan(3) == (2, ["0", "1"])
    runs = two.run_many([Variant(particles=100 + i) for i in range(4)], workers=3)
    assert all(r.ok for r in runs)
    with open(os.path.join(two.root, "calls.txt")) as fh:
        bound = sorted(line.rsplit("gpus=", 1)[1] for line in fh.read().splitlines())
    assert set(bound) <= {"0", "1"} and len(bound) == 4
    assert {r.record["engine"]["bound_gpus"] for r in runs} <= {"0", "1"}
    one = make_study(rproject, name="one", engine="new", device="cuda", engine_query=fake_query(cards=(0,)))
    assert one.parallel_plan(4) == (1, None)
    metal = make_study(rproject, name="m", engine="new", device="metal", engine_query=fake_query())
    assert metal.parallel_plan(4) == (1, None)
    cpu = make_study(rproject, name="c", engine="new", device="cpu", engine_query=fake_query())
    assert cpu.parallel_plan(4) == (4, None)


def test_research_refuses_a_device_that_cannot_run(rproject):
    from avas.research import Variant

    def broken(choice):
        return {"new": {"path": "fake", "info": {"version": "v"}},
                "resolved": {"sc": {"error": "device cuda: no NVIDIA driver found"},
                             "nosc": {"error": "device cuda: no NVIDIA driver found"}}}
    study = make_study(rproject, engine="new", device="cuda", engine_query=broken)
    with pytest.raises(engines.EngineChoiceError, match="cannot use device 'cuda': no NVIDIA driver"):
        study.run(Variant(particles=100), "engine")
    assert ncalls(study) == 0
