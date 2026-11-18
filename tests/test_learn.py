"""Primer: labs (avas/learn/labs.py), particle animations (avas/learn/motion.py), the guided tutorial (avas/learn/tutorial.py),
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
from avas.learn import labs, motion, tutorial

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


def test_distribution_lab_same_rms_different_shapes():
    out = labs.run("distribution", {"N": 20000})
    v = {x["key"]: x["value"] for x in out["values"]}
    # textbook edge / rms ratios of the 4D distributions (sampling noise of a 20000-particle cloud)
    assert v["kv_edge"] == pytest.approx(2.0, rel=0.03)
    assert v["wb_edge"] == pytest.approx(math.sqrt(6), rel=0.04)
    assert v["pb_edge"] == pytest.approx(math.sqrt(8), rel=0.06)
    assert v["gs_edge"] > 3.5 and v["gs_tail"] == pytest.approx(4.55, abs=0.6)
    assert v["kv_tail"] < 0.5 < v["wb_tail"] < v["pb_tail"] < v["gs_tail"]
    # the cloud follows the Twiss parameters: rms x = sqrt(β ε / (βγ))
    _g, bg, *_ = labs._kin(1.527, labs.PROTON, 1)
    assert v["rms_x"] == pytest.approx(math.sqrt(0.377 * 0.138 / bg), rel=0.03)
    assert v["emit_g"] == pytest.approx(0.138 / bg, rel=0.03)
    clouds = [p for p in out["plots"] if p["id"] in ("kv", "wb", "pb", "gs")]
    assert len(clouds) == 4 and all(len(p["series"][0]["x"]) <= 2500 for p in clouds)
    assert labs.run("distribution", {"N": 300})["values"]       # small clouds work too


def test_story_line_is_the_first_part():
    with open(os.path.join(ROOT, "frontend", "src", "learn", "chapters.ts"), encoding="utf-8") as fh:
        ids = re.findall(r'\{ id: "([\w-]+)", part: "(\w+)"', fh.read())
    assert ids[0] == ("journey-overview", "journey")
    story = [i for i, part in ids if part == "journey"]
    assert story == ["journey-overview", "journey-source", "journey-rfq", "journey-mebt", "journey-sc", "journey-hebt"]
    for cid, _part in ids:
        for lang in ("zh", "en"):
            assert os.path.isfile(os.path.join(CONTENT, f"{cid}.{lang}.md")), (cid, lang)


BEAMLINE_CHAPTERS = ["drift", "quadrupole", "solenoid", "dipole", "steerer", "rf-cavity", "static-fields", "field-maps", "superpose"]


def _beamline_blocks(name):
    with open(os.path.join(CONTENT, name), encoding="utf-8") as fh:
        return re.findall(r"^```avas-beamline[ \t]*\n(.*?)\n```", fh.read(), re.M | re.S)


@pytest.mark.parametrize("lab_id", ["fodo", "solenoid", "dipole", "rf-cavity", "space-charge"])
def test_beam_line_labs_return_their_preview_for_the_2d_3d_view(lab_id):
    if lab_id == "rf-cavity" and paths.example_input_dir() is None:
        pytest.skip("example project not present")
    out = labs.run(lab_id)
    bl = out["beamline"]
    assert out["lattice"] and bl is not None
    assert len(bl["z"]) == len(bl["rms_x"]) == len(bl["rms_y"]) == len(bl["energy"]) > 10
    assert bl["elements"] and isinstance(bl["fieldDirs"], list)
    assert labs.run("kinematics")["beamline"] is None


def test_beamline_snippets_get_start_and_end():
    assert labs._with_ends("drift 0.1 0.02 0") == "start\ndrift 0.1 0.02 0\nend\n"
    full = "start\ndrift 0.1 0.02 0\nend\n"
    assert labs._with_ends(full) == full


@pytest.mark.skipif(paths.example_input_dir() is None, reason="example project not present")
def test_chapter_beam_lines_are_valid_lattices():
    """Every element chapter shows a beam line; the snippet parses cleanly and the preview evaluates it."""
    problems = []
    for cid in BEAMLINE_CHAPTERS:
        zh, en = _beamline_blocks(f"{cid}.zh.md"), _beamline_blocks(f"{cid}.en.md")
        if not zh or zh != en:
            problems.append(f"{cid}: needs the same avas-beamline blocks in both languages")
    for name in _chapter_files():
        for body in _beamline_blocks(name):
            assert body.isascii(), name
            doc = LatticeDocument(labs._with_ends(body), [paths.example_input_dir()])
            if not doc.elements() or doc.issue_count():
                problems.append(f"{name}: {[i for st in doc.elements() for i in st.issues]}")
            res = labs.beamline(body)
            if res["beamline"] is None or res["warnings"]:
                problems.append(f"{name}: preview {res['warnings']}")
    assert not problems, "\n".join(problems)


@pytest.mark.skipif(paths.example_input_dir() is None, reason="example project not present")
def test_beamline_rpc_sends_the_preview_as_blobs():
    from avas.gui import bridge
    res = ok("learn.beamline", text="drift 0.05 0.02 0\nfield 0.35 0.02 0 3 0 0 0 0.35 sol\ndrift 0.2 0.02 0")
    assert res["lattice"].startswith("start\n") and res["fieldDirs"]
    assert len(bridge.take_blob(res["beamline"]["rms_x"]["__blob__"])) > 10
    bad = ok("learn.beamline", text="field 0.2 0.02 0 3 0 0 0 1 nosuchmap")
    assert bad["lattice"] and (bad["beamline"] is None or bad["warnings"])


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


def test_tutorial_periodic_steps_have_their_own_gentler_defaults():
    defs = {s["id"]: {p["key"]: p["default"] for p in s["params"]} for s in tutorial.definitions()}
    assert defs["accelerate"]["Kb"] == 0.35 and defs["accelerate"]["Ke"] == 1.36
    for step in ("periods", "current"):
        assert defs[step]["Kb"] == tutorial.PERIOD_DEFAULTS["Kb"] and defs[step]["Ke"] == tutorial.PERIOD_DEFAULTS["Ke"]
        _step, v = tutorial.values(step)
        assert (v["Kb"], v["Ke"]) == (0.30, 0.8)
    assert tutorial.values("periods", {"Kb": 0.4})[1]["Kb"] == 0.4          # the reader's value still wins


@needs_example
@pytest.mark.parametrize("settings,below_90", [({}, True), ({"Kb": 0.35, "Ke": 1.36}, False)])
def test_tutorial_default_period_keeps_the_longitudinal_phase_advance_below_90_degrees(settings, below_90):
    # with the single-cavity settings of step 3 the first period has about 128 deg longitudinally and the
    # engine loses particles from the bucket on an 8-period line (Windows, 2026-11-02)
    from avas.sim import linear_optics as lo
    text = tutorial.lattice("periods", dict(settings, periods=1))
    _doc, tracker, _warn = lo._track(text, tutorial.preview_beam("periods"), [paths.example_input_dir()])
    steps = tracker.arrays()
    period = (steps["z"] > 0.05 + 1e-9) & (steps["z"] <= 0.81 + 1e-9)       # sol entry to the next period's entry
    m = np.eye(6)
    for step in steps["mats"][period]:
        m = step @ m
    mz = m[4:6, 4:6]
    cos_mu = np.trace(mz) / 2 / np.sqrt(abs(np.linalg.det(mz)))
    assert (0 < cos_mu < 1) == below_90                                    # 0 < sigma_l < 90 deg
    # transversely stable: all eigenvalues on the unit circle once the momentum gain is divided out
    gain = lo._kin(float(np.interp(0.81, steps["z"], steps["w"])), tracker.mass)[3] / lo._kin(tutorial.preview_beam("periods")["energy"], tracker.mass)[3]
    moduli = np.abs(np.linalg.eigvals(np.diag([1, gain, 1, gain]) @ m[:4, :4]))
    assert np.allclose(moduli, 1, atol=2e-3)


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


# =========================================================================== particle animations
def _tracks(res, key):
    return np.asarray(res[key], dtype=float).reshape(res["n"], len(res["s"]))


@pytest.mark.parametrize("motion_id", list(motion.MOTIONS))
def test_every_motion_runs_with_its_defaults(motion_id):
    res = motion.run(motion_id)
    ns = len(res["s"])
    assert ns > 10 and res["n"] == motion.N_PARTICLES and res["marked"] == [0, 1]
    for key in ("x", "xp", "y", "yp", "z"):
        assert len(res[key]) == res["n"] * ns
    for key in ("cx", "cy", "rms_x", "rms_y", "energy"):
        assert len(res[key]) == ns
    assert len(res["lost"]) == res["n"] and res["values"] and res["model"]
    LatticeDocument(res["lattice"])                                # a lattice the editor can parse
    m = motion.MOTIONS[motion_id]
    assert set(res["params"]) == {p.key for p in m.params}
    for p in m.params:
        assert p.min <= p.default <= p.max, p.key


def test_drawn_particles_have_the_model_rms_everywhere():
    """The particles are whitened to the beam's rms matrix and moved by the model's own maps,
    so their spread follows the model's rms envelope (with space charge, as the model has it)."""
    for motion_id in ("drift", "quad", "fodo"):
        res = motion.run(motion_id)
        for plane in ("x", "y"):
            tr = _tracks(res, plane)[2:]                              # the random particles
            spread = np.sqrt(np.mean((tr - np.mean(tr, axis=0)) ** 2, axis=0) * len(tr) / (len(tr) - 1))
            assert np.allclose(spread, res[f"rms_{plane}"], rtol=1e-6), (motion_id, plane)


def test_drawn_particles_have_the_bunch_length_and_the_tracked_ones_sit_at_its_centre():
    """The 3D view spreads the particles along the beam by their place in the bunch: the example
    bunch's rms length at the entrance, sqrt(β·ε/(βγ)) with the normalised emittance of beam.txt."""
    res = motion.run("quad")
    z = _tracks(res, "z")
    assert np.all(z[:2] == 0)                                       # the tracked ones stay in the field plane
    beam, _dirs = motion.example_beam()
    _alpha, beta, emit = beam["twiss"]["z"]
    gamma = 1 + beam["energy"] / beam["mass"]
    bg = math.sqrt(gamma ** 2 - 1)
    assert np.std(z[2:, 0], ddof=1) == pytest.approx(math.sqrt(beta * emit / bg), rel=1e-4)


def test_drift_particles_fly_straight_and_the_waist_is_where_alpha_puts_it():
    res = motion.run("drift", {"I": 0, "alpha": 1.0})
    s = np.asarray(res["s"])
    x = _tracks(res, "x")[0]
    assert np.allclose(np.diff(x) / np.diff(s), (x[-1] - x[0]) / (s[-1] - s[0]), rtol=1e-4)   # a straight line (float32)
    beam, _dirs = motion.example_beam()
    beta = beam["twiss"]["x"][1]
    v = values(res)
    assert v["waist"] == pytest.approx(1.0 / ((1 + 1.0) / beta), abs=0.011)                 # s_w = α/γ


def test_quad_motion_focuses_x_and_defocuses_y_for_positive_gradient():
    pos, neg = values(motion.run("quad", {"G": 10})), values(motion.run("quad", {"G": -10}))
    assert pos["rx"] < 2 < 5 < pos["ry"]
    assert neg["ry"] < 2 < 5 < neg["rx"]
    beam, _dirs = motion.example_beam()
    assert pos["f"] == pytest.approx(values(motion.run("quad"))["brho"] / (10 * 0.1))


def test_unstable_fodo_loses_particles_on_the_wall():
    stable, unstable = motion.run("fodo"), motion.run("fodo", {"G": 10})
    assert all(k < 0 for k in stable["lost"]) and values(stable)["through"] == 100
    lost = [i for i, k in enumerate(unstable["lost"]) if k >= 0]
    assert len(lost) > 40 and values(unstable)["through"] < 30
    x, y = _tracks(unstable, "x"), _tracks(unstable, "y")
    i, k = lost[0], unstable["lost"][lost[0]]
    assert math.hypot(x[i, k], y[i, k]) > 20 - 1e-6                # hits the 20 mm bore there …
    assert np.all(np.isnan(x[i, k + 1:]))                           # … and is gone afterwards
    assert any("wall" in w[0] for w in unstable["warnings"])


def test_orbit_correction_reduces_the_orbit_until_the_noise_wins():
    v0, v6 = values(motion.run("orbit", {"svs": 0})), values(motion.run("orbit", {"svs": 6}))
    assert v0["kick"] == 0 and v0["ax"] == v0["bx"]
    assert v6["ax"] < 0.05 * v0["bx"] and v6["ay"] < 0.05 * v0["by"]
    all12 = motion.run("orbit", {"svs": 12, "noise": 50})
    assert values(all12)["kick"] == pytest.approx(motion.ORBIT_MAX_KICK * 1e3)
    assert any("singular" in w[0] for w in all12["warnings"])
    assert all12["ghosts"] and not motion.run("orbit", {"svs": 0})["ghosts"]


def test_orbit_response_matrix_is_causal():
    beam, dirs = motion.example_beam()
    _text, _model, monitors, knobs, matrix, _quads = motion._orbit_setup(beam, dirs)
    rx = matrix[::2, ::2]                                            # x readings against the By knobs
    assert rx.shape == (2 * motion.ORBIT_CELLS, 2 * motion.ORBIT_CELLS)
    assert np.allclose(np.triu(rx, 1), 0.0, atol=1e-12)               # nothing upstream of a steerer moves
    assert np.all(np.abs(np.diag(rx)) > 0.01)
    assert [k.param for k in knobs[:2]] == ["By", "Bx"]


def test_motion_parameters_are_clipped_and_checked():
    assert motion.run("fodo", {"cells": 1000})["params"]["cells"] == 30
    with pytest.raises(labs.LabError):
        motion.run("quad", {"G": "abc"})
    with pytest.raises(labs.LabError):
        motion.run("nope")


def test_front_end_knows_the_same_motions():
    with open(DIRECTIVES, encoding="utf-8") as fh:
        text = fh.read()
    ids = re.search(r"MOTION_IDS = \[([^\]]*)\]", text).group(1)
    assert sorted(re.findall(r'"([\w-]+)"', ids)) == sorted(motion.MOTIONS)


def test_chapter_motion_presets_name_real_parameters():
    problems = []
    for name in _chapter_files():
        with open(os.path.join(CONTENT, name), encoding="utf-8") as fh:
            text = fh.read()
        for body in re.findall(r"^\s*```avas-motion[ \t]*\n(.*?)\n\s*```", text, re.M | re.S):
            words = body.split()
            m = motion.MOTIONS.get(words[0])
            if m is None:
                problems.append(f"{name}: unknown animation {words[0]}")
                continue
            params = {p.key: p for p in m.params}
            for w in words[1:]:
                key, _, value = w.partition("=")
                p = params.get(key)
                if p is None:
                    problems.append(f"{name}: animation {m.id} has no parameter {key}")
                elif not p.min <= float(value) <= p.max:
                    problems.append(f"{name}: animation {m.id} {key}={value} outside {p.min}…{p.max}")
    assert not problems, "\n".join(problems)


def test_motion_rpc_sends_trajectories_as_blobs():
    from avas.gui import bridge
    defs = ok("learn.motions")
    assert {d["id"] for d in defs} == set(motion.MOTIONS)
    res = ok("learn.motion", id="quad", params={"G": 8})
    assert "__blob__" in res["x"] and res["n"] == motion.N_PARTICLES
    assert len(bridge.take_blob(res["x"]["__blob__"])) == res["n"] * res["s"]["shape"][0]
    assert not rpc("learn.motion", id="quad", params={"G": "abc"})["ok"]


def test_solenoid_motion_turns_the_particles_and_focuses_both_planes():
    """Entering a solenoid with Bz > 0, the radial fringe field kicks a positive particle at +x towards −y
    (clockwise seen from downstream); B < 0 turns it the other way.  Both planes are focused alike."""
    for B, sign in ((1.5, -1), (-1.5, 1)):
        res = motion.run("solenoid", {"B": B})
        s = np.asarray(res["s"])
        y = _tracks(res, "y")[0]
        inside = (s > 0.06) & (s < 0.24)
        assert np.all(np.sign(y[inside][5:]) == sign), B
    v = values(motion.run("solenoid"))
    assert v["rx"] == pytest.approx(v["ry"], rel=0.05)
    assert v["theta"] == pytest.approx(math.degrees(1.5 / (2 * v["brho"]) * 0.2))


def test_steerer_motion_moves_the_beam_centre_the_documented_way():
    res = motion.run("steerer", {"By": 3, "Bx": 2})
    v = values(res)
    assert v["cx"] < 0 < v["cy"]                       # By > 0 → −x, Bx > 0 → +y (positive particles)
    assert v["tx"] == pytest.approx(-3e-3 * motion.STEERER_LENGTH / v["brho"] * 1e3)
    st = next(e for e in res["elements"] if e["key"] == "steerer")
    assert st["span"] == pytest.approx([0.1, 0.1 + motion.STEERER_LENGTH]) and res["centre"]


@needs_example
def test_dipole_motion_has_dispersion_and_the_field_of_its_bend():
    res = motion.run("dipole", {"alpha": 30, "rho": 0.5})
    plus, minus = (np.asarray(g["y"]) for g in res["ghosts"])
    assert plus[-1] > 0 > minus[-1]                    # more momentum: outside of the bend (+x)
    v = values(res)
    assert v["D"] == pytest.approx(plus[-1], rel=1e-6)
    bend = next(e for e in res["elements"] if e["key"] == "bend")
    assert bend["B"] == pytest.approx(v["B"]) and bend["alpha"] == 30 and bend["rho"] == 0.5
    assert next(e for e in motion.run("dipole", {"alpha": -30})["elements"] if e["key"] == "bend")["B"] < 0


@needs_example
def test_cavity_motion_exports_the_field_the_reference_particle_sees():
    """Ke·Ez(z)·cos(rfPhase) integrated along the cavity gives the model's energy gain, so the page can draw
    the RF field and the forces from these two arrays."""
    res = motion.run("cavity")
    s, w, phase = (np.asarray(res[k], dtype=float) for k in ("s", "energy", "rfPhase"))
    cav = next(e for e in res["elements"] if e["key"] == "field")
    ez = np.interp(s - cav["z0"], cav["ez"]["z"], cav["ez"]["e"], left=0, right=0)
    inside = np.isfinite(phase)
    gain = np.trapezoid(np.where(inside, ez * np.cos(np.radians(np.where(inside, phase, 0))), 0), s)
    assert gain == pytest.approx(w[-1] - w[0], rel=0.02)
    z = _tracks(res, "z")
    assert res["marks"] == [["z", 1], ["z", -1]] and z[0, 0] > 0 > z[1, 0]
    assert len(res["dp"]) == len(res["z"])


# ---------------------------------------------------------------- control labs
@pytest.mark.parametrize("lab_id", ["bpm", "feedback", "llrf", "controllability", "henon", "chaos-control"])
def test_control_labs_run_with_their_defaults(lab_id):
    res = labs.run(lab_id, {})
    assert res["values"] and res["plots"] and res["model"]


def test_bpm_reads_the_position_to_first_order():
    v = values(labs.run("bpm", {"x": 0.5, "y": 0, "a": 20}))
    assert v["xest"] == pytest.approx(0.5, rel=0.01)
    v = values(labs.run("bpm", {"x": 10, "y": 0, "a": 20}))
    assert v["xest"] < 10                              # Δ/Σ saturates far from the centre


@needs_example
def test_feedback_lab_is_stable_below_the_theoretical_gain_limit():
    """Integral feedback with one pulse of delay is stable for g < 1 (z² − z + g = 0)."""
    ok_, bad = values(labs.run("feedback", {"gain": 0.5, "delay": 1})), values(labs.run("feedback", {"gain": 1.2, "delay": 1}))
    assert ok_["closed"] < ok_["open"] and ok_["modulus"] < 1
    assert bad["modulus"] > 1 and bad["closed"] > bad["open"]
    assert ok_["limit"] == pytest.approx(1.0, abs=0.03)
    assert values(labs.run("feedback", {"delay": 0}))["limit"] == pytest.approx(2.0, abs=0.03)


def test_llrf_feedback_holds_the_field_until_the_delay_eats_the_phase_margin():
    v = values(labs.run("llrf", {}))
    assert v["closed_a"] < 0.1 * v["open_a"] and v["closed_p"] < 0.1 * v["open_p"]
    bad = labs.run("llrf", {"gain": 200, "delay": 20})
    assert values(bad)["margin"] < 0 and bad["warnings"]


@needs_example
def test_controllability_lab_shows_unseen_and_uncorrectable_orbit():
    full = values(labs.run("controllability", {}))
    few_bpms = values(labs.run("controllability", {"bpms": 4}))
    few_steerers = values(labs.run("controllability", {"steerers": 4}))
    assert full["all"] < 0.05 * full["before"]
    assert few_bpms["used"] < 1e-3 < few_bpms["all"]                 # zero where we look, not elsewhere
    assert few_steerers["used"] > 0.1 and few_steerers["rank"] == 4  # four knobs cannot cancel twelve readings


def test_henon_lab_has_a_finite_dynamic_aperture():
    v = values(labs.run("henon", {"amp": 1.2}))
    assert 0.1 < v["da"] < 1.2 and v["lost"] > 0


def test_chaos_control_catches_and_holds_the_unstable_fixed_point():
    v = values(labs.run("chaos-control", {}))
    assert v["slope"] < -1                                           # unstable without control
    assert v["caught"] is not None and v["spread"] < 0.01 and v["nudge"] <= 0.1
    assert values(labs.run("chaos-control", {"dmax": 0}))["caught"] is None
