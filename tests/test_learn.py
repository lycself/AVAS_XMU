"""Primer: labs (avas/learn/labs.py), the guided tutorial (avas/learn/tutorial.py),
their RPC service (avas/gui/services/learn.py) and the assistant's primer tools.

The tutorial's engine run is replaced by a stand-in that writes a DataSet.txt, so
these tests do not need the kernel; tests/test_gui.py style end-to-end runs of
the real engine are left to Windows CI and manual checks.
"""
import json
import math
import os
import re
import threading
import time

import numpy as np
import pytest

from avas import paths
from avas.data.lattice_doc import LatticeDocument
from avas.learn import labs, tutorial

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CONTENT = os.path.join(ROOT, "frontend", "src", "learn", "content")
DIRECTIVES = os.path.join(ROOT, "frontend", "src", "learn", "directives.ts")


def rpc(method, **params):
    from avas.gui import bridge, services  # noqa: F401 - registers the handlers
    return json.loads(bridge.dispatch(method, params))


def ok(method, **params):
    reply = rpc(method, **params)
    assert reply["ok"], f"{method}: {reply.get('error')}\n{reply.get('detail', '')}"
    return reply["result"]


def values(result):
    return {v["key"]: v["value"] for v in result["values"]}


# =========================================================================== labs
@pytest.mark.parametrize("lab_id", list(labs.LABS))
def test_every_lab_runs_with_its_defaults(lab_id):
    res = labs.run(lab_id)
    assert res["values"] and res["plots"] and res["model"]
    for plot in res["plots"]:
        for s in plot["series"]:
            assert len(s["x"]) == len(s["y"]) > 0
    lab = labs.LABS[lab_id]
    assert set(res["params"]) == {p.key for p in lab.params}
    for p in lab.params:
        assert p.min <= p.default <= p.max, p.key


def test_kinematics_of_the_example_proton():
    v = values(labs.run("kinematics", {"mass": 938.272088, "q": 1, "W": 1.527, "f": 162.5}))
    gamma = 1 + 1.527 / 938.272088
    beta = math.sqrt(1 - 1 / gamma ** 2)
    assert v["gamma"] == pytest.approx(gamma)
    assert v["beta"] == pytest.approx(beta)
    assert v["brho"] == pytest.approx(938.272088 * beta * gamma / 299.792458)
    assert v["bl"] == pytest.approx(beta * 299792458 / 162.5e6 * 1e3)


def test_phase_space_conserves_the_emittance_and_finds_the_waist():
    res = labs.run("phase-space", {"alpha": 0, "beta": 0.4, "emit": 0.14, "W": 1.527, "f": 0.5, "L": 0.5})
    v = values(res)
    assert v["area"] == pytest.approx(v["eg"])
    # after a thin lens at a waist: α = β/f, γ = (1 + α²)/β, waist at α/γ
    a = 0.4 / 0.5
    assert v["waist"] == pytest.approx(a / ((1 + a * a) / 0.4))


def test_fodo_phase_advance_matches_the_thin_lens_estimate_for_short_quads():
    v = values(labs.run("fodo", {"Lq": 0.02, "G": 20, "Ld": 0.38, "W": 1.527}))
    assert v["mux"] == pytest.approx(v["muthin"], rel=0.05)
    assert v["mux"] == pytest.approx(v["muy"], rel=0.05)


def test_fodo_matched_beam_keeps_its_size_and_a_mismatched_one_breathes():
    matched = labs.run("fodo", {"cells": 8})
    env = next(p for p in matched["plots"] if p["id"] == "envelope")
    x = np.asarray(env["series"][0]["y"])
    period = len(x) // 8
    assert np.allclose(x[::period][:8], x[0], rtol=1e-3)
    mismatched = labs.run("fodo", {"cells": 8, "mismatch": 1.5})
    env2 = next(p for p in mismatched["plots"] if p["id"] == "envelope")
    y = np.asarray(env2["series"][0]["y"])
    assert np.ptp(y[::period][:8]) > 0.05 * y[0]


def test_unstable_fodo_says_so():
    res = labs.run("fodo", {"G": 40, "Ld": 1.0})
    assert values(res)["mux"] is None
    assert res["warnings"]


def test_solenoid_rotation_and_focusing():
    res = labs.run("solenoid", {"L": 0.2, "B": 1.0, "W": 1.527, "mass": 938.272088, "q": 1})
    v = values(res)
    brho = v["brho"]
    k = 1.0 / (2 * brho)
    assert v["theta"] == pytest.approx(math.degrees(k * 0.2))
    assert v["f"] == pytest.approx(1 / (k * math.sin(k * 0.2)))
    m = np.array(res["matrices"][0]["rows"])
    j = np.array([[0, 1, 0, 0], [-1, 0, 0, 0], [0, 0, 0, 1], [0, 0, -1, 0]])
    assert np.allclose(m.T @ j @ m, j)                 # symplectic


def test_dipole_dispersion_of_a_sector_magnet():
    v = values(labs.run("dipole", {"angle": 45, "rho": 1.0, "N": 0, "edge": 0}))
    a = math.radians(45)
    assert v["D"] == pytest.approx(1.0 * (1 - math.cos(a)))
    assert v["Dp"] == pytest.approx(math.sin(a))
    assert v["R43"] == pytest.approx(0.0)


def test_rf_cavity_gains_most_on_crest():
    if paths.example_input_dir() is None:
        pytest.skip("example project not present")
    res = labs.run("rf-cavity", {"phase": -30, "Ke": 1.36, "W": 1.527})
    v = values(res)
    assert v["phis"] == pytest.approx(-30, abs=1e-6)
    assert 0 < v["gain"] < v["crest"]
    off = values(labs.run("rf-cavity", {"phase": -80, "Ke": 1.36, "W": 1.527}))
    assert off["gain"] < v["gain"]


def test_space_charge_lowers_the_phase_advance():
    zero = values(labs.run("space-charge", {"I": 0}))
    assert zero["ratio"] == pytest.approx(1.0)
    v = values(labs.run("space-charge", {"I": 20}))
    assert 0 < v["ratio"] < 1
    assert v["max_sc"] > v["max_0"]


def test_lab_parameters_are_clipped_and_checked():
    res = labs.run("fodo", {"cells": 1000, "G": "4"})
    assert res["params"]["cells"] == 30
    with pytest.raises(labs.LabError):
        labs.run("fodo", {"G": "abc"})
    with pytest.raises(labs.LabError):
        labs.run("nope")


def test_front_end_knows_the_same_labs():
    with open(DIRECTIVES, encoding="utf-8") as fh:
        text = fh.read()
    ids = re.search(r"LAB_IDS = \[([^\]]*)\]", text).group(1)
    assert sorted(re.findall(r'"([\w-]+)"', ids)) == sorted(labs.LABS)


def _chapter_files():
    return sorted(n for n in os.listdir(CONTENT) if n.endswith(".md")) if os.path.isdir(CONTENT) else []


def test_chapter_lab_presets_name_real_parameters():
    """``avas-lab`` blocks may preset parameters; they must exist and lie in range."""
    problems = []
    for name in _chapter_files():
        with open(os.path.join(CONTENT, name), encoding="utf-8") as fh:
            text = fh.read()
        for body in re.findall(r"^\s*```avas-lab[ \t]*\n(.*?)\n\s*```", text, re.M | re.S):
            words = body.split()
            lab = labs.LABS.get(words[0])
            if lab is None:
                problems.append(f"{name}: unknown lab {words[0]}")
                continue
            params = {p.key: p for p in lab.params}
            for w in words[1:]:
                key, _, value = w.partition("=")
                p = params.get(key)
                if p is None:
                    problems.append(f"{name}: lab {lab.id} has no parameter {key}")
                elif not p.min <= float(value) <= p.max:
                    problems.append(f"{name}: lab {lab.id} {key}={value} outside {p.min}…{p.max}")
    assert not problems, "\n".join(problems)


def test_chapter_parameter_tables_name_real_keywords():
    from avas.data import schema
    tables = {"lattice": schema.LATTICE_KEYWORDS, "beam": schema.BEAM_KEYWORDS, "input": schema.INPUT_KEYWORDS}
    problems = []
    for name in _chapter_files():
        with open(os.path.join(CONTENT, name), encoding="utf-8") as fh:
            text = fh.read()
        for body in re.findall(r"^\s*```avas-params[ \t]*\n(.*?)\n\s*```", text, re.M | re.S):
            for w in body.split():
                group, _, key = w.rpartition(":")
                if key.lower() not in tables.get(group or "lattice", {}):
                    problems.append(f"{name}: unknown keyword {w}")
    assert not problems, "\n".join(problems)


# =========================================================================== tutorial
needs_example = pytest.mark.skipif(paths.example_input_dir() is None, reason="example project not present")


@needs_example
@pytest.mark.parametrize("step", [s.id for s in tutorial.STEPS])
def test_tutorial_lattices_are_valid_multi_particle_lattices(step):
    text = tutorial.lattice(step)
    assert text.isascii()
    doc = LatticeDocument(text, [paths.example_input_dir()])
    els = doc.elements()
    assert els and doc.issue_count() == 0, [i for st in els for i in st.issues]
    assert {st.key for st in els} <= {"drift", "field"}
    assert els[0].key == "drift" and els[-1].key == "drift"     # no matrix element at the ends


@needs_example
def test_tutorial_steps_grow_the_line():
    from avas.sim import linear_optics as lo
    energies = []
    for step in ("accelerate", "periods"):
        res = lo.linear_preview(tutorial.lattice(step, {"periods": 3}), tutorial.preview_beam(step), [paths.example_input_dir()])
        energies.append(res["energy"][-1])
    assert energies[1] > energies[0] > 1.527
    assert tutorial.lattice("periods", {"periods": 4}).count("hwr010") == 4


@needs_example
def test_tutorial_beam_sets_current_and_particles():
    text = tutorial.beam_text("current", {"I": 7.5, "particles": 1500})
    assert re.search(r"^current 7\.5\b", text, re.M)
    assert re.search(r"^particlenumber 1500\b", text, re.M)
    assert "kneticenergy" in text


@needs_example
def test_tutorial_export_writes_a_project(tmp_path):
    folder = tutorial.export(str(tmp_path / "primer"), "periods", {"periods": 2})
    inp = os.path.join(folder, "InputFile")
    names = set(os.listdir(inp))
    assert {"beam.txt", "input.txt", "lattice_mulp.txt", "hwr010.edz", "sol.bsz"} <= names
    assert os.path.isdir(os.path.join(folder, "OutputFile"))
    with pytest.raises(labs.LabError):
        tutorial.export(folder, "periods")                       # not empty any more


# =========================================================================== service
@pytest.fixture
def user_data(tmp_path, monkeypatch):
    monkeypatch.setenv("AVAS_GUI_SETTINGS", str(tmp_path / "gui.json"))
    from avas.gui.services import learn
    monkeypatch.setattr(learn, "work_root", lambda: str(tmp_path / "learn" / "runs"))
    return tmp_path


def test_lab_rpc_sends_curves_as_blobs():
    from avas.gui import bridge
    defs = ok("learn.labs")
    assert {d["id"] for d in defs} == set(labs.LABS)
    res = ok("learn.lab", id="fodo", params={"G": 4})
    series = res["plots"][0]["series"][0]
    assert "__blob__" in series["x"]
    assert len(bridge.take_blob(series["x"]["__blob__"])) > 10
    assert not rpc("learn.lab", id="fodo", params={"G": "abc"})["ok"]


@needs_example
def test_tutorial_preview_rpc():
    steps = ok("learn.tutorial.steps")
    assert steps["available"] and [s["id"] for s in steps["steps"]] == [s.id for s in tutorial.STEPS]
    res = ok("learn.tutorial.preview", step="periods", params={"periods": 2})
    assert "hwr010" in res["lattice"] and res["params"]["periods"] == 2
    assert any(e["key"] == "field" and e["phase_s"] is not None for e in res["elements"])
    assert ok("learn.examples")["available"]


def _fake_dataset(out, rows=20, length=1.5):
    os.makedirs(out, exist_ok=True)
    d = np.zeros((rows, 41))
    d[:, 0] = np.linspace(1.527, 2.5, rows)
    d[:, 5] = np.linspace(0, length, rows)
    d[:, 16] = d[:, 18] = 1e-3
    d[:, 20] = 2e-3
    d[:, 28] = 2000
    np.savetxt(os.path.join(out, "DataSet.txt"), d)


@needs_example
def test_tutorial_run_uses_a_sandbox_outside_any_project(user_data, monkeypatch):
    from avas.ai import sandbox
    from avas.gui.services import learn, runner
    seen = {}

    def fake_run(self, lattice_text=None, files=None, stop_event=None, progress=None, timeout=None, keep=False):
        seen.update(root=self.root, lattice=lattice_text, beam=(files or {}).get("beam.txt"),
                    source=sandbox.activity_state()["source"], busy=runner.any_active())
        progress and progress(50.0)
        out = os.path.join(self.root, "run_001")
        _fake_dataset(out)
        return out, 1.0

    monkeypatch.setattr(sandbox.Sandbox, "run", fake_run)
    events = []
    from avas.gui import bridge
    monkeypatch.setattr(bridge, "emit", lambda name, payload: events.append((name, payload)))
    state = ok("learn.tutorial.run", step="periods", params={"periods": 2, "particles": 500})
    assert state["status"] == "running"
    learn._current["thread"].join(10)
    final = ok("learn.tutorial.state")
    assert final["status"] == "done", final
    assert seen["root"].startswith(str(user_data / "learn" / "runs"))
    assert seen["source"] == "learn" and seen["busy"]
    assert "hwr010" in seen["lattice"] and "particlenumber 500" in seen["beam"]
    assert not sandbox.active()                                   # the activity ended
    assert {"z", "rmsX", "energy", "alive"} <= set(final["result"])
    names = [e[0] for e in events]
    assert "learn.tutorial" in names
    assert all("result" not in p for n, p in events if n == "learn.tutorial")


@needs_example
def test_tutorial_run_refuses_while_another_simulation_runs(user_data, monkeypatch):
    from avas.gui.services import runner
    monkeypatch.setattr(runner, "any_active", lambda: True)
    assert not rpc("learn.tutorial.run", step="drift")["ok"]


@needs_example
def test_tutorial_export_rpc(tmp_path):
    res = ok("learn.tutorial.export", folder=str(tmp_path / "p"), step="focus", params={"Kb": 0.3})
    with open(os.path.join(res["path"], "InputFile", "lattice_mulp.txt"), encoding="utf-8") as fh:
        assert " 0.3 sol" in fh.read()
    assert not rpc("learn.tutorial.export", folder=res["path"], step="focus")["ok"]


def test_live_display_names_the_study_source(monkeypatch):
    from avas.ai import sandbox
    from avas.gui.services import live
    monkeypatch.setattr(sandbox, "live_source", lambda: {"job": "j1", "label": "Primer tutorial", "source": "learn", "total": 1,
                                                         "running": False, "index": None, "out": None, "lattice": None,
                                                         "field_dirs": []})
    mon = live.Monitor()
    monkeypatch.setattr(live.bridge, "emit", lambda *a: None)
    assert mon.tick()
    assert mon.run.kind == "learn"


# =========================================================================== assistant tools
class _Host:
    auto_apply = False

    def __init__(self, answers=None):
        self.answers = answers or {}
        self.calls = []

    def project(self):
        from avas.ai import ToolError
        raise ToolError("No project is open.")

    def front(self, method, params=None, timeout=5.0):
        self.calls.append((method, params))
        return self.answers.get(method)


def _tools(host):
    from avas.ai.avas_tools import build_tools
    return {t.name: t for t in build_tools(host)}


def test_primer_lab_tool_works_without_a_project():
    tool = _tools(_Host())["primer_lab"]
    listing = tool.handler({}, None)
    assert {lab["id"] for lab in listing["labs"]} == set(labs.LABS)
    res = tool.handler({"lab": "dipole", "params": {"angle": 30}}, None)
    d = next(v for v in res["values"] if v["name"].startswith("Dispersion"))
    assert d["value"] == pytest.approx(1 - math.cos(math.radians(30)), rel=1e-4)
    assert res["curves"] and res["matrices"]
    json.dumps(res)                                               # plain data for the model


def test_primer_text_tools_ask_the_page():
    from avas.ai import ToolError
    host = _Host({"learn.search": [{"id": "quadrupole-matrix", "chapter": "Quadrupole", "section": "Matrix", "snippet": "…"}],
                  "learn.read": {"id": "quadrupole", "text": "# …", "status": "ai-draft"},
                  "learn.show": True})
    tools = _tools(host)
    assert tools["primer_search"].handler({"query": "quad"}, None)["hits"][0]["id"] == "quadrupole-matrix"
    assert tools["primer_read"].handler({"id": "quadrupole"}, None)["status"] == "ai-draft"
    assert tools["primer_show"].handler({"id": "quadrupole"}, None)["shown"]
    with pytest.raises(ToolError):
        _tools(_Host())["primer_read"].handler({"id": "x"}, None)  # no page answered


def test_system_prompt_describes_the_tutor():
    from avas.ai.avas_prompt import build_system_prompt
    text = build_system_prompt({"facts": ["No project is open (the primer tools still work)."]})
    assert "primer_search" in text and "$$" in text


def test_assistant_facts_without_a_project_mention_the_primer_page():
    from avas.gui.services import assistant
    facts, _run_time = assistant._facts({"page": "learn", "primer": {"chapter": "solenoid", "section": "solenoid-busch"}})
    assert any("primer" in f for f in facts) and any("solenoid-busch" in f for f in facts)


def test_threads_do_not_leak():
    # the tutorial's worker thread is a daemon and ends with its run
    time.sleep(0.05)
    assert all(t.daemon for t in threading.enumerate() if t.name == "avas-learn")
