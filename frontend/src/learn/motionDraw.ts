// Canvas pictures of a primer animation (```avas-motion``` blocks): the side views
// (x–z, y–z) with every particle's path, the cross-section seen from downstream and
// the phase space at the particles' current position.  Only drawing: the
// trajectories come from the back end (avas/learn/motion.py, the linear model).
import { cssColor, niceStep } from "../util";
import type { Bi } from "../lattice/types";
import { activeElements, betaOf, forceOn } from "./fields";

type Arr = ArrayLike<number>;
export type MotionElement = {
  line: number;
  key: string;
  name: string;
  z0: number | null;
  z1: number | null;
  aperture: number | null;
  /** quadrupole gradient, T/m */
  G?: number;
  /** steerer field (T, or V/m when kind is 1) and the [z0, z1] it acts over (the element after it) */
  Bx?: number;
  By?: number;
  kind?: number;
  span?: [number, number] | null;
  /** solenoid Bz; bend: By on the reference orbit (T) */
  B?: number;
  /** bend: angle (°), radius (m), field index, 0 horizontal / 1 vertical */
  alpha?: number;
  rho?: number;
  N?: number;
  hv?: number;
  /** RF field map: on-axis Ke·Ez (MV/m) against z from the element's start, and the frequency (Hz) */
  ez?: { z: Arr; e: Arr } | null;
  f?: number;
  /** RF map: its phase (°) at the moment the reference particle is at each sample; the field is then Ke·Ez·cos(phase) */
  phase?: Arr;
};
export type MotionGhost = { name: Bi; plane: "x" | "y"; y: Arr };
export type MotionTracks = {
  s: Arr;
  /** rest mass, MeV */
  mass?: number;
  n: number;
  marked: number[];
  /** sample index where each particle hits the bore (−1: it gets through); its coordinates are NaN after it */
  lost: number[];
  charge: number;
  x: Arr;
  xp: Arr;
  y: Arr;
  yp: Arr;
  /** place in the bunch, mm (positive: ahead of its centre); the 3D view spreads the particles along the beam by it */
  z?: Arr;
  /** δ = Δp/p in ‰ */
  dp?: Arr;
  /** plane and sign each highlighted particle starts off in */
  marks?: [string, number][];
  /** draw the beam centre as a line */
  centre?: boolean;
  /** RF phase (°) the reference particle sees inside an RF map, NaN elsewhere */
  rfPhase?: Arr;
  energy?: Arr;
  cx: Arr;
  cy: Arr;
  rms_x: Arr;
  rms_y: Arr;
  elements: MotionElement[];
  monitors: { name: string; z: number }[];
  ghosts: MotionGhost[];
};
export type Plane = "x" | "y";
/** Planes of the phase-space picture: transverse, or longitudinal (z, δ). */
export type PhasePlane = Plane | "z";

/** Sample index at or below *at* and the fraction to the next sample. */
export type Loc = { lo: number; hi: number; f: number };
export function locate(s: Arr, at: number): Loc {
  const n = s.length;
  if (!n || at <= s[0]) return { lo: 0, hi: 0, f: 0 };
  if (at >= s[n - 1]) return { lo: n - 1, hi: n - 1, f: 0 };
  let lo = 0;
  let hi = n - 1;
  while (hi - lo > 1) {
    const mid = (lo + hi) >> 1;
    if (s[mid] <= at) lo = mid;
    else hi = mid;
  }
  return { lo, hi, f: (at - s[lo]) / (s[hi] - s[lo]) };
}

/** Value of particle *i* (particle-major array *a*, *ns* samples each) at *loc*. */
export function valueAt(a: Arr, ns: number, i: number, loc: Loc): number {
  const v0 = a[i * ns + loc.lo];
  return v0 + (a[i * ns + loc.hi] - v0) * loc.f;
}

export function curveAt(a: Arr, loc: Loc): number {
  return a[loc.lo] + (a[loc.hi] - a[loc.lo]) * loc.f;
}

function maxAbs(a: Arr): number {
  let m = 0;
  for (let i = 0; i < a.length; i++) {
    const v = Math.abs(a[i]);
    if (v > m && Number.isFinite(v)) m = v;
  }
  return m;
}

/** Fixed axis ranges for the whole animation, so that growing and shrinking are visible. */
export type Ranges = { x: number; y: number; xp: number; yp: number; r: number; z: number; dp: number };
export function ranges(tr: MotionTracks): Ranges {
  const nice = (v: number) => {
    const step = niceStep(Math.max(v, 1e-3) / 4);
    return Math.ceil((v * 1.08) / step) * step;
  };
  const x = nice(maxAbs(tr.x));
  const y = nice(maxAbs(tr.y));
  return {
    x,
    y,
    xp: nice(maxAbs(tr.xp)),
    yp: nice(maxAbs(tr.yp)),
    r: Math.max(x, y),
    z: tr.z ? nice(maxAbs(tr.z)) : 1,
    dp: tr.dp ? nice(maxAbs(tr.dp)) : 1,
  };
}

/** The element whose span holds *s* (zero-length elements excluded). */
export function elementAt(tr: MotionTracks, s: number): MotionElement | null {
  for (const e of tr.elements) if (e.z0 != null && e.z1 != null && e.z1 > e.z0 && s >= e.z0 && s <= e.z1) return e;
  return null;
}

/** "F" when a quadrupole pushes particles of this charge back to the axis in *plane*, "D" when it pushes them out. */
export function focusing(e: MotionElement, plane: Plane, charge: number): "F" | "D" | null {
  if (e.key !== "quad" || !e.G) return null;
  const xFocus = e.G * (charge || 1) > 0;
  return (plane === "x") === xFocus ? "F" : "D";
}

/**
 * Pole of a quadrupole in quadrant *q* (0: upper right, 1: upper left, 2: lower left, 3: lower right, seen from
 * downstream).  B = G·(y, x) points into the poles of quadrants 0 and 2, so for G > 0 they are south poles.
 */
export function quadPole(q: number, G: number): "N" | "S" {
  return (q % 2 === 1) === G > 0 ? "N" : "S";
}

function cross(ctx: CanvasRenderingContext2D, x: number, y: number, r = 4) {
  if (!Number.isFinite(x) || !Number.isFinite(y)) return;
  ctx.beginPath();
  ctx.moveTo(x - r, y - r);
  ctx.lineTo(x + r, y + r);
  ctx.moveTo(x - r, y + r);
  ctx.lineTo(x + r, y - r);
  ctx.stroke();
}

/** Draw a red cross where each particle lost before *upto* hit the wall. */
function lossMarks(ctx: CanvasRenderingContext2D, tr: MotionTracks, upto: number, at: (i: number, k: number) => [number, number], colour: string) {
  ctx.strokeStyle = colour;
  ctx.lineWidth = 1.6;
  tr.lost.forEach((k, i) => {
    if (k >= 0 && k <= upto) cross(ctx, ...at(i, k));
  });
}

export const ELEMENT_COLOR: Record<string, string> = {
  drift: "--el-drift",
  quad: "--el-quad",
  solenoid: "--el-solenoid",
  bend: "--el-bend",
  edge: "--el-bend",
  steerer: "--el-steerer",
  field: "--el-rf",
};

type Palette = { fg: string; soft: string; muted: string; border: string; bunch: string; accent: string; danger: string; el: (key: string) => string };
function palette(): Palette {
  const cache = new Map<string, string>();
  const c = (name: string, fb = "gray") => {
    if (!cache.has(name)) cache.set(name, cssColor(name, fb));
    return cache.get(name)!;
  };
  return {
    fg: c("--fg"),
    soft: c("--fg-soft"),
    muted: c("--fg-muted"),
    border: c("--border-strong"),
    bunch: c("--bunch"),
    accent: c("--accent"),
    danger: c("--danger"),
    el: (key) => c(ELEMENT_COLOR[key] ?? "--el-other"),
  };
}

/** A canvas sized to its CSS box at the device pixel ratio; returns the 2D context in CSS pixels. */
export function prepare(canvas: HTMLCanvasElement): { ctx: CanvasRenderingContext2D; w: number; h: number } | null {
  const w = canvas.clientWidth;
  const h = canvas.clientHeight;
  if (!w || !h) return null;
  const dpr = Math.min(2, window.devicePixelRatio || 1);
  if (canvas.width !== Math.round(w * dpr) || canvas.height !== Math.round(h * dpr)) {
    canvas.width = Math.round(w * dpr);
    canvas.height = Math.round(h * dpr);
  }
  const ctx = canvas.getContext("2d");
  if (!ctx) return null;
  ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  ctx.clearRect(0, 0, w, h);
  return { ctx, w, h };
}

function ticks(lo: number, hi: number, count: number): number[] {
  const step = niceStep((hi - lo) / Math.max(1, count));
  const out: number[] = [];
  for (let v = Math.ceil(lo / step - 1e-9) * step; v <= hi + step * 1e-6; v += step) out.push(Math.abs(v) < step * 1e-6 ? 0 : v);
  return out;
}

const fmtTick = (v: number) => String(Number(v.toPrecision(6)));
const FONT = "11px system-ui, sans-serif";

export type SideText = { axis: string; focusing: string; defocusing: string; zLabel: string };

/** Side view of one plane: elements, the paths flown so far, the particles at *sNow*. */
export function drawSide(canvas: HTMLCanvasElement, tr: MotionTracks, plane: Plane, sNow: number, rg: Ranges, opt: { trails: boolean; envelope: boolean; centre?: boolean; text: SideText }) {
  const c = prepare(canvas);
  if (!c) return;
  const { ctx, w, h } = c;
  const p = palette();
  const L = 50, R = 10, T = 16, B = 22;
  const ns = tr.s.length;
  const s0 = tr.s[0] ?? 0;
  const s1 = Math.max(tr.s[ns - 1] ?? 1, s0 + 1e-9);
  const yMax = plane === "x" ? rg.x : rg.y;
  const X = (s: number) => L + ((s - s0) / (s1 - s0)) * (w - L - R);
  const Y = (v: number) => T + (1 - (v + yMax) / (2 * yMax)) * (h - T - B);
  ctx.font = FONT;
  ctx.textBaseline = "middle";

  // elements
  for (const e of tr.elements) {
    if (e.z0 == null || e.z1 == null) continue;
    if (e.z1 - e.z0 <= 1e-12) {
      if (e.key === "steerer") {
        // its field fills the element after it
        if (e.span) {
          ctx.globalAlpha = 0.16;
          ctx.fillStyle = p.el("steerer");
          ctx.fillRect(X(e.span[0]), T, X(e.span[1]) - X(e.span[0]), h - T - B);
          ctx.globalAlpha = 1;
        }
        ctx.strokeStyle = p.el("steerer");
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(X(e.z0), T);
        ctx.lineTo(X(e.z0), h - B);
        ctx.stroke();
      }
      continue;
    }
    if (e.key === "drift") continue;
    ctx.globalAlpha = 0.16;
    ctx.fillStyle = p.el(e.key);
    ctx.fillRect(X(e.z0), T, X(e.z1) - X(e.z0), h - T - B);
    ctx.globalAlpha = 1;
    const fd = focusing(e, plane, tr.charge);
    if (fd) {
      const word = fd === "F" ? opt.text.focusing : opt.text.defocusing;
      const width = X(e.z1) - X(e.z0);
      const label = ctx.measureText(word).width + 4 < width ? word : fd;
      ctx.fillStyle = p.el(e.key);
      ctx.textAlign = "center";
      ctx.fillText(label, (X(e.z0) + X(e.z1)) / 2, T - 8);
    }
    // the bore of the element, when it is within the picture
    if (e.aperture && e.aperture < yMax) {
      ctx.strokeStyle = p.border;
      ctx.lineWidth = 2;
      for (const sg of [1, -1]) {
        ctx.beginPath();
        ctx.moveTo(X(e.z0), Y(sg * e.aperture));
        ctx.lineTo(X(e.z1), Y(sg * e.aperture));
        ctx.stroke();
      }
    }
  }
  for (const m of tr.monitors) {
    const x = X(m.z);
    ctx.fillStyle = p.el("diag");
    ctx.beginPath();
    ctx.moveTo(x, h - B - 9);
    ctx.lineTo(x + 4, h - B - 4);
    ctx.lineTo(x, h - B + 1);
    ctx.lineTo(x - 4, h - B - 4);
    ctx.closePath();
    ctx.fill();
  }

  // axes
  ctx.strokeStyle = p.border;
  ctx.lineWidth = 1;
  ctx.strokeRect(L, T, w - L - R, h - T - B);
  ctx.setLineDash([3, 3]);
  ctx.beginPath();
  ctx.moveTo(L, Y(0));
  ctx.lineTo(w - R, Y(0));
  ctx.stroke();
  ctx.setLineDash([]);
  ctx.fillStyle = p.muted;
  ctx.textAlign = "right";
  for (const v of ticks(-yMax, yMax, 4)) ctx.fillText(fmtTick(v), L - 4, Y(v));
  ctx.textAlign = "center";
  for (const v of ticks(s0, s1, Math.max(2, Math.floor((w - L - R) / 70)))) ctx.fillText(fmtTick(v), X(v), h - B + 11);
  ctx.save();
  ctx.translate(12, (T + h - B) / 2);
  ctx.rotate(-Math.PI / 2);
  ctx.fillText(opt.text.axis, 0, 0);
  ctx.restore();
  ctx.textAlign = "right";
  ctx.fillText(opt.text.zLabel, w - R - 2, h - B - 8);

  ctx.save();
  ctx.beginPath();
  ctx.rect(L, T, w - L - R, h - T - B);
  ctx.clip();
  const loc = locate(tr.s, sNow);
  const last = loc.lo;
  const centre = plane === "x" ? tr.cx : tr.cy;
  const rms = plane === "x" ? tr.rms_x : tr.rms_y;
  const pathOf = (v: (k: number) => number, upto: number, tail: boolean) => {
    ctx.beginPath();
    ctx.moveTo(X(tr.s[0]), Y(v(0)));
    for (let k = 1; k <= upto; k++) ctx.lineTo(X(tr.s[k]), Y(v(k)));
    if (tail && loc.f > 0) ctx.lineTo(X(sNow), Y(v(loc.lo) + (v(loc.hi) - v(loc.lo)) * loc.f));
    ctx.stroke();
  };
  for (const g of tr.ghosts) {
    if (g.plane !== plane) continue;
    ctx.strokeStyle = p.fg;
    ctx.lineWidth = 1.5;
    ctx.setLineDash([6, 4]);
    pathOf((k) => g.y[k], ns - 1, false);
    ctx.setLineDash([]);
  }
  if (opt.envelope) {
    ctx.strokeStyle = p.muted;
    ctx.lineWidth = 1;
    ctx.setLineDash([4, 3]);
    for (const sg of [1, -1]) pathOf((k) => centre[k] + sg * rms[k], ns - 1, false);
    ctx.setLineDash([]);
  }
  const a = plane === "x" ? tr.x : tr.y;
  const marked = new Set(tr.marked);
  if (opt.trails) {
    ctx.lineWidth = 1;
    ctx.strokeStyle = p.bunch;
    ctx.globalAlpha = 0.28;
    for (let i = 0; i < tr.n; i++) if (!marked.has(i)) pathOf((k) => a[i * ns + k], last, true);
    ctx.globalAlpha = 1;
    ctx.lineWidth = 2;
    ctx.strokeStyle = p.accent;
    for (const i of tr.marked) pathOf((k) => a[i * ns + k], last, true);
  }
  if (opt.centre) {
    ctx.strokeStyle = p.fg;
    ctx.lineWidth = 2;
    pathOf((k) => centre[k], ns - 1, false);
  }
  const xNow = X(sNow);
  for (let i = 0; i < tr.n; i++) {
    const big = marked.has(i);
    ctx.fillStyle = big ? p.accent : p.bunch;
    ctx.beginPath();
    ctx.arc(xNow, Y(valueAt(a, ns, i, loc)), big ? 4 : 2.4, 0, 2 * Math.PI);
    ctx.fill();
  }
  lossMarks(ctx, tr, loc.lo, (i, k) => [X(tr.s[k]), Y(a[i * ns + k])], p.danger);
  ctx.restore();
  ctx.strokeStyle = p.accent;
  ctx.globalAlpha = 0.5;
  ctx.beginPath();
  ctx.moveTo(xNow, T);
  ctx.lineTo(xNow, h - B);
  ctx.stroke();
  ctx.globalAlpha = 1;
}

function arrow(ctx: CanvasRenderingContext2D, x0: number, y0: number, x1: number, y1: number) {
  const ang = Math.atan2(y1 - y0, x1 - x0);
  ctx.beginPath();
  ctx.moveTo(x0, y0);
  ctx.lineTo(x1, y1);
  ctx.stroke();
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x1 - 7 * Math.cos(ang - 0.45), y1 - 7 * Math.sin(ang - 0.45));
  ctx.lineTo(x1 - 7 * Math.cos(ang + 0.45), y1 - 7 * Math.sin(ang + 0.45));
  ctx.closePath();
  ctx.fill();
}

export type SectionText = { schematic: string; alongOut: string; alongIn: string; rf: string };

/** Two poles (or plates) facing each other across the bore and the uniform field between them. */
function facingPoles(ctx: CanvasRenderingContext2D, p: Palette, cx: number, cy: number, R: number, bx: number, by: number, electric: boolean, col: string) {
  for (const [comp, vertical] of [[by, true], [bx, false]] as const) {
    if (!comp) continue;
    // the field runs from the N pole (+ plate) to the S pole (− plate): By > 0 → N below, S above
    const from = comp > 0 ? -1 : 1; // side of the N pole: −1 below / left, +1 above / right
    for (const side of [-1, 1]) {
      const d = R * 0.86;
      const [px, py] = vertical ? [cx, cy - side * d] : [cx + side * d, cy];
      const [w, h] = vertical ? [R * 1.1, R * 0.2] : [R * 0.2, R * 1.1];
      ctx.globalAlpha = 0.3;
      ctx.fillStyle = col;
      ctx.fillRect(px - w / 2, py - h / 2, w, h);
      ctx.globalAlpha = 1;
      ctx.fillStyle = col;
      ctx.textAlign = "center";
      const name = electric ? (side === from ? "+" : "−") : side === from ? "N" : "S";
      ctx.fillText(name, vertical ? px + w / 2 + 10 : px, vertical ? py : py - h / 2 - 9);
    }
    // field lines between the poles: thin, with a small head, in the field colour
    ctx.strokeStyle = p.muted;
    ctx.fillStyle = p.muted;
    ctx.lineWidth = 1;
    const a = R * 0.66;
    const sg = comp > 0 ? 1 : -1; // the field points up (right) for a positive component
    for (const o of [-0.4, 0, 0.4]) {
      if (vertical) arrow(ctx, cx + o * R, cy + sg * a, cx + o * R, cy - sg * a);
      else arrow(ctx, cx - sg * a, cy + o * R, cx + sg * a, cy + o * R);
    }
  }
}

/**
 * Cross-section at *sNow*, seen from downstream (the beam comes out of the screen: x to the right, y up),
 * the same view as every cross-section of AVAS.  Inside a quadrupole its poles and the force on an
 * off-axis particle are drawn schematically.
 */
export function drawSection(canvas: HTMLCanvasElement, tr: MotionTracks, sNow: number, rg: Ranges, opt: { envelope: boolean; text: SectionText }) {
  const c = prepare(canvas);
  if (!c) return;
  const { ctx, w, h } = c;
  const p = palette();
  const cx = w / 2;
  const cy = h / 2;
  const R = Math.min(w, h) / 2 - 12;
  const k = R / rg.r;
  const X = (v: number) => cx + v * k;
  const Y = (v: number) => cy - v * k;
  ctx.font = FONT;
  const e = elementAt(tr, sNow);
  // frame and axes
  ctx.strokeStyle = p.border;
  ctx.lineWidth = 1;
  ctx.setLineDash([3, 3]);
  ctx.beginPath();
  ctx.moveTo(cx - R, cy);
  ctx.lineTo(cx + R, cy);
  ctx.moveTo(cx, cy - R);
  ctx.lineTo(cx, cy + R);
  ctx.stroke();
  ctx.setLineDash([]);
  ctx.fillStyle = p.muted;
  ctx.textBaseline = "middle";
  ctx.textAlign = "right";
  ctx.fillText("x", cx + R + 8, cy - 8);
  ctx.textAlign = "left";
  ctx.fillText("y", cx + 5, cy - R + 2);
  ctx.textAlign = "left";
  ctx.fillText(`${fmtTick(rg.r)} mm`, cx + R - 30, cy + 10);
  if (e?.aperture && e.aperture <= rg.r * 1.02) {
    ctx.strokeStyle = p.border;
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(cx, cy, e.aperture * k, 0, 2 * Math.PI);
    ctx.stroke();
  }
  if (e?.key === "quad" && e.G) {
    // four poles at 45°: B = G·(y, x) points into the poles of the first and third quadrants, so for G > 0
    // those are south poles and the north poles are upper left and lower right
    const col = p.el("quad");
    for (let q = 0; q < 4; q++) {
      const ang = Math.PI / 4 + (q * Math.PI) / 2;
      const r0 = R * 0.8;
      ctx.save();
      ctx.translate(cx + r0 * Math.cos(ang), cy - r0 * Math.sin(ang));
      ctx.rotate(-ang);
      ctx.globalAlpha = 0.28;
      ctx.fillStyle = col;
      ctx.beginPath();
      ctx.moveTo(0, -R * 0.24);
      ctx.quadraticCurveTo(-R * 0.16, 0, 0, R * 0.24);
      ctx.lineTo(R * 0.3, R * 0.24);
      ctx.lineTo(R * 0.3, -R * 0.24);
      ctx.closePath();
      ctx.fill();
      ctx.globalAlpha = 1;
      ctx.restore();
      ctx.fillStyle = col;
      ctx.textAlign = "center";
      ctx.fillText(quadPole(q, e.G), cx + R * 0.88 * Math.cos(ang), cy - R * 0.88 * Math.sin(ang));
    }
    // force on a particle off axis in x and in y (positive charge: F = q v × B)
    const xIn = e.G * (tr.charge || 1) > 0;
    ctx.strokeStyle = p.danger;
    ctx.fillStyle = p.danger;
    ctx.lineWidth = 2;
    const a0 = R * 0.62;
    const a1 = R * 0.38;
    for (const sg of [1, -1]) {
      if (xIn) arrow(ctx, cx + sg * a0, cy, cx + sg * a1, cy);
      else arrow(ctx, cx + sg * a1, cy, cx + sg * a0, cy);
      if (xIn) arrow(ctx, cx, cy - sg * a1, cx, cy - sg * a0);
      else arrow(ctx, cx, cy - sg * a0, cx, cy - sg * a1);
    }
    ctx.fillStyle = p.muted;
    ctx.textAlign = "left";
    ctx.fillText(opt.text.schematic, 4, h - 8);
  }
  const loc = locate(tr.s, sNow);
  const ns = tr.s.length;
  const others = activeElements(tr, sNow).filter((a) => a.key !== "quad");
  for (const a of others) {
    ctx.textBaseline = "middle";
    if (a.key === "solenoid") {
      // the coil all around the pipe; the field runs along the beam, out of the screen for B > 0
      ctx.strokeStyle = p.el("solenoid");
      ctx.globalAlpha = 0.35;
      ctx.lineWidth = 10;
      ctx.beginPath();
      ctx.arc(cx, cy, R * 0.94, 0, 2 * Math.PI);
      ctx.stroke();
      ctx.globalAlpha = 1;
      ctx.fillStyle = p.el("solenoid");
      ctx.textAlign = "left";
      ctx.fillText(((a.B ?? 0) >= 0) === true ? opt.text.alongOut : opt.text.alongIn, 4, 10);
    } else if (a.key === "steerer" || a.key === "bend") {
      const f = a.key === "bend" ? { bx: a.hv === 1 ? -(a.B ?? 0) : 0, by: a.hv === 1 ? 0 : a.B ?? 0 } : { bx: a.Bx ?? 0, by: a.By ?? 0 };
      facingPoles(ctx, p, cx, cy, R, f.bx, f.by, a.key === "steerer" && a.kind === 1, p.el(a.key));
    } else if (a.key === "field") {
      ctx.fillStyle = p.el("field");
      ctx.textAlign = "left";
      ctx.fillText(opt.text.rf, 4, 10);
    }
  }
  if (others.length && tr.mass) {
    // the force on the highlighted particles, in the plane of the picture
    const phaseOf = (a: MotionElement) => (a.phase ? curveAt(a.phase, loc) : NaN);
    const beta = tr.energy ? betaOf(curveAt(tr.energy, loc), tr.mass) : 0.05;
    const fs = tr.marked.map((i) => {
      const x = valueAt(tr.x, ns, i, loc) / 1000;
      const y = valueAt(tr.y, ns, i, loc) / 1000;
      const F = forceOn(others, tr.charge || 1, x, y, sNow, valueAt(tr.xp, ns, i, loc) / 1000, valueAt(tr.yp, ns, i, loc) / 1000, beta, phaseOf);
      return { x, y, F };
    });
    const big = Math.max(...fs.map((f) => Math.hypot(f.F[0], f.F[1])), 0);
    ctx.strokeStyle = p.danger;
    ctx.fillStyle = p.danger;
    ctx.lineWidth = 2;
    for (const f of fs) {
      const m = Math.hypot(f.F[0], f.F[1]);
      if (!(m > 1e-9) || !Number.isFinite(f.x)) continue;
      const len = 12 + 22 * (m / big);
      const x0 = X(f.x * 1000);
      const y0 = Y(f.y * 1000);
      arrow(ctx, x0, y0, x0 + (f.F[0] / m) * len, y0 - (f.F[1] / m) * len);
    }
  }
  if (opt.envelope) {
    const mx = curveAt(tr.cx, loc);
    const my = curveAt(tr.cy, loc);
    ctx.strokeStyle = p.muted;
    ctx.lineWidth = 1;
    ctx.setLineDash([4, 3]);
    ctx.beginPath();
    ctx.ellipse(X(mx), Y(my), Math.max(0.5, curveAt(tr.rms_x, loc) * k), Math.max(0.5, curveAt(tr.rms_y, loc) * k), 0, 0, 2 * Math.PI);
    ctx.stroke();
    ctx.setLineDash([]);
  }
  const marked = new Set(tr.marked);
  for (let i = 0; i < tr.n; i++) {
    const big = marked.has(i);
    ctx.fillStyle = big ? p.accent : p.bunch;
    ctx.beginPath();
    ctx.arc(X(valueAt(tr.x, ns, i, loc)), Y(valueAt(tr.y, ns, i, loc)), big ? 4 : 2.4, 0, 2 * Math.PI);
    ctx.fill();
  }
  lossMarks(ctx, tr, loc.lo, (i, k) => [X(tr.x[i * ns + k]), Y(tr.y[i * ns + k])], p.danger);
  // ⊙: the beam comes out of the screen
  ctx.strokeStyle = p.fg;
  ctx.fillStyle = p.fg;
  ctx.lineWidth = 1.2;
  ctx.beginPath();
  ctx.arc(cx, cy, 6, 0, 2 * Math.PI);
  ctx.stroke();
  ctx.beginPath();
  ctx.arc(cx, cy, 1.8, 0, 2 * Math.PI);
  ctx.fill();
}

/** Phase space (position, angle) of one plane at *sNow*, with the rms ellipse of the drawn particles. */
export function drawPhase(canvas: HTMLCanvasElement, tr: MotionTracks, plane: PhasePlane, sNow: number, rg: Ranges, opt: { envelope: boolean }) {
  const c = prepare(canvas);
  if (!c) return;
  const { ctx, w, h } = c;
  const p = palette();
  const L = 34, R = 8, T = 8, B = 20;
  const longitudinal = plane === "z" && !!tr.z && !!tr.dp;
  const uMax = longitudinal ? rg.z : plane === "x" ? rg.x : rg.y;
  const vMax = longitudinal ? rg.dp : plane === "x" ? rg.xp : rg.yp;
  const X = (u: number) => L + ((u + uMax) / (2 * uMax)) * (w - L - R);
  const Y = (v: number) => T + (1 - (v + vMax) / (2 * vMax)) * (h - T - B);
  ctx.font = FONT;
  ctx.textBaseline = "middle";
  ctx.strokeStyle = p.border;
  ctx.lineWidth = 1;
  ctx.strokeRect(L, T, w - L - R, h - T - B);
  ctx.setLineDash([3, 3]);
  ctx.beginPath();
  ctx.moveTo(L, Y(0));
  ctx.lineTo(w - R, Y(0));
  ctx.moveTo(X(0), T);
  ctx.lineTo(X(0), h - B);
  ctx.stroke();
  ctx.setLineDash([]);
  ctx.fillStyle = p.muted;
  ctx.textAlign = "center";
  ctx.fillText(longitudinal ? "z (mm)" : `${plane} (mm)`, (L + w - R) / 2, h - 7);
  ctx.textAlign = "right";
  ctx.fillText(fmtTick(vMax), L - 3, T + 5);
  ctx.fillText(fmtTick(-vMax), L - 3, h - B - 5);
  ctx.save();
  ctx.translate(10, (T + h - B) / 2);
  ctx.rotate(-Math.PI / 2);
  ctx.textAlign = "center";
  ctx.fillText(longitudinal ? "δ (‰)" : `${plane}′ (mrad)`, 0, 0);
  ctx.restore();

  const loc = locate(tr.s, sNow);
  const ns = tr.s.length;
  const a = longitudinal ? tr.z! : plane === "x" ? tr.x : tr.y;
  const b = longitudinal ? tr.dp! : plane === "x" ? tr.xp : tr.yp;
  const us: number[] = [];
  const vs: number[] = [];
  for (let i = 0; i < tr.n; i++) {
    us.push(valueAt(a, ns, i, loc));
    vs.push(valueAt(b, ns, i, loc));
  }
  ctx.save();
  ctx.beginPath();
  ctx.rect(L, T, w - L - R, h - T - B);
  ctx.clip();
  if (opt.envelope && us.filter(Number.isFinite).length > 3) {
    // rms ellipse of the random particles (the two highlighted ones are not part of the sample)
    const idx = us.map((_, i) => i).filter((i) => !tr.marked.includes(i) && Number.isFinite(us[i]) && Number.isFinite(vs[i]));
    const m = idx.length;
    const mu = idx.reduce((s, i) => s + us[i], 0) / m;
    const mv = idx.reduce((s, i) => s + vs[i], 0) / m;
    let suu = 0, svv = 0, suv = 0;
    for (const i of idx) {
      suu += (us[i] - mu) ** 2;
      svv += (vs[i] - mv) ** 2;
      suv += (us[i] - mu) * (vs[i] - mv);
    }
    suu /= m - 1;
    svv /= m - 1;
    suv /= m - 1;
    const l11 = Math.sqrt(Math.max(suu, 0));
    const l21 = l11 > 0 ? suv / l11 : 0;
    const l22 = Math.sqrt(Math.max(svv - l21 * l21, 0));
    ctx.strokeStyle = p.muted;
    ctx.setLineDash([4, 3]);
    ctx.beginPath();
    for (let j = 0; j <= 72; j++) {
      const t = (j / 72) * 2 * Math.PI;
      const u = mu + l11 * Math.cos(t);
      const v = mv + l21 * Math.cos(t) + l22 * Math.sin(t);
      if (j === 0) ctx.moveTo(X(u), Y(v));
      else ctx.lineTo(X(u), Y(v));
    }
    ctx.stroke();
    ctx.setLineDash([]);
  }
  const marked = new Set(tr.marked);
  for (let i = 0; i < tr.n; i++) {
    const big = marked.has(i);
    ctx.fillStyle = big ? p.accent : p.bunch;
    ctx.beginPath();
    ctx.arc(X(us[i]), Y(vs[i]), big ? 4 : 2.2, 0, 2 * Math.PI);
    ctx.fill();
  }
  ctx.restore();
}
