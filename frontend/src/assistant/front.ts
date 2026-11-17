// What the assistant's Python side may ask the page to do ("assistant.front"
// requests): read the lattice being edited, apply an approved change to the
// open editor (one undo step) and save it, reload pages after a file changed,
// start a run exactly like the Run button (or only prepare one that Python
// starts, e.g. a segment run), switch pages and show an element, search, read
// or show the primer, and search or read the user guide (texts load on first use).
import { call, on } from "../bridge";
import { prepareRun, runSimulation } from "../actions";
import { isZh } from "../i18n";
import { setPage, useApp, type PageId } from "../store/app";
import { requestLatticeSelection } from "../store/latticeUi";
import { fileSaved, getPage, useDirty } from "../store/pages";
import { showLearn, useLearn } from "../learn/store";

const PRIMER_TEXT = 6000; // characters of one section returned to the model
const GUIDE_SNIPPET = 700; // characters of a guide section in search results

/** The in-app user guide (help/*.md: guide, cases, reference) in one language, loaded on first use. */
async function guide(lang?: string) {
  const l = lang === "en" || lang === "zh" ? lang : isZh() ? "zh" : "en";
  const [content, docs] = await Promise.all([
    import("../help/content"),
    l === "zh"
      ? Promise.all([import("../help/manual.zh.md?raw"), import("../help/cases.zh.md?raw"), import("../help/reference.zh.md?raw")])
      : Promise.all([import("../help/manual.en.md?raw"), import("../help/cases.en.md?raw"), import("../help/reference.en.md?raw")]),
  ]);
  const sections = [
    ...content.manualSections(docs[0].default, "guide"),
    ...content.manualSections(docs[1].default, "cases"),
    ...content.manualSections(docs[2].default, "reference"),
  ];
  return { content, sections };
}

async function primer(lang?: string) {
  const content = await import("../learn/content");
  const l = lang === "en" || lang === "zh" ? lang : isZh() ? "zh" : "en";
  return { content, chapters: content.loadChapters(l) };
}

export type LatticeEditorApi = {
  name: () => string | null;
  path: () => string | null;
  getText: () => string | null;
  isDirty: () => boolean;
  replaceText: (text: string) => void;
  save: () => Promise<void>;
  selection: () => { line: number; keyword: string; name: string } | null;
};

let latticeApi: LatticeEditorApi | null = null;

export function registerLatticeEditor(api: LatticeEditorApi): () => void {
  latticeApi = api;
  return () => {
    if (latticeApi === api) latticeApi = null;
  };
}

export function currentLatticeSelection(): string | null {
  const sel = latticeApi?.selection();
  if (!sel) return null;
  return `line ${sel.line + 1}: ${sel.keyword}${sel.name ? ` (${sel.name})` : ""}`;
}

type Handler = (params: any) => Promise<unknown> | unknown;

const handlers: Record<string, Handler> = {
  "lattice.getText": () => {
    if (!latticeApi || latticeApi.getText() == null) return null;
    return { name: latticeApi.name(), text: latticeApi.getText(), dirty: latticeApi.isDirty() };
  },
  "lattice.apply": async ({ name, text }: { name: string; text: string }) => {
    if (!latticeApi || latticeApi.name() !== name || latticeApi.getText() == null) return { applied: false };
    latticeApi.replaceText(text);
    await latticeApi.save();
    return { applied: true, saved: true };
  },
  "pages.reload": async ({ paths, pages }: { paths?: string[]; pages?: (string | null)[] }) => {
    for (const p of paths ?? []) fileSaved(p, "assistant");
    for (const id of pages ?? []) {
      if (!id) continue;
      const page = getPage(id);
      if (page?.reload && !page.isDirty()) await page.reload();
    }
    return true;
  },
  "pages.dirty": () =>
    Object.entries(useDirty.getState().dirty)
      .filter(([, v]) => v)
      .map(([k]) => k),
  "ui.state": () => ({
    page: useApp.getState().page,
    primer: useApp.getState().page === "learn" ? { chapter: useLearn.getState().chapter, section: useLearn.getState().anchor } : undefined,
    selection: currentLatticeSelection(),
    dirty: Object.entries(useDirty.getState().dirty)
      .filter(([, v]) => v)
      .map(([k]) => k),
  }),
  "ui.select": ({ line }: { line: number }) => {
    setPage("lattice");
    requestLatticeSelection(line);
    return true;
  },
  "run.start": async () => {
    await runSimulation();
    const run = useApp.getState().run;
    return run.running ? { started: true } : { started: false, error: "The simulation was not started (see the message in the window)." };
  },
  // validate + save + engine checks before Python starts a run itself (segment runs)
  "run.prepare": async () => {
    if (useApp.getState().run.running) return { ok: false, error: "A simulation is already running." };
    return prepareRun();
  },
  "learn.contents": async ({ lang }: { lang?: string }) => {
    const { chapters } = await primer(lang);
    return chapters.map((c) => ({ id: c.id, part: c.part, title: c.title, status: c.status, sections: c.sections.map((s) => ({ id: s.id, title: s.title })) }));
  },
  "learn.search": async ({ query, lang, limit }: { query: string; lang?: string; limit?: number }) => {
    const { content, chapters } = await primer(lang);
    return content.searchChapters(chapters, query, Math.min(limit ?? 8, 20)).map((h) => ({
      id: h.section?.id ?? h.chapter.id, chapter: h.chapter.title, section: h.section?.title ?? null, snippet: h.snippet,
    }));
  },
  "learn.read": async ({ id, lang }: { id: string; lang?: string }) => {
    const { content, chapters } = await primer(lang);
    const hit = content.locate(chapters, id);
    if (!hit) return { error: `No primer chapter or section '${id}'.` };
    const text = hit.section ? hit.section.text : [hit.chapter.intro, ...hit.chapter.sections.map((s) => `## ${s.title} {#${s.id}}\n${s.text}`)].join("\n");
    return {
      id, chapter: hit.chapter.title, section: hit.section?.title ?? null, status: hit.chapter.status,
      text: text.length > PRIMER_TEXT ? text.slice(0, PRIMER_TEXT) + "\n… (truncated; read a section id for the rest)" : text,
      sections: hit.section ? undefined : hit.chapter.sections.map((s) => ({ id: s.id, title: s.title })),
    };
  },
  "learn.show": ({ id }: { id: string }) => showLearn(id),
  "manual.search": async ({ query, lang, limit }: { query: string; lang?: string; limit?: number }) => {
    const { content, sections } = await guide(lang);
    return content.rankManual(sections, query, Math.min(limit ?? 5, 8)).map(({ section }) => ({
      id: section.id, title: section.title, part: section.category,
      snippet: section.text.length > GUIDE_SNIPPET ? section.text.slice(0, GUIDE_SNIPPET) + " …" : section.text,
    }));
  },
  "manual.read": async ({ id, lang }: { id: string; lang?: string }) => {
    const { sections } = await guide(lang);
    const section = sections.find((s) => s.id === id);
    if (!section) return { error: `No guide section '${id}'.` };
    return { id, title: section.title, part: section.category, text: section.text.length > PRIMER_TEXT ? section.text.slice(0, PRIMER_TEXT) + "\n… (truncated)" : section.text };
  },
  "ui.page": ({ page }: { page: PageId }) => {
    setPage(page);
    return true;
  },
};

export function initFrontRequests() {
  on("assistant.front", async (req: { id: string; method: string; params: unknown }) => {
    const fn = handlers[req.method];
    try {
      if (!fn) throw new Error(`Unknown page request ${req.method}`);
      const result = await fn(req.params ?? {});
      await call("assistant.frontReply", { id: req.id, ok: true, result: result ?? null });
    } catch (e: any) {
      await call("assistant.frontReply", { id: req.id, ok: false, error: e?.message ?? String(e) }).catch(() => undefined);
    }
  });
}
