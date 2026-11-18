"""A variant: the project's inputs with a few changes, for either fidelity.

* ``lattice``: ``{target: {param: value}}`` as in :meth:`avas.model.LinearModel.with_knobs`
  (element name, 1-based line or ``{"index": n}``; parameter key or position);
* ``beam`` / ``input``: ``{keyword: value}`` for beam.txt / input.txt (a list sets the values
  from position 1 on);
* ``seed`` (input.txt ``randomseed``) and ``particles`` (beam.txt ``particlenumber``).  With the
  original kernel or the new kernel's compatibility mode (compat = dll) a seed other than 0 gives an
  initial distribution with x fully correlated with the phase (correlation -1); the new kernel's
  default mode does not have this defect.

Addresses in scans: ``"Q1.G"`` (lattice), ``"beam.current"``, ``"input.stepsize"``, ``"seed"``,
``"particles"``, or a tuple ``("lattice", target, param)``.
"""
import copy
import json
import os
from dataclasses import asdict, dataclass, field

from avas.data.lattice_edit import set_keyword_value


@dataclass
class Variant:
    lattice: dict = field(default_factory=dict)
    beam: dict = field(default_factory=dict)
    input: dict = field(default_factory=dict)
    seed: int = None
    particles: int = None
    label: str = ""

    def to_dict(self):
        return asdict(self)

    @classmethod
    def from_dict(cls, data):
        return cls(**{k: copy.deepcopy(v) for k, v in (data or {}).items()})

    def key(self):
        """Canonical text of the changes (the label is not part of it)."""
        data = self.to_dict()
        data.pop("label")
        return json.dumps(data, sort_keys=True, default=str)

    def set(self, address, value):
        """Copy with the value at *address* replaced."""
        kind, target, param = parse_address(address)
        out = Variant.from_dict(self.to_dict())
        if kind == "lattice":
            out.lattice.setdefault(target, {})[param] = value
        elif kind in ("beam", "input"):
            getattr(out, kind)[param] = value
        else:
            setattr(out, kind, None if value is None else int(round(float(value))))
        return out

    def get(self, address):
        kind, target, param = parse_address(address)
        if kind == "lattice":
            return self.lattice.get(target, {}).get(param)
        if kind in ("beam", "input"):
            return getattr(self, kind).get(param)
        return getattr(self, kind)

    def flat(self):
        """``{address: value}`` of every change (for tables and exports)."""
        out = {f"{t}.{p}": v for t, ps in self.lattice.items() for p, v in ps.items()}
        out.update({f"beam.{k}": v for k, v in self.beam.items()})
        out.update({f"input.{k}": v for k, v in self.input.items()})
        for k in ("seed", "particles"):
            if getattr(self, k) is not None:
                out[k] = getattr(self, k)
        return out


def parse_address(address):
    """``(kind, target, param)`` with kind lattice | beam | input | seed | particles."""
    if isinstance(address, tuple):
        if len(address) != 3:
            raise ValueError("An address tuple is (kind, target, param).")
        return address
    text = str(address).strip()
    if text in ("seed", "particles"):
        return text, None, None
    head, sep, tail = text.rpartition(".")
    if not sep or not head or not tail:
        raise ValueError(f"Address '{address}': write element.parameter, beam.keyword or input.keyword.")
    if head in ("beam", "input"):
        return head, None, tail
    target = int(head) if head.isdigit() else head
    return "lattice", target, tail


def _edit_keywords(path, values):
    with open(path, encoding="utf-8", errors="replace") as fh:
        text = fh.read()
    for keyword, value in values.items():
        items = value if isinstance(value, (list, tuple)) else [value]
        for position, item in enumerate(items, start=1):
            text = set_keyword_value(text, keyword, item, position)
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text)


def apply(variant, input_dir, lattice_name, model, seed=None):
    """Write the changes of *variant* into the copied inputs in *input_dir*.

    *model*: the study's :class:`~avas.model.LinearModel` (its ``with_knobs`` writes the lattice);
    *seed*: used when the variant has none.  The thread count is not an input: it is the kernel choice's
    ``threads`` (input.txt ``multithreading`` cannot switch threads off, see :mod:`avas.engines`).
    """
    if variant.lattice:
        with open(os.path.join(input_dir, lattice_name), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(model.with_knobs(variant.lattice))
    beam = dict(variant.beam)
    if variant.particles is not None:
        beam["particlenumber"] = int(variant.particles)
    if beam:
        _edit_keywords(os.path.join(input_dir, "beam.txt"), beam)
    inputs = dict(variant.input)
    chosen = variant.seed if variant.seed is not None else seed
    if chosen is not None:
        inputs["randomseed"] = int(chosen)
    if inputs:
        _edit_keywords(os.path.join(input_dir, "input.txt"), inputs)
