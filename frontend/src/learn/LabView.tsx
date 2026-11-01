// One interactive lab of the primer.  The page only draws: the parameters, their
// ranges and every number come from the back end (avas/learn/labs.py, the linear
// preview's own code), recomputed while a slider moves.
import { useEffect, useMemo, useState } from "react";
import { call } from "../bridge";
import { Plot } from "../components/Plot";
import { Button, Icon, NumberInput, Spinner } from "../components/ui";
import { fmtG } from "../format";
import { pick, useLang, useT } from "../i18n";
import type { Bi } from "../lattice/types";
import { BeamlineView, type BeamlinePreview } from "./BeamlineView";
import { askTutor } from "./tutor";
import { useLatest } from "./useLatest";

export { useLatest };

export type LabParam = { key: string; label: Bi; unit: string; default: number; min: number; max: number; step: number; log: boolean; integer: boolean; doc: Bi };
export type LabDef = { id: string; title: Bi; summary: Bi; params: LabParam[] };
/** marker: one highlighted point; cloud: many small points (a particle distribution) */
export type LabSeries = { name: Bi; x: ArrayLike<number>; y: ArrayLike<number>; dash?: boolean; marker?: boolean; cloud?: boolean };
export type LabAxis = { label: Bi; unit: string; log?: boolean };
export type LabPlot = { id: string; title: Bi; x: LabAxis; y: LabAxis; equal?: boolean; series: LabSeries[] };
export type LabValue = { key: string; label: Bi; unit: string; value: number | null };
export type LabResult = {
  values: LabValue[];
  plots: LabPlot[];
  matrices: { title: Bi; labels: string[]; rows: number[][] }[];
  lattice: string | null;
  /** linear preview of *lattice* for the 2D / 3D view */
  beamline: (BeamlinePreview & { fieldDirs: string[] }) | null;
  warnings: Bi[];
  model: Bi;
  params: Record<string, number>;
};

let defs: Promise<Map<string, LabDef>> | null = null;
export function labDefinitions(): Promise<Map<string, LabDef>> {
  defs ??= call<LabDef[]>("learn.labs").then((list) => new Map(list.map((d) => [d.id, d])));
  defs.catch(() => (defs = null));
  return defs;
}

/** Calls *method* for the latest *params*, one request at a time (sliders send many). */
const SLIDER = 1000;
function toSlider(p: LabParam, v: number) {
  if (p.log && p.min > 0) return (Math.log(v / p.min) / Math.log(p.max / p.min)) * SLIDER;
  return ((v - p.min) / (p.max - p.min)) * SLIDER;
}
function fromSlider(p: LabParam, s: number) {
  let v = p.log && p.min > 0 ? p.min * Math.pow(p.max / p.min, s / SLIDER) : p.min + ((p.max - p.min) * s) / SLIDER;
  if (p.integer) return Math.round(v);
  const digits = Math.max(0, -Math.floor(Math.log10(p.step || 1)));
  v = Number(v.toFixed(Math.min(digits + (p.log ? 2 : 0), 8)));
  return v;
}

export function ParamControl({ p, value, onChange }: { p: LabParam; value: number; onChange: (v: number) => void }) {
  const unit = p.unit ? ` (${p.unit})` : "";
  return (
    <label className="learn-param" data-tip={pick(p.doc) || undefined}>
      <span className="learn-param-label">
        {pick(p.label)}
        <span className="soft">{unit}</span>
      </span>
      <span className="learn-param-row">
        <input
          type="range"
          min={0}
          max={SLIDER}
          step={p.integer ? SLIDER / Math.max(1, p.max - p.min) : 1}
          value={toSlider(p, value)}
          onChange={(e) => onChange(fromSlider(p, Number(e.target.value)))}
          aria-label={pick(p.label)}
        />
        <NumberInput value={value} min={p.min} max={p.max} integer={p.integer} onCommit={onChange} />
      </span>
    </label>
  );
}

const axis = (a: LabAxis) => ({ title: { text: `${pick(a.label)}${a.unit ? ` (${a.unit})` : ""}` }, ...(a.log ? { type: "log" as const } : {}) });
// the article scrolls under the mouse wheel, so plots zoom with the tool bar / drag only
const PLOT_CONFIG = { displaylogo: false, responsive: true, scrollZoom: false };

export function LabPlotView({ plot }: { plot: LabPlot }) {
  const lang = useLang((s) => s.lang);
  // Plotly trips over keys that are present but undefined, so unset styles are left out
  const data = useMemo(
    () =>
      plot.series.map((s) => ({
        type: "scatter" as const,
        mode: s.marker || s.cloud ? ("markers" as const) : ("lines" as const),
        name: pick(s.name),
        x: Array.from(s.x),
        y: Array.from(s.y),
        ...(s.cloud
          ? { marker: { size: 3, opacity: 0.45 } }
          : s.marker
            ? { marker: { size: 10, symbol: "diamond" as const } }
            : { line: s.dash ? { dash: "dash" as const, width: 2 } : { width: 2 } }),
      })),
    [plot, lang],
  );
  const layout = useMemo(
    () => ({
      xaxis: axis(plot.x),
      yaxis: { ...axis(plot.y), ...(plot.equal ? { scaleanchor: "x" as const, scaleratio: 1 } : {}) },
      showlegend: plot.series.length > 1,
      legend: { orientation: "h" as const, y: 1.14, x: 0 },
      margin: { l: 60, r: 16, t: 30, b: 46 },
    }),
    [plot, lang],
  );
  return (
    <figure className="learn-plot">
      <figcaption>{pick(plot.title)}</figcaption>
      <Plot data={data} style={{ height: 250 }} layout={layout} config={PLOT_CONFIG} />
    </figure>
  );
}

export function ResultValues({ values }: { values: LabValue[] }) {
  return (
    <dl className="learn-values">
      {values.map((v) => (
        <div key={v.key}>
          <dt>{pick(v.label)}</dt>
          <dd className="mono">
            {v.value == null ? "—" : fmtG(v.value, 5)}
            {v.unit && v.value != null && <span className="soft"> {v.unit}</span>}
          </dd>
        </div>
      ))}
    </dl>
  );
}

function MatrixView({ m }: { m: LabResult["matrices"][number] }) {
  return (
    <figure className="learn-matrix">
      <figcaption>{pick(m.title)}</figcaption>
      <table className="mono">
        <thead>
          <tr>
            <th />
            {m.labels.map((l) => (
              <th key={l}>{l}</th>
            ))}
          </tr>
        </thead>
        <tbody>
          {m.rows.map((r, i) => (
            <tr key={i}>
              <th>{m.labels[i]}</th>
              {r.map((v, j) => (
                <td key={j}>{fmtG(Math.abs(v) < 1e-12 ? 0 : v, 4)}</td>
              ))}
            </tr>
          ))}
        </tbody>
      </table>
    </figure>
  );
}

export function LabView({ id, presets }: { id: string; presets: Record<string, string> }) {
  const t = useT();
  const [def, setDef] = useState<LabDef | null>(null);
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
    labDefinitions()
      .then((m) => {
        if (!live) return;
        const d = m.get(id);
        if (d) setDef(d);
        else setFailed(t("Unknown lab: {id}", { id }));
      })
      .catch((e) => live && setFailed(String(e?.message ?? e)));
    return () => {
      live = false;
    };
  }, [id, t]);
  useEffect(() => setValues(initial), [initial]);
  const { result, error, busy } = useLatest<LabResult>("learn.lab", values && def ? { id, params: values } : null);

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
    askTutor(t("In the primer lab \"{lab}\" I set: {params}. The lab reports: {values}. Explain the physics behind these numbers and what I should notice.", { lab: pick(def.title), params, values: out }));
  };
  return (
    <section className="learn-lab" aria-label={pick(def.title)}>
      <header className="learn-lab-head">
        <Icon name="beaker" />
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
          {result && (
            <>
              <ResultValues values={result.values} />
              {result.warnings.map((w, i) => (
                <div key={i} className="learn-warning">
                  <Icon name="warning" /> {pick(w)}
                </div>
              ))}
              {result.lattice && result.beamline && (
                <BeamlineView text={result.lattice} fieldDirs={result.beamline.fieldDirs} preview={result.beamline} />
              )}
              <div className="learn-plots">
                {result.plots.map((p) => (
                  <LabPlotView key={p.id} plot={p} />
                ))}
              </div>
              {result.matrices.map((m, i) => (
                <MatrixView key={i} m={m} />
              ))}
              <p className="learn-model">
                <Icon name="info" /> {pick(result.model)}
              </p>
              {result.lattice && (
                <details className="learn-lattice">
                  <summary>{t("AVAS lattice evaluated by this lab")}</summary>
                  <pre className="mono selectable">{result.lattice}</pre>
                </details>
              )}
            </>
          )}
        </div>
      </div>
    </section>
  );
}
