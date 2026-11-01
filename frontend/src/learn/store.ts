// Which primer chapter is open, and requests to show a chapter or section
// (links, element details, the assistant).  The last chapter is remembered per
// browser (localStorage), the rest is session state.
import { create } from "zustand";
import { isZh } from "../i18n";
import { setPage } from "../store/app";
import { CHAPTERS, chapterOf } from "./chapters";
import type { LearnLang } from "./content";

const KEY = "avas.learn.chapter";

function initialChapter(): string {
  try {
    const saved = localStorage.getItem(KEY);
    if (saved && CHAPTERS.some((c) => c.id === saved)) return saved;
  } catch {
    /* storage unavailable */
  }
  return CHAPTERS[0].id;
}

export const useLearn = create<{ chapter: string; anchor: string | null; nonce: number }>(() => ({
  chapter: initialChapter(),
  anchor: null,
  nonce: 0,
}));

export function learnLang(): LearnLang {
  return isZh() ? "zh" : "en";
}

/** Open the primer page at chapter or section *id* (a chapter id, a section id or nothing). */
export function showLearn(id?: string | null): boolean {
  let chapter = useLearn.getState().chapter;
  let anchor: string | null = null;
  if (id) {
    const c = chapterOf(id);
    if (!c) return false;
    chapter = c;
    anchor = id === c ? null : id;
  }
  try {
    localStorage.setItem(KEY, chapter);
  } catch {
    /* storage unavailable */
  }
  useLearn.setState((s) => ({ chapter, anchor, nonce: s.nonce + 1 }));
  setPage("learn");
  return true;
}
