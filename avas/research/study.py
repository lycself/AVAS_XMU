"""A study: runs of variants of one project, with records and a cache.

Engine runs live in ``<root>/runs/<fingerprint>/`` (``input/`` the inputs used, ``output/`` the kept
outputs, ``engine.log``, ``run.json``).  The fingerprint covers every input file of the run, the
field maps the lattice uses (name, size and time, or their contents with ``strict=True``) and the
engine library, so a run whose fingerprint exists is not repeated: a stopped batch resumes, and a
second notebook gets the same results.  With a fixed seed the engine is reproducible (checked
2026-11-03), so a cached result is the result.  The project's own folders are never written.
"""
import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import uuid

from avas import paths
from avas.data.inputs import copy_text_inputs
from avas.data.lattice_doc import LatticeDocument
from avas.model import LinearModel, read_beam
from avas.research import engine
from avas.research.runs import EngineRun, LinearRun
from avas.research.variant import Variant, apply

DEFAULT_SEED = 12345
RESEARCH_DIR = "Research"
FIELD_EXTENSIONS = ("edx", "edy", "edz", "bdx", "bdy", "bdz", "esx", "esy", "esz", "bsx", "bsy", "bsz")
TEXT_SUFFIXES = (".txt", ".ini", ".dat", ".csv")


def input_folder(project):
    """InputFile of a project folder, or the folder itself when it holds beam.txt."""
    project = os.fspath(project)
    inp = os.path.join(project, "InputFile")
    return inp if os.path.isfile(os.path.join(inp, "beam.txt")) else project


def _text_digest(path):
    with open(path, encoding="utf-8", errors="replace") as fh:
        text = re.sub(r"\r\n?", "\n", fh.read().lstrip("﻿")).rstrip("\n")
    return hashlib.sha1(text.encode("utf-8")).hexdigest()


def _file_digest(path):
    h = hashlib.sha1()
    with open(path, "rb") as fh:
        for block in iter(lambda: fh.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def _avas_commit():
    try:
        out = subprocess.run(["git", "rev-parse", "--short=7", "HEAD"], cwd=os.path.dirname(paths.PACKAGE_DIR),
                             capture_output=True, text=True, timeout=5)
        return out.stdout.strip() or None
    except (OSError, subprocess.SubprocessError):
        return None


class Study:
    """Runs of variants of the project at *project* (a project folder or an input folder).

    *name*: folder name under ``<project>/Research/`` unless *root* is given (e.g. outside a
    read-only project).  *seed*: input.txt ``randomseed`` of runs whose variant has none.
    *strict*: fingerprint the field maps by content.  *engine_command*: ``f(input_dir, output_dir,
    field_dir, lattice) -> argv`` (default ``avas run``; tests use a stand-in).  *timeout*: seconds.
    """

    def __init__(self, project, name="study", root=None, seed=DEFAULT_SEED, strict=False, engine_command=None,
                 timeout=None):
        self.input_dir = input_folder(project)
        project_dir = os.path.dirname(self.input_dir) if os.path.basename(self.input_dir) == "InputFile" else self.input_dir
        self.root = os.path.abspath(root or os.path.join(project_dir, RESEARCH_DIR, re.sub(r"[^\w\-.]+", "_", name)))
        self.name = name
        self.seed = seed
        self.strict = strict
        self.command = engine_command or engine.default_command
        self.timeout = timeout
        self.lattice_name = paths.lattice_source_name(self.input_dir)
        self.field_dirs = paths.field_dirs(self.input_dir)
        self._model = None
        os.makedirs(os.path.join(self.root, "runs"), exist_ok=True)
        meta = os.path.join(self.root, "study.json")
        if not os.path.isfile(meta):
            with open(meta, "w", encoding="utf-8") as fh:
                json.dump({"name": name, "project": self.input_dir, "seed": seed,
                           "created": datetime.datetime.now().isoformat(timespec="seconds")}, fh, indent=1)

    # ------------------------------------------------------------------ linear model
    @property
    def model(self):
        """The project's :class:`~avas.model.LinearModel` (built once; its field-map cache is shared)."""
        if self._model is None:
            self._model = LinearModel.from_project(self.input_dir)
        return self._model

    def _space_charge(self, variant, folder):
        value = variant.input.get("spacecharge")
        if value is None:
            path = os.path.join(folder, "input.txt")
            with open(path, encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    parts = line.split("!", 1)[0].split()
                    if len(parts) >= 2 and parts[0].lower() == "spacecharge":
                        value = parts[1]
        try:
            return None if value is None or float(value) != 0 else False
        except ValueError:
            return None

    def linear(self, variant=None, **evaluate):
        """:class:`LinearRun` of *variant* (*evaluate*: further arguments of ``LinearModel.evaluate``)."""
        variant = variant or Variant()
        beam = None
        folder = self.input_dir
        tmp = None
        if variant.beam or variant.input:
            tmp = os.path.join(self.root, ".staging", uuid.uuid4().hex)
            copy_text_inputs(self.input_dir, tmp)
            apply(Variant(beam=variant.beam, input=variant.input), tmp, self.lattice_name, self.model)
            beam, folder = read_beam(tmp), tmp
        try:
            evaluate.setdefault("space_charge", self._space_charge(variant, folder))
            res = self.model.evaluate(knobs=variant.lattice or None, beam=beam, **evaluate)
        finally:
            if tmp:
                shutil.rmtree(tmp, ignore_errors=True)
        return LinearRun(variant, res, {"status": "done", "fidelity": "linear", "variant": variant.to_dict()})

    # ------------------------------------------------------------------ engine runs
    def fingerprint(self, input_dir):
        """Hash of the inputs in *input_dir*, the field maps its lattice uses and the engine library."""
        parts = {"files": {}, "maps": {}, "engine": engine.identity()["sha1"]}
        for name in sorted(os.listdir(input_dir)):
            path = os.path.join(input_dir, name)
            if os.path.isfile(path):
                parts["files"][name] = _text_digest(path) if name.lower().endswith(TEXT_SUFFIXES) else _file_digest(path)
        with open(os.path.join(input_dir, self.lattice_name), encoding="utf-8", errors="replace") as fh:
            doc = LatticeDocument(fh.read())
        for base in sorted({st.param(8) for st in doc.statements if st.active and st.key == "field" and st.param(8)}):
            for d in self.field_dirs:
                found = {}
                for ext in FIELD_EXTENSIONS:
                    path = os.path.join(d, f"{base}.{ext}")
                    if os.path.isfile(path):
                        st = os.stat(path)
                        found[ext] = _file_digest(path) if self.strict else [st.st_size, st.st_mtime_ns]
                if found:
                    parts["maps"][base] = found
                    break
        return hashlib.sha1(json.dumps(parts, sort_keys=True).encode("utf-8")).hexdigest()

    def _stage(self, variant, single_thread):
        tmp = os.path.join(self.root, ".staging", uuid.uuid4().hex)
        inp = os.path.join(tmp, "input")
        copy_text_inputs(self.input_dir, inp)
        apply(variant, inp, self.lattice_name, self.model, self.seed, single_thread)
        return tmp, inp

    def engine(self, variant=None, force=False, single_thread=False, progress=None, stop=None):
        """:class:`EngineRun` of *variant*: the cached one when its fingerprint exists (unless *force*),
        else a new engine run.  A failed run is recorded (status "failed", the error) and returned."""
        variant = variant or Variant()
        tmp, inp = self._stage(variant, single_thread)
        try:
            fp = self.fingerprint(inp)
            folder = os.path.join(self.root, "runs", fp[:16])
            record_path = os.path.join(folder, "run.json")
            if not force and os.path.isfile(record_path):
                record = _read(record_path)
                if record.get("fingerprint") == fp and record.get("status") in ("done", "failed"):
                    return EngineRun(Variant.from_dict(record.get("variant")), folder, record)
            shutil.rmtree(folder, ignore_errors=True)
            os.makedirs(folder)
            shutil.move(inp, os.path.join(folder, "input"))
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
        out = os.path.join(folder, "output")
        os.makedirs(out)
        field_dir = self.field_dirs[0] if self.field_dirs else self.input_dir
        record = {"fingerprint": fp, "status": "running", "fidelity": "engine", "variant": variant.to_dict(),
                  "label": variant.label, "seed": variant.seed if variant.seed is not None else self.seed,
                  "single_thread": single_thread, "engine": engine.identity(), "avas_commit": _avas_commit(),
                  "project": self.input_dir, "lattice": self.lattice_name, "field_dir": field_dir,
                  "started": datetime.datetime.now().isoformat(timespec="seconds")}
        _write(record_path, record)
        command = self.command(os.path.join(folder, "input"), out, field_dir, self.lattice_name)
        try:
            record["seconds"] = engine.run(command, folder, os.path.join(folder, "engine.log"), self.timeout, stop,
                                           progress)
            engine.trim_output(out)
            from avas.post.analysis.run_diagnostics import dataset_metrics
            record["metrics"] = dataset_metrics(out) or {}
            record["status"] = "done" if record["metrics"] else "failed"
            if not record["metrics"]:
                record["error"] = "no DataSet.txt"
        except engine.EngineError as exc:
            record.update(status="failed" if str(exc) != "stopped" else "stopped", error=str(exc))
        record["finished"] = datetime.datetime.now().isoformat(timespec="seconds")
        _write(record_path, record)
        return EngineRun(variant, folder, record)

    def run(self, variant=None, fidelity="engine", **kwargs):
        """:meth:`engine` or :meth:`linear` by *fidelity*."""
        if fidelity == "linear":
            return self.linear(variant, **kwargs)
        if fidelity == "engine":
            return self.engine(variant, **kwargs)
        raise ValueError("fidelity is 'linear' or 'engine'.")

    def run_many(self, variants, fidelity="engine", **kwargs):
        """See :func:`avas.research.batch.run_many`."""
        from avas.research.batch import run_many
        return run_many(self, variants, fidelity, **kwargs)

    # ------------------------------------------------------------------ records
    def records(self):
        """``run.json`` of every engine run of the study (with ``folder``)."""
        out = []
        runs = os.path.join(self.root, "runs")
        for name in sorted(os.listdir(runs)):
            path = os.path.join(runs, name, "run.json")
            if os.path.isfile(path):
                rec = _read(path)
                rec["folder"] = os.path.dirname(path)
                out.append(rec)
        return out

    def load(self, record):
        """:class:`EngineRun` of a record (from :meth:`records`) or a run folder."""
        folder = record if isinstance(record, str) else record["folder"]
        rec = _read(os.path.join(folder, "run.json"))
        return EngineRun(Variant.from_dict(rec.get("variant")), folder, rec)

    def table(self):
        """pandas DataFrame: one row per engine run with its changes, status, seconds and metrics."""
        import pandas as pd
        rows = []
        for rec in self.records():
            row = {"folder": os.path.basename(rec["folder"]), "status": rec.get("status"), "label": rec.get("label"),
                   "seed": rec.get("seed"), "seconds": rec.get("seconds"), "error": rec.get("error"),
                   "note": rec.get("note")}
            row.update(Variant.from_dict(rec.get("variant")).flat())
            row.update({f"m.{k}": v for k, v in (rec.get("metrics") or {}).items() if not isinstance(v, (dict, list))})
            rows.append(row)
        return pd.DataFrame(rows)

    def annotate(self, run, note=None, tags=None):
        """Keep a note and tags in the run's record."""
        path = os.path.join(run.folder, "run.json")
        rec = _read(path)
        if note is not None:
            rec["note"] = note
        if tags is not None:
            rec["tags"] = list(tags)
        _write(path, rec)
        run.record = rec


def _read(path):
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def _write(path, data):
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        json.dump(data, fh, indent=1, ensure_ascii=False, default=str)
    os.replace(tmp, path)
