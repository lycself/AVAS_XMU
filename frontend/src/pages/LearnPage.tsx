// Primer page ("加速器入门"): chapters on accelerator physics and on every AVAS
// component, with interactive labs, parameter tables from the schema, field
// pictures and the guided tutorial (see src/learn).  Works without a project.
import { useEffect, useLayoutEffect, useMemo, useRef, useState } from "react";
import { Markdown, type FenceRenderer } from "../components/Markdown";
import { Button, Icon, TextInput, cx } from "../components/ui";
import { useLang, useT } from "../i18n";
import { PARTS, loadChapters, locate, searchChapters, type Chapter } from "../learn/content";
import { BeamlineBlock } from "../learn/BeamlineView";
import { parseDirective } from "../learn/directives";
import { FieldView } from "../learn/FieldView";
import { LabView } from "../learn/LabView";
import { ParamsTable } from "../learn/ParamsTable";
import { learnLang, setLearnPanel, showLearn, useLearn } from "../learn/store";
import { TutorialView } from "../learn/TutorialView";
import { askTutor } from "../learn/tutor";
import { useApp } from "../store/app";

const renderFence: FenceRenderer = (lang, body, key) => {
  const d = parseDirective(lang, body);
  if (!d) return undefined;
  switch (d.kind) {
    case "lab":
      return <LabView key={key} id={d.id} presets={d.args} />;
    case "params":
      return <ParamsTable key={key} directive={d} />;
    case "field":
      return <FieldView key={key} directive={d} />;
    case "tutorial":
      return <TutorialView key={key} />;
    case "beamline":
      return <BeamlineBlock key={key} text={d.text ?? ""} />;
  }
};

function StatusBadge({ chapter }: { chapter: Chapter }) {
  const t = useT();
  return chapter.status === "reviewed" ? (
    <span className="learn-status reviewed" data-tip={t("Checked by a physicist of the AVAS team.")}>
      <Icon name="verified" /> {t("Reviewed")}
    </span>
  ) : (
    <span className="learn-status draft" data-tip={t("Written by the AI assistant and not yet reviewed by a physicist: check formulas and numbers before relying on them.")}>
      <Icon name="sparkle" /> {t("AI-written, awaiting review")}
    </span>
  );
}

// below these widths of the page the panels float over the article instead of taking room from it
const NAV_DOCK_WIDTH = 720;
const TOC_DOCK_WIDTH = 1060;

export default function LearnPage() {
  const t = useT();
  const lang = useLang((s) => s.lang);
  const page = useApp((s) => s.page);
  const { chapter: chapterId, anchor, nonce, panels } = useLearn();
  const chapters = useMemo(() => loadChapters(learnLang()), [lang]);
  const [query, setQuery] = useState("");
  const main = useRef<HTMLDivElement>(null);
  const host = useRef<HTMLDivElement>(null);
  const [width, setWidth] = useState(1400);
  const [drawer, setDrawer] = useState<null | "nav" | "toc">(null);
  const chapter = chapters.find((c) => c.id === chapterId) ?? chapters[0];
  const index = chapters.indexOf(chapter);
  const hits = useMemo(() => (query.trim() ? searchChapters(chapters, query) : []), [chapters, query]);

  // docked panels where there is room, drawers over the article where there is not
  useLayoutEffect(() => {
    const el = host.current;
    if (!el) return;
    if (el.clientWidth) setWidth(el.clientWidth);
    const ro = new ResizeObserver(() => setWidth(el.clientWidth || 1400));
    ro.observe(el);
    return () => ro.disconnect();
  }, []);
  const floating = { nav: width < NAV_DOCK_WIDTH, toc: width < TOC_DOCK_WIDTH };

  // scroll to the requested section (or the top) once the chapter is on screen
  useEffect(() => {
    if (page !== "learn") return;
    const el = anchor ? document.getElementById(`learn-${anchor}`) : null;
    const box = main.current;
    if (!box) return;
    requestAnimationFrame(() => {
      if (el) box.scrollTop = el.offsetTop - 48;
      else box.scrollTop = 0;
    });
  }, [chapterId, anchor, nonce, page]);

  useEffect(() => {
    if (!drawer) return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") setDrawer(null);
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [drawer]);

  if (!chapter)
    return (
      <div className="page">
        <div className="page-inner">{t("The primer has no chapters.")}</div>
      </div>
    );
  const go = (id: string) => {
    setQuery("");
    setDrawer(null);
    showLearn(id);
  };
  const onClick = (e: React.MouseEvent) => {
    const link = (e.target as HTMLElement).closest("a");
    const href = link?.getAttribute("href");
    if (href?.startsWith("#")) {
      e.preventDefault();
      const id = href.slice(1);
      if (locate(chapters, id)) go(id);
    }
  };
  const prev = chapters[index - 1];
  const next = chapters[index + 1];
  const part = PARTS.find((p) => p.id === chapter.part);
  const hasToc = !query.trim() && chapter.sections.length > 1;
  const navOpen = floating.nav ? drawer === "nav" : panels.nav;
  const tocOpen = hasToc && (floating.toc ? drawer === "toc" : panels.toc);
  const toggle = (which: "nav" | "toc") => {
    if (floating[which]) setDrawer((d) => (d === which ? null : which));
    else setLearnPanel(which, !panels[which]);
  };
  const activeSection = chapter.sections.find((s) => s.id === anchor);
  return (
    <div className="learn-page" ref={host}>
      <nav className={cx("learn-nav", navOpen && "open", floating.nav && "drawer")} aria-label={t("Primer contents")} aria-hidden={!navOpen}>
        <div className="learn-panel-inner">
          <TextInput className="learn-search" placeholder={t("Search the primer")} aria-label={t("Search the primer")} value={query} onChange={(e) => setQuery(e.target.value)} />
          {PARTS.map((p) => (
            <div key={p.id} className="learn-part">
              <div className="learn-part-title">{t(p.title)}</div>
              {chapters
                .filter((c) => c.part === p.id)
                .map((c) => (
                  <button key={c.id} className={cx("learn-nav-item", c.id === chapter.id && !query && "active")} onClick={() => go(c.id)} tabIndex={navOpen ? 0 : -1}>
                    <span className="ellipsis">{c.title}</span>
                    {c.status === "ai-draft" && <span className="learn-dot" data-tip={t("AI-written, awaiting review")} />}
                  </button>
                ))}
            </div>
          ))}
        </div>
      </nav>
      <div className="learn-main" ref={main} onClick={onClick}>
        <div className="learn-bar">
          <button className={cx("learn-bar-btn", navOpen && "on")} onClick={() => toggle("nav")} data-tip={navOpen ? t("Hide the chapter list") : t("Show the chapter list")} aria-pressed={navOpen}>
            <Icon name="list-tree" />
            <span>{t("Chapters")}</span>
          </button>
          <div className="learn-crumbs ellipsis">
            {part && <span className="soft">{t(part.title)}</span>}
            {part && <Icon name="chevron-right" className="soft" />}
            <span>{chapter.title}</span>
            {activeSection && hasToc && (
              <>
                <Icon name="chevron-right" className="soft" />
                <span className="soft">{activeSection.title}</span>
              </>
            )}
          </div>
          {hasToc && (
            <button className={cx("learn-bar-btn", tocOpen && "on")} onClick={() => toggle("toc")} data-tip={tocOpen ? t("Hide \"On this page\"") : t("Show \"On this page\"")} aria-pressed={tocOpen}>
              <span>{t("On this page")}</span>
              <Icon name="list-selection" />
            </button>
          )}
        </div>
        {query.trim() ? (
          <div className="learn-article">
            <h1 className="page-title">{t("Search results")}</h1>
            {!hits.length && <p className="soft">{t("Nothing matches.")}</p>}
            {hits.map((h, i) => (
              <button key={i} className="learn-hit" onClick={() => go(h.section?.id ?? h.chapter.id)}>
                <span className="learn-hit-title">
                  {h.chapter.title}
                  {h.section && <> › {h.section.title}</>}
                </span>
                <span className="soft">{h.snippet}</span>
              </button>
            ))}
          </div>
        ) : (
          <article className="learn-article selectable" key={`${chapter.id}-${lang}`}>
            <header className="learn-head">
              <div className="learn-kicker">{part ? t(part.title) : ""}</div>
              <h1 className="page-title">{chapter.title}</h1>
              <div className="learn-head-row">
                <StatusBadge chapter={chapter} />
                <span className="grow" />
                <Button small icon="sparkle" onClick={() => askTutor(t("Explain the primer chapter \"{title}\" to me: the key ideas, the equations that matter and how they show up in AVAS.", { title: chapter.title }))}>
                  {t("Ask the tutor")}
                </Button>
              </div>
            </header>
            {chapter.intro.trim() && <Markdown text={chapter.intro} fence={renderFence} />}
            {chapter.sections.map((s) => (
              <section key={s.id} id={`learn-${s.id}`} className={cx("learn-section", s.id === `${chapter.id}-in-avas` && "learn-reference")}>
                {s.id === `${chapter.id}-in-avas` && (
                  <div className="learn-reference-label">
                    <Icon name="book" /> {t("Reference: how AVAS writes it")}
                  </div>
                )}
                <h2>{s.title}</h2>
                <Markdown text={s.text} fence={renderFence} />
              </section>
            ))}
            <footer className="learn-foot">
              {prev ? (
                <Button icon="arrow-left" onClick={() => go(prev.id)}>
                  {prev.title}
                </Button>
              ) : (
                <span />
              )}
              {next && (
                <Button icon="arrow-right" onClick={() => go(next.id)}>
                  {next.title}
                </Button>
              )}
            </footer>
          </article>
        )}
      </div>
      {hasToc && (
        <nav className={cx("learn-toc", tocOpen && "open", floating.toc && "drawer")} aria-label={t("On this page")} aria-hidden={!tocOpen}>
          <div className="learn-panel-inner">
            <div className="learn-part-title">{t("On this page")}</div>
            {chapter.sections.map((s) => (
              <button key={s.id} className={cx("learn-toc-item", anchor === s.id && "active")} onClick={() => go(s.id)} tabIndex={tocOpen ? 0 : -1}>
                {s.title}
              </button>
            ))}
          </div>
        </nav>
      )}
      {drawer && <div className="learn-scrim" onClick={() => setDrawer(null)} />}
    </div>
  );
}
