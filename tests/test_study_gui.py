"""GUI back-end tests of the Study page and the period panel (services study.* and analysis.*).

Linear-model calls need no engine; the engine path of a scan runs through the AI sandbox with the
stand-in engine of tests/fake_research_engine.py (Sandbox.run_in is replaced), so the activity,
records and events are exercised on any machine.
"""
import json
import os
import subprocess
import sys
import time

import numpy as np
import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXAMPLE = os.path.join(ROOT, "examples", "hwr010", "InputFile")
FAKE = os.path.join(ROOT, "tests", "fake_research_engine.py")
sys.path.insert(0, ROOT)
os.environ.setdefault("MPLBACKEND", "Agg")

LATTICE = """start
lattice 7 1
drift 0.05 0.03 0
S1: steerer 0 0.03 0 0 0 0 0.01
drift 0.05 0.03 0
QF: quad 0.1 0.03 0 8
B1: diag_position 0 0 0 0
drift 0.2 0.03 0
QD: quad 0.1 0.03 0 -8
drift 0.1 0.03 0
lattice_end
drift 0.05 0.03 0
S2: steerer 0 0.03 0 0 0 0 0.01
drift 0.05 0.03 0
quad 0.1 0.03 0 8
B2: diag_position 0 0 0 0
W2: diag_size 0 0 0 0
drift 0.2 0.03 0
quad 0.1 0.03 0 -8
drift 0.1 0.03 0
end
"""


def rpc(method, **params):
    from avas.gui import bridge
    return json.loads(bridge.dispatch(method, params))


def ok(method, **params):
    reply = rpc(method, **params)
    assert reply["ok"], f"{method}: {reply.get('error')}\n{reply.get('detail', '')}"
    return reply["result"]


def blob(ref):
    from avas.gui import bridge
    return bridge.take_blob(ref["__blob__"])


@pytest.fixture(scope="module")
def project(tmp_path_factory):
    root = tmp_path_factory.mktemp("studygui")
    os.environ["AVAS_GUI_SETTINGS"] = str(root / "gui-settings.json")
    inp = root / "proj" / "InputFile"
    from avas.data.inputs import copy_text_inputs
    copy_text_inputs(EXAMPLE, str(inp))
    (inp / "lattice_mulp.txt").write_text(LATTICE)
    os.makedirs(root / "proj" / "OutputFile")
    from avas.gui import services  # noqa: F401 - registers the handlers
    summary = ok("project.open", path=str(root / "proj"))
    assert summary["open"]
    yield root / "proj"
    ok("project.close")


def wait(method, key="running", timeout=120):
    deadline = time.time() + timeout
    while ok(method)[key]:
        assert time.time() < deadline, f"{method} did not finish"
        time.sleep(0.1)


def wait_event(events, name, timeout=5):
    """Events go out in batches (bridge flusher, every ~40 ms): wait for *name* to arrive."""
    deadline = time.time() + timeout
    while not any(name in e for e in events):
        assert time.time() < deadline, f"event {name} not delivered"
        time.sleep(0.05)


# --------------------------------------------------------------------------- scans
def test_linear_two_axis_scan_records_and_points(project, monkeypatch):
    from avas.gui import bridge
    from avas.gui.services import segments
    monkeypatch.setattr(segments, "_trash", lambda path: os.remove(path))
    axes = [{"kind": "lattice", "target": "6", "param": "G"}, {"kind": "beam", "keyword": "current"}]
    chk = ok("study.check", axes=axes, values=["6, 8, 10", "0:20:3"])
    assert chk["counts"] == [3, 3] and chk["count"] == 9 and chk["axes"][0]["param"] == "G"
    assert rpc("study.check", axes=[{"kind": "lattice", "target": "nosuch", "param": "G"}])["ok"] is False
    assert ok("study.metrics")["default"]
    events = []
    deliver = bridge.subscribe(lambda text: events.append(text))
    st = ok("study.start", axes=axes, values=["6, 8, 10", "0:20:3"], fidelity="linear",
            metrics=["rms_x_out", "energy_out"], label="grid")
    assert st["running"] is True
    wait("study.state")
    wait_event(events, "study.finished")
    bridge.unsubscribe(deliver)
    scan = ok("study.state")["scan"]
    assert scan["status"] == "finished" and len(scan["rows"]) == 9 and scan["shape"] == [3, 3]
    assert all(r["rms_x_out"] > 0 for r in scan["rows"])
    hot, cold = scan["rows"][2], scan["rows"][0]                       # same G, 20 mA against 0 mA
    assert hot["values"] == [6.0, 20.0] and hot["rms_x_out"] != cold["rms_x_out"]
    listed = ok("study.list")
    assert listed[0]["label"] == "grid" and listed[0]["fidelity"] == "linear" and listed[0]["done"] == 9
    assert ok("study.load", path=listed[0]["path"])["axes"][1]["keyword"] == "current"
    pt = ok("study.point", path=listed[0]["path"], index=5)
    assert len(blob(pt["z"])) == len(blob(pt["rms_x"])) > 10 and pt["fidelity"] == "linear"
    assert rpc("study.load", path=str(project / "InputFile" / "beam.txt"))["ok"] is False
    ok("study.delete", path=listed[0]["path"])
    assert not any(r["path"] == listed[0]["path"] for r in ok("study.list"))


def test_engine_scan_runs_through_the_sandbox(project, monkeypatch):
    from avas.ai import sandbox

    def fake_run_in(self, input_dir, out, field_dir, lattice, stop_event=None, progress=None, timeout=None, log_path=None):
        counter = os.path.join(str(project), "calls.txt")
        res = subprocess.run([sys.executable, FAKE, input_dir, out, lattice, counter], capture_output=True, text=True)
        if res.returncode:
            raise sandbox.SandboxError(res.stdout.strip() or "failed")
        return 0.1

    monkeypatch.setattr(sandbox.Sandbox, "run_in", fake_run_in)
    axes = [{"kind": "lattice", "target": "4", "param": "By"}]
    ok("study.start", axes=axes, values=["0, 0.001"], fidelity="engine", metrics=["centroid_x_max", "transmission"],
       label="kick")
    assert rpc("run.start")["ok"] is False                                    # the scan holds the runner
    wait("study.state")
    scan = ok("study.state")["scan"]
    assert scan["status"] == "finished", scan
    assert [r["values"] for r in scan["rows"]] == [[0.0], [0.001]]
    assert scan["rows"][1]["centroid_x_max"] > scan["rows"][0]["centroid_x_max"] and scan["rows"][0]["transmission"] == 1
    assert os.path.isdir(scan["rows"][0]["folder"]) and "Research" in scan["rows"][0]["folder"]
    # the notebook sees the same runs
    from avas import research
    assert len(research.Study(str(project), name="gui").records()) == 2
    # a second identical scan uses the cache
    ok("study.start", axes=axes, values=["0, 0.001"], fidelity="engine", label="again")
    wait("study.state")
    with open(os.path.join(str(project), "calls.txt")) as fh:
        assert len(fh.read().splitlines()) == 2
    pt = ok("study.point", path=ok("study.state")["scan"]["path"], index=2)
    assert pt["fidelity"] == "engine" and blob(pt["cx"]).max() > 0


# --------------------------------------------------------------------------- periods and charts
def test_period_panel_calls(project):
    periods = ok("analysis.periods", text=LATTICE)
    assert len(periods) == 1 and periods[0]["z1"] == pytest.approx(0.6)
    res = ok("analysis.period", text=LATTICE, period=1)
    assert res["stable"] and 0 < res["zero"]["x"]["mu"] < 180 and res["planes"] == "xy"
    assert res["matched"]["x"]["beta"] > 0 and res["matched"]["x"]["emit_n"] == pytest.approx(0.13768, rel=1e-3)
    assert res["envelope"]["stable"] and res["mismatch"]["x"] >= 0
    hot = ok("analysis.period", text=LATTICE, period=1, current=5.0)
    assert 0.5 < hot["tuneDepression"]["x"] < 0.7 and hot["current"] == 5.0          # σ ≈ 45° of σ0 ≈ 73°
    lost = ok("analysis.period", text=LATTICE, period=1, current=40.0)                  # no matched beam at 1.5 MeV
    assert any("did not converge" in m for m in lost["messages"])
    assert rpc("analysis.period", text=LATTICE, period=3)["ok"] is False
    table = ok("analysis.periodTable", text=LATTICE)
    assert table["count"] == 1 and blob(table["mu0_x"])[0] == pytest.approx(res["zero"]["x"]["mu"])


def test_stability_chart_in_a_thread(project):
    from avas.gui import bridge
    events = []
    deliver = bridge.subscribe(lambda text: events.append(text))
    ok("analysis.chartStart", period=1, x={"kind": "lattice", "target": "6", "param": "G"}, xs="6, 10",
       y={"kind": "current"}, ys="0, 5")
    wait("analysis.chartState")
    wait_event(events, "analysis.chartFinished")
    bridge.unsubscribe(deliver)
    result = ok("analysis.chartState")["result"]
    assert result["status"] == "finished" and result["done"] == 4
    grid = np.array(result["grid"]["sigma0"], dtype=float)
    assert grid.shape == (2, 2) and grid[0, 1] > grid[0, 0]                  # stronger quad, larger σ0
    assert np.array(result["grid"]["sigma"], dtype=float)[1, 0] < grid[1, 0]   # 5 mA depresses it


# --------------------------------------------------------------------------- monitors, response, feedback
def test_diagnostics_response_modes_and_feedback(project):
    spec = ok("analysis.diagnostics")
    assert spec["source"] == "lattice"
    assert [m["name"] for m in spec["monitors"]] == ["B1", "B2", "W2"]
    assert [k["param"] for k in spec["knobs"]] == ["By", "Bx", "By", "Bx"]
    ok("analysis.saveDiagnostics", monitors=spec["monitors"], knobs=spec["knobs"][:2])
    saved = ok("analysis.diagnostics")
    assert saved["source"] == "file" and len(saved["knobs"]) == 2
    resp = ok("analysis.response", monitors=spec["monitors"], knobs=spec["knobs"])
    assert len(resp["matrix"]) == 6 and len(resp["matrix"][0]) == 4 and resp["rank"] == 4
    assert resp["readings"][0] == "B1:px" and resp["knobs"][0] == "S1.By"
    modes = ok("analysis.periodModes", monitors=spec["monitors"], knobs=spec["knobs"], period=1)
    assert modes["knobs"] == ["S1.By", "S1.Bx"] and modes["controllableRank"] == 4 and modes["observableRank"] == 4
    assert {m["class"] for m in modes["modes"]} == {"stable"}
    fb = ok("analysis.feedback", monitors=spec["monitors"][:2], knobs=spec["knobs"], gain=0.5, delay=1, pulses=40,
            drift={"target": "QF", "key": "dx", "step": 1e-5}, noise=False)
    assert len(fb["rms"]) == 40 and fb["rms"][-1] < fb["openRms"][-1] and fb["predictedModulus"] < 1
    assert rpc("analysis.response", monitors=[], knobs=spec["knobs"])["ok"] is False


def test_assistant_analysis_tools(project):
    from avas.ai.avas_tools import build_tools
    from avas.gui import context

    class Host:
        def project(self):
            return context.project()

        def lattice_text(self):
            path = str(project / "InputFile" / "lattice_mulp.txt")
            with open(path) as fh:
                return "lattice_mulp.txt", path, fh.read(), "file"

    tools = {t.name: t for t in build_tools(Host())}
    period = tools["analyze_period"].handler({"period": 1, "changes": [{"name": "QF", "param": "G", "value": 9}]}, None)
    assert period["analysis"]["stable"] and period["hypothetical_changes"][0]["new"] == "9"
    grid = tools["linear_grid"].handler({"axes": [{"name": "QF", "param": "G", "values": [6, 8]},
                                                  {"param": "current", "values": [0, 2]}], "metrics": ["rms_x_max"]}, None)
    assert grid["columns"] == ["line6.G", "current_mA", "rms_x_max"] and len(grid["rows"]) == 4
    resp = tools["response_analysis"].handler({"period": 1}, None)
    assert resp["rank"] >= 1 and resp["period_modes"]["controllableRank"] == 4


def test_results_list_research_runs_and_overlay_the_linear_model(project):
    # the engine scan of test_engine_scan_runs_through_the_sandbox left two research runs (stand-in engine)
    sources = ok("results.sources")
    research = [s for s in sources if s["kind"] == "research"]
    assert len(research) == 2 and research[0]["study"] == "gui" and research[0]["seed"] == 12345
    out = research[0]["outputDir"]
    fig = ok("results.figure", plot="dataset", params={"type": "rms_x", "linear": True}, outputDir=out)
    dashed = [tr for tr in fig["traces"] if tr.get("dash")]
    assert len(dashed) == 1 and dashed[0]["name"] == "linear model rms x"
    plain = ok("results.figure", plot="dataset", params={"type": "rms_x"}, outputDir=out)
    assert not any(tr.get("dash") for tr in plain["traces"])
    from avas.gui.services.results import _run_inputs
    assert _run_inputs(str(project / "InputFile"), out).endswith(os.path.join(os.path.basename(os.path.dirname(out)), "input"))
