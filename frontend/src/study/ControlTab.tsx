// Study page, response & controllability and feedback tabs (avas/gui/services/analysis.py).
// Monitors and knobs are shared by both tabs and saved in Research/diagnostics.json.
import { useCallback, useEffect, useMemo, useState } from "react";
import { call, on } from "../bridge";
import { Plot, fieldScale, LEGEND_TOP } from "../components/Plot";
import { confirmDialog, reportError, toast } from "../components/overlays";
import { Badge, Button, Checkbox, CommitInput, Empty, Field, IconButton, NumberInput, Section, Select, Spinner, TextInput } from "../components/ui";
import { fmtG } from "../format";
import { useT } from "../i18n";
import { setPage, useApp } from "../store/app";
import { elementLabel, SeedDefectNote, useProjectLattice } from "./common";

export type Monitor = { name: string; z: number; kind: "position" | "size"; planes: string; resolution: number; line?: number | null };
export type Knob = { target: string | number; param: string; kind: "lattice" | "error" | "beam"; step?: number | null; lower?: number | null; upper?: number | null };
export type Spec = { monitors: Monitor[]; knobs: Knob[]; source: "file" | "lattice" | "edited"; path?: string };
type Response = { matrix: number[][]; readings: string[]; knobs: string[]; y0: number[]; singular: number[]; rank: number; condition: number | null; knobNoise: number[]; uncorrectable: number; invisible: number };
type Modes = { knobs: string[]; readings: string[]; states: string[]; controllableRank: number; observableRank: number; n: number; stateError: (number | null)[]; modes: { re: number; im: number; modulus: number; tune: number; class: string; controllable: boolean | null; observable: boolean | null }[] };
type Period = { index: number; z0: number; z1: number };

/** Monitors and knobs of the project, editable, shared by the control and feedback tabs. */
export function useDiagnosticsSpec() {
  const project = useApp((s) => s.project);
  const [spec, setSpec] = useState<Spec | null>(null);
  const load = useCallback(async () => {
    try {
      setSpec(await call<Spec>("analysis.diagnostics"));
    } catch (e) {
      reportError(e);
    }
  }, []);
  useEffect(() => {
    if (project.open) load();
  }, [project.open, project.path, load]);
  return { spec, setSpec, load };
}

const CLASS_LABEL: Record<string, string> = { stable: "stable", stabilizable: "feedback can stabilise", uncontrollable: "not controllable" };

function SpecEditor({ spec, setSpec, reload }: { spec: Spec; setSpec: (s: Spec) => void; reload: () => void }) {
  const t = useT();
  const { statements } = useProjectLattice();
  const edit = (patch: Partial<Spec>) => setSpec({ ...spec, ...patch, source: "edited" });
  const setMonitor = (k: number, m: Partial<Monitor>) => edit({ monitors: spec.monitors.map((x, i) => (i === k ? { ...x, ...m } : x)) });
  const setKnob = (k: number, m: Partial<Knob>) => edit({ knobs: spec.knobs.map((x, i) => (i === k ? { ...x, ...m } : x)) });
  const elements = statements.filter((s) => s.isElement && s.known && s.active);
  const save = async () => {
    try {
      const res = await call<{ path: string }>("analysis.saveDiagnostics", { monitors: spec.monitors, knobs: spec.knobs });
      setSpec({ ...spec, source: "file", path: res.path });
      toast(t("Monitors and knobs saved"), "success");
    } catch (e) {
      reportError(e);
    }
  };
  const fromLattice = async () => {
    if (spec.source === "file" && !(await confirmDialog(t("Replace the saved monitors and knobs by those of the lattice (diag_position, diag_size and steerers)? The file is changed only when you save."), { ok: t("Replace") }))) return;
    try {
      const path = spec.path;
      setSpec({ ...(await call<Spec>("analysis.diagnostics")), source: "edited", path });
    } catch (e) {
      reportError(e);
    }
  };
  return (
    <Section
      title={t("Monitors and knobs")}
      icon="symbol-ruler"
      description={spec.source === "file" ? t("Saved in Research/diagnostics.json.") : spec.source === "lattice" ? t("Taken from the lattice: diag_position / diag_size lines and steerers.") : t("Changed, not saved.")}
      actions={
        <>
          <Button small icon="refresh" onClick={reload}>{t("Reload")}</Button>
          <Button small icon="list-tree" onClick={fromLattice}>{t("From the lattice")}</Button>
          <Button small icon="save" onClick={save} disabled={spec.source === "file"}>{t("Save")}</Button>
        </>
      }
    >
      <div className="study-columns">
        <div>
          <h4>{t("Monitors")}</h4>
          <table className="mini-table study-table">
            <thead>
              <tr>
                <th>{t("Name")}</th><th>z (m)</th><th>{t("Reads")}</th><th>{t("Planes")}</th><th>{t("Resolution (µm)")}</th><th />
              </tr>
            </thead>
            <tbody>
              {spec.monitors.map((m, k) => (
                <tr key={k}>
                  <td><TextInput value={m.name} onChange={(e) => setMonitor(k, { name: e.target.value })} /></td>
                  <td><NumberInput value={m.z} onCommit={(v) => setMonitor(k, { z: v ?? 0 })} /></td>
                  <td><Select value={m.kind} options={[{ value: "position", label: t("position") }, { value: "size", label: t("rms size") }]} onChange={(v) => setMonitor(k, { kind: v as Monitor["kind"] })} /></td>
                  <td><Select value={m.planes} options={["xy", "x", "y"].map((p) => ({ value: p, label: p }))} onChange={(v) => setMonitor(k, { planes: v })} /></td>
                  <td><NumberInput value={m.resolution * 1e6} onCommit={(v) => setMonitor(k, { resolution: (v ?? 0) * 1e-6 })} /></td>
                  <td><IconButton icon="trash" tip={t("Remove")} onClick={() => edit({ monitors: spec.monitors.filter((_x, i) => i !== k) })} /></td>
                </tr>
              ))}
            </tbody>
          </table>
          <Button small icon="add" onClick={() => edit({ monitors: [...spec.monitors, { name: `M${spec.monitors.length + 1}`, z: 0, kind: "position", planes: "xy", resolution: 20e-6 }] })}>
            {t("Add a monitor")}
          </Button>
        </div>
        <div>
          <h4>{t("Knobs")}</h4>
          <table className="mini-table study-table">
            <thead>
              <tr>
                <th>{t("Element")}</th><th>{t("Parameter")}</th><th>{t("Kind")}</th><th>{t("Step")}</th><th />
              </tr>
            </thead>
            <tbody>
              {spec.knobs.map((kn, k) => (
                <tr key={k}>
                  <td>
                    <Select
                      value={String(kn.target)}
                      options={[...elements.map((s) => ({ value: s.name || String(s.line + 1), label: elementLabel(s, t) })), ...(elements.some((s) => (s.name || String(s.line + 1)) === String(kn.target)) ? [] : [{ value: String(kn.target), label: String(kn.target) }])]}
                      onChange={(v) => setKnob(k, { target: v })}
                    />
                  </td>
                  <td><TextInput value={kn.param} onChange={(e) => setKnob(k, { param: e.target.value })} /></td>
                  <td><Select value={kn.kind} options={[{ value: "lattice", label: t("parameter") }, { value: "error", label: t("offset (dx, dy, roll)") }]} onChange={(v) => setKnob(k, { kind: v as Knob["kind"] })} /></td>
                  <td>
                    <CommitInput
                      value={kn.step == null ? "" : String(kn.step)}
                      placeholder={t("default")}
                      tip={t("empty: default step")}
                      validate={(v) => !v.trim() || Number.isFinite(Number(v))}
                      onCommit={(v) => setKnob(k, { step: v.trim() ? Number(v) : null })}
                    />
                  </td>
                  <td><IconButton icon="trash" tip={t("Remove")} onClick={() => edit({ knobs: spec.knobs.filter((_x, i) => i !== k) })} /></td>
                </tr>
              ))}
            </tbody>
          </table>
          <Button small icon="add" onClick={() => edit({ knobs: [...spec.knobs, { target: elements[0] ? elements[0].name || String(elements[0].line + 1) : "", param: "By", kind: "lattice" }] })}>
            {t("Add a knob")}
          </Button>
        </div>
      </div>
    </Section>
  );
}

export function ControlTab({ spec, setSpec, reload }: { spec: Spec | null; setSpec: (s: Spec) => void; reload: () => void }) {
  const t = useT();
  const dark = useApp((s) => s.resolvedTheme === "dark");
  const run = useApp((s) => s.run);
  const { text } = useProjectLattice();
  const [resp, setResp] = useState<{ source: "linear" | "engine"; data: Response } | null>(null);
  const [busy, setBusy] = useState(false);
  const [engineBusy, setEngineBusy] = useState(false);
  const [particles, setParticles] = useState(200);
  const [periods, setPeriods] = useState<Period[]>([]);
  const [period, setPeriod] = useState(1);
  const [modes, setModes] = useState<Modes | null>(null);

  useEffect(() => {
    if (text) call<Period[]>("analysis.periods", { text }).then(setPeriods).catch(() => setPeriods([]));
  }, [text]);
  useEffect(() => {
    const off = on("analysis.responseFinished", (r: any) => {
      setEngineBusy(false);
      if (r.status === "finished") setResp({ source: "engine", data: r });
      else if (r.error) toast(r.error, "error", 6000);
    });
    return off;
  }, []);

  if (!spec) return <Spinner />;
  const compute = async () => {
    setBusy(true);
    try {
      setResp({ source: "linear", data: await call<Response>("analysis.response", { monitors: spec.monitors, knobs: spec.knobs }) });
    } catch (e) {
      reportError(e, t("Response matrix"));
    } finally {
      setBusy(false);
    }
  };
  const computeEngine = async () => {
    const n = spec.knobs.length + 1;
    if (!(await confirmDialog(t("Run the engine {n} times ({p} macro-particles, fixed seed) to measure the response?", { n, p: particles }), { ok: t("Start") }))) return;
    try {
      await call("analysis.responseEngineStart", { monitors: spec.monitors, knobs: spec.knobs, particles });
      setEngineBusy(true);
    } catch (e) {
      reportError(e);
    }
  };
  const computeModes = async () => {
    try {
      setModes(await call<Modes>("analysis.periodModes", { monitors: spec.monitors, knobs: spec.knobs, period }));
    } catch (e) {
      reportError(e, t("Modes of the period"));
    }
  };

  return (
    <>
      <SpecEditor spec={spec} setSpec={setSpec} reload={reload} />
      <div className="row" style={{ gap: 8, margin: "4px 0 12px", alignItems: "center" }}>
        <Button variant="primary" icon="zap" onClick={compute} disabled={busy}>{t("Response of the linear model")}</Button>
        {engineBusy ? (
          <>
            <Button icon="debug-stop" onClick={() => call("analysis.responseEngineStop").catch(reportError)}>{t("Stop")}</Button>
            <Spinner />
            <Button small variant="ghost" icon="pulse" onClick={() => setPage("run")}>{t("Show progress")}</Button>
          </>
        ) : (
          <Button icon="server-process" onClick={computeEngine} disabled={run.running} tip={t("Base run plus one run per knob; lattice parameters only.")}>
            {t("Response from the engine")}
          </Button>
        )}
        <Field label={t("Macro-particles")}>
          <NumberInput value={particles} onCommit={(v) => setParticles(Math.max(1, Math.round(v ?? 200)))} />
        </Field>
      </div>
      <SeedDefectNote />
      {resp ? <ResponseView resp={resp.data} source={resp.source} dark={dark} /> : <Empty icon="graph" title={t("No response computed yet")} />}

      <Section title={t("Modes of one period")} icon="pulse" description={t("The centroid of a period as x_{n+1} = A·x_n + B·u_n, y_n = C·x_n: which motions the period's knobs can steer and its monitors can see.")}>
        {periods.length === 0 ? (
          <p className="warning-text">{t("The run lattice has no period: mark one with 'lattice n 1' … 'lattice_end' on the Lattice page.")}</p>
        ) : (
          <div className="row" style={{ gap: 8, alignItems: "center" }}>
            <Select value={period} options={periods.map((p) => ({ value: p.index, label: `${p.index}  ·  z ${p.z0.toFixed(3)} – ${p.z1.toFixed(3)} m` }))} onChange={(v) => setPeriod(Number(v))} />
            <Button icon="play" onClick={computeModes}>{t("Analyse")}</Button>
          </div>
        )}
        {modes && (
          <>
            <p>
              {t("Controllable rank {c} of {n}, observable rank {o} of {n}.", { c: modes.controllableRank, o: modes.observableRank, n: modes.n })}{" "}
              <span className="muted">{t("Knobs: {k}. Readings: {r}.", { k: modes.knobs.join(", "), r: modes.readings.join(", ") || "–" })}</span>
            </p>
            <table className="mini-table">
              <thead>
                <tr><th>|λ|</th><th>{t("phase (deg)")}</th><th>{t("class")}</th><th>{t("controllable")}</th><th>{t("observable")}</th></tr>
              </thead>
              <tbody>
                {modes.modes.map((m, k) => (
                  <tr key={k}>
                    <td>{fmtG(m.modulus, 5)}</td>
                    <td>{fmtG(m.tune, 4)}</td>
                    <td><Badge tone={m.class === "stable" ? "success" : m.class === "stabilizable" ? "warning" : "danger"}>{t(CLASS_LABEL[m.class] ?? m.class)}</Badge></td>
                    <td>{m.controllable == null ? "–" : m.controllable ? t("yes") : t("no")}</td>
                    <td>{m.observable == null ? "–" : m.observable ? t("yes") : t("no")}</td>
                  </tr>
                ))}
              </tbody>
            </table>
            {modes.stateError.length > 0 && (
              <p className="muted">
                {t("Smallest error of the entry state from one period of readings: {e}", {
                  e: modes.states.map((s, k) => `${s} ${modes.stateError[k] == null ? "∞" : `${fmtG((modes.stateError[k] as number) * 1e6, 3)} ${s.includes("'") ? "µrad" : "µm"}`}`).join(", "),
                })}
              </p>
            )}
          </>
        )}
      </Section>
    </>
  );
}

function ResponseView({ resp, source, dark }: { resp: Response; source: "linear" | "engine"; dark: boolean }) {
  const t = useT();
  const heat = useMemo(
    () => ({
      data: [{ type: "heatmap", x: resp.knobs, y: resp.readings, z: resp.matrix.map((r) => r.map((v) => v * 1e3)), colorscale: fieldScale(dark, true), zmid: 0, colorbar: { title: { text: "mm/unit" } } }],
      layout: { margin: { t: 16, l: 90 }, yaxis: { autorange: "reversed" as const } },
    }),
    [resp, dark],
  );
  const bars = useMemo(
    () => ({
      data: [{ type: "bar", x: resp.singular.map((_s, i) => i + 1), y: resp.singular, name: t("singular value") }],
      layout: { margin: { t: 16 }, xaxis: { title: { text: t("mode") }, dtick: 1 }, yaxis: { type: "log" as const, title: { text: t("singular value") } } },
    }),
    [resp, t],
  );
  return (
    <Section title={t("Response matrix")} icon="graph" description={source === "engine" ? t("From engine runs (fixed seed).") : t("From the linear model.")}>
      <div className="study-columns">
        <div style={{ height: 300, position: "relative" }}><Plot data={heat.data as any} layout={heat.layout as any} style={{ position: "absolute", inset: 0 }} /></div>
        <div style={{ height: 300, position: "relative" }}><Plot data={bars.data as any} layout={bars.layout as any} style={{ position: "absolute", inset: 0 }} /></div>
      </div>
      <ul className="study-facts">
        <li>{t("Rank {r} of {k} knobs; condition number {c}.", { r: resp.rank, k: resp.knobs.length, c: resp.condition == null ? "∞" : fmtG(resp.condition, 3) })}</li>
        <li>{resp.uncorrectable > 0 ? t("{n} reading patterns cannot be corrected by any knob.", { n: resp.uncorrectable }) : t("Every reading pattern can be corrected.")}</li>
        <li>{resp.invisible > 0 ? t("{n} knob combinations are seen by no monitor.", { n: resp.invisible }) : t("Every knob combination is seen by the monitors.")}</li>
        <li>{t("Knob error caused by monitor noise through the correction: {list}", { list: resp.knobs.map((k, i) => `${k} ${fmtG(resp.knobNoise[i], 3)}`).join(", ") })}</li>
      </ul>
    </Section>
  );
}

export function FeedbackTab({ spec }: { spec: Spec | null }) {
  const t = useT();
  const { statements } = useProjectLattice();
  const [gain, setGain] = useState(0.5);
  const [delay, setDelay] = useState(1);
  const [pulses, setPulses] = useState(150);
  const [target, setTarget] = useState("");
  const [key, setKey] = useState("dx");
  const [step, setStep] = useState(5);
  const [noise, setNoise] = useState(true);
  const [result, setResult] = useState<{ rms: number[]; openRms: number[]; predictedModulus: number } | null>(null);
  const [busy, setBusy] = useState(false);
  const elements = statements.filter((s) => s.isElement && s.known && s.active && s.key !== "steerer");
  if (!spec) return <Spinner />;
  const simulate = async () => {
    setBusy(true);
    try {
      setResult(await call("analysis.feedback", {
        monitors: spec.monitors, knobs: spec.knobs, gain, delay, pulses, noise,
        drift: target ? { target, key, step: step * (key === "roll" ? 1e-6 : 1e-6) } : null,
      }));
    } catch (e) {
      reportError(e, t("Feedback"));
    } finally {
      setBusy(false);
    }
  };
  const figure = result && {
    data: [
      { type: "scatter", mode: "lines", y: result.openRms.map((v) => v * 1e6), name: t("without feedback") },
      { type: "scatter", mode: "lines", y: result.rms.map((v) => v * 1e6), name: t("with feedback") },
    ],
    layout: { margin: { t: 16 }, xaxis: { title: { text: t("pulse") } }, yaxis: { title: { text: t("rms of the readings (µm)") } }, legend: LEGEND_TOP },
  };
  return (
    <>
      <Section title={t("Feedback loop")} icon="sync" description={t("Every pulse: the linear model with the drifting error, noisy readings of the monitors, and u ← u − g·R⁺·y of a reading d pulses old. Monitors and knobs as on the Response tab.")}>
        <div className="form-grid">
          <Field label={t("Gain g")}><NumberInput value={gain} onCommit={(v) => setGain(v ?? 0.5)} /></Field>
          <Field label={t("Delay d (pulses)")}><NumberInput value={delay} onCommit={(v) => setDelay(Math.max(0, Math.round(v ?? 0)))} /></Field>
          <Field label={t("Pulses")}><NumberInput value={pulses} onCommit={(v) => setPulses(Math.max(2, Math.round(v ?? 150)))} /></Field>
          <Field label={t("Drifting element")}>
            <Select value={target} options={[{ value: "", label: t("(none)") }, ...elements.map((s) => ({ value: s.name || String(s.line + 1), label: elementLabel(s, t) }))]} onChange={setTarget} />
          </Field>
          <Field label={t("Drifting quantity")}>
            <Select value={key} options={[{ value: "dx", label: "dx" }, { value: "dy", label: "dy" }, { value: "roll", label: t("roll") }]} onChange={setKey} />
          </Field>
          <Field label={t("rms step per pulse")} unit={key === "roll" ? "µrad" : "µm"}><NumberInput value={step} onCommit={(v) => setStep(v ?? 0)} /></Field>
        </div>
        <Checkbox checked={noise} onChange={setNoise} label={t("Monitor noise (resolution of each monitor)")} />
        <div className="row" style={{ gap: 8, marginTop: 8 }}>
          <Button variant="primary" icon="play" onClick={simulate} disabled={busy}>{t("Simulate")}</Button>
          {busy && <Spinner />}
        </div>
      </Section>
      {result && figure && (
        <Section title={t("Readings with and without feedback")} icon="graph-line" description={t("Each corrected mode follows z^(d+1) − z^d + g = 0: largest |z| = {m} ({s}).", { m: fmtG(result.predictedModulus, 4), s: result.predictedModulus < 1 ? t("stable") : t("unstable") })}>
          <div style={{ height: 300, position: "relative" }}><Plot data={figure.data as any} layout={figure.layout as any} style={{ position: "absolute", inset: 0 }} /></div>
        </Section>
      )}
    </>
  );
}
