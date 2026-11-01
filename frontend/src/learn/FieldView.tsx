// Field pictures in the primer: a matrix-model element's field computed from its
// parameters (schematic, lattice/analyticField.ts) or a slice of one of the
// example field maps (data).  Both go through the same viewer as the Files page.
import { useEffect, useState } from "react";
import { call } from "../bridge";
import { Icon, Spinner } from "../components/ui";
import { FieldSliceView, type FieldSource } from "../files/FieldSlice";
import { useT } from "../i18n";
import type { ElementField } from "../lattice/analyticField";
import type { Directive } from "./directives";

type Examples = { available: boolean; fieldDirs: string[] };
let examples: Promise<Examples> | null = null;
export function exampleMaps(): Promise<Examples> {
  examples ??= call<Examples>("learn.examples");
  examples.catch(() => (examples = null));
  return examples;
}

function analyticSource(d: Directive): FieldSource | null {
  const n = (k: string, dflt: number) => (d.args[k] != null ? Number(d.args[k]) : dflt);
  const R = n("R", 0.02);
  let field: ElementField;
  let length = n("L", 0.2);
  const unit = "T";
  switch (d.id) {
    case "quadrupole":
      field = { kind: "quadrupole", g: n("G", 4) };
      length = n("L", 0.1);
      break;
    case "solenoid":
      field = { kind: "solenoid", b: n("B", 1) };
      break;
    case "steerer":
      field = { kind: "steerer", bx: n("Bx", 0), by: n("By", 0.01) };
      length = 0;
      break;
    case "dipole":
      // as in the component view: the lattice has no B for a dipole, the picture is relative to the axis
      field = { kind: "dipole", b: Math.sign(n("B", 1)) || 1, index: n("N", 0), rho: Math.max(1e-6, n("rho", 1)), vertical: d.args.HV === "1" };
      length = n("L", 0.785);
      break;
    default:
      return null;
  }
  return { kind: "analytic", field, length, r: R, name: d.id, unit };
}

export function FieldView({ directive }: { directive: Directive }) {
  const t = useT();
  const [dirs, setDirs] = useState<string[] | null>(null);
  const [error, setError] = useState<string | null>(null);
  const isMap = directive.id === "map";
  useEffect(() => {
    if (!isMap) return;
    let live = true;
    exampleMaps()
      .then((e) => {
        if (!live) return;
        if (e.available) setDirs(e.fieldDirs);
        else setError(t("The example field maps are not installed."));
      })
      .catch((e) => live && setError(String(e?.message ?? e)));
    return () => {
      live = false;
    };
  }, [isMap, t]);
  if (isMap) {
    if (error) return <p className="learn-lab-error">{error}</p>;
    if (!dirs) return <Spinner />;
    return (
      <figure className="learn-field">
        <figcaption>
          <Icon name="symbol-field" /> {t("Field map {name} of the example project (data)", { name: directive.items[0] })}
        </figcaption>
        <FieldSliceView source={{ kind: "map", name: directive.items[0], fieldDirs: dirs }} />
      </figure>
    );
  }
  const source = analyticSource(directive);
  if (!source) return null;
  return (
    <figure className="learn-field">
      <figcaption>
        <Icon name="symbol-field" /> {t("Field computed from the element parameters (schematic)")}
      </figcaption>
      <FieldSliceView source={source} />
    </figure>
  );
}
