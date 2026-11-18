// A beam line of the primer in 2D or 3D: the lattice editor's own LayoutView and
// Beamline3D, fed with a lattice text parsed by lattice.parse (no project
// needed) and the linear preview of it.  Labs, the tutorial and the element
// chapters (```avas-beamline``` blocks) use it; clicking an element shows the
// lattice editor's component view of it, read-only.
import { lazy, Suspense, useEffect, useMemo, useRef, useState } from "react";
import { call } from "../bridge";
import { Icon, IconButton, Segmented, Spinner } from "../components/ui";
import { pick, useT } from "../i18n";
import { ComponentView } from "../lattice/ComponentView";
import { elementType } from "../lattice/elementType";
import { LayoutView, type EnvelopeCurves, type LayoutShow } from "../lattice/LayoutView";
import { keywordMap, loadSchema, type Bi, type LatticeDoc, type Schema } from "../lattice/types";
import type { Preview } from "../lattice/usePreview";
import { useApp } from "../store/app";
import { ErrorBoundary } from "../components/ErrorBoundary";
import { useLatest } from "./useLatest";

const Beamline3D = lazy(() => import("../lattice/Beamline3D"));

type Arr = ArrayLike<number>;
/** The linear preview of the lattice, as labs and the tutorial return it. */
export type BeamlinePreview = Pick<Preview, "z" | "rms_x" | "rms_y" | "energy" | "elements">;
/** An engine result to draw over the preview (z in m, sizes in mm). */
export type BeamlineEngine = { z: Arr; rmsX: Arr; rmsY: Arr; energy?: Arr; label: string };

type Mode = "2d" | "3d";

const H2D = { min: 240, max: 420 };
const H3D = 340;

export function useSchema() {
  const [schema, setSchema] = useState<Schema | null>(null);
  useEffect(() => {
    let alive = true;
    loadSchema().then((s) => alive && setSchema(s), () => undefined);
    return () => void (alive = false);
  }, []);
  return schema;
}

/** Only mount the 3D view while it is near the screen: a chapter can hold several, and WebGL contexts are scarce. */
export function useNearScreen(ref: React.RefObject<HTMLElement | null>) {
  const [near, setNear] = useState(false);
  useEffect(() => {
    const el = ref.current;
    if (!el || typeof IntersectionObserver === "undefined") {
      setNear(true);
      return;
    }
    const io = new IntersectionObserver((entries) => setNear(entries.some((e) => e.isIntersecting)), { rootMargin: "300px 0px" });
    io.observe(el);
    return () => io.disconnect();
  }, [ref]);
  return near;
}

export function BeamlineView({ text, fieldDirs, preview, engine, caption }: {
  text: string;
  fieldDirs?: string[];
  preview?: BeamlinePreview | null;
  engine?: BeamlineEngine | null;
  caption?: string;
}) {
  const t = useT();
  const theme = useApp((s) => s.resolvedTheme);
  const schema = useSchema();
  const kw = useMemo(() => (schema ? keywordMap(schema.lattice) : null), [schema]);
  const dirs = fieldDirs ?? [];
  const { result: doc, error } = useLatest<LatticeDoc>("lattice.parse", { text, fieldDirs: dirs }, 60);
  const [mode, setMode] = useState<Mode>("2d");
  const [selected, setSelected] = useState<number | null>(null);
  const [height2d, setHeight2d] = useState(H2D.min);
  const box = useRef<HTMLDivElement>(null);
  const near = useNearScreen(box);

  // a new lattice may not have the selected line any more
  const st = doc && selected != null ? doc.statements.find((s) => s.line === selected && s.isElement) ?? null : null;
  useEffect(() => {
    if (doc && selected != null && !st) setSelected(null);
  }, [doc, selected, st]);

  const pv: Preview | null = useMemo(
    () => (preview ? { z: preview.z, rms_x: preview.rms_x, rms_y: preview.rms_y, energy: preview.energy, elements: preview.elements, warnings: [], model: {} } : null),
    [preview],
  );
  const live: EnvelopeCurves | null = useMemo(
    () => (engine ? { z: engine.z, rmsX: engine.rmsX, rmsY: engine.rmsY, energy: engine.energy } : null),
    [engine],
  );
  const gainsEnergy = !!preview && preview.energy.length > 1 && Math.abs(preview.energy[preview.energy.length - 1] - preview.energy[0]) > 1e-6;
  const show: LayoutShow = useMemo(
    () => ({ run: false, preview: true, aperture: true, losses: false, max: false, energy: gainsEnergy, scale: "beam" }),
    [gainsEnergy],
  );
  const envelope3d = useMemo(() => {
    if (engine) return { z: engine.z, x: engine.rmsX, y: engine.rmsY, label: engine.label };
    if (preview) return { z: preview.z, x: preview.rms_x, y: preview.rms_y, label: t("linear preview") };
    return null;
  }, [engine, preview, t]);
  const energyOf = st && preview ? preview.elements.find((e) => e.line === st.line) ?? null : null;
  const select = (line: number) => setSelected((cur) => (cur === line ? null : line));

  return (
    <figure className="learn-bl">
      <div className="learn-bl-bar">
        <Segmented<Mode>
          value={mode}
          onChange={setMode}
          options={[
            { value: "2d", label: "2D", tip: t("Beam-line layout with the rms envelope") },
            { value: "3d", label: "3D", tip: t("3D view of the beam line") },
          ]}
        />
        <span className="soft learn-bl-note">
          <Icon name="info" /> {t("Schematic: drawn from the lattice text; the envelope is the linear preview. Click an element for its details.")}
        </span>
      </div>
      <div ref={box} className="learn-bl-view" style={{ height: mode === "2d" ? height2d : H3D }}>
        {error ? (
          <div className="learn-lab-error">{error}</div>
        ) : !doc || !schema ? (
          <div className="empty-state"><Spinner size={20} /></div>
        ) : mode === "2d" ? (
          <LayoutView
            doc={doc}
            schema={schema}
            selected={selected}
            onSelect={select}
            run={null}
            preview={pv}
            show={show}
            live={live}
            liveLabel={engine?.label}
            readOnly
            onPreferredHeight={(h) => setHeight2d(Math.min(H2D.max, Math.max(H2D.min, h - 80)))}
          />
        ) : near ? (
          <ErrorBoundary name="3D view">
              <Suspense fallback={<div className="empty-state"><Spinner size={20} /></div>}>
            <Beamline3D doc={doc} selected={selected} onSelect={select} envelope={envelope3d} theme={theme} />
          </Suspense>
              </ErrorBoundary>
        ) : (
          <div className="empty-state soft">{t("3D view")}</div>
        )}
      </div>
      {st && kw && (
        <div className="learn-bl-detail">
          <div className="learn-bl-detail-head">
            <strong>{st.name || st.keyword}</strong>
            <span className="soft">{elementType(st, kw).label}</span>
            <code className="mono soft learn-bl-raw">{st.raw.trim()}</code>
            <IconButton icon="close" tip={t("Close")} onClick={() => setSelected(null)} />
          </div>
          <ComponentView st={st} kw={kw.get(st.key)} fieldDirs={dirs} readOnly energy={energyOf} onDraft={() => undefined} onCommit={() => undefined} />
        </div>
      )}
      {caption && <figcaption className="soft">{caption}</figcaption>}
    </figure>
  );
}

type Snippet = { lattice: string; fieldDirs: string[]; beamline: BeamlinePreview | null; warnings: Bi[] };

/** An ``avas-beamline`` block of a chapter: its lattice lines with the example beam's preview (learn.beamline). */
export function BeamlineBlock({ text }: { text: string }) {
  const [snippet, setSnippet] = useState<Snippet | null>(null);
  const [error, setError] = useState<string | null>(null);
  useEffect(() => {
    let alive = true;
    call<Snippet>("learn.beamline", { text })
      .then((r) => alive && setSnippet(r))
      .catch((e) => alive && setError(String(e?.message ?? e)));
    return () => void (alive = false);
  }, [text]);
  const t = useT();
  if (error) return <div className="learn-lab-error">{error}</div>;
  if (!snippet) return <div className="learn-bl"><div className="learn-bl-view" style={{ height: 120 }}><div className="empty-state"><Spinner size={20} /></div></div></div>;
  const caption = [
    t("Example beam: protons at 1.527 MeV from examples/hwr010."),
    ...snippet.warnings.map((w) => pick(w)),
  ].join(" ");
  return <BeamlineView text={snippet.lattice} fieldDirs={snippet.fieldDirs} preview={snippet.beamline} caption={caption} />;
}
