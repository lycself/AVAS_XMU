"""Two-parameter maps (stability charts and the like): evaluate a function on a grid and collect its numbers."""
import math

import numpy as np


def grid(func, xs, ys, progress=None):
    """``func(x, y)`` → dict of numbers, on every point of *xs* × *ys*.

    Returns ``{"x": xs, "y": ys, key: array[len(ys), len(xs)], ..., "errors": [(x, y, message)]}``;
    booleans become 0 / 1, a point that raises gives NaN for every key.  *progress(done, total)* is
    called after each point.
    """
    xs, ys = list(xs), list(ys)
    values, errors = {}, []
    total = len(xs) * len(ys)
    done = 0
    for j, y in enumerate(ys):
        for i, x in enumerate(xs):
            try:
                out = func(x, y)
            except (ValueError, ArithmeticError, np.linalg.LinAlgError) as exc:
                errors.append((x, y, str(exc)))
                out = {}
            for key, v in out.items():
                if isinstance(v, (bool, np.bool_)):
                    v = float(v)
                if isinstance(v, (int, float, np.integer, np.floating)):
                    if key not in values:
                        values[key] = np.full((len(ys), len(xs)), math.nan)
                    values[key][j, i] = float(v)
            done += 1
            if progress:
                progress(done, total)
    return dict(values, x=np.asarray(xs, dtype=float), y=np.asarray(ys, dtype=float), errors=errors)
