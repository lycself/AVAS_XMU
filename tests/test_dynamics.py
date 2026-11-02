"""Unit tests for the dynamics and control analysis (avas.dynamics); no engine runs.

Expectations are analytic (thick-lens FODO, drift kicks, integral loops) or cross-checks between two
independent routes (KV equations against the σ-matrix model, simulated loops against eigenvalues).
"""
import math
import os
import sys

import numpy as np
import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXAMPLE_INPUT = os.path.join(ROOT, "examples", "hwr010", "InputFile")
sys.path.insert(0, ROOT)

from avas import dynamics as dyn  # noqa: E402
from avas.dynamics import kv  # noqa: E402
from avas.dynamics.moments import J6, eigen_emittances, mismatch  # noqa: E402
from avas.dynamics.response import settings  # noqa: E402
from avas.model import LinearModel  # noqa: E402
from avas.model.common import MEV_TM  # noqa: E402
from avas.model.elements import drift_matrix, quad_matrix  # noqa: E402

PROTON = 938.272088
needs_example = pytest.mark.skipif(not os.path.isfile(os.path.join(EXAMPLE_INPUT, "sol.bsz")),
                                   reason="example field maps not installed")


def make_beam(energy=3.0, current=0.0, dc=True, emit=0.2):
    return {"mass": PROTON, "charge": 1.0, "energy": energy, "current": current, "frequency": 162.5e6, "dc": dc,
            "twiss": {"x": (0.0, 1.0, emit), "y": (0.0, 1.0, emit), "z": (0.0, 1.0, 0.3)}}


def fodo(gradient, periods=3):
    cell = (f"drift 0.1 0.05 0\nquad 0.1 0.05 0 {gradient}\ndrift 0.2 0.05 0\nquad 0.1 0.05 0 {-gradient}\n"
            "drift 0.1 0.05 0\n")
    return "start\nlattice 5 1\n" + cell + "lattice_end\n" + cell * (periods - 1) + "end\n"


def momentum(energy):
    gamma = 1 + energy / PROTON
    return PROTON * math.sqrt(gamma * gamma - 1), gamma


# --------------------------------------------------------------------------- periods
def test_fodo_period_matches_thick_lens_matrices():
    p, gamma = momentum(3.0)
    k = 8.0 * MEV_TM / p
    m = (drift_matrix(0.1, gamma) @ quad_matrix(0.1, -k, gamma) @ drift_matrix(0.2, gamma) @ quad_matrix(0.1, k, gamma)
         @ drift_matrix(0.1, gamma))
    cell = dyn.Period(LinearModel(fodo(8), make_beam()).evaluate(), 1)
    zc = cell.zero_current()
    assert np.allclose(zc["matrix"], m, rtol=1e-10, atol=1e-12)
    mu = math.degrees(math.acos((m[0, 0] + m[1, 1]) / 2))
    assert zc["planes"]["x"]["mu_deg"] == pytest.approx(mu, rel=1e-10)
    assert zc["stable"] and not zc["planes"]["z"]["stable"]          # no RF: z is not periodic
    matched = cell.matched(current=0)
    assert matched["planes"] == "xy"
    s = matched["sigma"]
    assert np.allclose(m[:4, :4] @ s[:4, :4] @ m[:4, :4].T, s[:4, :4], rtol=1e-9, atol=1e-20)
    alpha, beta = zc["planes"]["x"]["alpha"], zc["planes"]["x"]["beta"]
    assert s[0, 0] / matched["emittances"]["x"] == pytest.approx(beta, rel=1e-9)
    assert -s[0, 1] / matched["emittances"]["x"] == pytest.approx(alpha, rel=1e-8, abs=1e-12)
    assert matched["mismatch"]["x"] > 0.1                              # the beam of make_beam is not matched
    assert mismatch(s, s, "x") == pytest.approx(0, abs=1e-6)
    assert dyn.periods(cell.result.doc) == [{"index": 1, "title": "lattice", "line": 1, "z0": 0.0, "z1": pytest.approx(0.6)}]


def test_coupled_solenoid_period_is_matched_with_its_eigen_emittances():
    text = "start\nlattice 3 1\ndrift 0.2 0.05 0\nsolenoid 0.3 0.05 0 0.8\ndrift 0.2 0.05 0\nlattice_end\nend\n"
    cell = dyn.Period(LinearModel(text, make_beam()).evaluate(), 1)
    zc = cell.zero_current()
    assert zc["coupling"] > 0.01
    matched = cell.matched(current=0, emittances={"x": 2e-7, "y": 1e-7})
    s, m = matched["sigma"], zc["matrix"]
    assert matched["planes"] == "xy"
    assert np.allclose(m[:4, :4] @ s[:4, :4] @ m[:4, :4].T, s[:4, :4], rtol=1e-9, atol=1e-20)
    eps = sorted(eigen_emittances(s, "xy").values())
    assert eps == pytest.approx([1e-7, 2e-7], rel=1e-9)
    assert np.all(np.linalg.eigvalsh(s[:4, :4]) > 0)


def test_space_charge_matching_agrees_with_the_kv_equations():
    current = 20.0
    cell = dyn.Period(LinearModel(fodo(8), make_beam(current=current)).evaluate(), 1)
    matched = cell.matched()
    assert matched["converged"] and 0.85 < matched["tune_depression"]["x"] < 0.95
    s = matched["sigma"]
    segments = kv.segments_from_lattice(fodo(8, 1), 3.0, PROTON)
    perv = kv.perveance(current, 3.0, PROTON)
    eps = (4 * matched["emittances"]["x"], 4 * matched["emittances"]["y"])
    guess = [2 * math.sqrt(s[0, 0]), 2 * s[0, 1] / math.sqrt(s[0, 0]), 2 * math.sqrt(s[2, 2]), 2 * s[2, 3] / math.sqrt(s[2, 2])]
    a = kv.matched(segments, perv, eps, guess)
    assert a[0] == pytest.approx(2 * math.sqrt(s[0, 0]), rel=1e-4)
    assert a[2] == pytest.approx(2 * math.sqrt(s[2, 2]), rel=1e-4)
    # the matched beam comes back after a period
    assert np.allclose(cell.map(s)[:4, :4], s[:4, :4], rtol=1e-6, atol=1e-13)
    assert kv.perveance(1000.0, 1e6, PROTON) < 1e-12
    gamma = 1 + 3.0 / PROTON
    i0 = 2 * current * 1e-3 / (perv * (gamma * gamma - 1) ** 1.5)
    assert i0 == pytest.approx(3.13e7, rel=2e-3)                       # 4πε0·mc³/e for protons


def test_envelope_jacobian_and_the_90_degree_stop_band():
    zero = dyn.Period(LinearModel(fodo(14), make_beam()).evaluate(), 1)
    jac = dyn.envelope_jacobian(zero, current=0)
    mu = math.radians(zero.zero_current()["planes"]["x"]["mu_deg"])
    assert np.allclose(np.abs(jac["eigenvalues"]), 1, atol=1e-6)
    assert any(abs(math.radians(t) - min(2 * mu, 2 * math.pi - 2 * mu)) < 1e-5 for t in jac["mode_tunes_deg"])
    stable = {}
    for gradient, current in ((8, 60), (14, 5), (14, 60), (14, 120)):
        cell = dyn.Period(LinearModel(fodo(gradient), make_beam(current=current)).evaluate(), 1)
        out = dyn.envelope_jacobian(cell)
        stable[(gradient, current)] = out["stable"]
        assert out["matched"]["converged"]
    # σ0 ≈ 95°: the current that pushes σ just below 90° (about 83°) excites the envelope instability
    assert stable == {(8, 60): True, (14, 5): True, (14, 60): False, (14, 120): True}


@needs_example
def test_accelerating_period_is_normalised_and_listed():
    from avas.learn import tutorial
    text = tutorial.lattice("periods", {"periods": 2})
    res = LinearModel(text, tutorial.preview_beam("periods"), [EXAMPLE_INPUT]).evaluate(space_charge=False)
    cell = dyn.Period(res, z=(0.05, 0.81))
    zc = cell.zero_current()
    assert cell.damping > 1.05                                         # the HWR accelerates
    assert np.abs(np.abs(zc["eigenvalues"]) - 1).max() < 2e-3
    assert zc["coupling"] > 0.01                                       # the solenoid couples x and y
    assert zc["stable"]
    tagged = text.replace("! period 1\n", "! period 1\nlattice 4 1\n").replace("! period 2", "lattice_end\n! period 2")
    rows = dyn.period_table(LinearModel(tagged, tutorial.preview_beam("periods"), [EXAMPLE_INPUT]).evaluate())
    assert len(rows) == 1 and 0 < rows[0]["mu_z"] < rows[0]["mu0_z"] < 90


# --------------------------------------------------------------------------- diagnostics and responses
STEERED = ("start\ndrift 0.05 0.05 0\nS1: steerer 0 0.05 0 0 0 0 0.01\ndrift 0.25 0.05 0\n"
           "B1: diag_position 0 0 0 0\ndrift 1.0 0.05 0\nB2: diag_position 0 0 0 0\nW1: diag_size 0 0 0 0\n"
           "drift 0.1 0.05 0\nend\n")


def test_diagnostics_read_measure_and_round_trip(tmp_path):
    model = LinearModel(STEERED, make_beam(10.0, dc=False))
    diag = dyn.Diagnostics.from_lattice(model, resolution=1e-5, size_resolution=2e-5)
    assert [(m.name, m.kind) for m in diag.monitors] == [("B1", "position"), ("B2", "position"), ("W1", "size")]
    assert diag.labels() == ["B1:px", "B1:py", "B2:px", "B2:py", "W1:sx", "W1:sy"]
    res = model.evaluate(centroid0=[1e-3, 0, -2e-3, 0, 0, 0])
    y = diag.read(res)
    assert y[:4] == pytest.approx([1e-3, -2e-3, 1e-3, -2e-3])
    assert y[4] == pytest.approx(res.rms("x")[res.index(1.3, "right")], rel=1e-12)
    rng = np.random.default_rng(3)
    noisy = np.array([diag.vector(diag.measure(res, rng)) for _ in range(2000)])
    assert np.std(noisy[:, 0]) == pytest.approx(1e-5, rel=0.1) and np.std(noisy[:, 5]) == pytest.approx(2e-5, rel=0.1)
    obs = diag.measure(res, rng, pulse=7)
    path = tmp_path / "obs.json"
    from avas.dynamics.diagnostics import load_observations, save_observations
    save_observations(path, obs)
    assert np.allclose(diag.vector(load_observations(path)), diag.vector(obs))
    again = dyn.Diagnostics.from_spec(diag.to_spec())
    assert again.labels() == diag.labels()
    with pytest.raises(ValueError, match="kind"):
        dyn.Monitor("x", 0.0, kind="phase")


def test_steerer_response_is_the_drift_lever_arm():
    energy = 10.0
    model = LinearModel(STEERED, make_beam(energy, dc=False))
    diag = dyn.Diagnostics.from_lattice(model)
    knobs = dyn.steerers(model)
    assert [k.label for k in knobs] == ["S1.By", "S1.Bx"] and knobs[0].upper == pytest.approx(0.01)
    resp = dyn.response(model, knobs, diag)
    p, _g = momentum(energy)
    per_tesla = MEV_TM * 0.25 / p                                      # kick of 1 T over the next 0.25 m drift
    assert resp.matrix[0, 0] == pytest.approx(-per_tesla * 0.125, rel=1e-9)          # B1 x per By
    assert resp.matrix[2, 0] == pytest.approx(-per_tesla * 1.125, rel=1e-9)          # B2 x per By
    assert resp.matrix[3, 1] == pytest.approx(per_tesla * 1.125, rel=1e-9)           # B2 y per Bx
    assert resp.matrix[1, 0] == 0 and abs(resp.matrix[4, 0]) < 1e-12               # sizes do not see a kick
    # the centroid is linear: two kicks at once give the sum
    both = model.evaluate(**settings(knobs, [2e-3, -1e-3]))
    assert np.allclose(diag.read(both)[:4], resp.y0[:4] + resp.matrix[:4] @ [2e-3, -1e-3], rtol=1e-9, atol=1e-15)


def test_static_analysis_finds_what_no_knob_or_monitor_reaches():
    text = ("start\nB0: diag_position 0 0 0 0\ndrift 0.05 0.05 0\nS1: steerer 0 0.05 0 0 0 0 0\ndrift 0.3 0.05 0\n"
            "S2: steerer 0 0.05 0 0 0 0 0\ndrift 0.3 0.05 0\nB1: diag_position 0 0 0 0\ndrift 0.1 0.05 0\nend\n")
    model = LinearModel(text, make_beam(10.0, dc=False))
    diag = dyn.Diagnostics([m for m in dyn.Diagnostics.from_lattice(model).monitors])
    resp = dyn.response(model, dyn.steerers(model, "x"), diag)
    out = dyn.static(resp.matrix, noise=1e-5)
    assert out["rank"] == 1                                             # two kicks, one useful reading (B0 is upstream)
    assert out["invisible"].shape == (2, 1)
    assert np.allclose(resp.matrix @ out["invisible"], 0, atol=1e-12)
    assert out["uncorrectable"].shape[1] == 3


# --------------------------------------------------------------------------- control
def test_mode_classes_lqr_and_delayed_loops():
    a = np.diag([1.2, 0.5, 1.1])
    b = np.array([[1.0], [0.0], [0.0]])
    classes = [m["class"] for m in sorted(dyn.modes(a, b), key=lambda m: -m["modulus"])]
    assert classes == ["stabilizable", "uncontrollable", "stable"]
    assert dyn.controllability(a, b)["rank"] == 1
    obs = dyn.observability(a, np.array([[1.0, 1.0, 1.0]]), noise=[0.1], horizon=5)
    assert obs["rank"] == 3 and np.all(np.isfinite(obs["state_error"]))
    blind = dyn.observability(a, np.array([[1.0, 0.0, 0.0]]))
    assert blind["rank"] == 1 and np.isinf(blind["state_error"][1])
    a2, b2 = np.array([[1.1, 0.3], [0.0, 0.9]]), np.array([[0.0], [1.0]])
    k = dyn.lqr(a2, b2)
    assert np.max(np.abs(dyn.closed_loop(a2, b2, k))) < 1
    # scalar integrator x_{n+1} = x_n − g·x_{n−d}: the companion roots and the polynomial agree
    from avas.dynamics.control import integral_loop_modulus
    for gain, delay in ((0.5, 0), (1.9, 0), (2.1, 0), (0.9, 1), (1.1, 1), (0.3, 3)):
        roots = dyn.delayed_loop(np.eye(1), np.eye(1), -gain * np.eye(1), delay)
        assert np.max(np.abs(roots)) == pytest.approx(integral_loop_modulus(gain, delay), rel=1e-9)
    assert integral_loop_modulus(1.9, 0) < 1 < integral_loop_modulus(2.1, 0)
    assert integral_loop_modulus(0.9, 1) < 1 < integral_loop_modulus(1.1, 1)
    plain = dyn.difference_loop(a, b, np.zeros((1, 3)))
    assert sorted(np.abs(plain))[-3:] == pytest.approx([0.5, 1.1, 1.2])


def test_period_orbit_system_reproduces_the_model():
    cell = ("drift 0.05 0.05 0\nS{n}: steerer 0 0.05 0 0 0 0 0.01\ndrift 0.05 0.05 0\nquad 0.1 0.05 0 8\n"
            "B{n}: diag_position 0 0 0 0\ndrift 0.2 0.05 0\nquad 0.1 0.05 0 -8\ndrift 0.1 0.05 0\n")
    text = "start\nlattice 7 1\n" + cell.format(n=1) + "lattice_end\n" + cell.format(n=2) + "end\n"
    model = LinearModel(text, make_beam(dc=False))
    diag = dyn.Diagnostics.from_lattice(model)
    knobs = [k for k in dyn.steerers(model) if k.target == "S1"]
    sysd = dyn.orbit_system(model, 1, knobs, diag)
    assert sysd["A"].shape == (4, 4) and sysd["B"].shape == (4, 2) and sysd["C"].shape == (2, 4)
    assert sysd["reading_labels"] == ["B1:px", "B1:py"]
    x0, u = np.array([1e-3, 2e-4, -5e-4, 1e-4]), np.array([1e-3, -2e-3])
    res = model.evaluate(**settings(knobs, u, centroid0=np.r_[x0, 0, 0]))
    cell1 = sysd["period"]
    assert np.allclose(res.centroid[cell1.i1, :4], sysd["A"] @ x0 + sysd["B"] @ u, rtol=1e-9, atol=1e-15)
    reading = diag.read(res)[:2]
    assert np.allclose(reading, sysd["C"] @ x0 + sysd["D"] @ u, rtol=1e-9, atol=1e-15)
    assert dyn.controllability(sysd["A"], sysd["B"])["rank"] == 4
    assert dyn.observability(sysd["A"], sysd["C"])["rank"] == 4


def test_period_envelope_system_shapes_and_consistency():
    text = ("start\nlattice 6 1\ndrift 0.1 0.05 0\nQF: quad 0.1 0.05 0 8\nW: diag_size 0 0 0 0\ndrift 0.2 0.05 0\n"
            "QD: quad 0.1 0.05 0 -8\ndrift 0.1 0.05 0\nlattice_end\nend\n")
    model = LinearModel(text, make_beam(current=10.0))
    diag = dyn.Diagnostics.from_lattice(model)
    sysd = dyn.envelope_system(model, 1, [dyn.Knob("QF", "G"), dyn.Knob("QD", "G")], diag)
    plain = dyn.envelope_system(model, 1, [dyn.Knob("QF", "G")], diag, current=0)
    n = 10                                                              # upper triangle of the 4×4 transverse block
    assert sysd["A"].shape == (n, n) and sysd["B"].shape == (n, 2) and sysd["C"].shape == (2, n)
    assert np.all(np.isfinite(sysd["B"])) and np.abs(sysd["B"]).max() > 0
    assert sysd["jacobian"]["stable"]
    assert sysd["jacobian"]["matched"]["tune_depression"]["x"] < 1
    # without space charge rms_W² = (T Σ Tᵀ)_xx, so ∂rms_W/∂Σ_xx = T_xx² / (2·rms_W)
    k = plain["state_labels"].index("<x·x>")
    sigma, cell = plain["jacobian"]["matched"]["sigma"], plain["period"]
    t = cell.result.transfer(cell.i0, cell.result.index(0.2, "right"))
    rms_w = math.sqrt((t @ sigma @ t.T)[0, 0])
    assert plain["C"][0, k] == pytest.approx(t[0, 0] ** 2 / (2 * rms_w) * plain["jacobian"]["scale"][k], rel=1e-6)


# --------------------------------------------------------------------------- feedback and maps
def test_feedback_converges_below_the_gain_limit_and_diverges_above():
    text = ("start\ndrift 0.05 0.05 0\nS1: steerer 0 0.05 0 0 0 0 0.05\ndrift 0.3 0.05 0\nQ1: quad 0.1 0.05 0 6\n"
            "drift 0.3 0.05 0\nS2: steerer 0 0.05 0 0 0 0 0.05\ndrift 0.3 0.05 0\nB1: diag_position 0 0 0 0\n"
            "drift 0.4 0.05 0\nB2: diag_position 0 0 0 0\ndrift 0.1 0.05 0\nend\n")
    model = LinearModel(text, make_beam(10.0, dc=False))
    diag = dyn.Diagnostics.from_lattice(model, resolution=0.0)
    knobs = dyn.steerers(model)
    drift = dyn.Drift(offset={"Q1": {"dx": 0.5e-3, "dy": -0.3e-3}})
    good = dyn.simulate(model, knobs, diag, pulses=30, gain=0.8, disturbance=drift, noise=False)
    assert good["open_rms"][-1] > 1e-5 and good["rms"][-1] < 1e-6 * good["open_rms"][-1]
    assert good["predicted_modulus"] < 1
    bad = dyn.simulate(model, knobs, diag, pulses=30, gain=0.8, delay=2, disturbance=dyn.Drift(offset={"Q1": {"dx": 0.5e-3}}),
                       noise=False)
    assert bad["predicted_modulus"] > 1 and bad["rms"][-1] > bad["rms"][5]
    noisy = dyn.simulate(model, knobs, diag.__class__([dyn.Monitor(m.name, m.z, resolution=2e-5) for m in diag.monitors]),
                         pulses=40, gain=0.3, disturbance=dyn.Drift(offset={"Q1": {"dx": 0.5e-3}}), seed=2)
    assert noisy["rms"][-1] < 0.2 * noisy["open_rms"][-1]


def test_grid_collects_numbers_and_failures():
    def f(x, y):
        if x == 2 and y == 20:
            raise ValueError("no")
        return {"sum": x + y, "flag": x > 1, "text": "ignored"}
    out = dyn.grid(f, [1, 2, 3], [10, 20])
    assert out["sum"].shape == (2, 3) and out["sum"][1, 0] == 21 and math.isnan(out["sum"][1, 1])
    assert out["flag"][0].tolist() == [0.0, 1.0, 1.0] and "text" not in out
    assert out["errors"] == [(2, 20, "no")]


def test_eigen_emittances_are_invariant_under_symplectic_maps():
    rng = np.random.default_rng(1)
    s = np.diag([4e-6, 1e-6, 3e-6, 2e-6, 1e-6, 5e-7])
    s[0, 2] = s[2, 0] = 1e-6
    before = eigen_emittances(s)
    h = rng.normal(size=(6, 6))
    h = h + h.T
    from scipy.linalg import expm
    m = expm(J6 @ h * 0.3)                                                # a random symplectic map
    after = eigen_emittances(m @ s @ m.T)
    assert sorted(after.values()) == pytest.approx(sorted(before.values()), rel=1e-9)


def test_study_file_round_trip(tmp_path):
    model = LinearModel(STEERED, make_beam(10.0, dc=False))
    study = dyn.load_study({"monitors": [{"name": "extra", "z": 0.7, "resolution": 3e-5}],
                            "lattice_monitors": {"resolution": 1e-5}, "lattice_steerers": "x",
                            "knobs": [{"target": "S1", "param": "Bx", "step": 2e-4}],
                            "errors": {"S1": {"dx": 1e-4}}}, model)
    assert [m.name for m in study["diagnostics"].monitors] == ["extra", "B1", "B2", "W1"]
    assert [k.label for k in study["knobs"]] == ["S1.Bx", "S1.By"]
    path = tmp_path / "study.json"
    dyn.save_study(path, study["diagnostics"], study["knobs"], study["errors"])
    again = dyn.load_study(str(path))
    assert again["diagnostics"].labels() == study["diagnostics"].labels() and again["errors"] == {"S1": {"dx": 1e-4}}
    assert [k.step for k in again["knobs"]] == [2e-4, None]
    with pytest.raises(ValueError, match="pass the model"):
        dyn.load_study({"lattice_steerers": "xy"})
    with pytest.raises(ValueError, match="Unknown"):
        dyn.load_study({"bpms": []})
