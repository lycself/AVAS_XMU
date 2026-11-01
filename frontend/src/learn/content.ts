// The accelerator primer: chapter texts, parsing and search.  Chapters are
// Markdown files in ./content (<id>.zh.md and <id>.en.md), bundled with the
// front end like the user manual.  The first line is "# Title"; every "##"
// section carries a stable id "{#chapter-topic}" that links (#id) and the
// assistant use, the same in both languages.
import { isDirectiveLang, parseDirective, type Directive } from "./directives";

export { CHAPTERS, PARTS, chapterForKeyword, chapterOf, type ChapterMeta, type PartId, type ReviewStatus } from "./chapters";
import { CHAPTERS, type ChapterMeta } from "./chapters";

export type LearnLang = "zh" | "en";
export type LearnSection = { id: string; title: string; text: string };
export type Chapter = ChapterMeta & { title: string; intro: string; sections: LearnSection[] };

const HEADING = /^(#{1,2}) (.+?)(?: \{#([\w-]+)\})?\s*$/;

/** Split a chapter file into its title, the text before the first section, and the sections. */
export function parseChapter(meta: ChapterMeta, source: string): Chapter {
  const chapter: Chapter = { ...meta, title: meta.id, intro: "", sections: [] };
  let fence = "";
  let current: LearnSection | null = null;
  for (const line of source.replace(/^﻿/, "").split(/\r?\n/)) {
    const marker = line.match(/^\s*(`{3,}|~{3,})/);
    if (marker) {
      if (!fence) fence = marker[1];
      else if (marker[1][0] === fence[0] && marker[1].length >= fence.length) fence = "";
    }
    const h = !fence && line.match(HEADING);
    if (h && h[1] === "#" && chapter.title === meta.id && !current) {
      chapter.title = h[2];
      continue;
    }
    if (h && h[1] === "##") {
      current = { id: h[3] || `${meta.id}-${chapter.sections.length + 1}`, title: h[2], text: "" };
      chapter.sections.push(current);
      continue;
    }
    if (current) current.text += line + "\n";
    else chapter.intro += line + "\n";
  }
  return chapter;
}

const files = import.meta.glob("./content/*.md", { query: "?raw", import: "default", eager: true }) as Record<string, string>;

export function chapterSource(id: string, lang: LearnLang): string | undefined {
  return files[`./content/${id}.${lang}.md`];
}

export function contentFiles(): string[] {
  return Object.keys(files).map((p) => p.replace("./content/", ""));
}

const cache = new Map<LearnLang, Chapter[]>();
export function loadChapters(lang: LearnLang): Chapter[] {
  let list = cache.get(lang);
  if (!list) {
    list = CHAPTERS.filter((m) => chapterSource(m.id, lang) ?? chapterSource(m.id, lang === "zh" ? "en" : "zh")).map((m) =>
      parseChapter(m, chapterSource(m.id, lang) ?? chapterSource(m.id, lang === "zh" ? "en" : "zh")!),
    );
    cache.set(lang, list);
  }
  return list;
}

/** The chapter holding *id* (a chapter or section id) and the section, if any. */
export function locate(chapters: Chapter[], id: string): { chapter: Chapter; section?: LearnSection } | null {
  for (const c of chapters) {
    if (c.id === id) return { chapter: c };
    const s = c.sections.find((x) => x.id === id);
    if (s) return { chapter: c, section: s };
  }
  return null;
}

export type SearchHit = { chapter: Chapter; section?: LearnSection; snippet: string };

/** Sections (or chapter intros) containing every word of *query*, best matches first. */
export function searchChapters(chapters: Chapter[], query: string, limit = 40): SearchHit[] {
  const words = query.trim().toLocaleLowerCase().split(/\s+/).filter(Boolean);
  if (!words.length) return [];
  const hits: (SearchHit & { score: number })[] = [];
  for (const chapter of chapters) {
    const parts: { section?: LearnSection; title: string; text: string }[] = [
      { title: chapter.title, text: chapter.intro },
      ...chapter.sections.map((s) => ({ section: s, title: s.title, text: s.text })),
    ];
    for (const p of parts) {
      const title = p.title.toLocaleLowerCase();
      const body = p.text.toLocaleLowerCase();
      if (!words.every((w) => title.includes(w) || body.includes(w) || chapter.title.toLocaleLowerCase().includes(w))) continue;
      const score = words.reduce((n, w) => n + (title.includes(w) ? 5 : 0) + (body.split(w).length - 1), 0);
      if (!score) continue;
      const at = Math.max(0, body.indexOf(words[0]));
      const snippet = plain(p.text.slice(Math.max(0, at - 40), at + 120));
      hits.push({ chapter, section: p.section, snippet, score });
    }
  }
  return hits.sort((a, b) => b.score - a.score).slice(0, limit);
}

/** Markdown and LaTeX markers removed, for search snippets. */
export function plain(text: string): string {
  return text
    .replace(/```[\s\S]*?```/g, " ")
    .replace(/\$\$?([^$]*)\$\$?/g, "$1")
    .replace(/[*_`#>|]/g, "")
    .replace(/\[([^\]]+)\]\([^)]*\)/g, "$1")
    .replace(/\s+/g, " ")
    .trim();
}

// ---------------------------------------------------------------- pieces of a chapter (also used by the tests)

/** Display math ($$ … $$ on its own lines) and inline math ($…$) outside code. */
export function mathSnippets(text: string): { tex: string; display: boolean }[] {
  const out: { tex: string; display: boolean }[] = [];
  const noCode = text.replace(/```[\s\S]*?```/g, "").replace(/`[^`\n]*`/g, "");
  const rest = noCode.replace(/\$\$([\s\S]+?)\$\$/g, (_m, tex: string) => {
    out.push({ tex: tex.trim(), display: true });
    return " ";
  });
  for (const m of rest.matchAll(/\$([^$\n]+)\$/g)) out.push({ tex: m[1], display: false });
  return out;
}

/** Targets of in-primer links [text](#id). */
export function linkTargets(text: string): string[] {
  const noCode = text.replace(/```[\s\S]*?```/g, "");
  return [...noCode.matchAll(/\]\(#([\w-]+)\)/g)].map((m) => m[1]);
}

/** The interactive blocks of a chapter text. */
export function chapterDirectives(text: string): Directive[] {
  const out: Directive[] = [];
  for (const m of text.matchAll(/^\s*```([\w-]+)[ \t]*\n([\s\S]*?)^\s*```\s*$/gm)) {
    if (isDirectiveLang(m[1])) {
      const d = parseDirective(m[1], m[2]);
      if (d) out.push(d);
    }
  }
  return out;
}
