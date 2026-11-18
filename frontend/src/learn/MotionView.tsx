// A particle animation of the primer (```avas-motion``` blocks): a few dozen
// particles of the example beam fly through a short lattice, in 2D (side views,
// cross-section seen from downstream, phase space) or 3D, while the reader moves
// the parameters.  The trajectories, the parameters and every number come from
// the back end (avas/learn/motion.py, the linear model); this file only animates.
import { lazy, Suspense, useCallback, useEffect, useMemo, useRef, useState } from "react";
import { call } from "../bridge";
import { Button, Checkbox, Icon, IconButton, Segmented, Spinner } from "../components/ui";
import { fmtG } from "../format";
import { pick, useT } from "../i18n";
import type { CloseUp } from "./CloseUp3D";
import { useMotion } from "../lattice/bunchPlayer";
import { keywordMap, type Bi } from "../lattice/types";
import { useApp } from "../store/app";
import { ErrorBoundary } from "../components/ErrorBoundary";
import { useNearScreen, useSchema } from "./BeamlineView";
import { LabPlotView, ParamControl, ResultValues, type LabParam, type LabPlot, type LabValue } from "./LabView";
import { activeElements } from "./fields";
import { drawPhase, drawSection, drawSide, elementAt, focusing, ranges, type MotionTracks, type PhasePlane } from "./motionDraw";
import { askTutor } from "./tutor";
import { useLatest } from "./useLatest";

const CloseUp3D = lazy(() => import("./CloseUp3D"));

export type MotionDef = { id: string; title: Bi; summary: Bi; params: LabParam[] };
export type MotionResult = MotionTracks & {
  lattice: string;
  fieldDirs: string[];
  energy: ArrayLike<number>;
  values: LabValue[];
  plots: LabPlot[];
  warnings: Bi[];
  model: Bi;
  params: Record<string, number>;
};

let defs: Promise<Map<string, MotionDef>> | null = null;
export function motionDefinitions(): Promise<Map<string, MotionDef>> {
  defs ??= call<MotionDef[]>("learn.motions").then((list) => new Map(list.map((d) => [d.id, d])));
  defs.catch(() => (defs = null));
  return defs;
}

type Mode = "2d" | "3d";
const SPEEDS = [0.25, 0.5, 1, 2];
/** Seconds for one pass at 1×: slow enough to follow single particles through an element (about 8 s per metre,
 *  at least 12 s), so a short line is not over in a blink and a long one is not a crawl per element. */
function passSeconds(length: number) {
  return Math.max(12, 8 * length);
}
const HOLD_SECONDS = 1.2; // pause at the exit before the particles start again
const SCRUB = 1000;

export function MotionView({ id, presets }: { id: string; presets: Record<string, string> }) {
  const t = useT();
  const theme = useApp((s) => s.resolvedTheme);
  const motionSetting = useMotion();
  const schema = useSchema();
  const kw = useMemo(() => (schema ? keywordMap(schema.lattice) : null), [schema]);
  const [def, setDef] = useState<MotionDef | null>(null);
  const [failed, setFailed] = useState<string | null>(null);
  const presetKey = JSON.stringify(presets);
  const initial = useMemo(() => {
    const given = JSON.parse(presetKey) as Record<string, string>;
    const out: Record<string, number> = {};
    for (const p of def?.params ?? []) out[p.key] = given[p.key] != null ? Number(given[p.key]) : p.default;
    return out;
  }, [def, presetKey]);
  const [values, setValues] = useState<Record<string, number> | null>(null);
  useEffect(() => {
    let live = true;
    motionDefinitions()
      .then((m) => {
        if (!live) return;
        const d = m.get(id);
        if (d) setDef(d);
        else setFailed(t("Unknown animation: {id}", { id }));
      })
      .catch((e) => live && setFailed(String(e?.message ?? e)));
    return () => void (live = false);
  }, [id, t]);
  useEffect(() => setValues(initial), [initial]);
  const { result, error, busy } = useLatest<MotionResult>("learn.motion", values && def ? { id, params: values } : null);

  const [mode, setMode] = useState<Mode>("2d");
  const [playing, setPlaying] = useState(motionSetting !== "off");
  const [speed, setSpeed] = useState(1);
  const [trails, setTrails] = useState(true);
  const [envelope, setEnvelope] = useState(true);
  const [phasePlane, setPhasePlane] = useState<PhasePlane>("x");
  const box = useRef<HTMLDivElement>(null);
  const near = useNearScreen(box);

  // ---- animation: progress 0…1 along the line, drawn imperatively (no React render per frame)
  const progress = useRef(motionSetting === "off" ? 1 : 0);
  const hold = useRef(0);
  const sideX = useRef<HTMLCanvasElement>(null);
  const sideY = useRef<HTMLCanvasElement>(null);
  const section = useRef<HTMLCanvasElement>(null);
  const phase = useRef<HTMLCanvasElement>(null);
  const scrub = useRef<HTMLInputElement>(null);
  const where = useRef<HTMLSpanElement>(null);
  const viewer = useRef<CloseUp | null>(null);
  const rg = useMemo(() => (result ? ranges(result) : null), [result]);
  const pass = result && result.s.length ? passSeconds(result.s[result.s.length - 1] - result.s[0]) : 12;
  const text = useMemo(
    () => ({
      x: { axis: "x (mm)", focusing: t("focusing"), defocusing: t("defocusing"), zLabel: "z (m)" },
      y: { axis: "y (mm)", focusing: t("focusing"), defocusing: t("defocusing"), zLabel: "z (m)" },
      section: {
        schematic: t("poles and force: schematic"),
        alongOut: t("B along the beam, out of the screen (⊙)"),
        alongIn: t("B along the beam, into the screen (⊗)"),
        rf: t("Electric field along the beam: it pushes the particles forward or back"),
      },
    }),
    [t],
  );

  const draw = useCallback(() => {
    if (!result || !rg || !result.s.length) return;
    const s0 = result.s[0];
    const s1 = result.s[result.s.length - 1];
    const sNow = s0 + (s1 - s0) * progress.current;
    if (scrub.current) scrub.current.value = String(Math.round(progress.current * SCRUB));
    if (where.current) {
      const e = activeElements(result, sNow)[0] ?? elementAt(result, sNow);
      let label = t("between elements");
      if (e) {
        const title = kw?.get(e.key)?.title;
        label = title ? pick(title) : e.key;
        const fx = focusing(e, "x", result.charge);
        if (fx) label += ` · x ${fx === "F" ? t("focusing") : t("defocusing")}, y ${fx === "F" ? t("defocusing") : t("focusing")}`;
      }
      where.current.textContent = `z = ${sNow.toFixed(3)} m · ${label}`;
    }
    if (mode === "3d") {
      viewer.current?.setS(sNow);
      return;
    }
    if (sideX.current) drawSide(sideX.current, result, "x", sNow, rg, { trails, envelope, centre: result.centre ?? result.ghosts.length > 0, text: text.x });
    if (sideY.current) drawSide(sideY.current, result, "y", sNow, rg, { trails, envelope, centre: result.centre ?? result.ghosts.length > 0, text: text.y });
    if (section.current) drawSection(section.current, result, sNow, rg, { envelope, text: text.section });
    if (phase.current) drawPhase(phase.current, result, phasePlane, sNow, rg, { envelope });
  }, [result, rg, mode, trails, envelope, phasePlane, text, kw, t]);

  // redraw when the data, the options, the theme or the size change
  useEffect(() => draw(), [draw, theme]);
  useEffect(() => {
    const el = box.current;
    if (!el || typeof ResizeObserver === "undefined") return;
    const ro = new ResizeObserver(() => draw());
    ro.observe(el);
    return () => ro.disconnect();
  }, [draw]);

  useEffect(() => {
    if (motionSetting === "off") setPlaying(false);
  }, [motionSetting]);

  useEffect(() => {
    if (!playing || !near || !result) return;
    let raf = 0;
    let last = performance.now();
    const minFrame = 1000 / 30;
    const tick = (now: number) => {
      raf = requestAnimationFrame(tick);
      const dt = (now - last) / 1000;
      if (dt * 1000 < minFrame) return;
      last = now;
      if (progress.current >= 1) {
        hold.current += dt;
        if (hold.current < HOLD_SECONDS) return;
        hold.current = 0;
        progress.current = 0;
      } else {
        progress.current = Math.min(1, progress.current + (dt * speed) / pass);
      }
      draw();
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [playing, near, result, speed, draw, pass]);

  if (failed) return <div className="learn-lab learn-lab-error">{failed}</div>;
  if (!def || !values)
    return (
      <div className="learn-lab">
        <Spinner />
      </div>
    );

  const ask = () => {
    const params = def.params.map((p) => `${pick(p.label)} = ${values[p.key]}${p.unit ? ` ${p.unit}` : ""}`).join("; ");
    const out = (result?.values ?? []).map((v) => `${pick(v.label)} = ${v.value == null ? "—" : fmtG(v.value, 5)}${v.unit ? ` ${v.unit}` : ""}`).join("; ");
    askTutor(t("In the primer animation \"{lab}\" I set: {params}. It reports: {values}. Explain in simple words what the particles do and why.", { lab: pick(def.title), params, values: out }));
  };
  const restart = () => {
    progress.current = 0;
    hold.current = 0;
    draw();
  };

  return (
    <section className="learn-lab learn-motion" aria-label={pick(def.title)}>
      <header className="learn-lab-head">
        <Icon name="play-circle" />
        <div className="grow">
          <div className="learn-lab-title">{pick(def.title)}</div>
          <div className="soft">{pick(def.summary)}</div>
        </div>
        {busy && <Spinner size={14} />}
        <Button small icon="discard" onClick={() => setValues(initial)}>
          {t("Reset")}
        </Button>
        <Button small icon="sparkle" onClick={ask}>
          {t("Ask the tutor")}
        </Button>
      </header>
      <div className="learn-lab-body">
        <div className="learn-lab-params">
          {def.params.map((p) => (
            <ParamControl key={p.key} p={p} value={values[p.key]} onChange={(v) => setValues((s) => ({ ...(s ?? {}), [p.key]: v }))} />
          ))}
        </div>
        <div className="learn-lab-results">
          {error && <div className="learn-lab-error">{error}</div>}
          <div className="motion-bar">
            <IconButton icon={playing ? "debug-pause" : "play"} active={playing} tip={playing ? t("Pause") : t("Play")} onClick={() => setPlaying(!playing)} />
            <IconButton icon="debug-restart" tip={t("Back to the entrance")} onClick={restart} />
            <input
              ref={scrub}
              className="motion-scrub"
              type="range"
              min={0}
              max={SCRUB}
              defaultValue={Math.round(progress.current * SCRUB)}
              aria-label={t("Position along the beam line")}
              onChange={(e) => {
                progress.current = Number(e.target.value) / SCRUB;
                hold.current = 0;
                draw();
              }}
            />
            <Segmented<string>
              value={String(speed)}
              onChange={(v) => setSpeed(Number(v))}
              options={SPEEDS.map((v) => ({ value: String(v), label: `${v}×`, tip: t("Animation speed") }))}
            />
            <Segmented<Mode>
              value={mode}
              onChange={setMode}
              options={[
                { value: "2d", label: "2D", tip: t("Side views, cross-section and phase space") },
                { value: "3d", label: "3D", tip: t("The particles flying through the elements in 3D") },
              ]}
            />
          </div>
          <div className="motion-bar motion-bar-sub">
            <span ref={where} className="mono motion-where" />
            <span className="grow" />
            <Checkbox checked={trails} onChange={setTrails} label={t("Paths")} tip={t("Draw the path each particle has flown")} />
            <Checkbox checked={envelope} onChange={setEnvelope} label={t("rms envelope")} tip={t("The beam's rms size (dashed)")} />
          </div>
          <div ref={box} className={`motion-stage${mode === "3d" ? " is-3d" : ""}`}>
            {!result ? (
              <div className="empty-state"><Spinner size={20} /></div>
            ) : mode === "2d" ? (
              <>
                <div className="motion-side">
                  <figure>
                    <figcaption>{t("Seen from above: horizontal position x along the beam line")}</figcaption>
                    <canvas ref={sideX} />
                  </figure>
                  <figure>
                    <figcaption>{t("Seen from the side: vertical position y along the beam line")}</figcaption>
                    <canvas ref={sideY} />
                  </figure>
                </div>
                <div className="motion-right">
                  <figure>
                    <figcaption>{t("Cross-section here, seen from downstream (the beam comes out of the screen)")}</figcaption>
                    <canvas ref={section} className="motion-square" />
                  </figure>
                  <figure>
                    <figcaption className="motion-phase-head">
                      <span>{t("Phase space: position and angle")}</span>
                      <Segmented<PhasePlane>
                        value={phasePlane}
                        onChange={setPhasePlane}
                        options={[
                          { value: "x", label: "x" },
                          { value: "y", label: "y" },
                          { value: "z", label: "z", tip: t("Longitudinal: place in the bunch z and momentum deviation δ") },
                        ]}
                      />
                    </figcaption>
                    <canvas ref={phase} className="motion-phase" />
                  </figure>
                </div>
              </>
            ) : near ? (
              <ErrorBoundary name="3D view">
              <Suspense fallback={<div className="empty-state"><Spinner size={20} /></div>}>
                <CloseUp3D
                  tracks={result}
                  trails={trails}
                  onViewer={(v) => {
                    viewer.current = v;
                    if (v) requestAnimationFrame(draw);
                  }}
                />
              </Suspense>
              </ErrorBoundary>
            ) : (
              <div className="empty-state"><Spinner size={20} /></div>
            )}
          </div>
          <p className="soft motion-legend">
            <span className="motion-dot bunch" /> {t("particles of the example beam")}
            <span className="motion-dot accent" />{" "}
            {result?.marks?.[0]?.[0] === "z"
              ? t("two particles to follow: they start 1.5 rms ahead of and behind the bunch centre")
              : t("two particles to follow: they start 1.5 rms off axis in x and in y")}
            {mode === "2d" && result && (result.centre ?? result.ghosts.length > 0) && <> · {t("solid line: beam centre")}</>}
            {mode === "2d" && result && result.ghosts.length > 0 && (
              <> · {t("dashed: {list}", { list: result.ghosts.filter((g, i, all) => all.findIndex((o) => pick(o.name) === pick(g.name)) === i).map((g) => pick(g.name)).join(t(", ")) })}</>
            )}
            {mode === "3d" && <> · {t("3D close-up to scale: the element itself, the field where the particles are and the force on the two highlighted particles")}</>}
          </p>
          {result && (
            <>
              <ResultValues values={result.values} />
              {result.warnings.map((w, i) => (
                <div key={i} className="learn-warning">
                  <Icon name="warning" /> {pick(w)}
                </div>
              ))}
              {result.plots.length > 0 && (
                <div className="learn-plots">
                  {result.plots.map((p) => (
                    <LabPlotView key={p.id} plot={p} />
                  ))}
                </div>
              )}
              <p className="learn-model">
                <Icon name="info" /> {pick(result.model)}
              </p>
              <details className="learn-lattice">
                <summary>{t("AVAS lattice of this animation")}</summary>
                <pre className="mono selectable">{result.lattice}</pre>
              </details>
            </>
          )}
        </div>
      </div>
    </section>
  );
}
