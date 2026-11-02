"""Many runs: sequential or in parallel child processes, resumable through the study's cache."""
from concurrent.futures import ThreadPoolExecutor

from avas.research.runs import EngineRun


def run_many(study, variants, fidelity="engine", workers=1, force=False, progress=None, stop=None, **evaluate):
    """Runs of *variants* in their order (identical variants run once).

    *workers* > 1 runs that many engines at once, each single-threaded (``multithreading`` is removed
    from input.txt: the fingerprint differs from a multi-threaded run).  *progress(done, total, run)*
    is called after each run; *stop* (``threading.Event``) skips the runs not started yet and stops the
    running ones.  A failed run does not stop the others; its record holds the error.  *evaluate* goes
    to :meth:`Study.linear` for the linear fidelity.
    """
    variants = list(variants)
    unique, order = {}, []
    for v in variants:
        order.append(unique.setdefault(v.key(), v).key())
    keys = list(unique)
    total = len(keys)
    results = {}
    done = 0

    def report(key, run):
        nonlocal done
        results[key] = run
        done += 1
        if progress:
            progress(done, total, run)

    if fidelity == "linear":
        for key in keys:
            if stop is not None and stop.is_set():
                break
            report(key, study.linear(unique[key], **evaluate))
    elif fidelity == "engine":
        def one(key):
            if stop is not None and stop.is_set():
                return key, None
            return key, study.engine(unique[key], force=force, single_thread=workers > 1, stop=stop)

        if workers <= 1:
            for key in keys:
                k, run = one(key)
                if run is None:
                    break
                report(k, run)
        else:
            with ThreadPoolExecutor(max_workers=workers) as pool:
                for k, run in pool.map(one, keys):
                    if run is not None:
                        report(k, run)
    else:
        raise ValueError("fidelity is 'linear' or 'engine'.")
    return [results.get(key) for key in order]


def failed(runs):
    """The engine runs among *runs* that did not finish, with their errors."""
    return [(r.variant, r.record.get("error")) for r in runs if isinstance(r, EngineRun) and not r.ok]
