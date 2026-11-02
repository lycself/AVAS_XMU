# %% [markdown]
# # Orbit response, controllability and a feedback loop
#
# The example project with two steerers in front of the solenoid and the HWR and four position
# monitors (the line of `tools/compare_response_engine.py`): response matrix of the linear model,
# what the steerers can correct, how monitor noise propagates into the knobs, and a pulse-to-pulse
# feedback against a drifting solenoid offset.  With the engine available, the engine's response
# matrix is computed as well (6 runs in a temporary study folder).

# %%
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__)) if "__file__" in globals() else "."
sys.path.insert(0, os.path.join(HERE, "..", ".."))

import numpy as np  # noqa: E402

from avas import dynamics as dyn  # noqa: E402
from avas import paths, research  # noqa: E402
from avas.model import LinearModel, read_beam  # noqa: E402

LATTICE = """start
drift 0.05 0.03 0
S1: steerer 0 0.03 0 0 0 0 0.01
drift 0.1 0.03 0
SOL: field 0.35 0.02 0 3 0 0 0 0.3 sol
drift 0.1 0.03 0
S2: steerer 0 0.03 0 0 0 0 0.01
drift 0.05 0.03 0
field 0.21 0.02 0 1 162.5e6 -30 0.8 -0.8 hwr010
drift 0.4 0.03 0
end
"""
inp = paths.example_input_dir()
model = LinearModel(LATTICE, read_beam(inp), [inp])
diag = dyn.Diagnostics([dyn.Monitor(n, z, resolution=20e-6) for n, z in
                        (("M1", 0.60), ("M2", 0.86), ("M3", 1.06), ("M4", 1.26))])
knobs = dyn.steerers(model)

# %% response matrix and single-pass correction
resp = dyn.response(model, knobs, diag)
print("knobs:", [k.label for k in knobs])
print("response (mm per mT):\n", np.round(resp.matrix, 4))
corr = dyn.static(resp.matrix, noise=diag.resolution())
print("singular values:", np.round(corr["singular_values"], 4), " rank", corr["rank"])
print("knob error from 20 µm monitor noise (mT):", np.round(corr["knob_noise"] * 1e3, 3))

# %% pulse-to-pulse feedback against a random walk of the solenoid offset
drift = dyn.Drift(errors={"SOL": {"dx": 5e-6, "dy": 5e-6}}, seed=1)
loop = dyn.simulate(model, knobs, diag, pulses=150, gain=0.5, delay=1, disturbance=drift, response=resp)
print(f"rms reading at the end: open {loop['open_rms'][-1] * 1e6:.1f} µm, closed {loop['rms'][-1] * 1e6:.1f} µm; "
      f"predicted |z| of the corrected modes {loop['predicted_modulus']:.3f}")

# %% the same response from the engine, when it runs here
# A copy of the example project with this lattice; ini.ini points to the example's field maps.
import configparser  # noqa: E402

from avas.data.inputs import copy_text_inputs  # noqa: E402

work = tempfile.mkdtemp(prefix="avas_example_response_")
copy_text_inputs(inp, os.path.join(work, "InputFile"))
with open(os.path.join(work, "InputFile", paths.DEFAULT_LATTICE), "w", newline="\n") as fh:
    fh.write(LATTICE)
ini = configparser.ConfigParser()
ini.optionxform = str
ini.read(os.path.join(work, "InputFile", "ini.ini"))
ini.set("project", "fieldSource", inp)
ini.set("lattice", "source", "")
with open(os.path.join(work, "InputFile", "ini.ini"), "w") as fh:
    ini.write(fh)
study = research.Study(work, name="response")
try:
    engine_resp = research.engine_response(study, [dyn.Knob(k.target, k.param, step=1e-3) for k in knobs], diag,
                                           research.Variant(particles=200))
    print("engine response (mm per mT):\n", np.round(engine_resp.matrix, 4))
    print("largest difference / largest element:",
          f"{np.abs(engine_resp.matrix - resp.matrix).max() / np.abs(resp.matrix).max():.3g}")
except RuntimeError as exc:
    print("engine part skipped:", str(exc)[:200])

# %%
if __name__ == "__main__" or "get_ipython" in globals():
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(figsize=(7, 3.5))
    ax.plot(loop["open_rms"] * 1e6, label="without feedback")
    ax.plot(loop["rms"] * 1e6, label="with feedback")
    ax.set_xlabel("pulse")
    ax.set_ylabel("rms reading (µm)")
    ax.legend()
    plt.show()
