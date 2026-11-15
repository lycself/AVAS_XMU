"""GUI back-end tests of what the Beam and Simulation settings pages edit besides their own keywords:
the secondary beam (input.txt switch + particle file, checked before it is written), boundary.txt, the
scanphase line no kernel reads (reported, removed on save; scanData.txt left alone), restoring a
file-history version into the form (beam.parse / settings.parse), and the field-map facts the
element's field window shows (lattice.fieldInfo). No engine run is needed.
"""
import json
import os
import shutil
import sys

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXAMPLE = os.path.join(ROOT, "examples", "hwr010", "InputFile")
sys.path.insert(0, ROOT)


def rpc(method, **params):
    from avas.gui import bridge
    return json.loads(bridge.dispatch(method, params))


def ok(method, **params):
    reply = rpc(method, **params)
    assert reply["ok"], f"{method}: {reply.get('error')}\n{reply.get('detail', '')}"
    return reply["result"]


@pytest.fixture(scope="module")
def project(tmp_path_factory):
    root = tmp_path_factory.mktemp("inputpages")
    os.environ["AVAS_GUI_SETTINGS"] = str(root / "gui-settings.json")
    shutil.copytree(EXAMPLE, root / "proj" / "InputFile")
    os.makedirs(root / "proj" / "OutputFile")
    from avas.gui import services  # noqa: F401 - registers the handlers
    assert ok("project.open", path=str(root / "proj"))["open"]
    yield root / "proj" / "InputFile"
    ok("project.close")


def read(path):
    """The file as the pages get it (line endings kept: a Windows checkout has CRLF)."""
    from avas.gui.textio import read_text
    return read_text(path)


def keyword_lines(path, keyword):
    return [line.strip() for line in read(path).splitlines() if line.split() and line.split()[0].lower() == keyword]


def test_secondary_beam(project):
    data = ok("beam.load")
    sec = data["secondary"]
    assert sec["on"] is False and sec["exists"] is False and sec["name"] == "SeParticle.txt"
    assert data["inputPath"] == str(project / "input.txt") and data["text"] == read(project / "beam.txt")

    # an empty table with the switch off writes nothing
    ok("beam.save", form=data["form"], secondary={"on": False, "text": ""})
    assert not (project / "SeParticle.txt").exists() and keyword_lines(project / "input.txt", "secondarybeam") == []

    rows = "0 0 0 0 0 1e6 1 938.272 1 0 1\n"
    saved = ok("beam.save", form=data["form"], secondary={"on": True, "text": rows})
    assert saved["secondary"]["on"] is True and saved["secondary"]["text"] == rows
    assert keyword_lines(project / "input.txt", "secondarybeam") == ["secondarybeam 1"]
    # the settings page keeps the switch it does not manage
    settings = ok("settings.load")
    ok("settings.save", form=settings["form"], meta=settings["meta"])
    assert keyword_lines(project / "input.txt", "secondarybeam") == ["secondarybeam 1"]

    # blank lines are dropped; short lines and non-numeric values are refused before anything is written
    saved = ok("beam.save", form=data["form"], secondary={"on": True, "text": "\n" + rows + "  \n" + rows})
    assert saved["secondary"]["text"] == rows + rows
    beam_before = read(project / "beam.txt")
    for bad, line, words in (("0 0 0 0 0 1e6 1 938.272 1 0\n", 1, "10 columns"),
                             (rows + "\n0 0 0 0 0 1e6 1 938 x 0 1\n", 3, "column 9")):
        reply = rpc("beam.save", form={**data["form"], "current": "0.5"}, secondary={"on": True, "text": bad})
        assert reply["ok"] is False and reply["user"] and f"line {line}" in reply["error"] and words in reply["error"], reply
        assert reply["template"].startswith("Secondary particles, line {line}")
    assert read(project / "beam.txt") == beam_before and read(project / "SeParticle.txt") == rows + rows

    # segment runs refuse secondary particles
    from avas.gui import context
    from avas.gui.bridge import UserError
    from avas.gui.services import segments
    with pytest.raises(UserError, match="secondary particles"):
        segments.start(context.project(), {"section": "cav"}, "twiss")

    off = ok("beam.save", form=data["form"], secondary={"on": False, "text": rows})
    assert off["secondary"]["on"] is False and keyword_lines(project / "input.txt", "secondarybeam") == ["secondarybeam 0"]
    # switched off and unchanged: an invalid file does not block saving beam.txt
    (project / "SeParticle.txt").write_text("1 2 3\n", encoding="utf-8")
    ok("beam.save", form=data["form"], secondary={"on": False, "text": "1 2 3\n"})


def test_beam_restore_from_history(project):
    data = ok("beam.load")
    assert ok("beam.parse", text=data["text"]) == data["form"]
    # an older version: the page takes its values into the form, nothing is written yet
    original = data["text"]
    ok("beam.save", form={**data["form"], "current": "0.75"})
    versions = ok("history.list", path=data["path"])
    source = next(v for v in versions if ok("history.version", path=data["path"], revision=v["id"])["text"] == original)
    restored = ok("beam.parse", text=ok("history.version", path=data["path"], revision=source["id"])["text"])
    assert restored == data["form"] and keyword_lines(project / "beam.txt", "current")[0] == "current 0.75"
    # saving the restored form records where it came from
    ok("beam.save", form=restored, restoredFrom=source["id"])
    latest = ok("history.list", path=data["path"])[0]
    assert latest["restored_from"] == source["id"] and latest["source"] in ("restore", "restore-edited")


def test_settings_companion_tables(project):
    data = ok("settings.load")
    comp = data["companions"]
    assert list(comp) == ["boundary"] and comp["boundary"]["exists"] is False      # scanData.txt is not a table here
    assert "scanphase" not in data["form"]

    # an empty boundary table does not create the file
    ok("settings.save", form=data["form"], meta=data["meta"], companions={"boundary": ""})
    assert not (project / "boundary.txt").exists()

    boundary = "start\n1 copper 0.5 0.02 0.02 RLP 0 0 0 0 0 0\nend\n"
    # CRLF on disk, as in a Windows checkout
    scan_before = (project / "scanData.txt").read_bytes()
    saved = ok("settings.save", form={**data["form"], "boundary": True}, meta=data["meta"],
               companions={"boundary": boundary, "scanData": "-30 0\n"})            # an old page's table: ignored
    assert saved["form"]["boundary"] is True
    assert read(project / "boundary.txt") == boundary and (project / "scanData.txt").read_bytes() == scan_before
    assert saved["companions"]["boundary"]["exists"] is True
    # unchanged, even when the table hands back LF line endings: not rewritten
    mtime = os.path.getmtime(project / "boundary.txt")
    ok("settings.save", form=saved["form"], meta=saved["meta"], companions={"boundary": boundary.replace("\n", "\r\n")})
    assert os.path.getmtime(project / "boundary.txt") == mtime

    import re
    form = ok("settings.parse", text=re.sub(r"(?m)^steppercycle .*$", "steppercycle 77", data["text"]))
    assert form["steppercycle"] == "77" and form["error_type"] == saved["form"]["error_type"]


def test_scanphase_line_is_reported_and_removed_on_save(project):
    """No kernel reads scanphase: the page reports an existing line and saving removes it (other lines and
    comments stay); scanData.txt is left alone."""
    path = project / "input.txt"
    before = read(path)
    path.write_text(before.rstrip("\n") + "\n! keep this comment\nScanPhase 2\n", encoding="utf-8")
    data = ok("settings.load")
    assert data["meta"]["hasScanphase"] is True and "scanphase" not in data["form"]
    scan_before = (project / "scanData.txt").read_bytes()
    saved = ok("settings.save", form=data["form"], meta=data["meta"])
    assert saved["meta"]["hasScanphase"] is False and keyword_lines(path, "scanphase") == []
    assert "! keep this comment" in read(path) and (project / "scanData.txt").read_bytes() == scan_before
    assert keyword_lines(path, "steppercycle") == [ln.strip() for ln in before.splitlines() if ln.startswith("steppercycle")]


def test_field_info(project):
    info = ok("lattice.fieldInfo", name="sol")
    exts = [c["ext"] for c in info["components"]]
    assert exts == ["bsx", "bsy", "bsz"]
    bsz = info["components"][2]
    assert bsz["path"] == str(project / "sol.bsz") and bsz["nz"] > 0 and bsz["points"] > 0
    assert bsz["meaning"][0] and bsz["length"] > 0
    assert rpc("lattice.fieldInfo", name="no_such_map")["ok"] is False


def test_unreadable_versions_are_user_errors(project):
    for method in ("beam.parse", "settings.parse"):
        reply = rpc(method, text="current abc\nsteppercycle abc\n")
        assert reply["ok"] is False and reply.get("user") is True, reply
