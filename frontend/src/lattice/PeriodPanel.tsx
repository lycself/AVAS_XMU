// Period analysis of the lattice in the editor (avas/gui/services/analysis.py): Floquet stability,
// phase advances with and without space charge, the matched beam and the envelope stability of a
// "lattice … lattice_end" period, and the phase advance of every period along the line.
import { useEffect, useMemo, useState } from "react";
import { call } from "../bridge";
import { Plot, LEGEND_TOP } from "../components/Plot";
import { toast } from "../components/overlays";
import { Badge, Button, CommitInput, Field, Icon, Select, Spinner } from "../components/ui";
import { fmtG } from "../format";
import { useT } from "../i18n";
import { setPage, useApp, useInputsLocked } from "../store/app";

type PlaneInfo = { mu: number | null; alpha: number | null; beta: number | null; stable: boolean };
type Analysis = {
  z0: number; z1: number; energy: number; current: number; stable: boolean; coupling: number; eigenTunes: (number | null)[]; maxModulus: number;
  zero: Record<string, PlaneInfo>; depressed?: Record<string, PlaneInfo>; planes?: string; converged?: boolean;
  tuneDepression?: Record<string, number | null>; mismatch?: Record<string, number | null>;
  matched?: Record<string, { alpha: number | null; beta: number | null; emit_n: number | null }>;
  envelope?: { maxModulus: number; stable: boolean }; messages: string[];
};
type Period = { index: number; title: string; line: number; z0: number; z1: number };
type Table = { count: number; z0: Float64Array; mu0_x: Float64Array; mu0_y: Float64Array; mu0_z: Float64Array; mu_x: Float64Array; mu_y: Float64Array; mu_z: Float64Array };

/** Event the Beam page listens to: fill its (unsaved) Twiss fields with the matched beam. */
export const BEAM_TWISS_EVENT = "avas:beam-twiss";

export function PeriodPanel({ getText, fieldDirs, revision }: { getText: () => string; fieldDirs?: string[] | null; revision: unknown }) {
  const t = useT();
  const locked = useInputsLocked();
  const project = useApp((s) => s.project);
  const [periods, setPeriods] = useState<Period[]>([]);
  const [period, setPeriod] = useState(1);
  const [current, setCurrent] = useState<number | null>(null);
  const [result, setResult] = useState<Analysis | null>(null);
  const [table, setTable] = useState<Table | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);

  // the editor's text, re-read when it changes (debounced)
  useEffect(() => {
    if (!project.open) return;
    const h = setTimeout(async () => {
      const text = getText();
      try {
        const list = await call<Period[]>("analysis.periods", { text, fieldDirs: fieldDirs ?? undefined });
        setPeriods(list);
        if (!list.length) {
          setResult(null);
          setTable(null);
          return;
        }
        const p = list.some((x) => x.index === period) ? period : list[0].index;
        setPeriod(p);
        setBusy(true);
        const [res, tab] = await Promise.all([
          call<Analysis>("analysis.period", { text, period: p, current: current ?? undefined, fieldDirs: fieldDirs ?? undefined }),
          call<Table>("analysis.periodTable", { text, fieldDirs: fieldDirs ?? undefined }),
        ]);
        setResult(res);
        setTable(tab);
        setError(null);
      } catch (e: any) {
        setError(e?.message ?? String(e));
      } finally {
        setBusy(false);
      }
    }, 400);
    return () => clearTimeout(h);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [project.open, revision, period, current]);

  const figure = useMemo(() => {
    if (!table || !table.count) return null;
    const z = Array.from(table.z0);
    const line = (a: Float64Array, name: string, dash?: "dot") => ({ type: "scatter", mode: "lines+markers", x: z, y: Array.from(a, (v) => (Number.isFinite(v) ? v : null)), name, line: dash ? { dash } : undefined });
    return {
      data: [line(table.mu0_x, "σ0 x"), line(table.mu0_y, "σ0 y"), line(table.mu0_z, "σ0 z"), line(table.mu_x, "σ x", "dot"), line(table.mu_y, "σ y", "dot"), line(table.mu_z, "σ z", "dot")],
      layout: { margin: { t: 12 }, xaxis: { title: { text: t("period entry z (m)") } }, yaxis: { title: { text: t("phase advance per period (deg)") } }, legend: LEGEND_TOP },
    };
  }, [table, t]);

  if (!periods.length)
    return (
      <div className="period-panel">
        <p className="muted">{t("No period in this lattice. Mark one with 'lattice n 1' before its first element and 'lattice_end' after its last; the analysis then shows its phase advances, matched beam and stability.")}</p>
      </div>
    );

  const fillBeam = () => {
    if (!result?.matched) return;
    window.dispatchEvent(new CustomEvent(BEAM_TWISS_EVENT, { detail: result.matched }));
    setPage("beam");
    toast(t("Matched α and β filled in on the Beam page; save there to keep them."), "info", 5000);
  };
  const planes = ["x", "y", "z"].filter((p) => result?.zero[p]);
  const rows: { label: string; value: (p: string) => string }[] = result
    ? [
        { label: t("σ0, zero current (deg)"), value: (p) => fmtG(result.zero[p]?.mu, 4) },
        { label: t("σ, with space charge (deg)"), value: (p) => fmtG(result.depressed?.[p]?.mu, 4) },
        { label: t("σ / σ0"), value: (p) => fmtG(result.tuneDepression?.[p], 3) },
        { label: t("matched α"), value: (p) => fmtG(result.matched?.[p]?.alpha, 4) },
        { label: t("matched β (mm/mrad)"), value: (p) => fmtG(result.matched?.[p]?.beta, 4) },
        { label: t("εn (π·mm·mrad)"), value: (p) => fmtG(result.matched?.[p]?.emit_n, 4) },
        { label: t("mismatch of the beam"), value: (p) => fmtG(result.mismatch?.[p], 3) },
      ]
    : [];

  return (
    <div className="period-panel">
      <div className="row" style={{ gap: 12, flexWrap: "wrap", alignItems: "flex-end" }}>
        <Field label={t("Period")}>
          <Select value={period} options={periods.map((p) => ({ value: p.index, label: `${p.index}  ·  ${t("line {n}", { n: p.line + 1 })}  ·  z ${p.z0.toFixed(3)} – ${p.z1.toFixed(3)} m` }))} onChange={(v) => setPeriod(Number(v))} />
        </Field>
        <Field label={t("Current (mA)")} hint={t("Empty: the current of beam.txt.")}>
          <CommitInput
            value={current == null ? "" : String(current)}
            placeholder={result ? fmtG(result.current, 4) : ""}
            validate={(v) => !v.trim() || Number.isFinite(Number(v))}
            onCommit={(v) => setCurrent(v.trim() ? Number(v) : null)}
          />
        </Field>
        {busy && <Spinner />}
      </div>
      {error && <p className="warning-text">{error}</p>}
      {result && (
        <>
          <div className="row" style={{ gap: 8, flexWrap: "wrap", margin: "8px 0" }}>
            <Badge tone={result.stable ? "success" : "danger"}>{result.stable ? t("motion stable") : t("motion unstable")}</Badge>
            {result.envelope && <Badge tone={result.envelope.stable ? "success" : "danger"}>{result.envelope.stable ? t("envelope stable") : t("envelope unstable")}</Badge>}
            {result.coupling > 0.01 && <Badge tone="warning">{t("x–y coupled")}</Badge>}
            <span className="muted">
              {t("z {a} – {b} m, {w} MeV, {i} mA", { a: result.z0.toFixed(3), b: result.z1.toFixed(3), w: fmtG(result.energy, 5), i: fmtG(result.current, 4) })}
            </span>
          </div>
          <table className="mini-table period-table">
            <thead>
              <tr>
                <th />
                {planes.map((p) => (
                  <th key={p}>{p}</th>
                ))}
              </tr>
            </thead>
            <tbody>
              {rows.map((r) => (
                <tr key={r.label}>
                  <td className="muted">{r.label}</td>
                  {planes.map((p) => (
                    <td key={p}>{r.value(p)}</td>
                  ))}
                </tr>
              ))}
            </tbody>
          </table>
          {result.coupling > 0.01 && <p className="hint">{t("With coupling the per-plane phase advances mix x and y; the eigen-tunes are {v} deg.", { v: result.eigenTunes.map((v) => fmtG(v, 4)).join(", ") })}</p>}
          {result.envelope && <p className="hint">{t("Largest |eigenvalue| of the linearised envelope map: {m}.", { m: fmtG(result.envelope.maxModulus, 5) })}</p>}
          {result.messages.map((m, k) => (
            <p key={k} className="warning-text">
              <Icon name="warning" /> {t(m)}
            </p>
          ))}
          {result.matched && (
            <Button small icon="arrow-right" disabled={locked} onClick={fillBeam} tip={locked ? t("Inputs are read-only while a simulation runs.") : undefined}>
              {t("Use the matched α, β on the Beam page")}
            </Button>
          )}
          <p className="hint">{t("Linear model with the project's beam; accelerating periods are normalised by βγ. The engine is the reference.")}</p>
        </>
      )}
      {figure && (
        <div style={{ height: 220, position: "relative", marginTop: 8 }}>
          <Plot data={figure.data as any} layout={figure.layout as any} style={{ position: "absolute", inset: 0 }} />
        </div>
      )}
    </div>
  );
}
