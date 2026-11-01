import temml from "temml";
import { describe, expect, it } from "vitest";
import {
  CHAPTERS,
  chapterDirectives,
  chapterSource,
  contentFiles,
  linkTargets,
  loadChapters,
  mathSnippets,
  parseChapter,
  searchChapters,
  type LearnLang,
} from "./content";
import { directiveProblems, parseDirective } from "./directives";
import { MACROS } from "./mathMacros";

const LANGS: LearnLang[] = ["zh", "en"];

describe("primer content", () => {
  it("has both languages of every chapter and no stray files", () => {
    const expected = CHAPTERS.flatMap((c) => LANGS.map((l) => `${c.id}.${l}.md`)).sort();
    expect(contentFiles().sort()).toEqual(expected);
  });

  it("gives every chapter a title and the same section ids in both languages", () => {
    for (const meta of CHAPTERS) {
      const [zh, en] = LANGS.map((l) => parseChapter(meta, chapterSource(meta.id, l) ?? ""));
      expect(zh.title, meta.id).not.toBe(meta.id);
      expect(en.title, meta.id).not.toBe(meta.id);
      expect(zh.sections.map((s) => s.id), meta.id).toEqual(en.sections.map((s) => s.id));
      for (const s of zh.sections) expect(s.id.startsWith(`${meta.id}-`), `${meta.id}: section id ${s.id}`).toBe(true);
    }
  });

  it("uses unique section ids", () => {
    const ids = loadChapters("en").flatMap((c) => [c.id, ...c.sections.map((s) => s.id)]);
    expect(ids.length).toBe(new Set(ids).size);
  });

  it("links only to chapters and sections that exist", () => {
    for (const lang of LANGS) {
      const chapters = loadChapters(lang);
      const ids = new Set(chapters.flatMap((c) => [c.id, ...c.sections.map((s) => s.id)]));
      for (const meta of CHAPTERS) {
        const bad = linkTargets(chapterSource(meta.id, lang) ?? "").filter((id) => !ids.has(id));
        expect(bad, `${meta.id}.${lang}.md`).toEqual([]);
      }
    }
  });

  it("writes formulas that compile to MathML", () => {
    for (const meta of CHAPTERS)
      for (const lang of LANGS) {
        const bad: string[] = [];
        for (const m of mathSnippets(chapterSource(meta.id, lang) ?? "")) {
          try {
            temml.renderToString(m.tex, { displayMode: m.display, throwOnError: true, macros: { ...MACROS } });
          } catch (e) {
            bad.push(`${m.tex} → ${String((e as Error).message ?? e).slice(0, 120)}`);
          }
        }
        expect(bad, `${meta.id}.${lang}.md`).toEqual([]);
      }
  });

  it("keeps display math on its own lines and money-free dollars", () => {
    for (const meta of CHAPTERS)
      for (const lang of LANGS) {
        const text = (chapterSource(meta.id, lang) ?? "").replace(/```[\s\S]*?```/g, "");
        const odd = text.split("\n").filter((l) => l.includes("$$") && l.trim() !== "$$" && !/^\s*\$\$.+\$\$\s*$/.test(l));
        expect(odd, `${meta.id}.${lang}.md: put $$ on its own line`).toEqual([]);
        const singles = text.replace(/\$\$[\s\S]+?\$\$/g, "").split("\n").filter((l) => (l.match(/\$/g)?.length ?? 0) % 2 === 1);
        expect(singles, `${meta.id}.${lang}.md: unbalanced $`).toEqual([]);
      }
  });

  it("uses known interactive blocks, the same ones in both languages", () => {
    for (const meta of CHAPTERS) {
      const per = LANGS.map((l) => chapterDirectives(chapterSource(meta.id, l) ?? ""));
      for (const d of per.flat()) expect(directiveProblems(d), meta.id).toEqual([]);
      expect(per[0].map((d) => `${d.kind}:${d.id}`), meta.id).toEqual(per[1].map((d) => `${d.kind}:${d.id}`));
    }
  });

  it("finds sections by search words", () => {
    const hits = searchChapters(loadChapters("en"), "quadrupole");
    expect(hits.length).toBeGreaterThan(0);
  });
});

describe("primer directives", () => {
  it("parses ids, arguments and items", () => {
    expect(parseDirective("avas-lab", "fodo G=4 Ld=0.3\n")).toEqual({ kind: "lab", id: "fodo", args: { G: "4", Ld: "0.3" }, items: [] });
    expect(parseDirective("avas-field", "map hwr010")).toEqual({ kind: "field", id: "map", args: {}, items: ["hwr010"] });
    expect(parseDirective("python", "x")).toBeNull();
  });
  it("reports unknown names and bad values", () => {
    expect(directiveProblems(parseDirective("avas-lab", "nope")!)).toHaveLength(1);
    expect(directiveProblems(parseDirective("avas-lab", "fodo G=abc")!)).toHaveLength(1);
    expect(directiveProblems(parseDirective("avas-field", "map other")!)).toHaveLength(1);
    expect(directiveProblems(parseDirective("avas-field", "quadrupole G=4 L=0.1")!)).toEqual([]);
    const bl = parseDirective("avas-beamline", "drift 0.1 0.02 0\nquad 0.1 0.02 0 6\n\n")!;
    expect(bl).toEqual({ kind: "beamline", id: "beamline", args: {}, items: [], text: "drift 0.1 0.02 0\nquad 0.1 0.02 0 6\n" });
    expect(directiveProblems(bl)).toEqual([]);
    expect(directiveProblems(parseDirective("avas-beamline", "! only a comment")!)).toHaveLength(1);
  });
});
