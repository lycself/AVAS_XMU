// Parameter tables of the primer, straight from the keyword schema
// (avas/data/schema.py via schema.all), so the primer never repeats them by hand.
import { useEffect, useState } from "react";
import { Spinner } from "../components/ui";
import { pick, useT } from "../i18n";
import { loadSchema, type Keyword, type Schema } from "../lattice/types";
import { paramRefs, type Directive } from "./directives";

const FILE: Record<string, string> = { lattice: "lattice", beam: "beam.txt", input: "input.txt" };

export function ParamsTable({ directive }: { directive: Directive }) {
  const t = useT();
  const [schema, setSchema] = useState<Schema | null>(null);
  const [error, setError] = useState<string | null>(null);
  useEffect(() => {
    let live = true;
    loadSchema()
      .then((s) => live && setSchema(s))
      .catch((e) => live && setError(String(e?.message ?? e)));
    return () => {
      live = false;
    };
  }, []);
  if (error) return <p className="learn-lab-error">{error}</p>;
  if (!schema) return <Spinner />;
  const refs = paramRefs(directive);
  const beamOrInput = refs.filter((r) => r.group !== "lattice");
  const lattice = refs.filter((r) => r.group === "lattice");
  const find = (group: "lattice" | "beam" | "input", key: string): Keyword | undefined => schema[group].find((k) => k.key === key);
  return (
    <div className="learn-params">
      {lattice.map((r) => {
        const k = find("lattice", r.key);
        if (!k) return <p key={r.key} className="learn-lab-error">{t("Unknown keyword: {key}", { key: r.key })}</p>;
        return (
          <figure key={r.key} className="learn-keyword">
            <figcaption>
              <code>{k.key}</code> — {pick(k.title)}
              {pick(k.doc) && <span className="soft"> · {pick(k.doc)}</span>}
            </figcaption>
            <div className="md-table-wrap">
              <table className="md-table">
                <thead>
                  <tr>
                    <th>#</th>
                    <th>{t("Parameter")}</th>
                    <th>{t("Unit")}</th>
                    <th>{t("Meaning")}</th>
                  </tr>
                </thead>
                <tbody>
                  {k.params.map((p, i) => (
                    <tr key={i}>
                      <td className="mono">{i + 1}</td>
                      <td>{pick(p.label)}</td>
                      <td className="mono">{p.unit}</td>
                      <td>
                        {pick(p.doc)}
                        {p.choices.length > 0 && <span className="soft"> {p.choices.map(([v, l]) => `${v} = ${pick(l)}`).join("; ")}</span>}
                      </td>
                    </tr>
                  ))}
                  {!k.params.length && (
                    <tr>
                      <td colSpan={4} className="soft">
                        {t("No parameters.")}
                      </td>
                    </tr>
                  )}
                </tbody>
              </table>
            </div>
          </figure>
        );
      })}
      {beamOrInput.length > 0 && (
        <div className="md-table-wrap">
          <table className="md-table">
            <thead>
              <tr>
                <th>{t("File")}</th>
                <th>{t("Keyword")}</th>
                <th>{t("Values")}</th>
                <th>{t("Meaning")}</th>
              </tr>
            </thead>
            <tbody>
              {beamOrInput.map((r) => {
                const k = find(r.group as "beam" | "input", r.key);
                return (
                  <tr key={`${r.group}:${r.key}`}>
                    <td className="mono">{FILE[r.group] ?? r.group}</td>
                    <td className="mono">{r.key}</td>
                    <td>{k ? k.params.map((p) => `${pick(p.label)}${p.unit ? ` (${p.unit})` : ""}`).join(", ") : "—"}</td>
                    <td>
                      {k ? pick(k.title) : t("Unknown keyword: {key}", { key: r.key })}
                      {k && pick(k.doc) && <span className="soft"> · {pick(k.doc)}</span>}
                      {k?.params.some((p) => p.choices.length) && (
                        <span className="soft"> {k.params.flatMap((p) => p.choices.map(([v, l]) => `${v} = ${pick(l)}`)).join("; ")}</span>
                      )}
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>
      )}
    </div>
  );
}
