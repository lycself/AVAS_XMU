// React wrapper of the element close-up (closeupScene.ts) used by the primer
// animations.  Load it lazily (three.js stays out of the main bundle); the caller
// moves the particles with viewer.setS through *onViewer*.
import { useEffect, useRef, useState } from "react";
import { Checkbox, Icon, IconButton } from "../components/ui";
import { useT } from "../i18n";
import { useApp } from "../store/app";
import { CloseUp, type CloseUpView } from "./closeupScene";
import type { MotionTracks } from "./motionDraw";

export type { CloseUp } from "./closeupScene";

export default function CloseUp3D({ tracks, trails, onViewer }: { tracks: MotionTracks | null; trails: boolean; onViewer?: (v: CloseUp | null) => void }) {
  const t = useT();
  const theme = useApp((s) => s.resolvedTheme);
  const host = useRef<HTMLDivElement>(null);
  const viewer = useRef<CloseUp | null>(null);
  const onViewerRef = useRef(onViewer);
  onViewerRef.current = onViewer;
  const [failed, setFailed] = useState(false);
  const [follow, setFollow] = useState(true);
  const [field, setField] = useState(true);
  const [forces, setForces] = useState(true);
  const [cutaway, setCutaway] = useState(true);

  useEffect(() => {
    let v: CloseUp;
    try {
      v = new CloseUp(host.current!);
    } catch (err) {
      console.warn("3D close-up unavailable:", err);
      setFailed(true);
      return;
    }
    viewer.current = v;
    (host.current as any).__closeup = v; // for automated checks
    onViewerRef.current?.(v);
    return () => {
      onViewerRef.current?.(null);
      v.dispose();
      viewer.current = null;
    };
  }, []);
  useEffect(() => viewer.current?.setTracks(tracks), [tracks]);
  useEffect(() => viewer.current?.setOptions({ trails, field, forces, cutaway, follow }), [trails, field, forces, cutaway, follow]);
  useEffect(() => viewer.current?.setTheme(), [theme]);

  const keys = new Set((tracks?.elements ?? []).map((e) => (e.key === "steerer" && e.kind === 1 ? "esteerer" : e.key)));
  const magnetic = ["quad", "solenoid", "steerer", "bend"].some((k) => keys.has(k));
  const electric = keys.has("field") || keys.has("esteerer");

  const view = (name: CloseUpView) => {
    viewer.current?.view(name);
    setFollow(name !== "whole");
  };

  if (failed)
    return (
      <div className="empty-state">
        <Icon name="warning" /> {t("The 3D view is unavailable because WebGL could not be started.")}
      </div>
    );
  return (
    <div className="closeup" ref={host}>
      <div className="closeup-bar" onPointerDown={(e) => e.stopPropagation()}>
        <IconButton icon="zoom-in" tip={t("Zoom in")} onClick={() => viewer.current?.zoom(1)} />
        <IconButton icon="zoom-out" tip={t("Zoom out")} onClick={() => viewer.current?.zoom(-1)} />
        <div className="segmented">
          <button type="button" onClick={() => view("oblique")} data-tip={t("Look at the element from the side and from downstream")}>
            {t("Oblique")}
          </button>
          <button type="button" onClick={() => view("beam")} data-tip={t("Look along the beam from downstream, like the cross-section")}>
            {t("Along the beam")}
          </button>
          <button type="button" onClick={() => view("side")} data-tip={t("Look from the side (+x)")}>
            {t("Side")}
          </button>
          <button type="button" onClick={() => view("whole")} data-tip={t("The whole line at once")}>
            {t("Whole line")}
          </button>
        </div>
        <Checkbox checked={follow} onChange={setFollow} label={t("Follow the particles")} />
        <Checkbox checked={field} onChange={setField} label={t("Field")} tip={t("Field lines with cones for the direction of the field: where the particles are, or all through a solenoid or cavity")} />
        <Checkbox checked={forces} onChange={setForces} label={t("Force")} tip={t("Force F = qv × B on the two highlighted particles")} />
        <Checkbox checked={cutaway} onChange={setCutaway} label={t("Cut open")} tip={t("Leave out the upper-right quarter of the magnet (pole, coil and yoke) to see into the bore")} />
      </div>
      {(field || forces) && (
        <div className="closeup-key">
          {field && magnetic && (
            <span className="closeup-key-b">
              <svg width="26" height="10" aria-hidden="true">
                <line x1="1" y1="5" x2="25" y2="5" />
                <path d="M10 1 L16 5 L10 9 Z" />
              </svg>
              {t("Magnetic field B (field line; the cones give its direction)")}
            </span>
          )}
          {field && electric && (
            <span className="closeup-key-b">
              <svg width="26" height="10" aria-hidden="true">
                <line x1="1" y1="5" x2="25" y2="5" />
                <path d="M10 1 L16 5 L10 9 Z" />
              </svg>
              {t("Electric field E (the cones show its direction at this moment)")}
            </span>
          )}
          {forces && (
            <span className="closeup-key-f">
              <svg width="26" height="10" aria-hidden="true">
                <line x1="1" y1="5" x2="19" y2="5" />
                <path d="M18 1 L25 5 L18 9 Z" />
              </svg>
              {t("Force F = qv × B on a highlighted particle")}
            </span>
          )}
        </div>
      )}
      <div className="closeup-note soft">{t("Positions to scale; the particles are spread along the beam by their place in the bunch and drawn larger than life · drag to rotate, wheel to zoom, right-drag to pan")}</div>
    </div>
  );
}
