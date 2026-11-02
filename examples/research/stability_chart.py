# %% [markdown]
# # Stability chart of a FODO channel
#
# Zero-current phase advance σ0 and depressed phase advance σ of a matched dc proton beam,
# and the envelope stability of the period, over quadrupole gradient × beam current
# (linear model, `avas.dynamics`).  The 90° stop band appears where σ0 > 90° and the current
# pushes σ below about 90°.  Run cell by cell in VS Code or Jupyter, or as a script.

# %%
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)) if "__file__" in globals() else ".", "..", ".."))

import numpy as np  # noqa: E402

from avas import dynamics as dyn  # noqa: E402
from avas.model import LinearModel  # noqa: E402

BEAM = {"mass": 938.272088, "charge": 1.0, "energy": 3.0, "current": 0.0, "frequency": 162.5e6, "dc": True,
        "twiss": {"x": (0.0, 1.0, 0.2), "y": (0.0, 1.0, 0.2), "z": (0.0, 1.0, 0.3)}}


def fodo(gradient):
    cell = (f"drift 0.1 0.05 0\nquad 0.1 0.05 0 {gradient}\ndrift 0.2 0.05 0\nquad 0.1 0.05 0 {-gradient}\n"
            "drift 0.1 0.05 0\n")
    return "start\nlattice 5 1\n" + cell + "lattice_end\n" + cell + "end\n"


def point(gradient, current):
    model = LinearModel(fodo(gradient), dict(BEAM, current=current))
    cell = dyn.Period(model.evaluate(), 1)
    jac = dyn.envelope_jacobian(cell)
    m = jac["matched"]
    return {"sigma0": m["zero_current"]["planes"]["x"]["mu_deg"], "sigma": m["depressed"]["planes"]["x"]["mu_deg"],
            "envelope_max_modulus": jac["max_modulus"], "stable": jac["stable"], "converged": m["converged"]}


# %%
gradients = np.linspace(6, 18, 13)
currents = np.linspace(0, 120, 13)
chart = dyn.grid(point, gradients, currents, progress=lambda d, t: print(f"\r{d}/{t}", end=""))
print("\nunstable points:", int(np.sum(chart["stable"] == 0)), "of", chart["stable"].size)

# %%
if __name__ == "__main__" or "get_ipython" in globals():
    import matplotlib.pyplot as plt

    from avas.research import plots
    fig, axes = plt.subplots(1, 2, figsize=(11, 4))
    plots.heatmap(chart, "sigma", ax=axes[0])
    plots.heatmap(chart, "envelope_max_modulus", ax=axes[1], cmap="magma")
    for ax in axes:
        ax.set_xlabel("quadrupole gradient (T/m)")
        ax.set_ylabel("current (mA)")
    fig.tight_layout()
    plt.show()
