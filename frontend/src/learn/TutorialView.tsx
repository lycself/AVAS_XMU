// The primer's guided tutorial: build a small superconducting linac from the
// example cavity and solenoid maps step by step (avas/learn/tutorial.py).  Every
// change shows the linear preview at once; "Run with the engine" simulates the
// step with the multi-particle engine in a sandbox outside any project
// (avas/gui/services/learn.py) and overlays its result.
import { useEffect, useMemo, useState } from "react";
import { call, on } from "../bridge";
import { Plot } from "../components/Plot";
import { reportError, toast } from "../components/overlays";
import { Button, Icon, ProgressBar, Spinner, cx } from "../components/ui";
import { fmtG, fmtSeconds } from "../format";
import { pickSaveFile } from "../host";
import { openProject } from "../actions";
import { pick, useLang, useT } from "../i18n";
import type { Bi } from "../lattice/types";
import { setPage, useApp } from "../store/app";
import { ParamControl, type LabParam } from "./LabView";
import { useLatest } from "./useLatest";
import { BeamlineView, type BeamlinePreview } from "./BeamlineView";
import { exampleMaps } from "./FieldView";
import { askTutor } from "./tutor";

type Step = { id: string; title: Bi; params: LabParam[] };
type Element = { key: string; name: string; z0: number | null; z1: number | null; w_in: number | null; w_out: number | null; phase_rf: number | null; phase_s: number | null };
type Preview = {
  z: Float64Array; rms_x: Float64Array; rms_y: Float64Array; rms_z: Float64Array; energy: Float64Array;
  elements: Element[]; warnings: Bi[]; lattice: string; params: Record<string, number>;
};
type EngineCurves = { z: Float64Array; rmsX: Float64Array; rmsY: Float64Array; rmsZ: Float64Array; energy: Float64Array; alive: Float64Array };
type RunState = {
  status: "idle" | "running" | "done" | "failed" | "stopped";
  running?: boolean; step?: string; params?: Record<string, number>; percent?: number; seconds?: number;
  error?: string; result?: EngineCurves; hasResult?: boolean;
};

const PLOT_CONFIG = { displaylogo: false, responsive: true, scrollZoom: false };

function defaults(step: Step, carry: Record<string, number> = {}) {
  const out: Record<string, number> = {};
  for (const p of step.params) out[p.key] = carry[p.key] ?? p.default;
  return out;
}

/** The beam line as boxes along z: solenoids, cavities and drifts. */
export function TutorialView() {
  const t = useT();
  const lang = useLang((s) => s.lang);
  const busyElsewhere = useApp((s) => s.run.running);
  const [steps, setSteps] = useState<Step[] | null>(null);
  const [available, setAvailable] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [active, setActive] = useState(0);
  const [values, setValues] = useState<Record<string, Record<string, number>>>({});
  const [run, setRun] = useState<RunState>({ status: "idle" });
  const [dirs, setDirs] = useState<string[]>([]);

  useEffect(() => {
    let live = true;
    call<{ steps: Step[]; available: boolean }>("learn.tutorial.steps")
      .then((r) => {
        if (!live) return;
        setSteps(r.steps);
        setAvailable(r.available);
        setValues(Object.fromEntries(r.steps.map((s) => [s.id, defaults(s)])));
      })
      .catch((e) => live && setError(String(e?.message ?? e)));
    const refresh = () =>
      call<RunState>("learn.tutorial.state")
        .then((s) => live && setRun(s))
        .catch(() => undefined);
    refresh();
    exampleMaps()
      .then((e) => live && setDirs(e.fieldDirs))
      .catch(() => undefined);
    const offs = [
      on("learn.tutorial", (ev: RunState) => {
        if (!live) return;
        if (ev.hasResult) refresh();
        else setRun((r) => ({ ...r, ...ev, result: ev.status === "running" ? undefined : r.result }));
      }),
      on("bridge.reconnected", refresh),
    ];
    return () => {
      live = false;
      offs.forEach((off) => off());
    };
  }, []);

  const step = steps?.[active];
  const params = step ? values[step.id] : undefined;
  const { result: preview, error: previewError, busy } = useLatest<Preview>("learn.tutorial.preview", step && params ? { step: step.id, params } : null);

  const engine = run.result && run.step === step?.id ? run.result : null;
  const sameParams = engine && run.params && params && step!.params.every((p) => run.params![p.key] === params[p.key]);

  const plots = useMemo(() => {
    if (!preview) return [];
    const z = Array.from(preview.z);
    const line = (name: string, x: ArrayLike<number>, y: ArrayLike<number>, dash?: boolean) => ({
      type: "scatter" as const, mode: "lines" as const, name, x: Array.from(x), y: Array.from(y), line: dash ? { width: 2, dash: "dash" as const } : { width: 2 },
    });
    const pre = t("preview");
    const eng = t("engine");
    const env = [line(`rms x · ${pre}`, z, preview.rms_x, !!engine), line(`rms y · ${pre}`, z, preview.rms_y, !!engine)];
    const lon = [line(`rms z · ${pre}`, z, preview.rms_z, !!engine)];
    const energy = [line(`W · ${pre}`, z, preview.energy, !!engine)];
    if (engine) {
      env.push(line(`rms x · ${eng}`, engine.z, engine.rmsX), line(`rms y · ${eng}`, engine.z, engine.rmsY));
      lon.push(line(`rms z · ${eng}`, engine.z, engine.rmsZ));
      energy.push(line(`W · ${eng}`, engine.z, engine.energy));
    }
    const layout = (y: string) => ({
      xaxis: { title: { text: "z (m)" } },
      yaxis: { title: { text: y } },
      legend: { orientation: "h" as const, y: 1.16, x: 0 },
      margin: { l: 60, r: 16, t: 30, b: 46 },
    });
    return [
      { key: "env", title: t("Transverse rms envelope"), data: env, layout: layout(t("rms size (mm)")) },
      { key: "lon", title: t("Bunch length"), data: lon, layout: layout(t("rms z (mm)")) },
      { key: "w", title: t("Kinetic energy"), data: energy, layout: layout("W (MeV)") },
    ];
  }, [preview, engine, lang, t]);

  if (error) return <div className="learn-lab learn-lab-error">{error}</div>;
  if (!steps || !step || !params) return <div className="learn-lab"><Spinner /></div>;
  if (!available) return <div className="learn-lab learn-lab-error">{t("The example project (examples/hwr010) is not installed; the tutorial needs its field maps.")}</div>;

  const setParam = (k: string, v: number) => setValues((s) => ({ ...s, [step.id]: { ...s[step.id], [k]: v } }));
  const goto = (i: number) => {
    const next = steps[i];
    // carry the settings the reader changed into the next step; untouched ones take the next step's own
    // defaults (steps 4 and 5 start from gentler Kb / Ke than step 3)
    const own = defaults(step);
    const changed = Object.fromEntries(Object.entries(values[step.id] ?? {}).filter(([k, v]) => v !== own[k]));
    setValues((s) => ({ ...s, [next.id]: { ...defaults(next, changed), ...(i > active ? {} : s[next.id]) } }));
    setActive(i);
  };
  const running = run.status === "running";
  const start = async () => {
    try {
      setRun(await call<RunState>("learn.tutorial.run", { step: step.id, params }));
    } catch (e) {
      reportError(e, t("Run with the engine"));
    }
  };
  const stop = () => call("learn.tutorial.stop").catch(reportError);
  const save = async () => {
    try {
      const target = await pickSaveFile({ filename: `avas_primer_${step.id}`, title: t("Save as a new project") });
      if (!target) return;
      const r = await call<{ path: string }>("learn.tutorial.export", { folder: target, step: step.id, params });
      toast(t("Project saved: {path}", { path: r.path }), "success");
      await openProject(r.path);
      if (useApp.getState().project.path) setPage("lattice");
    } catch (e) {
      reportError(e, t("Save as a new project"));
    }
  };
  const last = (a?: ArrayLike<number>) => (a && a.length ? a[a.length - 1] : null);
  const transmission = engine && engine.alive.length ? (100 * last(engine.alive)!) / engine.alive[0] : null;
  const ask = () =>
    askTutor(
      t("I am at tutorial step \"{step}\" of the primer with {params}. Explain what happens to the beam in this step and what to try next.", {
        step: pick(step.title),
        params: step.params.map((p) => `${pick(p.label)} = ${params[p.key]}${p.unit ? ` ${p.unit}` : ""}`).join("; "),
      }),
    );

  return (
    <section className="learn-lab learn-tutorial" aria-label={t("Guided tutorial")}>
      <header className="learn-lab-head">
        <Icon name="tools" />
        <div className="grow">
          <div className="learn-lab-title">{t("Guided tutorial: a small superconducting linac")}</div>
          <div className="soft">{t("Example HWR cavities (162.5 MHz) and solenoids, protons from 1.53 MeV.")}</div>
        </div>
        <Button small icon="sparkle" onClick={ask}>
          {t("Ask the tutor")}
        </Button>
      </header>
      <ol className="learn-steps" role="tablist">
        {steps.map((s, i) => (
          <li key={s.id}>
            <button role="tab" aria-selected={i === active} className={cx("learn-step", i === active && "active")} onClick={() => goto(i)}>
              {pick(s.title)}
            </button>
          </li>
        ))}
      </ol>
      <div className="learn-lab-body">
        <div className="learn-lab-params">
          {step.params.map((p) => (
            <ParamControl key={p.key} p={p} value={params[p.key]} onChange={(v) => setParam(p.key, v)} />
          ))}
          <div className="learn-tutorial-actions">
            {running ? (
              <Button icon="debug-stop" className="btn-stop" onClick={stop}>
                {t("Stop")}
              </Button>
            ) : (
              <Button variant="primary" icon="play" disabled={busyElsewhere} onClick={start} tip={busyElsewhere ? t("A simulation is already running; wait for it to finish.") : undefined}>
                {t("Run with the engine")}
              </Button>
            )}
            <Button icon="save" onClick={save}>
              {t("Save as a new project")}
            </Button>
            <StepNav active={active} count={steps.length} goto={goto} />
          </div>
          {running && (
            <div className="learn-run">
              <ProgressBar value={run.percent ?? 0} indeterminate={!run.percent} />
              <span className="soft">{t("Engine run in the sandbox: {pct} %", { pct: fmtG(run.percent ?? 0, 3) })}</span>
            </div>
          )}
          {run.status === "failed" && run.step === step.id && <div className="learn-lab-error">{t("The engine run failed: {error}", { error: run.error ?? "?" })}</div>}
          {run.status === "stopped" && run.step === step.id && <div className="soft">{t("The engine run was stopped.")}</div>}
          <p className="soft learn-note">{t("The engine runs on copies in your AVAS data folder; no project file is changed.")}</p>
        </div>
        <div className="learn-lab-results">
          {previewError && <div className="learn-lab-error">{previewError}</div>}
          {preview && (
            <>
              <BeamlineView
                text={preview.lattice}
                fieldDirs={dirs}
                preview={preview as unknown as BeamlinePreview}
                engine={engine && sameParams ? { z: engine.z, rmsX: engine.rmsX, rmsY: engine.rmsY, energy: engine.energy, label: t("engine") } : null}
              />
              <dl className="learn-values">
                <div>
                  <dt>{t("Exit energy (preview)")}</dt>
                  <dd className="mono">{fmtG(last(preview.energy), 5)} <span className="soft">MeV</span></dd>
                </div>
                <div>
                  <dt>{t("Largest rms size (preview)")}</dt>
                  <dd className="mono">{fmtG(Math.max(...Array.from(preview.rms_x), ...Array.from(preview.rms_y)), 4)} <span className="soft">mm</span></dd>
                </div>
                {engine && (
                  <>
                    <div>
                      <dt>{t("Exit energy (engine)")}</dt>
                      <dd className="mono">{fmtG(last(engine.energy), 5)} <span className="soft">MeV</span></dd>
                    </div>
                    <div>
                      <dt>{t("Transmission (engine)")}</dt>
                      <dd className="mono">{fmtG(transmission, 4)} <span className="soft">%</span></dd>
                    </div>
                    <div>
                      <dt>{t("Engine run time")}</dt>
                      <dd className="mono">{fmtSeconds(run.seconds ?? null)}</dd>
                    </div>
                  </>
                )}
              </dl>
              {engine && !sameParams && <div className="learn-warning"><Icon name="warning" /> {t("The engine result belongs to earlier parameters; run again to compare.")}</div>}
              {preview.warnings.map((w, i) => (
                <div key={i} className="learn-warning">
                  <Icon name="warning" /> {pick(w)}
                </div>
              ))}
              <div className="learn-plots">
                {plots.map((p) => (
                  <figure key={p.key} className="learn-plot">
                    <figcaption>{p.title}</figcaption>
                    <Plot data={p.data} layout={p.layout} config={PLOT_CONFIG} style={{ height: 250 }} />
                  </figure>
                ))}
              </div>
              <p className="learn-model">
                <Icon name="info" /> {t("Dashed or single curves: linear preview (approximate). Solid curves after an engine run: multi-particle engine (the reference).")}
              </p>
              <details className="learn-lattice" open>
                <summary>{t("AVAS lattice of this step")}{busy && <Spinner size={12} />}</summary>
                <pre className="mono selectable">{preview.lattice}</pre>
              </details>
            </>
          )}
        </div>
      </div>
    </section>
  );
}

function StepNav({ active, count, goto }: { active: number; count: number; goto: (i: number) => void }) {
  const t = useT();
  return (
    <span className="learn-step-nav">
      <Button small icon="arrow-left" disabled={active === 0} onClick={() => goto(active - 1)}>
        {t("Previous step")}
      </Button>
      <Button small icon="arrow-right" disabled={active === count - 1} onClick={() => goto(active + 1)}>
        {t("Next step")}
      </Button>
    </span>
  );
}
