// Study page, scan tab: one or two axes, linear model or engine (avas/gui/services/study.py).
// Engine scans are research runs in Research/gui/ (notebooks see them); old Scans/ results stay readable.
import { useCallback, useEffect, useMemo, useState } from "react";
import { call, on } from "../bridge";
import { openPath } from "../host";
import { Plot, fieldScale } from "../components/Plot";
import { confirmDialog, reportError, toast } from "../components/overlays";
import { Badge, Button, Checkbox, CommitInput, Empty, Field, IconButton, Section, Segmented, Select, Spinner, cx } from "../components/ui";
import { fmtG, fmtSeconds, runStatusLabel } from "../format";
import { useT } from "../i18n";
import { setPage, showResults, useApp } from "../store/app";
import { AxisPicker, type AxisSpec, METRIC_LABEL, ValuesField, axisComplete, useProjectLattice } from "./common";

type Target = { kind: string; file: string; param: string; label: string; unit: string; original: string | null };
type Row = { index: number; values: number[]; seconds?: number; folder?: string; output_dir?: string; error?: string; [metric: string]: unknown };
export type Scan = { label: string; created: string; finished?: string; status: string; path: string; fidelity: "linear" | "engine"; axes: Target[]; values: number[][]; metrics: string[]; rows: Row[]; shape: number[]; legacy?: boolean };
type ListItem = { path: string; label: string; created: string; finished?: string; status: string; fidelity: string; axes: Target[]; count: number; done: number; legacy: boolean };
type Check = { axes: Target[]; counts: number[]; count: number; estimate_s: number | null };
type Point = { fidelity: string; label: string; z: Float32Array; rms_x: Float32Array; rms_y: Float32Array; cx: Float32Array; cy: Float32Array; energy: Float32Array };

/** An old Scans/ result (scan.json of avas/sim/scan.py) in the new shape. */
function fromLegacy(r: any): Scan {
  return {
    label: r.label, created: r.created, finished: r.finished, status: r.status, path: r.folder, fidelity: "engine", legacy: true,
    axes: [r.target], values: [r.values], metrics: r.metrics, shape: [r.values.length],
    rows: (r.rows ?? []).map((row: any) => ({ ...row, values: [row.value] })),
  };
}

export function ScanTab() {
  const t = useT();
  const project = useApp((s) => s.project);
  const run = useApp((s) => s.run);
  const { statements, schema, reload } = useProjectLattice();
  const [fidelity, setFidelity] = useState<"linear" | "engine">("linear");
  const [axes, setAxes] = useState<AxisSpec[]>([{ kind: "lattice" }, { kind: "beam" }]);
  const [values, setValues] = useState<string[]>(["", ""]);
  const [two, setTwo] = useState(false);
  const [label, setLabel] = useState("");
  const [metricsAll, setMetricsAll] = useState<{ default: string[]; all: string[]; linear: string[] }>({ default: [], all: [], linear: [] });
  const [metrics, setMetrics] = useState<string[]>([]);
  const [check, setCheck] = useState<Check | null>(null);
  const [checkError, setCheckError] = useState<string | null>(null);
  const [state, setState] = useState<{ running: boolean; scan: Scan | null }>({ running: false, scan: null });
  const [list, setList] = useState<ListItem[]>([]);
  const [shown, setShown] = useState<Scan | null>(null);

  const used = two ? axes : axes.slice(0, 1);
  const usedValues = two ? values : values.slice(0, 1);
  const complete = used.every(axisComplete);

  useEffect(() => {
    if (!project.open) return;
    call<typeof metricsAll>("study.metrics")
      .then((m) => {
        setMetricsAll(m);
        setMetrics((cur) => (cur.length ? cur : m.default));
      })
      .catch(reportError);
  }, [project.open, project.path]);

  const loadList = useCallback(async () => {
    try {
      setList(await call<ListItem[]>("study.list"));
    } catch {
      setList([]);
    }
  }, []);
  const refreshState = useCallback(async () => {
    try {
      const s = await call<{ running: boolean; scan: Scan | null }>("study.state");
      setState(s);
      if (s.scan) setShown(s.scan);
    } catch {
      /* the next call reports it */
    }
  }, []);
  useEffect(() => {
    if (!project.open) return;
    loadList();
    refreshState();
    const offs = [
      on("study.progress", (s: Scan) => {
        setState({ running: true, scan: s });
        setShown(s);
      }),
      on("study.finished", (info: { error?: string | null; status?: string }) => {
        refreshState();
        loadList();
        if (info.error) toast(info.error, "error", 6000);
        else toast(info.status === "stopped" ? t("Scan stopped") : t("Scan finished"), info.status === "stopped" ? "info" : "success");
      }),
      on("bridge.reconnected", refreshState),
    ];
    return () => offs.forEach((f) => f());
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [project.open, project.path]);

  useEffect(() => {
    if (!project.open || !complete) {
      setCheck(null);
      setCheckError(null);
      return;
    }
    const h = setTimeout(async () => {
      try {
        const vals = usedValues.every((v) => v.trim()) ? usedValues.map((v) => v.trim()) : undefined;
        setCheck(await call<Check>("study.check", { axes: used, values: vals, fidelity }));
        setCheckError(null);
      } catch (e: any) {
        setCheck(null);
        setCheckError(e?.message ?? String(e));
      }
    }, 250);
    return () => clearTimeout(h);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [project.open, complete, JSON.stringify(used), JSON.stringify(usedValues), fidelity]);

  const busy = state.running || (fidelity === "engine" && run.running);
  const canStart = complete && usedValues.every((v) => v.trim()) && !!check && check.count > 0 && !busy;
  const available = fidelity === "linear" ? metricsAll.linear : metricsAll.all;

  const start = async () => {
    if (!canStart || !check) return;
    if (fidelity === "engine" && check.count > 20) {
      const ok = await confirmDialog(
        t("Run {n} simulations{est}?", { n: check.count, est: check.estimate_s ? ` (${t("about {time}", { time: fmtSeconds(check.estimate_s) })})` : "" }),
        { title: t("Parameter scan"), ok: t("Start") },
      );
      if (!ok) return;
    }
    try {
      const s = await call<{ running: boolean; scan: Scan }>("study.start", {
        axes: used, values: usedValues.map((v) => v.trim()), fidelity, label,
        metrics: metrics.filter((m) => available.includes(m)),
      });
      setState(s);
      setShown(s.scan);
    } catch (e) {
      reportError(e, t("Parameter scan"));
    }
  };

  const open = async (item: ListItem) => {
    try {
      setShown(item.legacy ? fromLegacy(await call<any>("scan.load", { folder: item.path })) : await call<Scan>("study.load", { path: item.path }));
    } catch (e) {
      reportError(e);
    }
  };
  const remove = async (item: ListItem) => {
    const ok = await confirmDialog(
      item.legacy ? t("Move scan {label} and all its files to the recycle bin?", { label: item.label }) : t("Move the description of scan {label} to the recycle bin? Its engine runs stay in the study's cache.", { label: item.label }),
      { title: t("Delete"), ok: t("Move to recycle bin"), danger: true },
    );
    if (!ok) return;
    try {
      await call(item.legacy ? "scan.delete" : "study.delete", item.legacy ? { folder: item.path } : { path: item.path });
      if (shown?.path === item.path) setShown(null);
      loadList();
    } catch (e) {
      reportError(e);
    }
  };

  const setAxis = (k: number, s: AxisSpec) => setAxes((cur) => cur.map((a, i) => (i === k ? s : a)));
  const setValue = (k: number, v: string) => setValues((cur) => cur.map((a, i) => (i === k ? v : a)));

  return (
    <>
      <Section title={t("How to compute")} icon="settings">
        <Segmented
          value={fidelity}
          onChange={setFidelity}
          options={[
            { value: "linear", label: t("Linear model"), icon: "zap", tip: t("Seconds for hundreds of points; envelope and centroid of the linear model, no losses or emittance growth.") },
            { value: "engine", label: t("Engine"), icon: "server-process", tip: t("One multi-particle simulation per point, one at a time; shown on the Run page and kept in Research/gui/.") },
          ]}
        />
        <p className="muted" style={{ marginTop: 8 }}>
          {fidelity === "linear"
            ? t("The linear model gives a quick map; check the interesting points with the engine.")
            : t("Each point runs on a copy of the inputs with a fixed random seed; a point already computed with the same inputs is taken from the cache.")}
        </p>
      </Section>

      {used.map((spec, k) => (
        <Section
          key={k}
          title={two ? t("Axis {n}", { n: k + 1 }) : t("What to scan")}
          icon={k === 0 ? "arrow-right" : "arrow-up"}
          actions={k === 1 ? <IconButton icon="close" tip={t("Remove the second axis")} onClick={() => setTwo(false)} /> : undefined}
        >
          <AxisPicker spec={spec} onChange={(s) => setAxis(k, s)} statements={statements} schema={schema} onReload={reload} name={`study-axis-${k}`} />
          <div className="form-grid" style={{ marginTop: 8 }}>
            <ValuesField value={values[k]} onCommit={(v) => setValue(k, v)} unit={check?.axes[k]?.unit} original={check?.axes[k]?.original} />
          </div>
        </Section>
      ))}
      {!two && (
        <Button small icon="add" onClick={() => setTwo(true)} style={{ marginBottom: 12 }}>
          {t("Add a second axis")}
        </Button>
      )}

      <Section title={t("Results to collect")} icon="list-unordered">
        <div className="row scan-metrics">
          {available.map((m) => (
            <Checkbox key={m} checked={metrics.includes(m)} onChange={(v) => setMetrics((cur) => (v ? [...cur, m] : cur.filter((x) => x !== m)))} label={t(METRIC_LABEL[m] ?? m)} />
          ))}
        </div>
        <div className="form-grid" style={{ marginTop: 8 }}>
          <Field label={t("Name")} hint={t("Name of the scan (optional).")}>
            <CommitInput value={label} onCommit={setLabel} placeholder={check?.axes.map((a) => a.param).join("_") ?? ""} />
          </Field>
        </div>
        {checkError && <div className="warning-text" style={{ marginTop: 8 }}>{checkError}</div>}
        {check && check.count > 0 && (
          <div className="muted" style={{ marginTop: 8 }}>
            {t("{n} points", { n: check.count })}
            {check.estimate_s ? `, ${t("about {time}", { time: fmtSeconds(check.estimate_s) })}` : ""}
          </div>
        )}
      </Section>

      <div className="row" style={{ gap: 8, margin: "4px 0 12px" }}>
        {state.running ? (
          <Button icon="debug-stop" onClick={() => call("study.stop").catch(reportError)}>
            {t("Stop scan")}
          </Button>
        ) : (
          <Button variant="primary" icon="play" disabled={!canStart} onClick={start} tip={busy ? t("A simulation is already running; wait for it to finish.") : undefined}>
            {t("Start scan")}
          </Button>
        )}
        {state.running && state.scan && (
          <>
            <Spinner />
            <span className="muted">{t("point {i} of {n}", { i: state.scan.rows.length, n: state.scan.shape.reduce((a, b) => a * b, 1) })}</span>
            {state.scan.fidelity === "engine" && (
              <Button small variant="ghost" icon="pulse" onClick={() => setPage("run")}>
                {t("Show progress")}
              </Button>
            )}
          </>
        )}
      </div>

      {shown && <ScanResult scan={shown} live={state.running && state.scan?.path === shown.path} />}

      <Section title={t("Scans of this project")} icon="history" actions={<IconButton icon="refresh" tip={t("Refresh")} onClick={loadList} />}>
        {list.length === 0 ? (
          <div className="muted">{t("no scans yet")}</div>
        ) : (
          <div className="run-records">
            {list.map((item) => (
              <div className={cx("run-record", shown?.path === item.path && "active")} key={item.path}>
                <div className="run-record-status">
                  <Badge tone={item.status === "finished" ? "success" : item.status === "running" ? "accent" : item.status === "failed" ? "danger" : "neutral"}>{runStatusLabel(item.status)}</Badge>
                </div>
                <div className="run-record-main">
                  <div className="run-record-title">
                    {item.label} <Badge>{item.legacy ? t("Scans/ (old)") : item.fidelity === "linear" ? t("Linear model") : t("Engine")}</Badge>
                  </div>
                  <div className="run-record-sub selectable">{[(item.axes ?? []).map((a) => a?.label).join(" × "), t("{done} / {count} points", { done: item.done, count: item.count }), item.created].filter(Boolean).join("   ·   ")}</div>
                </div>
                <div className="run-record-actions">
                  <IconButton icon="table" tip={t("Show")} onClick={() => open(item)} />
                  {item.legacy && <IconButton icon="folder" tip={t("Open folder")} onClick={() => openPath(item.path).catch(reportError)} />}
                  <IconButton icon="trash" className="run-record-delete" tip={t("Move to recycle bin")} disabled={state.running && state.scan?.path === item.path} onClick={() => remove(item)} />
                </div>
              </div>
            ))}
          </div>
        )}
      </Section>
    </>
  );
}

function ScanResult({ scan, live }: { scan: Scan; live: boolean }) {
  const t = useT();
  const dark = useApp((s) => s.resolvedTheme === "dark");
  const [metric, setMetric] = useState(scan.metrics[0] ?? "");
  const [point, setPoint] = useState<{ row: Row; data: Point | null } | null>(null);
  useEffect(() => {
    if (!scan.metrics.includes(metric)) setMetric(scan.metrics[0] ?? "");
    setPoint(null);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [scan.path]);
  const label = (k: number) => `${scan.axes[k]?.param ?? ""}${scan.axes[k]?.unit ? ` (${scan.axes[k].unit})` : ""}`;
  const metricLabel = t(METRIC_LABEL[metric] ?? metric);
  const byIndex = useMemo(() => new Map(scan.rows.map((r) => [r.index, r])), [scan.rows]);

  const figure = useMemo(() => {
    if (scan.shape.length === 1) {
      const rows = scan.rows.filter((r) => typeof r[metric] === "number");
      return {
        data: [{ type: "scatter" as const, mode: "lines+markers" as const, x: rows.map((r) => r.values[0]), y: rows.map((r) => r[metric] as number), name: metricLabel, marker: { size: 7 }, customdata: rows.map((r) => r.index) }],
        layout: { xaxis: { title: { text: label(0) } }, yaxis: { title: { text: metricLabel } }, showlegend: false, margin: { t: 16 } },
      };
    }
    const [n1, n2] = scan.shape;
    const z = Array.from({ length: n1 }, (_, i) => Array.from({ length: n2 }, (_, j) => {
      const v = byIndex.get(i * n2 + j + 1)?.[metric];
      return typeof v === "number" ? v : null;
    }));
    return {
      data: [{ type: "heatmap" as const, x: scan.values[1], y: scan.values[0], z, colorscale: fieldScale(dark, false) as any, colorbar: { title: { text: metricLabel } }, hoverongaps: false }],
      layout: { xaxis: { title: { text: label(1) } }, yaxis: { title: { text: label(0) } }, margin: { t: 16 } },
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [scan.rows, scan.shape, metric, dark]);

  const pick = async (row: Row | undefined) => {
    if (!row || row.error) return;
    setPoint({ row, data: null });
    if (scan.legacy) return;
    try {
      setPoint({ row, data: await call<Point>("study.point", { path: scan.path, index: row.index }) });
    } catch (e) {
      reportError(e);
      setPoint(null);
    }
  };
  const onClick = (e: any) => {
    const p = e.points?.[0];
    if (!p) return;
    if (scan.shape.length === 1) pick(byIndex.get(p.customdata));
    else {
      const i = scan.values[0].indexOf(p.y);
      const j = scan.values[1].indexOf(p.x);
      pick(byIndex.get(i * scan.shape[1] + j + 1));
    }
  };

  return (
    <Section
      title={`${t("Scan")} ${scan.label}: ${scan.axes.map((a) => a?.label).join(" × ")}`}
      icon={scan.shape.length === 2 ? "symbol-color" : "graph-line"}
      actions={<Select value={metric} options={scan.metrics.map((m) => ({ value: m, label: t(METRIC_LABEL[m] ?? m) }))} onChange={setMetric} />}
      description={live ? t("Updating while the scan runs.") : scan.finished ? `${runStatusLabel(scan.status)}   ·   ${scan.finished}   ·   ${scan.fidelity === "linear" ? t("Linear model") : t("Engine")}` : undefined}
    >
      {scan.rows.length === 0 ? (
        <Empty icon="graph-line" title={t("Waiting for the first point...")} />
      ) : (
        <>
          <div style={{ height: 300, position: "relative", marginBottom: 4 }}>
            <Plot data={figure.data as any} layout={figure.layout as any} onClick={onClick} style={{ position: "absolute", inset: 0 }} />
          </div>
          <p className="hint">{t("Click a point to see its envelope along the line.")}</p>
          {point && <PointView scan={scan} row={point.row} data={point.data} onClose={() => setPoint(null)} />}
          <div className="md-table-wrap">
            <table className="mini-table scan-table">
              <thead>
                <tr>
                  <th>#</th>
                  {scan.axes.map((_a, k) => (
                    <th key={k}>{label(k)}</th>
                  ))}
                  {scan.metrics.map((m) => (
                    <th key={m}>{t(METRIC_LABEL[m] ?? m)}</th>
                  ))}
                  <th>{t("time")}</th>
                  <th></th>
                </tr>
              </thead>
              <tbody>
                {scan.rows.map((r) => (
                  <tr key={r.index} className={cx(r.error && "danger-text", point?.row.index === r.index && "active")}>
                    <td>{r.index}</td>
                    {r.values.map((v, k) => (
                      <td key={k}>{fmtG(v)}</td>
                    ))}
                    {scan.metrics.map((m) => (
                      <td key={m}>{typeof r[m] === "number" ? fmtG(r[m] as number, 5) : "–"}</td>
                    ))}
                    <td>{r.seconds != null ? fmtSeconds(r.seconds) : ""}</td>
                    <td>
                      {r.error ? (
                        <span className="selectable">{r.error}</span>
                      ) : (
                        <>
                          {!scan.legacy && <IconButton icon="graph-line" tip={t("Envelope of this point")} onClick={() => pick(r)} />}
                          {r.output_dir && <IconButton icon="output" tip={t("Show results")} onClick={() => showResults(r.output_dir)} />}
                        </>
                      )}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </>
      )}
    </Section>
  );
}

function PointView({ scan, row, data, onClose }: { scan: Scan; row: Row; data: Point | null; onClose: () => void }) {
  const t = useT();
  const figure = useMemo(() => {
    if (!data) return null;
    const z = Array.from(data.z);
    const mm = (a: Float32Array) => Array.from(a, (v) => (Number.isFinite(v) ? v * 1e3 : null));
    return {
      data: [
        { type: "scatter" as const, mode: "lines" as const, x: z, y: mm(data.rms_x), name: "rms x" },
        { type: "scatter" as const, mode: "lines" as const, x: z, y: mm(data.rms_y), name: "rms y" },
        { type: "scatter" as const, mode: "lines" as const, x: z, y: mm(data.cx), name: t("centroid x"), line: { dash: "dot" as const } },
        { type: "scatter" as const, mode: "lines" as const, x: z, y: mm(data.cy), name: t("centroid y"), line: { dash: "dot" as const } },
      ],
      layout: { xaxis: { title: { text: "z (m)" } }, yaxis: { title: { text: "mm" } }, margin: { t: 16 }, legend: { orientation: "h" as const } },
    };
  }, [data, t]);
  return (
    <div className="study-point">
      <div className="row" style={{ gap: 8, alignItems: "center" }}>
        <strong>{t("Point {i}", { i: row.index })}</strong>
        <span className="muted">{row.values.map((v, k) => `${scan.axes[k]?.param} = ${fmtG(v)}`).join(", ")}</span>
        <span className="grow" />
        {row.output_dir && (
          <Button small icon="output" onClick={() => showResults(row.output_dir)}>
            {t("Open on the Results page")}
          </Button>
        )}
        <IconButton icon="close" tip={t("Close")} onClick={onClose} />
      </div>
      {scan.legacy ? (
        <p className="muted">{t("Old scans keep only their result folders; open them on the Results page.")}</p>
      ) : !figure ? (
        <Spinner />
      ) : (
        <div style={{ height: 240, position: "relative" }}>
          <Plot data={figure.data} layout={figure.layout} style={{ position: "absolute", inset: 0 }} />
        </div>
      )}
    </div>
  );
}
