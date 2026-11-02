"""Diagnostics: where the beam is measured, how precisely, and the readings as observations.

Simulated and (later) measured data share :class:`Observation`; the analyses take readings as a
vector in the order of :meth:`Diagnostics.channels`, so a measurement only has to be turned into
observations with the same monitor names.
"""
import json
import math
from dataclasses import asdict, dataclass

import numpy as np

from avas.data import schema
from avas.model.model import PLANES

KINDS = ("position", "size")


@dataclass
class Monitor:
    """One monitor: *kind* "position" (centroid, a BPM) or "size" (rms size), *planes* "x", "y" or "xy",
    *resolution* the rms error of one reading [m]."""
    name: str
    z: float
    kind: str = "position"
    planes: str = "xy"
    resolution: float = 0.0
    line: int = None

    def __post_init__(self):
        if self.kind not in KINDS:
            raise ValueError(f"Monitor '{self.name}': kind must be one of {KINDS}.")
        if not self.planes or set(self.planes) - {"x", "y"}:
            raise ValueError(f"Monitor '{self.name}': planes must be 'x', 'y' or 'xy'.")


@dataclass
class Observation:
    """One reading [m]: simulated by :meth:`Diagnostics.measure` or measured."""
    name: str
    z: float
    kind: str
    plane: str
    value: float
    sigma: float = 0.0
    pulse: int = None
    time: float = None


class Diagnostics:
    """A list of :class:`Monitor`."""

    def __init__(self, monitors):
        self.monitors = list(monitors)
        names = [m.name for m in self.monitors]
        if len(set(names)) != len(names):
            raise ValueError("Monitor names must be unique.")

    @classmethod
    def from_lattice(cls, model, resolution=20e-6, size_resolution=50e-6):
        """The ``diag_position`` (position) and ``diag_size`` (size) lines of the model's lattice."""
        out = []
        for st in model.doc.statements:
            if not st.active or st.category != schema.DIAG or st.key not in ("diag_position", "diag_size"):
                continue
            kind = "position" if st.key == "diag_position" else "size"
            out.append(Monitor(st.name or f"line {st.line_no + 1}", float(st.z_start), kind, "xy",
                               resolution if kind == "position" else size_resolution, st.line_no))
        return cls(out)

    @classmethod
    def from_spec(cls, spec):
        """From ``[{"name", "z", "kind", "planes", "resolution"}, ...]`` (or a dict with "monitors")."""
        items = spec.get("monitors", []) if isinstance(spec, dict) else spec
        return cls(Monitor(**item) for item in items)

    def to_spec(self):
        return {"monitors": [asdict(m) for m in self.monitors]}

    def channels(self):
        """``[(monitor, plane)]``: the order of reading vectors."""
        return [(m, p) for m in self.monitors for p in m.planes]

    def labels(self):
        return [f"{m.name}:{m.kind[0]}{p}" for m, p in self.channels()]

    def resolution(self):
        return np.array([m.resolution for m, _p in self.channels()])

    def _index(self, result, monitor):
        if monitor.z > result.z[-1] + 1e-9:
            raise ValueError(f"Monitor '{monitor.name}' (z = {monitor.z} m) is beyond the end of the result.")
        return result.index(monitor.z, "right")

    def read(self, result):
        """Noiseless readings of *result* [m]."""
        out = []
        for m, p in self.channels():
            i = self._index(result, m)
            if m.kind == "position":
                out.append(result.centroid[i, PLANES[p]])
            else:
                out.append(math.sqrt(max(result.sigma[i, PLANES[p], PLANES[p]], 0.0)))
        return np.array(out)

    def measure(self, result, rng=None, pulse=None):
        """Readings with Gaussian noise of each monitor's resolution, as observations."""
        rng = rng if rng is not None else np.random.default_rng()
        values = self.read(result)
        sigma = self.resolution()
        noisy = values + rng.normal(0.0, 1.0, len(values)) * sigma
        return [Observation(m.name, m.z, m.kind, p, float(v), float(s), pulse)
                for (m, p), v, s in zip(self.channels(), noisy, sigma)]

    def vector(self, observations):
        """Reading vector from observations (missing channels are NaN)."""
        by_key = {(o.name, o.kind, o.plane): o.value for o in observations}
        return np.array([by_key.get((m.name, m.kind, p), math.nan) for m, p in self.channels()])

    def output_matrix(self, result, start=0, planes="xy"):
        """Rows mapping the centroid (x, x', y, y', z, δ) at point *start* to the position readings
        downstream of it (size channels are not linear in the centroid and are left out)."""
        rows, labels = [], []
        for (m, p), label in zip(self.channels(), self.labels()):
            if m.kind != "position" or p not in planes:
                continue
            i = self._index(result, m)
            if i < start:
                continue
            rows.append(result.transfer(start, i)[PLANES[p]])
            labels.append(label)
        return np.array(rows).reshape(-1, 6), labels


def save_observations(path, observations):
    with open(path, "w", encoding="utf-8") as fh:
        json.dump([asdict(o) for o in observations], fh, indent=1)


def load_observations(path):
    with open(path, encoding="utf-8") as fh:
        return [Observation(**item) for item in json.load(fh)]
