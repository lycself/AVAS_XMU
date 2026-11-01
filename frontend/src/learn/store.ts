// Which primer chapter is open, and requests to show a chapter or section
// (links, element details, the assistant).  The last chapter and whether the
// chapter list and the "On this page" panel are shown are remembered per
// browser (localStorage), the rest is session state.
import { create } from "zustand";
import { isZh } from "../i18n";
import { setPage } from "../store/app";
import { CHAPTERS, chapterOf } from "./chapters";
import type { LearnLang } from "./content";

const KEY = "avas.learn.chapter";
const PANELS_KEY = "avas.learn.panels";

export type LearnPanels = { nav: boolean; toc: boolean };

function initialPanels(): LearnPanels {
  try {
    const saved = JSON.parse(localStorage.getItem(PANELS_KEY) ?? "null") as Partial<LearnPanels> | null;
    if (saved) return { nav: saved.nav !== false, toc: saved.toc !== false };
  } catch {
    /* storage unavailable */
  }
  // first visit: the right-hand panel only where there is room for it
  return { nav: true, toc: typeof window === "undefined" || window.innerWidth >= 1250 };
}

function initialChapter(): string {
  try {
    const saved = localStorage.getItem(KEY);
    if (saved && CHAPTERS.some((c) => c.id === saved)) return saved;
  } catch {
    /* storage unavailable */
  }
  return CHAPTERS[0].id;
}

export const useLearn = create<{ chapter: string; anchor: string | null; nonce: number; panels: LearnPanels }>(() => ({
  chapter: initialChapter(),
  anchor: null,
  nonce: 0,
  panels: initialPanels(),
}));

/** Show or hide the chapter list ("nav") or the "On this page" panel ("toc"). */
export function setLearnPanel(which: keyof LearnPanels, open: boolean) {
  const panels = { ...useLearn.getState().panels, [which]: open };
  useLearn.setState({ panels });
  try {
    localStorage.setItem(PANELS_KEY, JSON.stringify(panels));
  } catch {
    /* storage unavailable */
  }
}

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
