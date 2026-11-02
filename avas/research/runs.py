"""One run of either fidelity, read the same way.

Both kinds give ``z`` [m], ``energy`` [MeV], ``sigma`` (·, 6, 6) and ``centroid`` (·, 6) in SI units,
so :meth:`avas.dynamics.Diagnostics.read` works on either.  The engine's DataSet.txt has only the
rms sizes and divergences and the x / y centroid: the other entries are NaN.  ``alive`` and
``transmission`` exist for engine runs only; ``metrics`` share the keys and units of
:func:`avas.post.analysis.run_diagnostics.dataset_metrics` (mm, MeV, π·mm·mrad).
"""
import math
import os

import numpy as np

from avas.model.model import PLANES

COMMON_METRICS = ("energy_out", "rms_x_max", "rms_y_max", "rms_x_out", "rms_y_out", "emit_x_out", "emit_y_out",
                  "emit_x_growth", "emit_y_growth", "centroid_x_max", "centroid_y_max")


class Run:
    """Common reading methods; see :class:`LinearRun` and :class:`EngineRun`."""

    fidelity = ""
    alive = None
    transmission = None

    def __init__(self, variant, record=None):
        self.variant = variant
        self.record = record or {}

    @property
    def ok(self):
        return self.record.get("status", "done") == "done" and len(self.z) > 0

    def index(self, z, side="left"):
        if side == "left":
            i = int(np.searchsorted(self.z, z - 1e-12, side="left"))
        else:
            i = int(np.searchsorted(self.z, z + 1e-12, side="right")) - 1
        return min(max(i, 0), len(self.z) - 1)

    def rms(self, plane):
        i = PLANES[plane]
        return np.sqrt(np.maximum(self.sigma[:, i, i], 0.0))

    def curve(self, quantity):
        """``rms_x`` … ``rms_z`` [m], ``cx`` / ``cy`` [m], ``energy`` [MeV], ``emit_x`` / ``emit_y`` (normalised, m·rad)."""
        if quantity.startswith("rms_"):
            return self.rms(quantity[4:])
        if quantity in ("cx", "cy"):
            return self.centroid[:, PLANES[quantity[1]]]
        if quantity == "energy":
            return self.energy
        if quantity.startswith("emit_"):
            return self.emittance(quantity[5:])
        raise ValueError(f"Unknown quantity '{quantity}'.")


class LinearRun(Run):
    """A :class:`avas.model.ModelResult` (``result``) as a run."""

    fidelity = "linear"

    def __init__(self, variant, result, record=None):
        super().__init__(variant, record)
        self.result = result
        self.z, self.energy, self.sigma, self.centroid = result.z, result.energy, result.sigma, result.centroid
        self.metrics = _linear_metrics(result)

    def emittance(self, plane, normalized=True):
        return self.result.emittance(plane, normalized)


class EngineRun(Run):
    """An engine run of a study: ``folder`` with ``input/``, ``output/``, ``run.json`` and ``engine.log``."""

    fidelity = "engine"

    def __init__(self, variant, folder, record):
        super().__init__(variant, record)
        self.folder = folder
        self.output_dir = os.path.join(folder, "output")
        self.metrics = dict(record.get("metrics") or {})
        self._load()

    def _load(self):
        empty = np.zeros(0)
        self.z = self.energy = self.alive = empty
        self.sigma, self.centroid, self._emit = np.zeros((0, 6, 6)), np.zeros((0, 6)), np.zeros((0, 3))
        path = os.path.join(self.output_dir, "DataSet.txt")
        if self.record.get("status") != "done" or not os.path.isfile(path):
            return
        from avas.data.dataset_stream import PathLength
        from avas.post.analysis.run_diagnostics import read_dataset_array
        d = read_dataset_array(path)
        if not d.shape[0]:
            return
        z, keep = PathLength().update(d)
        d, z = d[keep], np.asarray(z, dtype=float)
        n = len(d)
        sigma = np.full((n, 6, 6), math.nan)
        for i, col in ((0, 16), (1, 17), (2, 18), (3, 19), (4, 20)):
            sigma[:, i, i] = d[:, col] ** 2
        centroid = np.full((n, 6), math.nan)
        centroid[:, 0], centroid[:, 2] = d[:, 1] + d[:, 29], d[:, 3] + d[:, 31]
        self.z, self.energy, self.alive = z, d[:, 0], d[:, 28]
        self.sigma, self.centroid, self._emit = sigma, centroid, d[:, 13:16]
        self.transmission = float(self.alive[-1] / self.alive[0]) if self.alive[0] else None

    def emittance(self, plane, normalized=True):
        """Normalised rms emittance [m·rad] from DataSet.txt (the engine gives only the normalised one)."""
        if not normalized:
            raise ValueError("DataSet.txt has normalised emittances only.")
        return self._emit[:, "xyz".index(plane)]


def _linear_metrics(res):
    """The :data:`COMMON_METRICS` of a model result, in the units of ``dataset_metrics``."""
    rx, ry = res.rms("x") * 1e3, res.rms("y") * 1e3
    ex, ey = res.emittance("x", True) * 1e6, res.emittance("y", True) * 1e6

    def growth(e):
        return float(e[-1] / e[0] - 1) if e[0] else None

    return {"energy_in": float(res.energy[0]), "energy_out": float(res.energy[-1]),
            "rms_x_max": float(rx.max()), "rms_y_max": float(ry.max()), "rms_x_out": float(rx[-1]),
            "rms_y_out": float(ry[-1]), "emit_x_in": float(ex[0]), "emit_y_in": float(ey[0]),
            "emit_x_out": float(ex[-1]), "emit_y_out": float(ey[-1]), "emit_x_growth": growth(ex),
            "emit_y_growth": growth(ey), "centroid_x_max": float(np.abs(res.centroid[:, 0]).max() * 1e3),
            "centroid_y_max": float(np.abs(res.centroid[:, 2]).max() * 1e3), "z_out": float(res.z[-1])}
