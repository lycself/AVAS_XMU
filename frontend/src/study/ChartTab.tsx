// Study page, stability chart: phase advances and envelope stability of one period over two axes
// (avas/gui/services/analysis.py, linear model).
import { useEffect, useMemo, useState } from "react";
import { call, on } from "../bridge";
import { Plot, fieldScale } from "../components/Plot";
import { reportError, toast } from "../components/overlays";
import { Button, Empty, Field, ProgressBar, Radio, Section, Select } from "../components/ui";
import { pick, useT } from "../i18n";
import { useApp } from "../store/app";
import { type Schema, type Statement, ValuesField, elementLabel, useProjectLattice } from "./common";

type ChartAxis = { kind: "lattice" | "current"; target?: string; param?: string };
type Period = { index: number; title: string; line: number; z0: number; z1: number };
type Grid = Record<"sigma0" | "sigma" | "envelope" | "stable" | "converged", (number | null)[][]>;
type Result = { x: { label: string; unit: string; values: number[] }; y: { label: string; unit: string; values: number[] }; period: number; done: number; total: number; status: string; error?: string; grid: Grid };
type Quantity = "sigma0" | "sigma" | "ratio" | "envelope" | "stable";

const QUANTITIES: { value: Quantity; label: string }[] = [
  { value: "sigma0", label: "σ0, zero-current phase advance x (deg)" },
  { value: "sigma", label: "σ, depressed phase advance x (deg)" },
  { value: "ratio", label: "σ / σ0, tune depression x" },
  { value: "envelope", label: "largest |eigenvalue| of the envelope map" },
  { value: "stable", label: "envelope stable (1) or unstable (0)" },
];

function AxisChoice({ axis, onChange, statements, schema, name }: { axis: ChartAxis; onChange: (a: ChartAxis) => void; statements: Statement[]; schema: Schema | null; name: string }) {
  const t = useT();
  const elements = statements.filter((s) => s.isElement && s.known && s.active);
  const chosen = elements.find((s) => String(s.line + 1) === axis.target);
  const params = (chosen && schema?.lattice.find((k) => k.key === chosen.key)?.params.filter((p) => p.kind !== "reserved")) ?? [];
  return (
    <>
      <div className="row" style={{ gap: 16, marginBottom: 8 }}>
        <Radio checked={axis.kind === "lattice"} onChange={() => onChange({ kind: "lattice" })} label={t("Lattice element parameter")} name={name} />
        <Radio checked={axis.kind === "current"} onChange={() => onChange({ kind: "current" })} label={t("Beam current (mA)")} name={name} />
      </div>
      {axis.kind === "lattice" && (
        <div className="form-grid">
          <Field label={t("Element")}>
            <Select value={axis.target ?? ""} options={[{ value: "", label: t("(choose an element)") }, ...elements.map((s) => ({ value: String(s.line + 1), label: elementLabel(s, t) }))]} onChange={(v) => onChange({ kind: "lattice", target: v, param: "" })} />
          </Field>
          <Field label={t("Parameter")}>
            <Select
              value={axis.param ?? ""}
              options={[{ value: "", label: t("(choose a parameter)") }, ...params.map((p) => ({ value: p.key, label: `${p.key}  ·  ${pick(p.label)}${p.unit ? ` (${p.unit})` : ""}` }))]}
              onChange={(v) => onChange({ ...axis, param: v })}
              disabled={!chosen}
            />
          </Field>
        </div>
      )}
    </>
  );
}

export function ChartTab() {
  const t = useT();
  const project = useApp((s) => s.project);
  const dark = useApp((s) => s.resolvedTheme === "dark");
  const { statements, text, schema } = useProjectLattice();
  const [periods, setPeriods] = useState<Period[]>([]);
  const [period, setPeriod] = useState(1);
  const [x, setX] = useState<ChartAxis>({ kind: "lattice" });
  const [y, setY] = useState<ChartAxis>({ kind: "current" });
  const [xs, setXs] = useState("");
  const [ys, setYs] = useState("0:20:11");
  const [result, setResult] = useState<Result | null>(null);
  const [running, setRunning] = useState(false);
  const [quantity, setQuantity] = useState<Quantity>("sigma");

  useEffect(() => {
    if (!text) return;
    call<Period[]>("analysis.periods", { text }).then(setPeriods).catch(() => setPeriods([]));
  }, [text]);
  useEffect(() => {
    if (!project.open) return;
    call<{ running: boolean; result: Result | null }>("analysis.chartState").then((s) => {
      setRunning(s.running);
      if (s.result) setResult(s.result);
    }).catch(() => undefined);
    const offs = [
      on("analysis.chart", (r: Result) => setResult(r)),
      on("analysis.chartFinished", (r: Result) => {
        setResult(r);
        setRunning(false);
        if (r.error) toast(r.error, "error", 6000);
      }),
    ];
    return () => offs.forEach((f) => f());
  }, [project.open, project.path]);

  const axisOk = (a: ChartAxis) => a.kind === "current" || (!!a.target && !!a.param);
  const canStart = periods.length > 0 && axisOk(x) && axisOk(y) && !!xs.trim() && !!ys.trim() && !running;
  const start = async () => {
    try {
      setResult(await call<Result>("analysis.chartStart", { period, x, xs: xs.trim(), y, ys: ys.trim() }));
      setRunning(true);
    } catch (e) {
      reportError(e, t("Stability chart"));
    }
  };

  const figure = useMemo(() => {
    if (!result) return null;
    const g = result.grid;
    const z = g.sigma.map((row, j) =>
      row.map((s, i) => {
        if (quantity === "ratio") return s != null && g.sigma0[j][i] ? s / (g.sigma0[j][i] as number) : null;
        return g[quantity][j][i];
      }),
    );
    const label = t(QUANTITIES.find((q) => q.value === quantity)!.label);
    return {
      data: [{ type: "heatmap", x: result.x.values, y: result.y.values, z, colorscale: fieldScale(dark, false), colorbar: { title: { text: "" } }, hoverongaps: false, name: label }],
      layout: {
        xaxis: { title: { text: `${result.x.label}${result.x.unit ? ` (${result.x.unit})` : ""}` } },
        yaxis: { title: { text: `${result.y.label}${result.y.unit ? ` (${result.y.unit})` : ""}` } },
        margin: { t: 16 },
      },
    };
  }, [result, quantity, dark, t]);

  return (
    <>
      <Section title={t("Period and axes")} icon="symbol-color">
        {periods.length === 0 ? (
          <p className="warning-text">{t("The run lattice has no period: mark one with 'lattice n 1' … 'lattice_end' on the Lattice page.")}</p>
        ) : (
          <div className="form-grid">
            <Field label={t("Period")}>
              <Select value={period} options={periods.map((p) => ({ value: p.index, label: `${p.index}  ·  z ${p.z0.toFixed(3)} – ${p.z1.toFixed(3)} m` }))} onChange={(v) => setPeriod(Number(v))} />
            </Field>
          </div>
        )}
        <h4>{t("Horizontal axis")}</h4>
        <AxisChoice axis={x} onChange={setX} statements={statements} schema={schema} name="chart-x" />
        <div className="form-grid" style={{ marginTop: 8 }}>
          <ValuesField value={xs} onCommit={setXs} />
        </div>
        <h4>{t("Vertical axis")}</h4>
        <AxisChoice axis={y} onChange={setY} statements={statements} schema={schema} name="chart-y" />
        <div className="form-grid" style={{ marginTop: 8 }}>
          <ValuesField value={ys} onCommit={setYs} />
        </div>
        <p className="muted">
          {t("For every point: the period's matched beam with space charge, its phase advances and the stability of the linearised envelope map (linear model). The envelope is unstable where an eigenvalue leaves the unit circle, e.g. in the 90° stop band.")}
        </p>
      </Section>
      <div className="row" style={{ gap: 8, margin: "4px 0 12px", alignItems: "center" }}>
        {running ? (
          <Button icon="debug-stop" onClick={() => call("analysis.chartStop").catch(reportError)}>
            {t("Stop")}
          </Button>
        ) : (
          <Button variant="primary" icon="play" disabled={!canStart} onClick={start}>
            {t("Compute the chart")}
          </Button>
        )}
        {result && running && (
          <div style={{ width: 240 }}>
            <ProgressBar value={result.done / Math.max(result.total, 1)} />
          </div>
        )}
        {result && <span className="muted">{t("{done} / {count} points", { done: result.done, count: result.total })}</span>}
      </div>
      {!result ? (
        <Empty icon="symbol-color" title={t("No chart yet")} />
      ) : (
        <Section title={t("Stability chart of period {n}", { n: result.period })} icon="symbol-color" actions={<Select value={quantity} options={QUANTITIES.map((q) => ({ value: q.value, label: t(q.label) }))} onChange={setQuantity} />}>
          <div style={{ height: 380, position: "relative" }}>{figure && <Plot data={figure.data as any} layout={figure.layout as any} style={{ position: "absolute", inset: 0 }} />}</div>
          <p className="hint">{t("Empty cells: no matched beam (the period is unstable or space charge is too strong for a matched solution).")}</p>
        </Section>
      )}
    </>
  );
}
