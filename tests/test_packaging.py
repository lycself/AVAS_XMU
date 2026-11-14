"""Packaging preflight must fail before expensive or mutating build steps."""
import importlib.util
from pathlib import Path

import pytest


spec = importlib.util.spec_from_file_location(
    "avas_packaging_build_test", Path(__file__).resolve().parents[1] / "packaging/build.py"
)
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)


@pytest.mark.parametrize("content", [None, b"", b"[LangOptions]\n", b"\xff"])
def test_bad_language_fails_before_build(tmp_path, monkeypatch, content):
    monkeypatch.setattr(build, "ROOT", str(tmp_path))
    monkeypatch.setattr(build, "find_iscc", lambda explicit: "ISCC.exe")
    monkeypatch.setattr(build, "run", lambda *a, **k: pytest.fail("build started before preflight"))
    if content is not None:
        language = tmp_path / "packaging/languages/ChineseSimplified.isl"
        language.parent.mkdir(parents=True)
        language.write_bytes(content)
    with pytest.raises(RuntimeError, match="bundled installer language file"):
        build.main(["--require-installer"])
    assert not (tmp_path / "avas/_build.json").exists()


def test_bundled_language_is_readable():
    build.check_installer_inputs()


def test_required_compiler_fails_before_build(monkeypatch):
    monkeypatch.setattr(build, "find_iscc", lambda explicit: None)
    monkeypatch.setattr(build, "run", lambda *a, **k: pytest.fail("build started without compiler"))
    with pytest.raises(RuntimeError, match="Inno Setup is required"):
        build.main(["--require-installer"])


def test_console_launcher_dispatch(tmp_path, monkeypatch):
    import sys
    from avas.cli import main as cli
    entry_spec = importlib.util.spec_from_file_location(
        "avas_console_test", Path(build.ROOT) / "packaging/avas_cli.py")
    entry = importlib.util.module_from_spec(entry_spec)
    entry_spec.loader.exec_module(entry)
    monkeypatch.setattr(sys, "executable", str(tmp_path / "AVAS.exe"))
    launched = []
    monkeypatch.setattr(entry.subprocess, "Popen", lambda args: launched.append(args))
    calls = []
    monkeypatch.setattr(cli, "main", lambda args: calls.append(args) or 7)
    assert entry.main([]) == 0
    assert launched == [[str(tmp_path / "AVASGui.exe")]]
    for args in (["--help"], ["run", "--help"], ["info"]):
        assert entry.main(args) == 7
    assert calls == [["--help"], ["run", "--help"], ["info"]]
    assert len(launched) == 1


@pytest.mark.parametrize("language", ["en", "zh_CN"])
@pytest.mark.parametrize("encoding", ["utf-8", "utf-16"])
def test_installer_language_saved_once(tmp_path, monkeypatch, language, encoding):
    import sys
    from avas.gui.settings import Settings
    monkeypatch.setattr(sys, "frozen", True, raising=False)
    monkeypatch.setattr(sys, "executable", str(tmp_path / "AVASGui.exe"))
    marker = tmp_path / "avas-install.ini"
    marker.write_text(f"[UI]\nLanguage={language}\n", encoding=encoding)
    prefs = str(tmp_path / "gui.json")
    assert Settings(prefs).get("ui/language") == language
    marker.write_text("[UI]\nLanguage=invalid\n", encoding="utf-8")
    assert Settings(prefs).get("ui/language") == language
    chosen = "en" if language == "zh_CN" else "zh_CN"
    Settings(prefs).set("ui/language", chosen)
    marker.write_text(f"[UI]\nLanguage={language}\n", encoding="utf-8")
    assert Settings(prefs).get("ui/language") == chosen


@pytest.mark.parametrize("content", [None, "[UI]\nLanguage=unknown", "invalid ini"])
def test_missing_or_invalid_installer_language(tmp_path, monkeypatch, content):
    import sys
    from avas.gui.settings import Settings
    monkeypatch.setattr(sys, "frozen", True, raising=False)
    monkeypatch.setattr(sys, "executable", str(tmp_path / "AVASGui.exe"))
    if content is not None:
        (tmp_path / "avas-install.ini").write_text(content, encoding="utf-8")
    assert Settings(str(tmp_path / "gui.json")).get("ui/language") == "en"


def test_source_settings_ignore_installer_language(tmp_path, monkeypatch):
    import sys
    from avas.gui.settings import Settings
    monkeypatch.delattr(sys, "frozen", raising=False)
    monkeypatch.setattr(sys, "executable", str(tmp_path / "python.exe"))
    (tmp_path / "avas-install.ini").write_text("[UI]\nLanguage=zh_CN\n", encoding="utf-8")
    assert Settings(str(tmp_path / "gui.json")).get("ui/language") == "en"


def _spec_engine_datas(root, platform):
    """The avas/engine part of packaging/avas.spec run against *root* for *platform* (collect_data_files
    replaced by a walk of root/avas/engine that, like PyInstaller on Windows, keeps every file)."""
    import os
    import types
    text = (Path(build.ROOT) / "packaging/avas.spec").read_text(encoding="utf-8")
    block = text[text.index("datas = []"):text.index('datas += collect_data_files("avas", subdir="static")')]

    def collect(package, subdir):
        base = os.path.join(root, package, subdir)
        return [(os.path.join(d, f), os.path.relpath(d, root)) for d, _, fs in os.walk(base) for f in fs]
    ns = {"os": os, "sys": types.SimpleNamespace(platform=platform), "ROOT": str(root), "collect_data_files": collect}
    exec(block, ns)
    return sorted((Path(src).relative_to(root).as_posix(), Path(dst).as_posix()) for src, dst in ns["datas"])


def test_spec_bundles_the_new_kernel_of_the_platform(tmp_path):
    new = tmp_path / "avas/engine/new"
    new.mkdir(parents=True)
    (tmp_path / "avas/engine/AVAS.dll").write_bytes(b"MZ")
    for name in ("libAVAS.so", "libPIC.so", "libfftw3.so", "libfftw3.so.3.6.10", "libfftw3-3.dll", "MSVCP140.dll"):
        (tmp_path / "avas/engine" / name).write_bytes(b"x")
    for name in ("avas_engine.dll", "libavas_engine.so", "libavas_engine.dylib", "build-stamp.json"):
        (new / name).write_bytes(b"x")
    datas = _spec_engine_datas(tmp_path, "win32")
    assert ("avas/engine/AVAS.dll", "avas/engine") in datas
    # the Windows bundle has no Linux files of the original kernel; a Linux bundle keeps everything
    assert sorted(d[0] for d in datas if d[1] == "avas/engine") == [
        "avas/engine/AVAS.dll", "avas/engine/MSVCP140.dll", "avas/engine/libfftw3-3.dll"]
    assert "avas/engine/libPIC.so" in [d[0] for d in _spec_engine_datas(tmp_path, "linux")]
    assert [d for d in datas if d[1] == "avas/engine/new"] == [
        ("avas/engine/new/avas_engine.dll", "avas/engine/new"), ("avas/engine/new/build-stamp.json", "avas/engine/new")]
    linux = [d[0] for d in _spec_engine_datas(tmp_path, "linux") if d[1] == "avas/engine/new"]
    assert linux == ["avas/engine/new/build-stamp.json", "avas/engine/new/libavas_engine.so"]
    for name in ("avas_engine.dll", "libavas_engine.so", "libavas_engine.dylib", "build-stamp.json"):
        (new / name).unlink()
    assert not [d for d in _spec_engine_datas(tmp_path, "win32") if d[1] == "avas/engine/new"]


def test_update_manifest_lists_the_new_kernel(tmp_path):
    import importlib.util
    spec = importlib.util.spec_from_file_location("avas_update_release_pkg", Path(build.ROOT) / "packaging/update_release.py")
    publisher = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(publisher)
    new = tmp_path / "_internal/avas/engine/new"
    new.mkdir(parents=True)
    (new / "avas_engine.dll").write_bytes(b"MZ new kernel")
    (new / "build-stamp.json").write_text("{}", encoding="utf-8")
    files = publisher.manifest(tmp_path, "0" * 40, "frozen")["files"]
    assert files["_internal/avas/engine/new/avas_engine.dll"] == publisher.worker.digest(new / "avas_engine.dll")
    assert "_internal/avas/engine/new/build-stamp.json" in files
