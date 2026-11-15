"""Tests of the research interface (avas.research) with a stand-in engine (tests/fake_research_engine.py).

The stand-in makes a deterministic DataSet.txt from the inputs, so the tests check that variants
reach the input files, records, caching, resuming, parallel runs, failures, comparisons, engine
response matrices and exports; the linear fidelity is checked against the model directly.
"""
import math
import os
import shutil
import sys
import threading

import numpy as np
import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXAMPLE_INPUT = os.path.join(ROOT, "examples", "hwr010", "InputFile")
FAKE = os.path.join(ROOT, "tests", "fake_research_engine.py")
sys.path.insert(0, ROOT)

from avas import dynamics as dyn  # noqa: E402
from avas import research  # noqa: E402
from avas.data.inputs import copy_text_inputs  # noqa: E402
from avas.research import Variant  # noqa: E402

LATTICE = ("start\ndrift 0.1 0.03 0\nS1: steerer 0 0.03 0 0 0 0 0.01\ndrift 0.4 0.03 0\nQ1: quad 0.1 0.03 0 5\n"
           "drift 0.5 0.03 0\nend\n")


@pytest.fixture
def project(tmp_path):
    inp = tmp_path / "proj" / "InputFile"
    copy_text_inputs(EXAMPLE_INPUT, str(inp))
    (inp / "lattice_mulp.txt").write_text(LATTICE)
    return tmp_path / "proj"


def calls(study):
    path = os.path.join(study.root, "calls.txt")
    if not os.path.isfile(path):
        return 0
    with open(path) as fh:
        return len(fh.read().splitlines())


def make_study(project, **kwargs):
    root = str(project / "Research" / "t")
    counter = os.path.join(root, "calls.txt")
    kwargs.setdefault("engine", "original")         # the stand-in engine, whatever AVAS_ENGINE says (see test_engines.py)
    return research.Study(str(project), name="t", engine_command=lambda inp, out, field, lat: [
        sys.executable, FAKE, inp, out, lat, counter], **kwargs)


# --------------------------------------------------------------------------- variants
def test_variant_addresses_and_flat_values():
    v = Variant().set("Q1.G", 6.0).set("beam.current", 2.0).set("input.stepsize", 0.01).set("seed", 3.0)
    assert v.lattice == {"Q1": {"G": 6.0}} and v.beam == {"current": 2.0} and v.seed == 3
    assert v.flat() == {"Q1.G": 6.0, "beam.current": 2.0, "input.stepsize": 0.01, "seed": 3}
    assert research.parse_address("12.Ke") == ("lattice", 12, "Ke")
    assert research.parse_address(("lattice", "Q.1", "G")) == ("lattice", "Q.1", "G")
    assert Variant.from_dict(v.to_dict()).key() == v.key() and v.get("Q1.G") == 6.0
    with pytest.raises(ValueError):
        research.parse_address("nodot")


# --------------------------------------------------------------------------- engine runs
def test_engine_run_record_cache_and_force(project):
    study = make_study(project)
    run = study.run(Variant(beam={"current": 50.0}, particles=777), "engine")
    assert run.ok and run.fidelity == "engine" and calls(study) == 1
    assert run.rms("x")[0] == pytest.approx(1.5e-3)
    assert run.centroid[0, 0] == pytest.approx(research.study.DEFAULT_SEED * 1e-6)
    assert run.alive[0] == 777 and run.transmission == 1.0
    assert run.metrics["rms_x_max"] == pytest.approx(1.5)
    assert sorted(os.listdir(run.output_dir)) == ["DataSet.txt", "avas_run.json"]     # bulky outputs removed
    assert "particlenumber 777" in (open(os.path.join(run.folder, "input", "beam.txt")).read())
    rec = run.record
    assert rec["status"] == "done" and rec["seed"] == research.study.DEFAULT_SEED and rec["variant"]["particles"] == 777
    again = study.run(Variant(beam={"current": 50.0}, particles=777, label="same"), "engine")
    assert calls(study) == 1 and again.folder == run.folder                          # cached: the label is not an input
    study.run(Variant(beam={"current": 50.0}, particles=777), "engine", force=True)
    assert calls(study) == 2
    other = study.run(Variant(beam={"current": 10.0}), "engine")
    assert other.folder != run.folder and calls(study) == 3
    table = study.table()
    assert len(table) == 2 and set(table["beam.current"]) == {50.0, 10.0}
    assert "m.rms_x_max" in table.columns
    study.annotate(other, note="reference", tags=["base"])
    assert study.load(other.folder).record["note"] == "reference"
    # the project's inputs were not touched
    assert "current 0.18409999999999999" in (project / "InputFile" / "beam.txt").read_text()


def test_a_relative_project_path_becomes_absolute(project, monkeypatch):
    # the engine runs in the run folder: a relative field map folder would not be found there
    monkeypatch.chdir(project.parent)
    st = research.Study("proj", name="rel", engine="original")
    assert st.input_dir == str(project / "InputFile")
    assert os.path.isabs(st.root) and st.root.startswith(str(project))


def test_failures_are_recorded_and_do_not_stop_a_batch(project):
    study = make_study(project)
    runs = study.run_many([Variant(input={"failme": 1}), Variant(beam={"current": 1.0})], "engine")
    assert not runs[0].ok and runs[0].record["status"] == "failed" and "told to fail" in runs[0].record["error"]
    assert runs[1].ok
    assert research.failed(runs) == [(runs[0].variant, runs[0].record["error"])]
    study.run(Variant(input={"failme": 1}), "engine")
    assert calls(study) == 2                                                          # a failure is cached too
    stop = threading.Event()
    stop.set()
    assert study.run_many([Variant(beam={"current": 3.0})], "engine", stop=stop) == [None]


def test_parallel_batch_dedupes_and_runs_single_threaded(project):
    study = make_study(project)
    variants = [Variant(beam={"current": float(c)}, input={"multithreading": 1}) for c in (1, 2, 3, 1, 2, 4)]
    seen = []
    runs = study.run_many(variants, "engine", workers=3, progress=lambda d, t, r: seen.append((d, t)))
    assert calls(study) == 4 and len(runs) == 6 and runs[0].folder == runs[3].folder
    assert seen[-1] == (4, 4)
    text = open(os.path.join(runs[0].folder, "input", "input.txt")).read()
    assert "multithreading" not in text and runs[0].record["single_thread"]
    assert [r.rms("x")[0] for r in runs] == pytest.approx([1.01e-3, 1.02e-3, 1.03e-3, 1.01e-3, 1.02e-3, 1.04e-3])


def test_fingerprint_follows_inputs_field_maps_and_strictness(project):
    inp = project / "InputFile"
    (inp / "um.bsz").write_text("1 0.1\n0 0 0\n0 0 0\n1\n0 0\n")
    (inp / "lattice_mulp.txt").write_text(LATTICE.replace("Q1: quad 0.1 0.03 0 5\n", "field 0.1 0.03 0 3 0 0 0 1 um\n"))
    study = make_study(project)
    first = study.run(None, "engine")
    assert calls(study) == 1
    st = os.stat(inp / "um.bsz")
    os.utime(inp / "um.bsz", ns=(st.st_atime_ns, st.st_mtime_ns + 10 ** 9))
    second = study.run(None, "engine")
    assert calls(study) == 2 and second.folder != first.folder
    strict = make_study(project, strict=True)
    a = strict.fingerprint(os.path.join(second.folder, "input"))
    os.utime(inp / "um.bsz", ns=(st.st_atime_ns, st.st_mtime_ns + 2 * 10 ** 9))
    assert strict.fingerprint(os.path.join(second.folder, "input")) == a          # same content
    assert study.fingerprint(os.path.join(second.folder, "input")) != a


# --------------------------------------------------------------------------- linear fidelity, scans, comparisons
def test_linear_runs_follow_the_model_and_the_inputs(project):
    study = make_study(project)
    base = study.run(None, "linear")
    tuned = study.run(Variant(lattice={"Q1": {"G": 8.0}}), "linear")
    direct = study.model.evaluate(knobs={"Q1": {"G": 8.0}})
    assert np.array_equal(tuned.sigma, direct.sigma) and not np.allclose(tuned.sigma[-1], base.sigma[-1])
    hot = study.run(Variant(beam={"current": 30.0}), "linear")
    assert hot.result.info["space_charge"] and hot.rms("x")[-1] > base.rms("x")[-1]
    off = study.run(Variant(beam={"current": 30.0}, input={"spacecharge": 0}), "linear")
    assert not off.result.info["space_charge"]
    kicked = study.run(Variant(lattice={"S1": {"By": 1e-3}}), "linear")
    assert kicked.metrics["centroid_x_max"] > 0 and base.metrics["centroid_x_max"] == 0
    assert calls(study) == 0 and not os.listdir(os.path.join(study.root, "runs"))


def test_grid_and_latin_hypercube(project):
    study = make_study(project)
    out = research.grid(study, {"Q1.G": [4.0, 6.0, 8.0], "beam.current": [0.0, 20.0]}, "linear",
                        metrics=["rms_x_out", lambda r: r.metrics["rms_y_out"]])
    assert out["rms_x_out"].shape == (3, 2) and out["<lambda>"].shape == (3, 2)
    assert out["runs"][1].variant.flat() == {"Q1.G": 4.0, "beam.current": 20.0}
    assert out["rms_x_out"][0, 1] > out["rms_x_out"][0, 0]
    eng = research.grid(study, {"beam.current": [0.0, 50.0]}, "engine", metrics=["rms_x_max", "transmission"])
    assert eng["rms_x_max"].tolist() == pytest.approx([1.0, 1.5]) and eng["transmission"].tolist() == [1.0, 1.0]
    sample = research.latin_hypercube({"Q1.G": (4, 8), "beam.current": (0, 10)}, 20, seed=1)
    g = np.array([v.get("Q1.G") for v in sample])
    assert len(sample) == 20 and g.min() >= 4 and g.max() <= 8
    assert sorted(np.floor((g - 4) / 4 * 20).astype(int).tolist()) == list(range(20))   # one sample per stratum


def test_compare_ensemble_and_engine_response(project):
    study = make_study(project)
    cmp = research.compare(study, Variant(lattice={"S1": {"By": 1e-3}}))
    assert len(cmp["z"]) == 400 and {r["quantity"] for r in cmp["summary"]} >= {"rms_x", "cx", "energy"}
    assert cmp["rms_x"]["engine"][0] == pytest.approx(1e-3 * (1 + 0.1841 / 100))      # the example's 0.1841 mA
    ens = research.ensemble(study, seeds=(1, 2, 3))
    assert ens["curves"]["cx"]["mean"][0] == pytest.approx(2e-6)
    assert ens["curves"]["cx"]["std"][0] == pytest.approx(1e-6)
    diag = dyn.Diagnostics([dyn.Monitor("M1", 0.6), dyn.Monitor("M2", 1.1)])
    resp = research.engine_response(study, [dyn.Knob("S1", "By", step=1e-3)], diag)
    assert resp.matrix[:, 0].tolist() == pytest.approx([0.6e-3, 0.0, 1.1e-3, 0.0])     # 1e-3 m/T · z of the monitor
    with pytest.raises(ValueError, match="deterministic"):
        research.engine_response(study, [dyn.Knob("Q1", "dx", kind="error")], diag)


def test_export_writes_arrays_and_table(project, tmp_path):
    study = make_study(project)
    runs = study.run_many([Variant(beam={"current": c}) for c in (0.0, 20.0)], "engine")
    runs += [study.run(Variant(lattice={"Q1": {"G": 6.0}}), "linear")]
    diag = dyn.Diagnostics([dyn.Monitor("M1", 0.6)])
    arrays = research.export(str(tmp_path / "data"), runs, points=50, diagnostics=diag)
    data = np.load(tmp_path / "data.npz", allow_pickle=True)
    assert data["inputs"].shape == (3, 2) and list(data["input_names"]) == ["Q1.G", "beam.current"]
    assert data["rms_x"].shape == (3, 50) and data["readings"].shape == (3, 2)
    assert list(data["fidelity"]) == ["engine", "engine", "linear"]
    assert math.isnan(arrays["inputs"][0, 0]) and arrays["inputs"][2, 0] == 6.0
    lines = (tmp_path / "data.csv").read_text().splitlines()
    assert len(lines) == 4 and lines[0].startswith("fidelity,ok,label,Q1.G,beam.current")


def test_study_root_outside_the_project_and_plots(project, tmp_path):
    root = tmp_path / "elsewhere"
    study = research.Study(str(project / "InputFile"), root=str(root))
    assert os.path.isfile(root / "study.json") and not (project / "Research").exists()
    pytest.importorskip("matplotlib")
    import matplotlib
    matplotlib.use("Agg")
    from avas.research import plots
    runs = [study.run(None, "linear"), study.run(Variant(lattice={"Q1": {"G": 7.0}}), "linear")]
    assert plots.runs(runs, "rms_x").get_lines()
    out = research.grid(study, {"Q1.G": [4.0, 6.0], "beam.current": [0.0, 10.0]}, "linear", metrics=["rms_x_out"])
    assert plots.heatmap(out, "rms_x_out") is not None
    shutil.rmtree(root)


def _engine_reason():
    import subprocess
    code = "from avas.core.MultiParticleEngine import MultiParticleEngine; MultiParticleEngine()"
    res = subprocess.run([sys.executable, "-c", code], cwd=ROOT, capture_output=True, text=True)
    return None if res.returncode == 0 else (res.stderr.strip().splitlines() or ["unknown"])[-1]


@pytest.mark.skipif(not os.path.isfile(os.path.join(EXAMPLE_INPUT, "hwr010.edz")), reason="example field maps not installed")
def test_real_engine_run_of_the_example(tmp_path):
    reason = _engine_reason()
    if reason:
        pytest.skip(f"engine not available: {reason}")
    study = research.Study(os.path.dirname(EXAMPLE_INPUT), root=str(tmp_path / "study"))
    run = study.run(Variant(particles=200), "engine")
    assert run.ok, run.record.get("error")
    lin = study.run(None, "linear")
    assert run.z[-1] == pytest.approx(lin.z[-1], abs=0.02)
    gain_engine, gain_model = run.energy[-1] - run.energy[0], lin.energy[-1] - lin.energy[0]
    assert gain_engine == pytest.approx(gain_model, rel=0.1)          # mean vs reference particle: within ~5 %
    again = study.run(Variant(particles=200), "engine")
    assert again.folder == run.folder and again.record["seconds"] == run.record["seconds"]
