"""matplotlib figures for notebooks and reports (matplotlib is imported only here)."""
import numpy as np

UNITS = {"rms_x": ("rms x", "mm", 1e3), "rms_y": ("rms y", "mm", 1e3), "rms_z": ("rms z", "mm", 1e3),
         "cx": ("centroid x", "mm", 1e3), "cy": ("centroid y", "mm", 1e3), "energy": ("energy", "MeV", 1.0)}


def _axes(ax):
    if ax is None:
        import matplotlib.pyplot as plt
        _fig, ax = plt.subplots(figsize=(7, 3.5))
    return ax


def runs(runs, quantity="rms_x", labels=None, ax=None):
    """*quantity* along z for several runs."""
    ax = _axes(ax)
    name, unit, scale = UNITS[quantity]
    for k, run in enumerate(r for r in runs if r is not None and r.ok):
        label = (labels[k] if labels else None) or run.variant.label or f"{run.fidelity} {k + 1}"
        ax.plot(run.z, np.asarray(run.curve(quantity)) * scale, label=label,
                linestyle="--" if run.fidelity == "linear" else "-")
    ax.set_xlabel("z (m)")
    ax.set_ylabel(f"{name} ({unit})")
    ax.legend(fontsize="small")
    return ax


def comparison(cmp, quantity="rms_x", ax=None):
    """Linear model and engine of :func:`avas.research.compare.compare`, with the difference below."""
    import matplotlib.pyplot as plt
    name, unit, scale = UNITS[quantity]
    if ax is None:
        _fig, (ax, ax2) = plt.subplots(2, 1, figsize=(7, 4.5), sharex=True, gridspec_kw={"height_ratios": [3, 1]})
    else:
        ax2 = None
    data = cmp[quantity]
    ax.plot(cmp["z"], data["engine"] * scale, label="engine")
    ax.plot(cmp["z"], data["linear"] * scale, "--", label="linear model")
    ax.set_ylabel(f"{name} ({unit})")
    ax.legend(fontsize="small")
    if ax2 is not None:
        ax2.plot(cmp["z"], data["diff"] * scale, color="0.4")         # design:allow-colour matplotlib figure
        ax2.axhline(0, color="0.7", linewidth=0.8)                     # design:allow-colour matplotlib figure
        ax2.set_ylabel(f"diff ({unit})")
        ax2.set_xlabel("z (m)")
    return ax


def heatmap(result, key, ax=None, cmap="viridis"):
    """A metric of a two-axis :func:`avas.research.scan.grid` (or :func:`avas.dynamics.grid`) result."""
    ax = _axes(ax)
    if "axes" in result:
        (xname, xs), (yname, ys) = result["axes"][1], result["axes"][0]
        values = result[key]
    else:
        xname, xs, yname, ys, values = "x", result["x"], "y", result["y"], result[key]
    mesh = ax.pcolormesh(xs, ys, values, shading="nearest", cmap=cmap)
    ax.figure.colorbar(mesh, ax=ax, label=key)
    ax.set_xlabel(xname)
    ax.set_ylabel(yname)
    return ax
