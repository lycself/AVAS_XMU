"""Research interface for notebooks and scripts: variants of a project run by the linear model or the
engine, with records, a cache, batches, scans, comparisons and exports.

    from avas import research
    study = research.Study("examples/hwr010", name="first look")
    lin = study.run(research.Variant(lattice={3: {"Ke": 1.2}}), "linear")
    eng = study.run(research.Variant(lattice={3: {"Ke": 1.2}}), "engine")
    cmp = research.compare(study)

* :mod:`~avas.research.variant` – :class:`Variant` (lattice / beam / input changes, seed, particles).
* :mod:`~avas.research.study` – :class:`Study`: runs, fingerprints, records, ``table()``.
* :mod:`~avas.research.runs` – :class:`LinearRun` / :class:`EngineRun`, read the same way.
* :mod:`~avas.research.batch` – many runs, sequential or parallel, resumable.
* :mod:`~avas.research.scan` – grids over any addresses, Latin-hypercube samples.
* :mod:`~avas.research.compare` – linear vs engine, seed ensembles, engine response matrices.
* :mod:`~avas.research.export` – npz / CSV for learning.  :mod:`~avas.research.plots` – figures.

Engine runs are child processes (an engine crash cannot take the notebook down) and never write
the project's own folders; they go to ``<project>/Research/<name>/runs/``.
"""
from avas.research.batch import failed, run_many  # noqa: F401
from avas.research.compare import compare, engine_response, ensemble  # noqa: F401
from avas.research.engine import EngineError  # noqa: F401
from avas.research.export import export  # noqa: F401
from avas.research.runs import EngineRun, LinearRun  # noqa: F401
from avas.research.scan import grid, grid_variants, latin_hypercube  # noqa: F401
from avas.research.study import Study  # noqa: F401
from avas.research.variant import Variant, parse_address  # noqa: F401
