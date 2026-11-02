# %% [markdown]
# # Linear model against the engine, seed ensemble and a training set
#
# `examples/hwr010` as a study (records go to a temporary folder, the example stays untouched):
# one variant by both fidelities on a common z grid, the engine's spread over five seeds, and a
# Latin-hypercube sample of the cavity's Ke and phase exported for learning.  Engine parts are
# skipped where the engine does not run.

# %%
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__)) if "__file__" in globals() else "."
sys.path.insert(0, os.path.join(HERE, "..", ".."))

from avas import paths, research  # noqa: E402
from avas.research import Variant  # noqa: E402

study = research.Study(os.path.dirname(paths.example_input_dir()),
                       root=os.path.join(tempfile.gettempdir(), "avas_example_linear_vs_engine"))
print("study folder:", study.root)
variant = Variant(lattice={2: {"Ke": 1.2}}, particles=2000, label="Ke 1.2")       # line 2: the HWR

# %% one variant, both fidelities
try:
    cmp = research.compare(study, variant)
    for row in cmp["summary"]:
        print(f"{row['quantity']:7s} {row['at']:3s}  linear {row['linear']:9.4f}  engine {row['engine']:9.4f}"
              f"  diff {row['diff_rel'] * 100:6.2f} %")
except RuntimeError as exc:
    cmp = None
    print("engine comparison skipped:", str(exc)[:200])

# %% the engine's spread over seeds
if cmp is not None:
    ens = research.ensemble(study, variant, seeds=(1, 2, 3, 4, 5))
    for key in ("energy_out", "rms_x_out", "rms_y_out", "emit_x_growth"):
        m = ens["metrics"].get(key)
        if m:
            print(f"{key:14s} {m['mean']:.4g} ± {m['std']:.2g}")

# %% a training set: Latin hypercube in Ke and phase, linear model (add fidelity="engine" for the engine)
sample = research.latin_hypercube({"2.Ke": (1.0, 1.5), "2.phase": (-40.0, -20.0)}, 64, seed=1)
runs = study.run_many(sample, "linear")
arrays = research.export(os.path.join(study.root, "training"), runs, points=100)
print("exported:", arrays["inputs"].shape, "inputs,", arrays["metrics"].shape, "metrics,", arrays["rms_x"].shape,
      "rms x curves →", os.path.join(study.root, "training.npz"))
print(study.table().tail() if study.records() else "no engine runs recorded")

# %%
if (__name__ == "__main__" or "get_ipython" in globals()) and cmp is not None:
    import matplotlib.pyplot as plt

    from avas.research import plots
    plots.comparison(cmp, "rms_x")
    plt.show()
