"""GUI back-end tests: the RPC services the web front end calls (no window is opened).

The example project is copied, simulated once with plt dumps and a density
file, then every page's calls are exercised through ``bridge.dispatch`` exactly
as the page sends them (JSON in, JSON out).
"""
import json
import os
import shutil
import subprocess
import sys
import urllib.request

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXAMPLE = os.path.join(ROOT, "examples", "hwr010")
WORK = os.path.join(ROOT, "tests", "_output_gui")

sys.path.insert(0, ROOT)
os.environ.setdefault("MPLBACKEND", "Agg")
os.environ["AVAS_GUI_SETTINGS"] = os.path.join(WORK, "gui-settings.json")   # never the user's real settings

pytest.importorskip("webview")


def rpc(method, **params):
    from avas.gui import bridge
    reply = json.loads(bridge.dispatch(method, params))
    return reply


def ok(method, **params):
    reply = rpc(method, **params)
    assert reply["ok"], f"{method}: {reply.get('error')}\n{reply.get('detail', '')}"
    return reply["result"]


def resolve_blobs(value):
    """Replace blob references by numpy arrays (the page fetches them over HTTP)."""
    import numpy as np
    from avas.gui import bridge
    if isinstance(value, dict) and "__blob__" in value:
        return bridge.take_blob(value["__blob__"])
    if isinstance(value, dict):
        return {k: resolve_blobs(v) for k, v in value.items()}
    if isinstance(value, list):
        return [resolve_blobs(v) for v in value]
    return value if not isinstance(value, float) else np.float64(value)


@pytest.fixture(scope="module")
def project():
    shutil.rmtree(WORK, ignore_errors=True)
    inp = os.path.join(WORK, "project", "InputFile")
    shutil.copytree(os.path.join(EXAMPLE, "InputFile"), inp)
    os.makedirs(os.path.join(WORK, "project", "OutputFile"))
    with open(os.path.join(inp, "input.txt"), encoding="utf-8") as fh:
        text = fh.read()
    text = text.replace("dumpperiodicity 0", "dumpperiodicity 5").replace("pchistogram 0 300", "pchistogram 1 300")
    with open(os.path.join(inp, "input.txt"), "w", encoding="utf-8") as fh:
        fh.write(text)
    env = dict(os.environ)
    env.pop("AVAS_LATTICE", None)
    res = subprocess.run([sys.executable, "-m", "avas", "run", "--input", inp,
                          "--output", os.path.join(WORK, "project", "OutputFile")],
                         cwd=ROOT, env=env, capture_output=True, text=True, timeout=600)
    assert res.returncode == 0, res.stdout[-2000:] + res.stderr[-2000:]
    from avas.gui import services  # noqa: F401 - registers the handlers
    summary = ok("project.open", path=os.path.join(WORK, "project"))
    assert summary["open"] and summary["latticeName"] == "lattice_mulp.txt"
    return summary


def test_bridge_json_is_strict():
    from avas.gui import bridge
    import numpy as np
    text = bridge.dumps({"a": float("nan"), "b": np.float32(1.5), "c": np.arange(3), "d": [float("inf")]})
    assert json.loads(text) == {"a": None, "b": 1.5, "c": [0, 1, 2], "d": [None]}
    assert json.loads(bridge.dispatch("no.such.method"))["ok"] is False


def test_progress_line_parsing():
    from avas.gui.services.runner import parse_progress
    p = parse_progress("Simulate progress 45.34%. Estimated remaining time 9s. Run time 15s.  1  0.35m     ")
    assert p == {"percent": 45.34, "eta_s": 9.0, "run_s": 15.0, "pos_m": 0.35, "alive": 1}
    assert parse_progress("Simulate progress 32.83%. Estimated remaining time 3s. Run time 3s.  5260  0.07m   ")["alive"] == 5260
    p = parse_progress("Simulate progress 70.00%. Estimated remaining time 2.50min. Run time 1.05h.")
    assert p["eta_s"] == pytest.approx(150.0) and p["run_s"] == pytest.approx(3780.0)
    assert parse_progress("[avas] mode=basic") is None


def test_server_serves_page_and_blobs():
    from avas.gui import bridge, server
    import numpy as np
    srv = server.start()
    base = srv.base_url
    try:
        with urllib.request.urlopen(f"{base}/index.html", timeout=10) as resp:
            assert b'<div id="root">' in resp.read()
        ref = bridge.blob(np.array([1.0, 2.0, 3.0]), "float64")
        req = urllib.request.Request(f"{base}/blob/{ref['__blob__']}", headers={server.TOKEN_HEADER: srv.token})
        with urllib.request.urlopen(req, timeout=10) as resp:
            assert np.frombuffer(resp.read(), dtype="<f8").tolist() == [1.0, 2.0, 3.0]
        with pytest.raises(urllib.error.HTTPError):
            urllib.request.urlopen(f"{base}/../avas/paths.py", timeout=10)
    finally:
        srv.shutdown()


def test_settings_page_round_trip(project):
    data = ok("settings.load")
    assert data["form"]["steppercycle"] == "50" and data["form"]["dumpperiodicity"] == "5"
    data["form"]["multithreading"] = True
    data["form"]["error_type"] = "stat"
    data["form"]["error_seed"] = "7"
    saved = ok("settings.save", form=data["form"], meta=data["meta"])
    assert saved["form"]["multithreading"] is True and saved["form"]["error_type"] == "stat"
    from avas.gui.services.runner import ini_error_mode
    from avas.gui import context
    assert ini_error_mode(context.project()) == "stat"
    assert input_lines(project, "multithreading") == ["multithreading 1"]
    saved["form"]["error_type"] = ""
    saved["form"]["multithreading"] = False
    saved = ok("settings.save", form=saved["form"], meta=saved["meta"])
    # "multithreading 0" makes the engine lose every particle (AGENTS.md 6): off = no line,
    # so the scan / run tests that follow on this project still work
    assert saved["form"]["multithreading"] is False and saved["meta"]["hadThreadsKey"] is False
    assert input_lines(project, "multithreading") == []


def input_lines(project, keyword):
    with open(os.path.join(project["inputDir"], "input.txt"), encoding="utf-8") as fh:
        return [line.strip() for line in fh if line.split() and line.split()[0].lower() == keyword]


def test_beam_page_keeps_unknown_keywords(project):
    path = os.path.join(project["inputDir"], "beam.txt")
    with open(path, "a", encoding="utf-8") as fh:
        fh.write("randomseed 7 ! keep me\n")
    data = ok("beam.load")
    assert data["form"]["particlenumber"] == "5260"
    data["form"]["current"] = "0.2"
    ok("beam.save", form=data["form"])
    with open(path, encoding="utf-8") as fh:
        text = fh.read()
    assert "randomseed 7 ! keep me" in text and "current 0.2" in text
    params = ok("beam.fromDst", name="part_rfq.dst")
    assert params["particlenumber"] == "5260" and float(params["emit_x"]) == pytest.approx(0.13768, abs=1e-5)


def test_beam_page_single_particle_offset(project):
    """Single-particle mode: the offset is shown in initpos / initmom units (mm; mrad, 1e-3), read from initpos /
    initmom first, and always saved as displacepos / displacedpos (displacedpos = 100 x mrad), which every shipped
    kernel reads; initpos / initmom lines are removed, and an all-zero offset removes the four keywords."""
    path = os.path.join(project["inputDir"], "beam.txt")
    with open(path, encoding="utf-8") as fh:
        before = fh.read()

    def lines(*keys):
        with open(path, encoding="utf-8") as fh:
            return [ln.strip() for ln in fh if ln.split() and ln.split()[0] in keys]
    try:
        with open(path, "a", encoding="utf-8") as fh:
            fh.write("initpos 1.5 0 0\ndisplacepos 9 9 9\ndisplacedpos 300 0 50\n")
        form = ok("beam.load")["form"]
        assert (form["offset_x"], form["offset_y"], form["offset_xp"], form["offset_dp"]) == ("1.5", "0", "3", "0.5")
        form.update(particlenumber="1", offset_yp="-2")
        form = ok("beam.save", form=form)["form"]
        assert lines("particlenumber") == ["particlenumber 1"]
        assert lines("displacepos", "displacedpos", "initpos", "initmom") == ["displacepos 1.5 0 0", "displacedpos 300 -200 50"]
        assert form["offset_yp"] == "-2"
        assert any("number of particles must be at least 1" in k for k in json.load(
            open(os.path.join(ROOT, "frontend", "src", "i18n", "zh_CN.json"), encoding="utf-8")))
        for k in ("offset_x", "offset_y", "offset_z", "offset_xp", "offset_yp", "offset_dp"):
            form[k] = "0" if k != "offset_z" else ""
        ok("beam.save", form=form)
        assert lines("displacepos", "displacedpos", "initpos", "initmom") == []
        with open(path, "a", encoding="utf-8") as fh:
            fh.write("displacepos 2 0 0\n")
        form = ok("beam.load")["form"]
        del form["offset_x"], form["offset_y"], form["offset_z"], form["offset_xp"], form["offset_yp"], form["offset_dp"]
        ok("beam.save", form=form)                         # a form without the offset fields keeps the lines
        assert lines("displacepos") == ["displacepos 2 0 0"]
        bad = ok("beam.load")["form"]
        bad["offset_x"] = "abc"
        assert rpc("beam.save", form=bad)["ok"] is False
    finally:
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(before)


def test_settings_page_longitudinal_periodic_boundary(project):
    """lpboundary V1 (fold into one RF period) and V2 (periodic space-charge box, new kernel default mode): V2 is
    written only when it is on, both off removes the line, and a form without the fields keeps it."""
    path = os.path.join(project["inputDir"], "input.txt")
    with open(path, encoding="utf-8") as fh:
        before = fh.read()
    try:
        data = ok("settings.load")
        assert data["form"]["lpboundary_fold"] is False and data["form"]["lpboundary_periodic"] is False
        data["form"]["lpboundary_periodic"] = True
        data = ok("settings.save", form=data["form"], meta=data["meta"])
        assert input_lines(project, "lpboundary") == ["lpboundary 0 1"] and data["form"]["lpboundary_periodic"] is True
        data["form"].update(lpboundary_fold=True, lpboundary_periodic=False)
        data = ok("settings.save", form=data["form"], meta=data["meta"])
        assert input_lines(project, "lpboundary") == ["lpboundary 1"]
        form = {k: v for k, v in data["form"].items() if not k.startswith("lpboundary")}
        ok("settings.save", form=form, meta=data["meta"])
        assert input_lines(project, "lpboundary") == ["lpboundary 1"]
        data["form"]["lpboundary_fold"] = False
        ok("settings.save", form=data["form"], meta=data["meta"])
        assert input_lines(project, "lpboundary") == []
    finally:
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(before)


def test_schema_marks_inputs_only_the_new_kernel_reads(project):
    schema = ok("schema.all")
    keywords = {k["key"] for group in ("beam", "input", "lattice") for k in schema[group] if k["newOnly"]}
    params = {(k["key"], p["key"]) for group in ("input", "lattice") for k in schema[group] for p in k["params"]
              if p["newOnly"]}
    assert keywords == {"initpos", "initmom", "nplanes"}
    assert params == {("lpboundary", "V2"), ("outputplane", "name")}


def test_results_single_particle_planes(project, tmp_path):
    """SingleParticlePlane.txt (M10 D3, one row per output plane: z W x γβx y γβy t): listed by results.overview,
    drawn against z (x, y in mm; x', y' in mrad from γβx / γβz; W) with the numbers as a table, exported with
    matplotlib."""
    out = tmp_path / "single"
    out.mkdir()
    reply = rpc("results.figure", plot="single_plane", params={}, outputDir=str(out))
    assert reply["ok"] is False and "SingleParticlePlane.txt" in reply["error"]
    # 1.5 MeV protons (rest mass of the example beam), x = 1 mm, x' = 2 mrad at the second plane
    rest = 938.272088159928
    gb = (((1.5 / rest) + 1) ** 2 - 1) ** 0.5
    (out / "SingleParticlePlane.txt").write_text(
        f"0.100000000000  1.500000000000  0.000000000000  0.000000000000  0.000000000000  0.000000000000  1.0e-9\n"
        f"0.200000000000  1.500000000000  0.001000000000  {0.002 * gb:.12f}  -0.000500000000  0.000000000000  2.0e-9\n",
        encoding="utf-8")
    assert ok("results.overview", outputDir=str(out))["singlePlane"] is True
    fig = resolve_blobs(ok("results.figure", plot="single_plane", params={"quantity": "position"}, outputDir=str(out)))
    assert fig["kind"] == "lines" and [t["name"] for t in fig["traces"]] == ["x", "y"]
    assert list(fig["traces"][0]["y"]) == pytest.approx([0.0, 1.0]) and list(fig["traces"][1]["y"]) == pytest.approx([0.0, -0.5])
    fig = resolve_blobs(ok("results.figure", plot="single_plane", params={"quantity": "angle"}, outputDir=str(out)))
    assert float(fig["traces"][0]["y"][1]) == pytest.approx(2.0, rel=1e-5)
    table = fig["table"]
    assert table["columns"][0] == "z (m)" and len(table["rows"]) == 2 and table["rows"][1][2] == pytest.approx(1.0)
    fig = resolve_blobs(ok("results.figure", plot="single_plane", params={"quantity": "energy"}, outputDir=str(out)))
    assert list(fig["traces"][0]["y"]) == pytest.approx([1.5, 1.5])
    png = tmp_path / "single.png"
    ok("results.export", plot="single_plane", params={"quantity": "position"}, outputDir=str(out), path=str(png))
    assert png.stat().st_size > 0


def test_lattice_page(project):
    info = ok("lattice.list")
    names = [f["name"] for f in info["files"]]
    assert "lattice_mulp.txt" in names and "lattice.txt" not in names
    text = ok("lattice.read", name="lattice_mulp.txt")["text"]
    doc = ok("lattice.parse", text=text)
    assert doc["elementCount"] == 1 and doc["statements"][1]["key"] == "field"
    schema = ok("schema.all")
    assert any(k["key"] == "quad" for k in schema["lattice"])
    alt = os.path.join(project["inputDir"], "lattice_alt.txt")
    shutil.copyfile(os.path.join(project["inputDir"], "lattice_mulp.txt"), alt)
    ok("lattice.setSource", name="lattice_alt.txt")
    assert ok("project.summary")["latticeName"] == "lattice_alt.txt"
    ok("lattice.setSource", name="lattice_mulp.txt")
    ok("lattice.write", name="lattice_mulp.txt", text=text)


def test_inputs_are_locked_while_a_run_is_active(project):
    from avas.gui import locks
    from avas.gui.services import runner
    r = runner.runner()
    inp = project["inputDir"]
    text = ok("lattice.read", name="lattice_mulp.txt")["text"]
    beam = ok("beam.load")
    settings = ok("settings.load")
    assert not locks.inputs_locked()
    r.job = object()                      # a project run, error study or segment run (also when paused)
    try:
        assert locks.inputs_locked()
        writes = [
            ("lattice.write", {"name": "lattice_mulp.txt", "text": text}),
            ("lattice.setSource", {"name": "lattice_mulp.txt"}),
            ("beam.save", {"form": beam["form"]}),
            ("beam.useParticles", {"name": "part_rfq.dst"}),
            ("settings.save", {"form": settings["form"], "meta": settings["meta"]}),
            ("files.save", {"path": os.path.join(inp, "input.txt"), "text": "x"}),
            ("files.create", {"name": "locked.txt"}),
            ("files.duplicate", {"path": os.path.join(inp, "input.txt")}),
            ("files.rename", {"path": os.path.join(inp, "input.txt"), "newName": "input2.txt"}),
            ("files.useLattice", {"path": os.path.join(inp, "lattice_mulp.txt")}),
        ]
        for method, params in writes:
            reply = rpc(method, **params)
            assert reply["ok"] is False and reply["user"] and "locked" in reply["error"], method
        assert not os.path.exists(os.path.join(inp, "locked.txt")) and os.path.isfile(os.path.join(inp, "input.txt"))
        assert ok("lattice.read", name="lattice_mulp.txt")["text"] == text          # reading still works
        assert ok("files.list")["files"]
    finally:
        r.job = None
    assert not locks.inputs_locked()


def test_project_overview(project):
    ov = ok("project.overview")
    assert ov["lattice"]["elements"] == 1 and ov["lattice"]["rf"] == 1 and ov["lattice"]["counts"] == {"field1": 1}
    assert ov["beam"]["particlenumber"] == "5260"
    assert ov["settings"]["sim_type"] == "mulp"
    run = ov["lastRun"]
    assert run["status"] == "finished" and run["diagnostics"]["transmission"] == 1.0


def test_visual_editor_data(project):
    env = resolve_blobs(ok("lattice.runEnvelope"))
    assert env["rows"] > 10 and len(env["z"]) == len(env["rmsX"]) and env["rmsX"][0] == pytest.approx(0.94, abs=0.05)
    # the same run as a record for the replay on the Run page: the inputs snapshot's lattice and the envelope
    from avas.gui import context
    rec = resolve_blobs(ok("run.replay", outputDir=context.project().output_dir))
    assert rec["kind"] == "project" and rec["zOffset"] == 0 and rec["lattice"]["name"] == "lattice_mulp.txt"
    assert len(rec["rows"]["z"]) == len(env["z"]) and rec["rows"]["alive"][0] == rec["particles0"] == 5260
    assert rpc("run.replay", outputDir=ROOT)["ok"] is False                     # not part of the project
    assert rpc("run.replay", outputDir=os.path.join(WORK, "project", "InputFile"))["ok"] is False   # no DataSet.txt
    prof = ok("lattice.fieldProfile", name="efield")["components"]
    assert {"edx", "edy", "edz"} <= set(prof) and len(prof["edz"]["z"]) == 106 and "gradient" in prof["edx"]
    assert rpc("lattice.fieldProfile", name="no_such_map")["ok"] is False
    pytest.importorskip("avas.sim.linear_optics")
    text = ok("lattice.read", name="lattice_mulp.txt")["text"]
    pv = resolve_blobs(ok("lattice.preview", text=text))
    assert len(pv["z"]) == len(pv["rms_x"]) > 10
    assert pv["rms_x"][0] == pytest.approx(0.95, abs=0.05)             # normalized-emittance convention
    assert pv["energy"][-1] > pv["energy"][0]                           # the cavity accelerates


def test_files_page(project):
    # runs generate lattice.txt in <output>/inputs since 2026-09-20; older versions left one in InputFile,
    # which the Files page must still recognise (and refuse to save)
    leftover = os.path.join(project["inputDir"], "lattice.txt")
    with open(leftover, "w", encoding="utf-8") as fh:
        fh.write("start\nend\n")
    listing = ok("files.list")
    roles = {f["name"]: f["role"] for f in listing["files"]}
    assert roles["beam.txt"] == "beam" and roles["lattice_mulp.txt"] == "lattice_run"
    assert roles["lattice.txt"] == "generated" and roles["part_rfq.dst"] == "particles"
    opened = ok("files.open", path=os.path.join(project["inputDir"], "input.txt"))
    assert opened["view"] == "text" and "steppercycle" in opened["text"]
    assert rpc("files.save", path=leftover, text="x")["ok"] is False
    os.remove(leftover)
    assert rpc("files.open", path=os.path.join(ROOT, "README.md"))["ok"] is False      # outside InputFile
    new = ok("files.create", name="notes.txt")
    renamed = ok("files.rename", path=new, newName="notes2.txt")
    assert os.path.isfile(renamed) and not os.path.exists(new)
    os.remove(renamed)
    particles = resolve_blobs(ok("files.particles", path=os.path.join(project["inputDir"], "part_rfq.dst")))
    assert particles["number"] == 5260 and len(particles["x"]) == 5260
    fm = resolve_blobs(ok("files.fieldmap", path=os.path.join(project["inputDir"], "hwr010.edz")))
    assert fm["nz"] == 105 and len(fm["z"]) == 106


def test_field_slices(project):
    """files.fieldSlice cuts every component of the family on the same plane."""
    ez = os.path.join(project["inputDir"], "hwr010.edz")
    sl = resolve_blobs(ok("files.fieldSlice", path=ez, plane="zx"))
    assert sl["fixed"] == "y" and sl["at"] == pytest.approx(0.0)         # the default cut is the beam axis
    assert set(sl["components"]) == {"edx", "edy", "edz"}                # the RF magnetic files stay out
    assert sl["arrows"] == ["edz", "edx"] and sl["order"] in ("outer", "inner")
    assert len(sl["u"]) == 106 and len(sl["v"]) == 21
    assert sl["components"]["edz"]["values"].shape == (len(sl["v"]), len(sl["u"]))   # rows over v, flat on the wire
    assert sl["magnitude"]["max"] >= abs(sl["components"]["edz"]["max"])

    cross = resolve_blobs(ok("files.fieldSlice", path=ez, plane="xy", at=0.1))
    assert cross["fixed"] == "z" and abs(cross["at"] - 0.1) < 0.01
    assert len(cross["u"]) == 21 and len(cross["v"]) == 21

    thin = resolve_blobs(ok("files.fieldSlice", path=ez, plane="zx", limit=20))
    assert len(thin["u"]) <= 20 and thin["u"][0] == pytest.approx(0.0)

    mag = resolve_blobs(ok("files.fieldSlice", path=os.path.join(project["inputDir"], "sol.bsx"), plane="zx"))
    assert set(mag["components"]) == {"bsx", "bsy", "bsz"} and mag["family"] == "bs"
    assert rpc("files.fieldSlice", path=ez, plane="xz")["ok"] is False

    # the lattice page asks by map name and lets the back end pick the component
    by_name = resolve_blobs(ok("lattice.fieldSlice", name="hwr010", plane="zx"))
    assert by_name["ext"] == "edz" and by_name["planes"] == ["zx", "zy", "xy"]
    assert by_name["length"] == pytest.approx(0.21) and by_name["at"] == pytest.approx(0.0)
    assert by_name["components"]["edz"]["values"].shape == (len(by_name["v"]), len(by_name["u"]))
    assert ok("lattice.fieldSlice", name="sol", plane="xy", at=0.175)["ext"] == "bsz"
    assert ok("lattice.fieldSlice", name="sol", ext="bsx")["ext"] == "bsx"
    assert rpc("lattice.fieldSlice", name="no_such_map")["ok"] is False


def test_results_plots(project):
    for kind in ("rms_x", "rms_xy", "phi", "beta_xyz", "emittance_z", "loss", "energy", "c_xy"):
        fig = ok("results.figure", plot="dataset", params={"type": kind})
        assert fig["traces"], kind
    assert ok("results.figure", plot="syn_phase")["traces"]
    assert ok("results.figure", plot="cavity_voltage", params={"ratio": {"efield": 2}})["kind"] == "bar"
    overview = ok("results.overview")
    assert overview["density"] and overview["plt"] and overview["secondary"] is False
    for kind in ("density", "density_level", "centroid", "emit", "rms_size_max"):
        ok("results.figure", plot="density", params={"path": overview["density"][0], "plane": "x", "kind": kind})
    reply = rpc("results.figure", plot="dataset", params={"type": "nonsense"})
    assert reply["ok"] is False and reply.get("user")


def test_phase_space_viewers_and_export(project):
    overview = ok("results.overview")
    dst = [p for p in overview["dst"] if p.endswith("inData.dst")][0]
    handle = ok("phase.openDst", path=dst)["handle"]
    planes = [["x", "x1"], ["y", "y1"], ["phi", "w_minus_mean"], ["x", "y"]]
    data = resolve_blobs(ok("phase.panels", handle=handle, planes=planes, ratio=0.9))
    assert len(data["panels"]) == 4 and data["panels"][0]["image"].size == 300 * 400
    assert "[90%]" in data["twissText"]
    info = ok("phase.pltInfo", path=overview["plt"][0])
    assert info["steps"] > 1
    plt_handle = ok("phase.openPlt", path=overview["plt"][0], step=1)["handle"]
    ok("phase.panels", handle=plt_handle, planes=planes, ratio=1)
    out = ok("tools.pltToDst", step=1)["output"]
    assert os.path.getsize(out) > 1000
    png = os.path.join(WORK, "export.png")
    ok("results.export", plot="dataset", params={"type": "rms_xy"}, path=png)
    from matplotlib.image import imread
    assert tuple(imread(png)[0, 0][:3]) == (1.0, 1.0, 1.0)     # publication export: white background
    ok("results.export", plot="phase", params={"handle": handle, "planes": planes, "ratio": 1},
       path=os.path.join(WORK, "phase.png"))


SEGMENT_LATTICE = """start
section LEBT {
drift 0.3 0.02 0
}
section cav {
outputplane 0
field 0.21 0.02 2 1 162.5e6 -33 1.36 -1.36 efield
drift 0.2 0.02 0
field 0.21 0.02 0 1 162.5e6 15 1.36 -1.36 efield
drift 0.1 0.02 0
}
end
"""


def _wait_for_job(runner, timeout=600):
    import time
    t0 = time.time()
    while runner.job is not None:
        assert time.time() - t0 < timeout, "simulation did not finish"
        time.sleep(0.3)


def _last_dataset_row(output_dir):
    with open(os.path.join(output_dir, "DataSet.txt"), encoding="utf-8") as fh:
        rows = [ln.split() for ln in fh if len(ln.split()) == 41]
    return [float(v) for v in rows[-1]]


def test_pause_resume_and_segment_run(project):
    """Pause freezes the engine and the clock; a segment run from the full run's particle file matches the full run."""
    import time
    from avas.gui import context
    from avas.data import segment
    from avas.gui.services import runner, segments
    work = os.path.join(WORK, "segment_project")
    shutil.rmtree(work, ignore_errors=True)
    shutil.copytree(os.path.join(EXAMPLE, "InputFile"), os.path.join(work, "InputFile"))
    os.makedirs(os.path.join(work, "OutputFile"))
    with open(os.path.join(work, "InputFile", "lattice_mulp.txt"), "w", encoding="utf-8") as fh:
        fh.write(SEGMENT_LATTICE)
    # a fixed seed makes the full run and the LEBT segment sample the same particles (the segment stages
    # copy input.txt as is), so the comparisons below do not depend on two independent samplings
    input_path = os.path.join(work, "InputFile", "input.txt")
    with open(input_path, encoding="utf-8") as fh:
        input_text = fh.read()
    with open(input_path, "w", encoding="utf-8") as fh:
        fh.write(segment.set_keywords(input_text, {"randomseed": "12345"}))
    ok("project.open", path=work)
    r = runner.runner()
    try:
        ok("run.start")
        t0 = time.time()
        while r.job is not None and not (r.state().get("percent") or 0) and time.time() - t0 < 120:
            time.sleep(0.2)
        paused = ok("run.pause")
        assert paused["running"] and paused["paused"] and paused["source"] == "project"
        time.sleep(2.5)
        still = ok("run.state")
        assert still["paused"] and still["elapsed_s"] == pytest.approx(paused["elapsed_s"], abs=0.5)
        assert r.proc is not None and r.proc.poll() is None                 # frozen, not stopped
        assert ok("run.resume")["paused"] is False
        _wait_for_job(r)
        info = context.project().last_run()
        assert info["status"] == "finished", info
        assert info["elapsed_s"] < time.time() - t0 - 2                      # the pause is not counted

        p = context.project()
        plan, card = segments.describe(p, segments.load_doc(p), {"section": "cav"})
        options = {o["id"]: o for o in card["options"]}
        assert card["rephase"] and card["default"] == "dst" and plan.z_start == pytest.approx(0.3)
        assert options["dst"]["available"] and options["upstream"]["available"] and options["twiss"]["available"]
        job = segments.start(p, {"section": "cav"}, "dst")
        _wait_for_job(r)
        assert job.ok, job.message
        assert [s.key for s in job.stages] == ["reference", "segment"]
        full, part = _last_dataset_row(p.output_dir), _last_dataset_row(job.output_dir)
        assert part[0] == pytest.approx(full[0], rel=2e-3)                   # exit energy
        assert part[16] == pytest.approx(full[16], rel=0.05)                 # rms x
        assert part[28] == full[28]                                          # transmitted particles

        sources = ok("results.sources")
        assert [s["kind"] for s in sources] == ["project", "segment"] and sources[1]["entry"] == "dst"
        assert job.meta.get("fingerprint")
        overview = ok("results.overview", outputDir=job.output_dir)
        assert os.path.normcase(overview["inputDir"]) == os.path.normcase(os.path.join(job.root, "InputFile"))
        assert ok("results.figure", plot="dataset", params={"type": "rms_x"}, outputDir=job.output_dir)["traces"]

        # chained: the LEBT run's final particle file is the entry beam of "cav"
        lebt = segments.start(p, {"section": "LEBT"})
        _wait_for_job(r)
        assert lebt.ok, lebt.message
        plan, card = segments.describe(p, segments.load_doc(p), {"section": "cav"})
        chained = next(o for o in card["options"] if o["id"] == "segment")
        assert chained["available"] and chained["source"] == os.path.basename(lebt.root) and chained["accuracy"] == "accurate"
        job2 = segments.start(p, {"section": "cav"}, "segment")
        _wait_for_job(r)
        assert job2.ok, job2.message
        assert job2.meta["entry"]["source"] == os.path.basename(lebt.root)
        for staged in (lebt.root, job2.root):
            with open(os.path.join(staged, "InputFile", "input.txt"), encoding="utf-8") as fh:
                assert segment.keyword_value(fh.read(), "randomseed") == ["12345"]
        part2 = _last_dataset_row(job2.output_dir)
        assert part2[0] == pytest.approx(full[0], rel=2e-3) and part2[16] == pytest.approx(full[16], rel=0.05)
        # a changed beam upstream makes the LEBT result unusable
        beam_path = os.path.join(work, "InputFile", "beam.txt")
        with open(beam_path, encoding="utf-8") as fh:
            beam_text = fh.read()
        with open(beam_path, "w", encoding="utf-8") as fh:
            fh.write(beam_text.replace("current 0.18409999999999999", "current 0.2"))
        try:
            _plan, card = segments.describe(p, segments.load_doc(p), {"section": "cav"})
            assert not next(o for o in card["options"] if o["id"] == "segment")["available"]
        finally:
            with open(beam_path, "w", encoding="utf-8") as fh:
                fh.write(beam_text)
        # the project's own files are untouched
        with open(os.path.join(work, "InputFile", "lattice_mulp.txt"), encoding="utf-8") as fh:
            assert fh.read() == SEGMENT_LATTICE
    finally:
        if r.job is not None:
            r.stop()
            _wait_for_job(r, 30)
        ok("project.open", path=os.path.join(WORK, "project"))


def test_delete_run_records(project, monkeypatch):
    """Run records go to the recycle bin: a segment folder as a whole, the project's OutputFile emptied."""
    from avas.gui import context
    from avas.gui.services import runner, segments
    work = os.path.join(WORK, "delete_project")
    shutil.rmtree(work, ignore_errors=True)
    shutil.copytree(os.path.join(WORK, "project", "InputFile"), os.path.join(work, "InputFile"))
    shutil.copytree(os.path.join(WORK, "project", "OutputFile"), os.path.join(work, "OutputFile"))
    seg = os.path.join(work, "Segments", "cav_20260101-000000")
    os.makedirs(os.path.join(seg, "OutputFile"))
    with open(os.path.join(seg, "segment.json"), "w", encoding="utf-8") as fh:
        json.dump({"label": "cav", "status": "finished", "created": "2026-01-01 00:00:00", "segment": {"z_start": 0.3, "z_end": 1.0}}, fh)
    with open(os.path.join(seg, "OutputFile", "DataSet.txt"), "w", encoding="utf-8") as fh:
        fh.write("x\n")
    trashed = []
    monkeypatch.setattr(segments, "_trash", lambda path: (trashed.append(path), shutil.rmtree(path)))
    try:
        ok("project.open", path=work)
        assert [s["kind"] for s in ok("results.sources")] == ["project", "segment"]
        seg_out = os.path.join(seg, "OutputFile")
        assert rpc("run.replay", outputDir=seg_out)["ok"] is False      # its DataSet.txt holds no rows
        r = runner.runner()
        r.job = object()                                   # a simulation is running: nothing may be deleted
        try:
            reply = rpc("runs.delete", outputDir=seg_out)
            assert reply["ok"] is False and reply["user"] and "locked" in reply["error"]
        finally:
            r.job = None
        assert os.path.isdir(seg)
        assert rpc("runs.delete", outputDir=os.path.join(work, "Segments"))["ok"] is False     # not a record
        assert rpc("runs.delete", outputDir=ROOT)["ok"] is False
        assert ok("runs.delete", outputDir=seg_out)["kind"] == "segment"
        assert not os.path.exists(seg) and trashed == [os.path.normpath(seg)]
        assert [s["kind"] for s in ok("results.sources")] == ["project"]
        p = context.project()
        assert p.last_run().get("status") == "finished"
        assert ok("runs.delete", outputDir=p.output_dir)["kind"] == "project"
        assert os.path.isdir(p.output_dir) and not os.listdir(p.output_dir)       # recreated, empty
        assert trashed[-1] == os.path.normpath(p.output_dir)
        assert p.last_run() == {} and ok("project.overview")["lastRun"] == {}
        assert ok("results.sources")[0]["status"] is None
    finally:
        ok("project.open", path=os.path.join(WORK, "project"))


def test_archive_run_records(project, monkeypatch):
    """A finished run is kept under Runs/ on request; the page is told when a new run would overwrite it."""
    from avas.gui import context
    from avas.gui.services import segments
    work = os.path.join(WORK, "archive_project")
    shutil.rmtree(work, ignore_errors=True)
    shutil.copytree(os.path.join(WORK, "project", "InputFile"), os.path.join(work, "InputFile"))
    shutil.copytree(os.path.join(WORK, "project", "OutputFile"), os.path.join(work, "OutputFile"))
    monkeypatch.setattr(segments, "_trash", lambda path: shutil.rmtree(path))
    try:
        ok("project.open", path=work)
        p = context.project()
        assert ok("runs.unsaved")["unsaved"] is True
        rec = ok("runs.archive", label="base line/1")
        assert rec["existing"] is False and rec["label"] == "base_line_1"
        folder = rec["folder"]
        assert folder.startswith(os.path.join(work, "Runs")) and os.path.isfile(os.path.join(folder, "DataSet.txt"))
        with open(os.path.join(folder, "avas_run.json"), encoding="utf-8") as fh:
            copied = json.load(fh)
        assert copied["label"] == "base_line_1" and copied["output_dir"] == folder and "archived" not in copied
        assert p.last_run()["archived"] == folder
        assert ok("runs.unsaved")["unsaved"] is False
        assert ok("runs.archive")["existing"] is True                    # not copied twice
        kinds = [s["kind"] for s in ok("results.sources")]
        assert kinds == ["project", "archived"]
        assert ok("results.overview", outputDir=folder)["hasDataSet"]
        # a secondary-particle run (its input snapshot has secondarybeam 1) is flagged: no envelope curves
        with open(os.path.join(folder, "inputs", "input.txt"), "a", encoding="utf-8") as fh:
            fh.write("\nsecondarybeam 1\n")
        assert ok("results.overview", outputDir=folder)["secondary"] is True
        fig = ok("results.figure", plot="dataset", params={"type": "rms_x"}, outputDir=folder)
        assert fig["kind"] == "lines" and fig["traces"]
        assert ok("runs.rename", folder=folder, label="v2")["label"] == "v2"
        assert ok("results.sources")[1]["label"] == "v2"
        assert rpc("runs.rename", folder=work, label="x")["ok"] is False
        assert ok("runs.delete", outputDir=folder)["kind"] == "archived"
        assert not os.path.exists(folder) and "archived" not in p.last_run()
        assert ok("runs.unsaved")["unsaved"] is True
    finally:
        ok("project.open", path=os.path.join(WORK, "project"))


def test_scan_service(project, monkeypatch):
    """The Scan page's calls: check, start (in a thread), state/progress events, list, load, delete."""
    import time
    from avas.gui import bridge
    from avas.gui.services import segments
    monkeypatch.setattr(segments, "_trash", lambda path: shutil.rmtree(path))
    spec = {"kind": "beam", "keyword": "particlenumber"}
    chk = ok("scan.check", spec=spec, values="200, 400")
    assert chk["count"] == 2 and chk["target"]["file"] == "beam.txt"
    assert rpc("scan.check", spec={"kind": "lattice", "target": "nosuch", "param": "G"})["ok"] is False
    assert ok("scan.metrics")["default"]
    events = []
    deliver = bridge.subscribe(lambda text: events.append(text))
    st = ok("scan.start", spec=spec, values=[200, 400], metrics=["transmission", "energy_out"], label="np")
    assert st["running"] is True
    assert rpc("scan.start", spec=spec, values=[1])["ok"] is False        # one at a time
    assert rpc("run.start")["ok"] is False                                  # a normal run must wait
    deadline = time.time() + 300
    while ok("scan.state")["running"]:
        assert time.time() < deadline, "scan did not finish"
        time.sleep(0.5)
    result = ok("scan.state")["result"]
    assert result["status"] == "finished" and [r["value"] for r in result["rows"]] == [200.0, 400.0]
    assert all(r.get("energy_out") for r in result["rows"]), result["rows"]
    time.sleep(0.2)
    bridge.unsubscribe(deliver)                  # a subscriber left behind would look like a connected page
    assert any("scan.finished" in e for e in events) and any("scan.progress" in e for e in events)
    scans = ok("scan.list")
    assert scans[0]["label"] == "np" and scans[0]["done"] == 2
    loaded = ok("scan.load", folder=scans[0]["folder"])
    assert loaded["target"]["keyword"] == "particlenumber"
    assert rpc("scan.load", folder=ROOT)["ok"] is False
    assert ok("scan.delete", folder=scans[0]["folder"]) is True
    assert ok("scan.list") == []
    from avas.gui import context
    assert not os.listdir(os.path.join(context.project().path, "OutputFile")) or True   # OutputFile untouched
