"""Many runs: sequential or in parallel child processes, resumable through the study's cache."""
import os
import queue
from concurrent.futures import ThreadPoolExecutor

from avas.research.runs import EngineRun


def threads_per_run(choice, workers, cpus=None):
    """CPU threads of each of *workers* parallel runs: the original kernel 1 (its results depend on the
    thread count, and one thread avoids AVAS.dll's data race), the new kernel an equal share of the cores
    (at least 1; a positive ``threads`` of the choice is the total to share)."""
    if not choice.is_new:
        return 1
    total = int(choice.threads) or int(cpus or os.cpu_count() or 1)
    return max(1, total // max(1, int(workers)))


def run_many(study, variants, fidelity="engine", workers=1, force=False, progress=None, stop=None, **evaluate):
    """Runs of *variants* in their order (identical variants run once).

    *workers* > 1 runs that many engines at once with :func:`threads_per_run` CPU threads each (the kernel
    choice's ``threads``: the new kernel's results do not depend on it, so parallel and sequential runs share
    the cache; the original kernel runs single-threaded).  New-kernel runs on CUDA GPUs
    bind one GPU each (``gpus`` of the run), with no more workers than GPUs; with one GPU (or Metal)
    the runs go one at a time (:meth:`Study.parallel_plan`, noted in the log).  *progress(done, total, run)*
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
        workers, slots = study.parallel_plan(workers) if workers > 1 else (workers, None)
        per_run = threads_per_run(study.engine_choice(), workers) if workers > 1 else None
        free = None
        if slots:
            free = queue.Queue()
            for slot in slots:
                free.put(slot)

        def one(key):
            if stop is not None and stop.is_set():
                return key, None
            if free is None:
                return key, study.engine(unique[key], force=force, threads=per_run, stop=stop)
            slot = free.get()
            try:
                return key, study.engine(unique[key], force=force, threads=per_run, stop=stop, gpus=slot)
            finally:
                free.put(slot)

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
