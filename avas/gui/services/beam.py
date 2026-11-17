"""Beam page: ``InputFile/beam.txt``, and the secondary beam (``secondarybeam`` in input.txt and its particle file).

Saving rewrites only the keywords the page manages, in place; every other
line (``randomseed``, comments ...) is kept as it was.

Single-particle mode (``particlenumber 1``): the start offset is shown in the units of ``initpos`` / ``initmom``
(mm; mrad and 1e-3).  It is read from those keywords when they are present (the new kernel's default mode prefers
them, M10 D2), else from ``displacepos`` / ``displacedpos``; it is always written as ``displacepos`` /
``displacedpos``, which every shipped kernel reads (the original kernel 2026-04-02 and the mode "Reproduce AVAS.dll"
know only these), and ``initpos`` / ``initmom`` lines are removed so that no kernel sees two different offsets.
"""
import logging
import os
import shutil
import tempfile

from avas.api.qt.api import cal_beam_parameter
from avas.data import filekinds
from avas.data.lattice_edit import keyword_line, set_keyword_value
from avas.utils.beamconfig import BeamConfig
from avas.gui import context
from avas.gui.bridge import UserError, rpc
from avas.gui.locks import require_unlocked
from avas.gui.textio import normalized_write_text, read_text, write_text

log = logging.getLogger("avas.gui")

DISTRIBUTIONS = ["GS", "WB", "PB", "KV"]
MANAGED = ("readparticledistribution", "numofcharge", "particlerestmass", "current", "particlenumber",
           "frequency", "kneticenergy", "use_dst", "beamtype", "twissx", "twissy", "twissz", "distribution")
TWISS_KEYS = [f"{q}_{pl}" for pl in "xyz" for q in ("alpha", "beta", "emit")]
# single-particle offset: form keys (units of initpos / initmom) and the four keywords the page manages
OFFSET_POS = ("offset_x", "offset_y", "offset_z")          # mm
OFFSET_MOM = ("offset_xp", "offset_yp", "offset_dp")       # mrad, mrad, 1e-3
OFFSET_LINES = ("displacepos", "displacedpos", "initpos", "initmom")
# displacedpos value = initmom value x 100: the kernels read x' = displacedpos x 1e-5 rad (300 = 3 mrad) and
# delta = displacedpos x 1e-5; initmom gives x' in mrad and delta in 1e-3
DISPLACEDPOS_PER_INITMOM = 100.0


def _s(v):
    """Text for the form: numbers as written by the user (1.0 from the config class shows as 1)."""
    if v is None:
        return ""
    if isinstance(v, float) and v.is_integer() and abs(v) < 1e15:
        return str(int(v))
    return str(v)


def _three(value):
    """Three floats from a beam.txt keyword value (missing ones 0), or None when the keyword is absent."""
    if value is None:
        return None
    vals = value if isinstance(value, list) else [value]
    out = []
    for i in range(3):
        try:
            out.append(float(vals[i]) if i < len(vals) and vals[i] is not None else 0.0)
        except (TypeError, ValueError):
            out.append(0.0)
    return out


def _short(v):
    """A number for the form without float noise (3.0000000000000004 -> 3)."""
    return _s(float(f"{v:.12g}"))


def _offset_form(params):
    pos = _three(params.get("initpos"))
    if pos is None:
        pos = _three(params.get("displacepos"))          # the same unit (mm)
    mom = _three(params.get("initmom"))
    if mom is None:
        old = _three(params.get("displacedpos"))
        mom = None if old is None else [v / DISPLACEDPOS_PER_INITMOM for v in old]
    out = {}
    for keys, vals in ((OFFSET_POS, pos), (OFFSET_MOM, mom)):
        for k, v in zip(keys, vals or [None] * 3):
            out[k] = "" if v is None else _short(v)
    return out


def _form(params):
    dist_x = _s(params.get("distribution_x")) or "GS"
    dist_y = _s(params.get("distribution_y")) or "GS"
    form = {
        "use_dst": str(params.get("use_dst") or 0) == "1",
        "dst": _s(params.get("readparticledistribution")),
        "numofcharge": _s(params.get("numofcharge")),
        "particlerestmass": _s(params.get("particlerestmass")),
        "current": _s(params.get("current")),
        "particlenumber": _s(params.get("particlenumber")),
        "frequency": _s(params.get("frequency")),
        "kneticenergy": _s(params.get("kneticenergy")),
        "cw": params.get("beamtype") == "dc",
        "distribution_x": dist_x,
        "distribution_y": dist_y,
    }
    for k in TWISS_KEYS:
        form[k] = _s(params.get(k))
    form.update(_offset_form(params))
    return form


def _params(item):
    try:
        return BeamConfig().create_from_file(item)["data"]["beamParams"]
    except Exception as exc:  # noqa: BLE001 - a malformed file is reported, not fatal
        raise UserError(f"beam.txt: {exc}") from exc


def _secondary_path(p):
    """The secondary-particle file in InputFile (the engine reads SeParticle.txt)."""
    if os.path.isdir(p.input_dir):
        for name in sorted(os.listdir(p.input_dir), key=str.lower):
            path = p.input_file(name)
            if os.path.isfile(path) and filekinds.detect(path) == filekinds.KIND_SEPARTICLE:
                return path
    return p.input_file("SeParticle.txt")


def _secondary(p):
    input_path = p.input_file("input.txt")
    _, parts = keyword_line(read_text(input_path), "secondarybeam") if os.path.isfile(input_path) else (None, None)
    path = _secondary_path(p)
    exists = os.path.isfile(path)
    return {"on": bool(parts) and len(parts) > 1 and parts[1] == "1", "path": path, "name": os.path.basename(path),
            "exists": exists, "text": read_text(path) if exists else ""}


@rpc("beam.load")
def load():
    p = context.project().require()
    path = p.input_file("beam.txt")
    exists = os.path.isfile(path)
    return {"form": _form(_params(p.item()) if exists else {}), "path": path, "exists": exists,
            "text": read_text(path) if exists else "", "inputDir": p.input_dir, "inputPath": p.input_file("input.txt"),
            "secondary": _secondary(p),
            "dstFiles": sorted((n for n in os.listdir(p.input_dir) if n.lower().endswith((".dst", ".edst"))), key=str.lower)
            if os.path.isdir(p.input_dir) else []}


@rpc("beam.parse")
def parse(text):
    """The page's form for a beam.txt text (a version from the file history), without writing anything."""
    with tempfile.TemporaryDirectory() as tmp:
        os.makedirs(os.path.join(tmp, "InputFile"))
        with open(os.path.join(tmp, "InputFile", "beam.txt"), "w", encoding="utf-8") as fh:
            fh.write(text)
        return _form(_params({"projectPath": tmp}))


def _line_key(line):
    code = line.split("!", 1)[0].split()
    return code[0].lower() if code else None


def _format_lines(form):
    if form.get("cw"):
        form = {**form, "alpha_z": "0", "beta_z": "0", "emit_z": "0"}     # a DC beam has no longitudinal Twiss
    def num(key, cast):
        text = str(form.get(key, "")).strip()
        if text == "":
            return None
        try:
            return cast(text)
        except ValueError as exc:
            raise UserError(f"beam.txt: {key} = '{text}' is not a valid number") from exc

    ints = {k: num(k, lambda t: int(float(t)) if float(t).is_integer() else int(t))
            for k in ("numofcharge", "particlenumber")}
    floats = {k: num(k, float) for k in ("particlerestmass", "current", "frequency", "kneticenergy", *TWISS_KEYS)}
    use_dst = 1 if form.get("use_dst") else 0
    dist = [form.get("distribution_x") or "GS", form.get("distribution_y") or "GS"]
    for d in dist:
        if d not in DISTRIBUTIONS:
            raise UserError(f"beam.txt: unknown distribution '{d}' (choose one of KV, GS, PB, WB)")
    # the same type checks the engine-side config class applies
    BeamConfig().set_param(**{**ints, **floats, "use_dst": use_dst, "distribution_x": dist[0], "distribution_y": dist[1]})

    def text(v):
        return repr(v) if isinstance(v, float) else str(v)

    dst = (form.get("dst") or "").strip()
    lines = {
        "readparticledistribution": dst if use_dst else "unknown",
        "numofcharge": ints["numofcharge"],
        "particlerestmass": floats["particlerestmass"],
        "current": floats["current"],
        "particlenumber": ints["particlenumber"],
        "frequency": floats["frequency"],
        "kneticenergy": floats["kneticenergy"],
        "use_dst": use_dst,
        "beamtype": "dc" if form.get("cw") else "notdc",
    }
    for pl in "xyz":
        vals = [floats[f"alpha_{pl}"], floats[f"beta_{pl}"], floats[f"emit_{pl}"]]
        lines[f"twiss{pl}"] = None if None in vals else " ".join(text(v) for v in vals)
    lines["distribution"] = " ".join(dist)
    out = {}
    for key in MANAGED:
        v = lines.get(key)
        if v is None or v == "":
            out[key] = None                      # the keyword is removed
        else:
            out[key] = f"{key} {text(v)}"
    out.update(_offset_lines(form))
    return out


def _offset_lines(form):
    """The single-particle offset lines (``{keyword: line | None}``); empty for a form without the offset fields
    (the lines are then left as they are).  All zero or empty removes the four keywords."""
    if not any(k in form for k in OFFSET_POS + OFFSET_MOM):
        return {}
    def value(key):
        text = str(form.get(key, "") or "").strip()
        if text == "":
            return 0.0
        try:
            return float(text)
        except ValueError as exc:
            raise UserError(f"beam.txt: {key} = '{text}' is not a valid number") from exc
    pos = [value(k) for k in OFFSET_POS]
    mom = [value(k) * DISPLACEDPOS_PER_INITMOM for k in OFFSET_MOM]
    out = dict.fromkeys(OFFSET_LINES)
    def words(vals):
        return " ".join(f"{v:.12g}" for v in vals)
    if any(pos):
        out["displacepos"] = f"displacepos {words(pos)}"
    if any(mom):
        out["displacedpos"] = f"displacedpos {words(mom)}"
    return out


SECONDARY_COLUMNS = 11       # x y z vx vy vz charge mass weight time kind (the first ten numeric)


def clean_secondary(text):
    """The secondary-particle table without blank lines; :class:`UserError` naming the first line with fewer
    than 11 columns or a non-numeric value in the first ten.  The original kernel reads every line as a particle
    and reads past the end of a short or blank line (undefined behaviour), so the file is checked before it
    is written."""
    rows = []
    for number, line in enumerate((text or "").splitlines(), 1):
        parts = line.split()
        if not parts:
            continue                                     # blank lines are dropped
        if len(parts) < SECONDARY_COLUMNS:
            raise UserError("Secondary particles, line {line}: {count} columns; every particle needs 11 (x y z vx vy vz charge mass weight time kind).",
                            line=number, count=len(parts))
        for col, value in enumerate(parts[:SECONDARY_COLUMNS - 1], 1):
            try:
                float(value)
            except ValueError:
                raise UserError("Secondary particles, line {line}, column {column}: '{value}' is not a number.",
                                line=number, column=col, value=value) from None
        rows.append(line.rstrip())
    return "\n".join(rows) + ("\n" if rows else "")


@rpc("beam.save")
def save(form, secondary=None, restoredFrom=None):
    require_unlocked()
    p = context.project().require()
    if secondary is not None and secondary.get("text") is not None:
        # checked before anything is written (blank lines are dropped) when it is used or edited; an unused,
        # unchanged file does not block saving beam.txt
        changed = normalized_write_text(secondary["text"]) != normalized_write_text(_secondary(p)["text"])
        if secondary.get("on") or changed:
            secondary = dict(secondary, text=clean_secondary(secondary["text"]))
    path = p.input_file("beam.txt")
    new = _format_lines(form)
    old_lines = read_text(path).splitlines() if os.path.isfile(path) else []
    written = set()
    result = []
    for line in old_lines:
        key = _line_key(line)
        if key in new:
            if key in written:
                continue                         # duplicates collapse to one line
            written.add(key)
            if new[key] is None:
                continue
            comment = ""
            if "!" in line:
                comment = " " + line[line.index("!"):]
            result.append(new[key] + comment)
        else:
            result.append(line)
    for key in MANAGED + OFFSET_LINES:
        if key in new and key not in written and new[key] is not None:
            result.append(new[key])
    write_text(path, "\n".join(result), source="beam", restored_from=restoredFrom or None)
    log.info("saved %s", os.path.basename(path))
    if secondary is not None:
        _save_secondary(p, secondary)
    return load()


def _save_secondary(p, secondary):
    """``secondarybeam`` in input.txt (only that line) and the secondary-particle file, where they changed."""
    now = _secondary(p)
    on = bool(secondary.get("on"))
    if on != now["on"]:
        input_path = p.input_file("input.txt")
        text = read_text(input_path) if os.path.isfile(input_path) else ""
        line_no, _ = keyword_line(text, "secondarybeam")
        if on or line_no is not None:
            write_text(input_path, set_keyword_value(text, "secondarybeam", 1 if on else 0), source="beam")
    text = secondary.get("text")
    changed = text is not None and normalized_write_text(text) != normalized_write_text(now["text"])
    if changed and (now["exists"] or text.strip()):
        write_text(now["path"], text, source="beam")
        log.info("saved %s", now["name"])


@rpc("beam.importDst")
def import_dst(source, overwrite=False):
    """Copy a particle file into InputFile (if it is not there yet); returns its name."""
    require_unlocked()
    p = context.project().require()
    if not source or not os.path.isfile(source):
        raise UserError(f"Not found: {source}")
    name = os.path.basename(source)
    target = p.input_file(name)
    if os.path.normcase(os.path.abspath(source)) == os.path.normcase(os.path.abspath(target)):
        return {"name": name, "copied": False}
    if os.path.exists(target) and not overwrite:
        return {"name": name, "exists": True}
    shutil.copyfile(source, target)
    log.info("particle file copied to InputFile: %s", name)
    return {"name": name, "copied": True}


@rpc("beam.fromDst")
def from_dst(name):
    """Mass, current, energy and rms Twiss parameters computed from a particle file."""
    p = context.project().require()
    path = name if os.path.isabs(name) else p.input_file(name)
    if not os.path.isfile(path):
        raise UserError(f"Not found: {path}")
    res = cal_beam_parameter({"dstPath": path})
    if res["code"] != 0:
        raise UserError(res["data"]["msg"])
    b = res["data"]["beamParams"]
    out = {k: str(b[k]) for k in ("particlerestmass", "current", "particlenumber", "frequency", "kneticenergy")}
    for pl in "xyz":
        for q in ("alpha", "beta", "emit"):
            out[f"{q}_{pl}"] = str(round(b[f"{q}_{pl}"], 5))
    return out


@rpc("beam.useParticles")
def use_particles(name):
    """Make a particle file in InputFile the initial beam (readparticledistribution)."""
    form = load()["form"]
    form.update(use_dst=True, dst=name)
    result = save(form)
    log.info("initial beam read from %s", name)
    return result
