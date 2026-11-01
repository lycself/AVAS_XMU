// Interactive blocks inside the primer chapters.  A chapter embeds them as
// fenced blocks whose language names the kind and whose body is one line:
//
//   ```avas-lab            fodo G=4 Ld=0.3        an interactive lab (labs run on the back end)
//   ```avas-params         quad beam:twissx       parameter tables from the keyword schema
//   ```avas-field          quadrupole G=4 L=0.1   field picture of a matrix-model element (schematic)
//   ```avas-field          map hwr010             a slice of one of the example field maps (data)
//   ```avas-tutorial       build-linac            the guided tutorial
//   ```avas-beamline       (lattice lines)        a 2D / 3D beam line with the example beam's linear preview;
//                                                 the body is AVAS lattice text, start / end may be left out
//
// Lab parameters are checked against the back end's lab definitions by
// tests/test_learn.py; this module only knows the names.

export type DirectiveKind = "lab" | "params" | "field" | "tutorial" | "beamline";
/** *text* is the body of an ``avas-beamline`` block (lattice lines); the other kinds take one line of words. */
export type Directive = { kind: DirectiveKind; id: string; args: Record<string, string>; items: string[]; text?: string };

export const LAB_IDS = ["kinematics", "phase-space", "distribution", "fodo", "solenoid", "dipole", "rf-cavity", "space-charge"] as const;
export type LabId = (typeof LAB_IDS)[number];

/** Matrix-model fields drawn from parameters (lattice/analyticField.ts) and the parameters each takes. */
export const FIELD_MODELS: Record<string, string[]> = {
  quadrupole: ["G", "L", "R"],
  solenoid: ["B", "L", "R"],
  steerer: ["Bx", "By", "R"],
  dipole: ["B", "N", "rho", "L", "R", "HV"],
};
/** Example field maps shipped with AVAS (examples/hwr010). */
export const FIELD_MAPS = ["hwr010", "sol"];
export const TUTORIALS = ["build-linac"];
export const PARAM_GROUPS = ["lattice", "beam", "input"] as const;

const KINDS: Record<string, DirectiveKind> = {
  "avas-lab": "lab",
  "avas-params": "params",
  "avas-field": "field",
  "avas-tutorial": "tutorial",
  "avas-beamline": "beamline",
};

export function isDirectiveLang(lang: string): boolean {
  return lang in KINDS;
}

/** ``fodo G=4 Ld=0.3`` → id fodo, args {G: "4", Ld: "0.3"}; plain words go to *items*. */
export function parseDirective(lang: string, body: string): Directive | null {
  const kind = KINDS[lang];
  if (!kind) return null;
  if (kind === "beamline") return { kind, id: "beamline", args: {}, items: [], text: body.replace(/\s+$/, "") + "\n" };
  const words = body.trim().split(/\s+/).filter(Boolean);
  const args: Record<string, string> = {};
  const items: string[] = [];
  for (const w of words.slice(1)) {
    const eq = w.indexOf("=");
    if (eq > 0) args[w.slice(0, eq)] = w.slice(eq + 1);
    else items.push(w);
  }
  return { kind, id: words[0] ?? "", args, items };
}

/** Parameter references of an ``avas-params`` block: ``quad`` means ``lattice:quad``. */
export function paramRefs(d: Directive): { group: (typeof PARAM_GROUPS)[number]; key: string }[] {
  return [d.id, ...d.items].filter(Boolean).map((w) => {
    const [g, k] = w.includes(":") ? w.split(":", 2) : ["lattice", w];
    return { group: g as (typeof PARAM_GROUPS)[number], key: k };
  });
}

/** Problems a chapter author should fix (empty when the block is fine). */
export function directiveProblems(d: Directive): string[] {
  const out: string[] = [];
  const numeric = (k: string, v: string) => {
    if (!Number.isFinite(Number(v))) out.push(`${d.kind} ${d.id}: ${k}=${v} is not a number`);
  };
  switch (d.kind) {
    case "lab":
      if (!(LAB_IDS as readonly string[]).includes(d.id)) out.push(`unknown lab '${d.id}' (known: ${LAB_IDS.join(", ")})`);
      if (d.items.length) out.push(`lab ${d.id}: write parameters as key=value, not '${d.items.join(" ")}'`);
      for (const [k, v] of Object.entries(d.args)) numeric(k, v);
      break;
    case "params":
      for (const r of paramRefs(d)) if (!(PARAM_GROUPS as readonly string[]).includes(r.group)) out.push(`params: unknown group '${r.group}'`);
      if (Object.keys(d.args).length) out.push("params: takes keyword names only");
      break;
    case "field":
      if (d.id === "map") {
        if (d.items.length !== 1 || !FIELD_MAPS.includes(d.items[0])) out.push(`field map: name one of ${FIELD_MAPS.join(", ")}`);
      } else if (!(d.id in FIELD_MODELS)) out.push(`unknown field model '${d.id}' (known: ${Object.keys(FIELD_MODELS).join(", ")}, map)`);
      else
        for (const [k, v] of Object.entries(d.args)) {
          if (!FIELD_MODELS[d.id].includes(k)) out.push(`field ${d.id}: unknown parameter '${k}'`);
          else if (k !== "HV") numeric(k, v);
        }
      break;
    case "tutorial":
      if (!TUTORIALS.includes(d.id)) out.push(`unknown tutorial '${d.id}'`);
      break;
    case "beamline":
      // the lattice itself is checked by tests/test_learn.py (parser and linear preview)
      if (!(d.text ?? "").split("\n").some((l) => l.trim() && !l.trim().startsWith("!"))) out.push("beamline: the block has no lattice lines");
      break;
  }
  return out;
}
