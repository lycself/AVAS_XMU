"""Building blocks shared by the primer's labs (``labs.py``, ``control.py``): parameters, lab definitions and
the dictionaries of values and plots the page renders.  A module of its own so that the lab modules import it
without importing each other."""
import math

import numpy as np


class LabError(Exception):
    """The parameters cannot be evaluated (a message for the page)."""


class Param:
    __slots__ = ("key", "label", "unit", "default", "min", "max", "step", "log", "integer", "doc")

    def __init__(self, key, label, unit, default, lo_, hi, step, log=False, integer=False, doc=("", "")):
        self.key, self.label, self.unit = key, label, unit
        self.default, self.min, self.max, self.step = default, lo_, hi, step
        self.log, self.integer, self.doc = log, integer, doc

    def public(self):
        return {"key": self.key, "label": list(self.label), "unit": self.unit, "default": self.default,
                "min": self.min, "max": self.max, "step": self.step, "log": self.log, "integer": self.integer,
                "doc": list(self.doc)}


class Lab:
    def __init__(self, id_, title, params, fn, summary):
        self.id, self.title, self.params, self.fn, self.summary = id_, title, params, fn, summary

    def public(self):
        return {"id": self.id, "title": list(self.title), "summary": list(self.summary),
                "params": [p.public() for p in self.params]}


def _value(key, en, zh, unit, value):
    # NaN / inf (a run that blew up) are shown as missing: JSON has no NaN
    v = None if value is None else float(value)
    return {"key": key, "label": (en, zh), "unit": unit, "value": v if v is not None and math.isfinite(v) else None}


def _series(en, zh, x, y, **style):
    return {"name": (en, zh), "x": np.asarray(x, dtype=float), "y": np.asarray(y, dtype=float), **style}


def _plot(id_, title, x, y, series, **opts):
    return {"id": id_, "title": title, "x": x, "y": y, "series": series, **opts}


def _axis(en, zh, unit="", **opts):
    return {"label": (en, zh), "unit": unit, **opts}
