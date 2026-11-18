"""A study: runs of variants of one project, with records and a cache.

Engine runs live in ``<root>/runs/<fingerprint>/`` (``input/`` the inputs used, ``output/`` the kept
outputs, ``engine.log``, ``run.json``).  The fingerprint covers every input file of the run, the
field maps the lattice uses (name, size and time, or their contents with ``strict=True``) and the
kernel: the original kernel's library, or the new kernel's *result tier* and version
(:func:`avas.engines.result_tier`; the thread count and the GPU numbers are not part of it).  A run
whose fingerprint exists is not repeated: a stopped batch resumes, and a second notebook gets the
same results.  With a fixed seed the engine is reproducible (checked 2026-11-03), so a cached result
is the result.  The project's own folders are never written.

The default seed is :data:`DEFAULT_SEED` (12345).  With the original kernel or the new kernel's
compatibility mode (compat = dll) a seed other than 0 gives an initial distribution with x fully
correlated with the phase (correlation -1, a defect of the original kernel; the new kernel's default
mode does not have it): centroid responses are hardly affected, but emittance growth and space-charge
results of such runs are biased.  Use the new kernel, or ``seed=0`` (a new sample every run, so no
cache reuse), when that matters.

The kernel comes from the study's arguments over the environment over the project's ini.ini
``[engine]`` over the global default kernel of the GUI preferences (:func:`avas.engines.resolve_choice`);
every run's ``input/ini.ini`` carries the
resolved choice in ``[engine]`` (not part of the fingerprint), so ``avas run`` in the child uses it.
"""
import datetime
import hashlib
import json
import logging
import os
import re
import shutil
import subprocess
import uuid
from dataclasses import replace

from avas import engines, paths
from avas.data.inputs import copy_text_inputs
from avas.data.lattice_doc import LatticeDocument
from avas.model import LinearModel, read_beam
from avas.research import engine as engine_proc
from avas.research.runs import EngineRun, LinearRun
from avas.research.variant import Variant, apply

log = logging.getLogger(__name__)

DEFAULT_SEED = 12345
RESEARCH_DIR = "Research"
FIELD_EXTENSIONS = ("edx", "edy", "edz", "bdx", "bdy", "bdz", "esx", "esy", "esz", "bsx", "bsy", "bsz")
TEXT_SUFFIXES = (".txt", ".ini", ".dat", ".csv")


def input_folder(project):
    """InputFile of a project folder, or the folder itself when it holds beam.txt (absolute: the engine
    runs in the run folder, where a relative field map folder would not be found)."""
    project = os.path.abspath(os.fspath(project))
    inp = os.path.join(project, "InputFile")
    return inp if os.path.isfile(os.path.join(inp, "beam.txt")) else project


def _text_digest(path, drop_section=None):
    with open(path, encoding="utf-8", errors="replace") as fh:
        text = re.sub(r"\r\n?", "\n", fh.read().lstrip("﻿"))
    if drop_section:
        text = strip_ini_section(text, drop_section)
    text = text.rstrip("\n")
    return hashlib.sha1(text.encode("utf-8")).hexdigest()


def strip_ini_section(text, section):
    """*text* (an ini file, LF line ends) without the section ``[section]`` and its keys."""
    out, skip = [], False
    head = re.compile(r"^\s*\[([^\]]*)\]\s*$")
    for line in text.split("\n"):
        m = head.match(line)
        if m:
            skip = m.group(1).strip().lower() == section
        if not skip:
            out.append(line)
    return "\n".join(out)


def write_engine_section(input_dir, choice, gpus=None):
    """Put *choice* into ``[engine]`` of ``<input_dir>/ini.ini`` (the rest of the text unchanged, so the
    fingerprint of the other sections stays that of the project's file).  Every choice is written, the
    original kernel too: the child's own default is the new kernel since M8b (and the global preference
    could choose either), so leaving it out ran original-kernel studies on the new kernel."""
    path = os.path.join(input_dir, "ini.ini")
    exists = os.path.isfile(path)
    text = ""
    if exists:
        with open(path, encoding="utf-8", errors="replace") as fh:
            text = re.sub(r"\r\n?", "\n", fh.read().lstrip("﻿"))
        text = strip_ini_section(text, engines.INI_SECTION).rstrip("\n")
    values = choice.to_dict()
    if gpus is not None and choice.is_new:
        values["gpus"] = engines.parse_value("gpus", gpus, "gpus")
    block = "[engine]\n" + "".join(f"{k} = {v}\n" for k, v in values.items())
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write((text + "\n\n" if text else "") + block)


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
    read-only project).  *seed*: input.txt ``randomseed`` of runs whose variant has none (default 12345;
    with the original kernel or compat = dll a seed ≠ 0 correlates x with the phase, see the module notes).
    *strict*: fingerprint the field maps by content.  *engine_command*: ``f(input_dir, output_dir,
    field_dir, lattice) -> argv`` (default ``avas run``; tests use a stand-in).  *timeout*: seconds.
    *runner*: ``f(input_dir, output_dir, field_dir, lattice, log_path, stop, progress) -> seconds``
    replacing the plain child process (the GUI runs studies through its sandbox, so they can be
    paused and show on the Run page); it raises :class:`~avas.research.engine.EngineError`.
    *engine*, *device*, *precision*, *gpus*, *compat*, *threads*: the kernel (``"original"`` /
    ``"new"``) and the new kernel's options; None takes the environment, then the project's ini.ini
    ``[engine]`` (:mod:`avas.engines`).  *engine_query*: ``f(choice) -> dict`` like
    :func:`avas.engines.query_subprocess` (tests); the new kernel is never loaded in this process.
    """

    def __init__(self, project, name="study", root=None, seed=DEFAULT_SEED, strict=False, engine_command=None,
                 timeout=None, runner=None, engine=None, device=None, precision=None, gpus=None, compat=None,
                 threads=None, engine_query=None):
        self.input_dir = input_folder(project)
        project_dir = os.path.dirname(self.input_dir) if os.path.basename(self.input_dir) == "InputFile" else self.input_dir
        self.root = os.path.abspath(root or os.path.join(project_dir, RESEARCH_DIR, re.sub(r"[^\w\-.]+", "_", name)))
        self.name = name
        self.seed = seed
        self.strict = strict
        self.command = engine_command or engine_proc.default_command
        self.timeout = timeout
        self.runner = runner
        self.engine_settings = {"kernel": engine, "device": device, "precision": precision, "gpus": gpus,
                                "compat": compat, "threads": threads}
        self.engine_query = engine_query
        self._profiles = {}
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

    # ------------------------------------------------------------------ kernel
    def engine_choice(self):
        """The :class:`~avas.engines.EngineChoice` of the study's runs (arguments > environment > ini.ini)."""
        return engines.resolve_choice(cli=self.engine_settings, input_dir=self.input_dir)

    def engine_profile(self, space_charge=True, choice=None):
        """``{"tier", "version", …}`` the runs will have (the new kernel resolves ``auto`` in a child
        process, cached); :class:`~avas.engines.EngineChoiceError` when they cannot run."""
        choice = choice or self.engine_choice()
        key = (choice, bool(space_charge))
        if key not in self._profiles:
            if not choice.is_new:
                ident = engine_proc.identity()
                self._profiles[key] = {"tier": engines.TIER_ORIGINAL, "version": None, "sha1": ident["sha1"],
                                       "file": ident["file"]}
            else:
                self._profiles[key] = engines.predicted_profile(choice, space_charge, self.engine_query)
        return dict(self._profiles[key])

    def parallel_plan(self, workers):
        """``(workers, gpu slots)`` for *workers* parallel engine runs: GPU runs bind one GPU each (no
        more workers than GPUs; one GPU or Metal: one run at a time); CPU runs: ``(workers, None)``."""
        if workers <= 1:
            return workers, None
        choice = self.engine_choice()
        if not choice.is_new or choice.device == "cpu" or choice.compat == "dll":
            return workers, None
        profile = self.engine_profile(True, choice)
        if profile.get("device") == "metal":
            log.info("study %s: Metal has one GPU, the runs go one at a time (workers 1 instead of %d)",
                     self.name, workers)
            return 1, None
        if profile.get("device") != "cuda":
            return workers, None
        cards = list(profile.get("gpus") or [])
        if len(cards) <= 1:
            log.info("study %s: one GPU, the runs go one at a time (workers 1 instead of %d)", self.name, workers)
            return 1, None
        slots = [str(c) for c in cards[:workers]]
        if len(slots) < workers:
            log.info("study %s: %d GPUs, %d parallel runs instead of %d", self.name, len(slots), len(slots), workers)
        return len(slots), slots

    def _engine_part(self, choice, space_charge):
        profile = self.engine_profile(space_charge, choice)
        if profile["tier"] == engines.TIER_ORIGINAL:
            if choice.threads:                     # AVAS.dll's results depend on its threads (data race)
                return {"sha1": profile["sha1"], "threads": int(choice.threads)}
            return profile["sha1"]                 # as before M7: caches of the original kernel stay valid
        return {"tier": profile["tier"], "version": profile["version"]}

    # ------------------------------------------------------------------ engine runs
    def fingerprint(self, input_dir, choice=None):
        """Hash of the inputs in *input_dir*, the field maps its lattice uses and the kernel (the
        original kernel's library, or the new kernel's result tier and version)."""
        choice = choice or self.engine_choice()
        sc = engines.input_space_charge(input_dir)
        parts = {"files": {}, "maps": {}, "engine": self._engine_part(choice, sc)}
        for name in sorted(os.listdir(input_dir)):
            path = os.path.join(input_dir, name)
            if os.path.isfile(path):
                if name.lower() == "ini.ini":       # [engine] is in parts["engine"] (without threads / GPUs)
                    parts["files"][name] = _text_digest(path, drop_section=engines.INI_SECTION)
                else:
                    parts["files"][name] = (_text_digest(path) if name.lower().endswith(TEXT_SUFFIXES)
                                            else _file_digest(path))
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

    def _stage(self, variant, choice=None, gpus=None):
        tmp = os.path.join(self.root, ".staging", uuid.uuid4().hex)
        inp = os.path.join(tmp, "input")
        copy_text_inputs(self.input_dir, inp)
        apply(variant, inp, self.lattice_name, self.model, self.seed)
        write_engine_section(inp, choice or self.engine_choice(), gpus)
        return tmp, inp

    def engine(self, variant=None, force=False, single_thread=False, progress=None, stop=None, gpus=None, threads=None):
        """:class:`EngineRun` of *variant*: the cached one when its fingerprint exists (unless *force*),
        else a new engine run.  A failed run is recorded (status "failed", the error) and returned.
        *gpus*: the new kernel's GPUs for this run (parallel batches bind one each; not part of the
        fingerprint).  *threads*: CPU threads of this run (parallel batches), *single_thread* = threads 1;
        the new kernel's results do not depend on it (not part of the fingerprint), the original kernel's
        do (part of it).  A kernel or device that cannot run raises :class:`~avas.engines.EngineChoiceError`."""
        variant = variant or Variant()
        choice = self.engine_choice()
        if single_thread and not threads:
            threads = 1
        if threads:
            choice = replace(choice, threads=int(threads))
        tmp, inp = self._stage(variant, choice, gpus)
        try:
            sc = engines.input_space_charge(inp)
            profile = self.engine_profile(sc, choice)
            fp = self.fingerprint(inp, choice)
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
        engine_rec = dict(profile, kernel=choice.kernel, requested=choice.to_dict(), space_charge=sc)
        if gpus is not None:
            engine_rec["bound_gpus"] = str(gpus)
        record = {"fingerprint": fp, "status": "running", "fidelity": "engine", "variant": variant.to_dict(),
                  "label": variant.label, "seed": variant.seed if variant.seed is not None else self.seed,
                  "single_thread": choice.threads == 1, "engine": engine_rec, "avas_commit": _avas_commit(),
                  "project": self.input_dir, "lattice": self.lattice_name, "field_dir": field_dir,
                  "started": datetime.datetime.now().isoformat(timespec="seconds")}
        _write(record_path, record)
        log_path = os.path.join(folder, "engine.log")
        try:
            if self.runner is not None:
                record["seconds"] = self.runner(os.path.join(folder, "input"), out, field_dir, self.lattice_name,
                                                log_path, stop, progress)
            else:
                command = self.command(os.path.join(folder, "input"), out, field_dir, self.lattice_name)
                record["seconds"] = engine_proc.run(command, folder, log_path, self.timeout, stop, progress)
            engine_proc.trim_output(out)
            self._check_actual(record, out)
            from avas.post.analysis.run_diagnostics import dataset_metrics
            record["metrics"] = dataset_metrics(out) or {}
            record["status"] = "done" if record["metrics"] else "failed"
            if not record["metrics"]:
                record["error"] = "no DataSet.txt"
        except engine_proc.EngineError as exc:
            record.update(status="failed" if str(exc) != "stopped" else "stopped", error=str(exc))
        record["finished"] = datetime.datetime.now().isoformat(timespec="seconds")
        _write(record_path, record)
        return EngineRun(variant, folder, record)

    def _check_actual(self, record, out):
        """The kernel / device the run actually used (its avas_run.json) into the record; a warning when
        its result tier is not the predicted one."""
        try:
            with open(os.path.join(out, "avas_run.json"), encoding="utf-8") as fh:
                used = json.load(fh).get("engine")
        except (OSError, ValueError):
            return
        if not isinstance(used, dict):
            return
        tier = used.get("tier") or engines.result_tier(used.get("kernel"), used.get("device"), used.get("precision"),
                                                        used.get("compat"))
        record["engine"]["actual"] = {k: used.get(k) for k in ("kernel", "version", "device", "precision", "gpus",
                                                               "compat", "threads", "space_charge")}
        record["engine"]["actual"]["tier"] = tier
        predicted = record["engine"].get("tier")
        if tier and predicted and tier != predicted:
            msg = f"the run used result tier {tier}, the study predicted {predicted} (its cache entry is filed under the prediction)"
            record.setdefault("warnings", []).append(msg)
            log.warning("study %s: %s", self.name, msg)

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
