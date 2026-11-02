"""Pulse-to-pulse feedback on the linear model: drifting errors, noisy monitors, delayed corrections.

Every pulse evaluates the model with the present knob values and the disturbance of that pulse,
reads the monitors with noise and updates the knobs with the integral law

    u_{k+1} = u_k − g · R⁺ · (y_{k−d} − y_ref)

(R⁺ the pseudo-inverse of the response matrix with small singular values cut, d the delay in
pulses).  For a static plant each corrected mode follows z^{d+1} − z^d + g = 0
(:func:`avas.dynamics.control.integral_loop_modulus`): stable for 0 < g < 2 without delay and for
0 < g < 1 with one pulse of delay.
"""
import numpy as np

from avas.dynamics.control import integral_loop_modulus, static
from avas.dynamics.response import response as compute_response
from avas.dynamics.response import settings


class Drift:
    """Disturbance of each pulse: element errors and an initial centroid that random-walk and / or
    oscillate.

    *errors*: ``{target: {"dx": rms step per pulse [m], ...}}`` (random walk); *centroid*: rms step per
    pulse of the initial centroid (6 values, m and rad); *oscillation*: ``{(target, key): (amplitude,
    period in pulses)}`` added on top; *offset*: errors present from the first pulse.
    """

    def __init__(self, errors=None, centroid=None, oscillation=None, offset=None, seed=0):
        self.steps = {t: dict(e) for t, e in (errors or {}).items()}
        self.centroid_step = None if centroid is None else np.asarray(centroid, dtype=float)
        self.oscillation = dict(oscillation or {})
        self.walk = {t: {k: float(v) for k, v in e.items()} for t, e in (offset or {}).items()}
        for t, e in self.steps.items():
            for k in e:
                self.walk.setdefault(t, {}).setdefault(k, 0.0)
        self.c0 = np.zeros(6)
        self.rng = np.random.default_rng(seed)

    def __call__(self, pulse):
        """``(errors, centroid0)`` of *pulse* (call in order 0, 1, 2 …)."""
        if pulse > 0:
            for t, e in self.steps.items():
                for k, s in e.items():
                    self.walk[t][k] += self.rng.normal(0.0, s)
            if self.centroid_step is not None:
                self.c0 = self.c0 + self.rng.normal(0.0, 1.0, 6) * self.centroid_step
        errors = {t: dict(e) for t, e in self.walk.items()}
        for (t, k), (amp, period) in self.oscillation.items():
            errors.setdefault(t, {})[k] = errors.get(t, {}).get(k, 0.0) + amp * np.sin(2 * np.pi * pulse / period)
        return errors, self.c0.copy()


def simulate(model, knobs, diagnostics, pulses=200, gain=0.5, delay=0, cut=1e-3, disturbance=None,
             noise=True, reference=None, response=None, seed=0, **evaluate):
    """Run the loop for *pulses* pulses.

    *reference*: target readings (default zero for positions, the uncorrected readings for sizes);
    *response*: a :class:`~avas.dynamics.response.Response` (default computed around the model);
    *cut*: singular values below this × the largest are not corrected; *noise*: monitor noise on/off.
    Returns ``readings`` (pulses × channels, as measured), ``true`` (without noise), ``knobs``
    (pulses × knobs, the values used in each pulse), ``rms`` (rms of true − reference per pulse),
    ``open_rms`` (the same without feedback, same disturbance), ``predicted_modulus`` and ``labels``.
    """
    knobs = list(knobs)
    resp = response or compute_response(model, knobs, diagnostics, **evaluate)
    corr = static(resp.matrix, cut=cut)
    pinv = corr["pinv"]
    if reference is None:
        reference = np.where([m.kind == "position" for m, _p in diagnostics.channels()], 0.0, resp.y0)
    rng = np.random.default_rng(seed)
    sigma = diagnostics.resolution() if noise else np.zeros(len(resp.y0))
    lower = np.array([-np.inf if k.lower is None else k.lower for k in knobs])
    upper = np.array([np.inf if k.upper is None else k.upper for k in knobs])
    disturbance = disturbance or (lambda _k: ({}, None))
    u = resp.u0.copy()
    measured, true, used, open_true = [], [], [], []
    for k in range(pulses):
        errors, c0 = disturbance(k)
        merged = {t: dict(e) for t, e in (evaluate.get("errors") or {}).items()}
        for t, e in errors.items():
            for key, v in e.items():
                merged.setdefault(t, {})[key] = merged.get(t, {}).get(key, 0.0) + v
        base = dict(evaluate, centroid0=c0 if c0 is not None else evaluate.get("centroid0"))
        base.pop("errors", None)
        y = diagnostics.read(model.evaluate(**settings(knobs, u, merged, **base)))
        open_true.append(diagnostics.read(model.evaluate(**settings(knobs, resp.u0, merged, **base))))
        true.append(y)
        measured.append(y + rng.normal(0.0, 1.0, len(y)) * sigma)
        used.append(u.copy())
        if k >= delay:
            u = np.clip(u - gain * pinv @ (measured[k - delay] - reference), lower, upper)
    true, open_true = np.array(true), np.array(open_true)
    return {"readings": np.array(measured), "true": true, "knobs": np.array(used),
            "rms": np.sqrt(np.mean((true - reference) ** 2, axis=1)),
            "open_rms": np.sqrt(np.mean((open_true - reference) ** 2, axis=1)),
            "predicted_modulus": integral_loop_modulus(gain, delay), "labels": diagnostics.labels(),
            "knob_labels": [k.label for k in knobs], "response": resp}
