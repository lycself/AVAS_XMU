// Shared pieces of the Study page: the project's lattice elements and keyword schema, an axis picker
// (lattice parameter, beam.txt or input.txt keyword) and metric labels.
import { useCallback, useEffect, useState } from "react";
import { call } from "../bridge";
import { reportError } from "../components/overlays";
import { CommitInput, Field, IconButton, Radio, Select } from "../components/ui";
import { fmtG } from "../format";
import { pick, useT } from "../i18n";
import { useApp } from "../store/app";

export type Statement = { line: number; keyword: string; key: string; name: string; params: string[]; isElement: boolean; known: boolean; active: boolean; zStart: number | null; zEnd?: number | null };
export type KeywordSpec = { key: string; title: [string, string]; params: { key: string; label: [string, string]; unit: string; kind: string }[] };
export type Schema = { lattice: KeywordSpec[]; beam: KeywordSpec[]; input: KeywordSpec[] };
export type AxisSpec = { kind: "lattice" | "beam" | "input"; target?: string; param?: string; keyword?: string; position?: number };

export const METRIC_LABEL: Record<string, string> = {
  transmission: "transmission",
  lost: "lost macro-particles",
  energy_in: "energy in (MeV)",
  energy_out: "energy out (MeV)",
  emit_x_in: "εx in",
  emit_y_in: "εy in",
  emit_z_in: "εz in",
  emit_x_out: "εx out",
  emit_y_out: "εy out",
  emit_z_out: "εz out",
  emit_x_growth: "εx growth",
  emit_y_growth: "εy growth",
  emit_z_growth: "εz growth",
  rms_x_max: "max rms x (mm)",
  rms_y_max: "max rms y (mm)",
  rms_x_out: "rms x out (mm)",
  rms_y_out: "rms y out (mm)",
  max_x_max: "max |x| (mm)",
  max_y_max: "max |y| (mm)",
  centroid_x_max: "max centroid x (mm)",
  centroid_y_max: "max centroid y (mm)",
  z_out: "z out (m)",
};

/** The run lattice of the open project (text, parsed statements) and the keyword schema. */
export function useProjectLattice() {
  const project = useApp((s) => s.project);
  const [statements, setStatements] = useState<Statement[]>([]);
  const [text, setText] = useState("");
  const [schema, setSchema] = useState<Schema | null>(null);
  const reload = useCallback(async () => {
    try {
      const list = await call<{ active: string; fieldDirs: string[] }>("lattice.list");
      const file = await call<{ text: string }>("lattice.read", { name: list.active });
      const doc = await call<{ statements: Statement[] }>("lattice.parse", { text: file.text, fieldDirs: list.fieldDirs });
      setText(file.text);
      setStatements(doc.statements);
    } catch {
      setStatements([]);
    }
  }, []);
  useEffect(() => {
    if (!project.open) return;
    reload();
    call<any>("schema.all")
      .then((s) => setSchema({ lattice: s.lattice, beam: s.beam, input: s.input }))
      .catch(reportError);
  }, [project.open, project.path, reload]);
  return { statements, text, schema, reload };
}

export function elementLabel(s: Statement, t: (k: string, a?: Record<string, string | number>) => string): string {
  return `${s.name || s.keyword}  ·  ${t("line {n}", { n: s.line + 1 })}  ·  ${s.keyword}${s.zStart != null ? `  ·  z ${fmtG(s.zStart, 4)} m` : ""}`;
}

/** Element + parameter, or keyword (+ position) of beam.txt / input.txt. */
export function AxisPicker({ spec, onChange, statements, schema, onReload, kinds = ["lattice", "beam", "input"], name }: {
  spec: AxisSpec;
  onChange: (s: AxisSpec) => void;
  statements: Statement[];
  schema: Schema | null;
  onReload?: () => void;
  kinds?: AxisSpec["kind"][];
  name: string;
}) {
  const t = useT();
  const elements = statements.filter((s) => s.isElement && s.known && s.active);
  const chosen = elements.find((s) => String(s.line + 1) === spec.target);
  const paramOptions = (chosen && schema?.lattice.find((k) => k.key === chosen.key)?.params.filter((p) => p.kind !== "reserved").map((p) => ({ value: p.key, label: `${p.key}  ·  ${pick(p.label)}${p.unit ? ` (${p.unit})` : ""}` }))) ?? [];
  const keywords = spec.kind === "beam" ? schema?.beam : schema?.input;
  const keywordSpec = keywords?.find((k) => k.key === spec.keyword);
  const positionOptions = keywordSpec && keywordSpec.params.length > 1 ? keywordSpec.params.map((p, i) => ({ value: i + 1, label: `${i + 1}  ·  ${pick(p.label)}${p.unit ? ` (${p.unit})` : ""}` })) : [];
  return (
    <>
      <div className="row" style={{ gap: 16, marginBottom: 8 }}>
        {kinds.includes("lattice") && <Radio checked={spec.kind === "lattice"} onChange={() => onChange({ kind: "lattice" })} label={t("Lattice element parameter")} name={name} />}
        {kinds.includes("beam") && <Radio checked={spec.kind === "beam"} onChange={() => onChange({ kind: "beam" })} label="beam.txt" name={name} />}
        {kinds.includes("input") && <Radio checked={spec.kind === "input"} onChange={() => onChange({ kind: "input" })} label="input.txt" name={name} />}
      </div>
      <div className="form-grid">
        {spec.kind === "lattice" ? (
          <>
            <Field label={t("Element")}>
              <Select value={spec.target ?? ""} options={[{ value: "", label: t("(choose an element)") }, ...elements.map((s) => ({ value: String(s.line + 1), label: elementLabel(s, t) }))]} onChange={(v) => onChange({ kind: "lattice", target: v, param: "" })} />
              {onReload && <IconButton icon="refresh" tip={t("Reload the lattice")} onClick={onReload} />}
            </Field>
            <Field label={t("Parameter")}>
              <Select value={spec.param ?? ""} options={[{ value: "", label: t("(choose a parameter)") }, ...paramOptions]} onChange={(v) => onChange({ ...spec, param: v })} disabled={!chosen} />
            </Field>
          </>
        ) : (
          <>
            <Field label={t("Keyword")}>
              <Select value={spec.keyword ?? ""} options={[{ value: "", label: t("(choose a keyword)") }, ...(keywords ?? []).map((k) => ({ value: k.key, label: `${k.key}  ·  ${pick(k.title)}` }))]} onChange={(v) => onChange({ kind: spec.kind, keyword: v, position: 1 })} />
            </Field>
            {positionOptions.length > 0 && (
              <Field label={t("Value")}>
                <Select value={spec.position ?? 1} options={positionOptions} onChange={(v) => onChange({ ...spec, position: Number(v) })} />
              </Field>
            )}
          </>
        )}
      </div>
    </>
  );
}

export function axisComplete(spec: AxisSpec): boolean {
  return spec.kind === "lattice" ? !!spec.target && !!spec.param : !!spec.keyword;
}

export function ValuesField({ value, onCommit, unit, original }: { value: string; onCommit: (v: string) => void; unit?: string; original?: string | null }) {
  const t = useT();
  return (
    <Field label={t("Values")} unit={unit || undefined} hint={t("Comma-separated, or start:stop:count (e.g. 10:20:6). Current value: {v}", { v: original ?? "–" })}>
      <CommitInput value={value} onCommit={onCommit} placeholder="10, 12, 14  |  10:20:6" mono />
    </Field>
  );
}
