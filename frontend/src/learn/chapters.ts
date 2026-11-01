// The primer's chapter list (small, imported by the shell and the element views).
// The texts are in ./content and only the primer page loads them (./content.ts).

export type PartId = "basics" | "elements" | "assembly";
/** "ai-draft": written by the AI assistant, not yet checked by a physicist. */
export type ReviewStatus = "ai-draft" | "reviewed";
export type ChapterMeta = { id: string; part: PartId; status: ReviewStatus; keywords?: string[] };

export const PARTS: { id: PartId; title: string }[] = [
  { id: "basics", title: "Fundamentals" },
  { id: "elements", title: "Components" },
  { id: "assembly", title: "Building an accelerator" },
];

/** In reading order.  *keywords*: lattice keywords whose element details link here. */
export const CHAPTERS: ChapterMeta[] = [
  { id: "intro", part: "basics", status: "ai-draft" },
  { id: "kinematics", part: "basics", status: "ai-draft" },
  { id: "phase-space", part: "basics", status: "ai-draft" },
  { id: "linear-optics", part: "basics", status: "ai-draft" },
  { id: "longitudinal", part: "basics", status: "ai-draft" },
  { id: "space-charge", part: "basics", status: "ai-draft" },
  { id: "drift", part: "elements", status: "ai-draft", keywords: ["drift"] },
  { id: "quadrupole", part: "elements", status: "ai-draft", keywords: ["quad"] },
  { id: "solenoid", part: "elements", status: "ai-draft", keywords: ["solenoid"] },
  { id: "dipole", part: "elements", status: "ai-draft", keywords: ["bend", "edge"] },
  { id: "steerer", part: "elements", status: "ai-draft", keywords: ["steerer"] },
  { id: "rf-cavity", part: "elements", status: "ai-draft", keywords: ["field"] },
  { id: "static-fields", part: "elements", status: "ai-draft" },
  { id: "field-maps", part: "elements", status: "ai-draft" },
  { id: "superpose", part: "elements", status: "ai-draft", keywords: ["superpose", "superposeend", "superposeout"] },
  { id: "lattice-design", part: "assembly", status: "ai-draft" },
  { id: "matching", part: "assembly", status: "ai-draft" },
  { id: "errors", part: "assembly", status: "ai-draft", keywords: ["err_step", "err_beam_dyn", "err_beam_stat", "adjust", "diag_energy", "diag_size", "diag_position"] },
  { id: "simulation", part: "assembly", status: "ai-draft", keywords: ["outputplane", "automaticoutput", "spacechargecomp"] },
  { id: "tutorial", part: "assembly", status: "ai-draft" },
  { id: "references", part: "assembly", status: "ai-draft" },
];

export function chapterForKeyword(keyword: string): string | null {
  const k = keyword.toLowerCase();
  return CHAPTERS.find((c) => c.keywords?.includes(k))?.id ?? null;
}

/** The chapter of a chapter or section id: section ids start with "<chapter>-". */
export function chapterOf(id: string): string | null {
  let best: string | null = null;
  for (const c of CHAPTERS) if ((id === c.id || id.startsWith(`${c.id}-`)) && (!best || c.id.length > best.length)) best = c.id;
  return best;
}
