// Element close-up in 3D for the primer animations (```avas-motion```): the
// elements drawn to scale as an accelerator physicist pictures them — the four
// hyperbolic poles of a quadrupole, the coil and iron of a solenoid, the window frame
// of a steerer, the H magnet of a bend following its arc, the tank, inner conductor
// and gaps of a half-wave RF cavity, the vacuum pipe and BPM buttons — and the
// particles of the animation flying through at their true transverse positions.
//
// Fields are drawn as thin field lines with small cones for their direction: in the
// plane of the particles for a quadrupole, steerer or bend (that plane travels with
// the bunch), along the axis and back through the iron for a solenoid, across the
// gaps for an RF cavity (the cones turn and grow with the RF phase of the moment).
// The two highlighted particles carry the force on them (a pink arrow with an "F"),
// computed from the same field formulas (./fields).  The camera follows the bunch;
// the reader can orbit and zoom.
//
// The beam's coordinates are x to the right, y up, s along the orbit, in metres;
// ./closeupPath maps them to the world (the identity on a straight line, so that
// seen from +z this is the AVAS cross-section convention: from downstream, beam
// coming out of the screen).  Rendering is on demand.  Only drawing: the
// trajectories come from the back end (avas/learn/motion.py, the linear model).
import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";
import { cssColor } from "../util";
import { BeamPath, OrbitCurve } from "./closeupPath";
import { actingSpan, activeElements, betaOf, forceOn, rfProfile } from "./fields";
import { curveAt, locate, quadPole, type Loc, type MotionElement, type MotionTracks } from "./motionDraw";

export type CloseUpView = "oblique" | "beam" | "side" | "whole";
export type CloseUpOptions = { trails: boolean; field: boolean; forces: boolean; cutaway: boolean; follow: boolean };

const COLOR_VARS = [
  "--beamline-bg", "--el-drift", "--el-quad", "--el-steerer", "--el-diag", "--el-solenoid", "--el-bend", "--el-rf",
  "--pole-n", "--pole-s", "--coil", "--iron", "--field-arrow", "--force-arrow", "--bunch", "--accent", "--fg-soft",
] as const;
type ColorVar = (typeof COLOR_VARS)[number];

const DEFAULT_APERTURE = 0.02; // m
// Drawn radius of a particle (the tracked ones larger).  Small next to the spacing of the particles, which
// are also spread along the beam by their place in the bunch, so that each reads as a ball of its own;
// the close default camera makes them large on screen, and they never shrink below a few pixels.
const PARTICLE_R = 0.00032; // m
const MARKED_R = 0.0006;
const PARTICLE_PX = 3; // least radius on screen, in CSS pixels
const MARKED_PX = 5;
/** Camera offsets from the followed point in the orbit's frame (multiples of the pipe radius): close to the
 *  beam, with the pole tips at the edge of the picture.  The oblique camera sits in the cut-away quarter. */
const VIEWS: Record<Exclude<CloseUpView, "whole">, [number, number, number]> = {
  oblique: [1.15, 0.9, 1.4],
  beam: [0.12, 0.09, 3.2],
  side: [5, 0.25, 0.01],
};
/** A long coil or a cavity tank would wall the close camera in: there it starts outside, this much farther. */
const OUTSIDE: Record<string, number> = { solenoid: 3.5, field: 2.4 };
/** Without the cut-away the oblique camera would sit inside the iron: back it off this much. */
const UNCUT = 2.6;
/** The cut-away quarter (x > 0, y > 0) as the angles a shell keeps. */
const KEEP: [number, number] = [Math.PI / 2, 2 * Math.PI];

function readColors(): Map<ColorVar, THREE.Color> {
  const out = new Map<ColorVar, THREE.Color>();
  for (const name of COLOR_VARS) {
    const c = new THREE.Color(0.5, 0.5, 0.5);
    const v = cssColor(name);
    if (v) {
      try {
        c.setStyle(v);
      } catch {
        /* keep grey */
      }
    }
    out.set(name, c);
  }
  return out;
}

/** A text label that always faces the camera. */
function label(text: string, colour: THREE.Color, size: number): THREE.Sprite {
  const c = document.createElement("canvas");
  c.width = c.height = 128;
  const ctx = c.getContext("2d")!;
  ctx.font = "bold 92px system-ui, sans-serif";
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  ctx.fillStyle = `#${colour.getHexString()}`;
  ctx.fillText(text, 64, 70);
  const tex = new THREE.CanvasTexture(c);
  const sprite = new THREE.Sprite(new THREE.SpriteMaterial({ map: tex, depthTest: false, transparent: true }));
  sprite.scale.setScalar(size);
  sprite.renderOrder = 20;
  return sprite;
}

/** One pole of a quadrupole in its own frame: u along the pole axis, v across, extruded along z over *length*. */
function poleGeometry(R: number, length: number): THREE.ExtrudeGeometry {
  // the pole face is the hyperbola u² − v² = R² (x·y = R²/2 turned by 45°)
  const w = 0.55 * R;
  const uOut = 2.1 * R;
  const shape = new THREE.Shape();
  const n = 16;
  for (let i = 0; i <= n; i++) {
    const v = -w + (2 * w * i) / n;
    const u = Math.sqrt(R * R + v * v);
    if (i === 0) shape.moveTo(u, v);
    else shape.lineTo(u, v);
  }
  shape.lineTo(uOut, w);
  shape.lineTo(uOut, -w);
  shape.closePath();
  return new THREE.ExtrudeGeometry(shape, { depth: length, bevelEnabled: false, curveSegments: 4 });
}

/** An annulus from r1 to r2 over the angles a0…a1 (in the x–y plane), extruded along z over *length*. */
function shellGeometry(r1: number, r2: number, a0: number, a1: number, length: number): THREE.ExtrudeGeometry {
  const shape = new THREE.Shape();
  shape.absarc(0, 0, r2, a0, a1, false);
  shape.lineTo(r1 * Math.cos(a1), r1 * Math.sin(a1));
  shape.absarc(0, 0, r1, a1, a0, true);
  shape.closePath();
  return new THREE.ExtrudeGeometry(shape, { depth: length, bevelEnabled: false, curveSegments: 48 });
}

/** A box of cross-section [x0, x1] × [y0, y1] swept along the orbit from s0 to s1 (follows a bend). */
function sweptBox(path: BeamPath, s0: number, s1: number, x0: number, x1: number, y0: number, y1: number, n: number): THREE.BufferGeometry {
  const corners: [number, number][] = [[x0, y0], [x1, y0], [x1, y1], [x0, y1]];
  const ring = (s: number) => corners.map(([x, y]) => path.toWorld(s, x, y));
  const pos: number[] = [];
  const quad = (a: THREE.Vector3, b: THREE.Vector3, c: THREE.Vector3, d: THREE.Vector3) => {
    for (const v of [a, b, c, a, c, d]) pos.push(v.x, v.y, v.z);
  };
  let prev = ring(s0);
  quad(prev[3], prev[2], prev[1], prev[0]);
  for (let k = 1; k <= n; k++) {
    const cur = ring(s0 + ((s1 - s0) * k) / n);
    for (let j = 0; j < 4; j++) quad(prev[j], prev[(j + 1) % 4], cur[(j + 1) % 4], cur[j]);
    prev = cur;
  }
  quad(prev[0], prev[1], prev[2], prev[3]);
  const g = new THREE.BufferGeometry();
  g.setAttribute("position", new THREE.Float32BufferAttribute(pos, 3));
  g.computeVertexNormals();
  return g;
}

/** Field lines of B = G·(y, x) in a plane: the hyperbolas y² − x² = c inside radius *r*. */
function quadLinePoints(r: number): THREE.Vector3[][] {
  const lines: THREE.Vector3[][] = [];
  for (const c of [0.12, 0.35, 0.7]) {
    for (const sign of [1, -1]) {
      // y² − x² = ±c·r²: branches opening along y (sign +) or along x (sign −)
      for (const branch of [1, -1]) {
        const pts: THREE.Vector3[] = [];
        for (let i = -40; i <= 40; i++) {
          const t = (i / 40) * r;
          const a = Math.sqrt(c * r * r + t * t) * branch;
          const [x, y] = sign > 0 ? [t, a] : [a, t];
          if (Math.hypot(x, y) <= r) pts.push(new THREE.Vector3(x, y, 0));
        }
        if (pts.length > 1) lines.push(pts);
      }
    }
  }
  return lines;
}

/** Parallel field lines of a uniform field along (ux, uy) across a disc of radius *r*. */
function uniformLinePoints(r: number, ux: number, uy: number): THREE.Vector3[][] {
  const lines: THREE.Vector3[][] = [];
  for (const o of [-0.6, -0.3, 0, 0.3, 0.6]) {
    const half = Math.sqrt(Math.max(r * r - (o * r) ** 2, 0));
    const px = -uy * o * r;
    const py = ux * o * r;
    lines.push([new THREE.Vector3(px - ux * half, py - uy * half, 0), new THREE.Vector3(px + ux * half, py + uy * half, 0)]);
  }
  return lines;
}

const UNIT_SHAFT = new THREE.CylinderGeometry(1, 1, 1, 12);
const UNIT_HEAD = new THREE.ConeGeometry(1, 1, 16);
const UP = new THREE.Vector3(0, 1, 0);
const isShared = (g: unknown) => g === UNIT_SHAFT || g === UNIT_HEAD;

/** A solid arrow (cylinder shaft and cone head) that can be re-aimed every frame; WebGL lines are one pixel thin. */
class SolidArrow extends THREE.Group {
  private shaft: THREE.Mesh;
  private head: THREE.Mesh;
  constructor(material: THREE.Material) {
    super();
    this.shaft = new THREE.Mesh(UNIT_SHAFT, material);
    this.head = new THREE.Mesh(UNIT_HEAD, material);
    this.add(this.shaft, this.head);
  }
  /** Arrow from *origin* along *dir* (normalised here) of total *length*, shaft radius *r*. */
  aim(origin: THREE.Vector3, dir: THREE.Vector3, length: number, r: number) {
    const head = Math.min(length * 0.45, r * 5);
    const shaft = Math.max(length - head, 1e-6);
    this.position.copy(origin);
    this.quaternion.setFromUnitVectors(UP, dir.clone().normalize());
    this.shaft.scale.set(r, shaft, r);
    this.shaft.position.set(0, shaft / 2, 0);
    this.head.scale.set(r * 2.6, head, r * 2.6);
    this.head.position.set(0, shaft + head / 2, 0);
  }
}

/** A cone marking a field direction, sitting at a fixed point of its group. */
type Mark = { cone: THREE.Mesh; at: THREE.Vector3; size: number };
/** A field picture: in the plane of the particles (quad, steerer, bend), fixed in space (solenoid) or
 *  fixed with cones that follow the RF phase (cavity). */
type FieldPicture = { e: MotionElement; kind: "plane" | "static" | "rf"; group: THREE.Group; marks: Mark[]; gaps?: number[] };

export class CloseUp {
  private renderer: THREE.WebGLRenderer;
  private scene = new THREE.Scene();
  private camera = new THREE.PerspectiveCamera(38, 1, 0.0005, 50);
  private controls: OrbitControls;
  private statics = new THREE.Group();
  private moving = new THREE.Group();
  private colors = readColors();
  private mats = {
    pipe: new THREE.MeshStandardMaterial({ transparent: true, opacity: 0.14, depthWrite: false, side: THREE.DoubleSide, roughness: 0.3 }),
    poleN: new THREE.MeshStandardMaterial({ roughness: 0.45, metalness: 0.15, side: THREE.DoubleSide }),
    poleS: new THREE.MeshStandardMaterial({ roughness: 0.45, metalness: 0.15, side: THREE.DoubleSide }),
    coil: new THREE.MeshStandardMaterial({ roughness: 0.35, metalness: 0.6, side: THREE.DoubleSide }),
    iron: new THREE.MeshStandardMaterial({ roughness: 0.75, metalness: 0.2, side: THREE.DoubleSide }),
    steerer: new THREE.MeshStandardMaterial({ roughness: 0.4, metalness: 0.4 }),
    diag: new THREE.MeshStandardMaterial({ roughness: 0.4, metalness: 0.5 }),
    flange: new THREE.MeshStandardMaterial({ roughness: 0.5, metalness: 0.5 }),
    // the RF tank: see-through, so that the inner conductor and the gaps stay visible
    tank: new THREE.MeshStandardMaterial({ transparent: true, opacity: 0.2, depthWrite: false, side: THREE.DoubleSide, roughness: 0.4, metalness: 0.3 }),
    // shaded, so that overlapping particles still read as separate balls
    particle: new THREE.MeshLambertMaterial(),
    marked: new THREE.MeshLambertMaterial(),
    trail: new THREE.LineBasicMaterial({ transparent: true, opacity: 0.45 }),
    markedTrail: new THREE.LineBasicMaterial(),
    plane: new THREE.MeshBasicMaterial({ transparent: true, opacity: 0.16, depthWrite: false, side: THREE.DoubleSide }),
    fieldLine: new THREE.MeshBasicMaterial({ transparent: true, opacity: 0.5 }),
    fieldArrow: new THREE.MeshBasicMaterial(),
    // drawn over everything: the force on a particle at the edge of the bunch points into the bunch
    forceArrow: new THREE.MeshBasicMaterial({ depthTest: false }),
  };
  private tracks: MotionTracks | null = null;
  private path = new BeamPath([]);
  private opts: CloseUpOptions = { trails: true, field: true, forces: true, cutaway: true, follow: true };
  private s = 0;
  private followed = 0; // the s the camera was last moved to
  private R = DEFAULT_APERTURE;
  private viewScale = 1;
  private dots: THREE.InstancedMesh | null = null;
  private marks: THREE.InstancedMesh | null = null;
  private trails: THREE.Line[] = [];
  private pictures: FieldPicture[] = [];
  private forceArrows: { arrow: SolidArrow; tag: THREE.Sprite }[] = [];
  /** largest force on the highlighted particles in each element over the whole line: scales the arrows */
  private forceScale = new Map<MotionElement, number>();
  private frameReq = 0;
  private disposed = false;
  private ro: ResizeObserver;
  private mo: MutationObserver;

  constructor(private host: HTMLElement) {
    this.renderer = new THREE.WebGLRenderer({ antialias: true, alpha: false });
    if (!this.renderer.getContext()) throw new Error("no WebGL context");
    this.renderer.setPixelRatio(Math.min(2, window.devicePixelRatio || 1));
    const canvas = this.renderer.domElement;
    canvas.className = "closeup-canvas";
    host.prepend(canvas);
    this.scene.add(new THREE.HemisphereLight(0xffffff, 0x666666, 2.2));
    const sun = new THREE.DirectionalLight(0xffffff, 1.6);
    sun.position.set(1, 2, 1.5);
    this.scene.add(sun, this.statics, this.moving);
    this.controls = new OrbitControls(this.camera, canvas);
    this.controls.enableDamping = false;
    this.controls.zoomToCursor = true;
    this.controls.addEventListener("change", this.invalidate);
    this.applyColors();
    this.ro = new ResizeObserver(() => this.resize());
    this.ro.observe(host);
    this.mo = new MutationObserver(() => this.setTheme());
    this.mo.observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme", "class"] });
    this.resize();
  }

  /* ------------------------------------------------------------ public API */
  setTracks(tr: MotionTracks | null) {
    const first = !this.tracks;
    this.tracks = tr;
    this.path = new BeamPath(tr?.elements ?? []);
    this.rebuild();
    if (first && tr) this.view("oblique");
    this.invalidate();
  }

  setOptions(o: Partial<CloseUpOptions>) {
    const cut = o.cutaway !== undefined && o.cutaway !== this.opts.cutaway;
    const trails = o.trails !== undefined && o.trails !== this.opts.trails;
    this.opts = { ...this.opts, ...o };
    if (cut) {
      this.buildStatics();
      this.clearIron();
    }
    if (trails) this.buildMoving();
    if (o.follow) this.followed = this.s;
    this.invalidate();
  }

  /** Arc length (m) the particles have reached; the camera moves along when following. */
  setS(s: number) {
    if (this.opts.follow && s !== this.followed) {
      // keep the camera where it is relative to the orbit's frame (a bend turns it with the beam)
      const p0 = this.path.toWorld(this.followed, 0, 0);
      const q0 = this.path.frame(this.followed);
      const p1 = this.path.toWorld(s, 0, 0);
      const turn = this.path.frame(s).multiply(q0.invert());
      for (const v of [this.camera.position, this.controls.target]) v.sub(p0).applyQuaternion(turn).add(p1);
      this.camera.up.applyQuaternion(turn);
      this.controls.update();
      this.followed = s;
    }
    this.s = s;
    this.invalidate();
  }

  view(name: CloseUpView) {
    const tr = this.tracks;
    if (!tr || !tr.s.length) return;
    if (name === "whole") {
      const box = new THREE.Box3();
      const s0 = tr.s[0];
      const s1 = tr.s[tr.s.length - 1];
      for (let k = 0; k <= 40; k++) box.expandByPoint(this.path.toWorld(s0 + ((s1 - s0) * k) / 40, 0, 0));
      const mid = box.getCenter(new THREE.Vector3());
      const len = Math.max(box.getSize(new THREE.Vector3()).length(), 0.1);
      this.controls.target.copy(mid);
      this.camera.position.copy(mid).add(new THREE.Vector3(0.55, 0.32, 0.35).normalize().multiplyScalar(len * 1.15));
      this.camera.up.set(0, 1, 0);
      this.opts.follow = false;
    } else {
      const [ox, oy, oz] = VIEWS[name];
      const R = this.R * (name === "oblique" ? this.viewScale : 1);
      this.controls.target.copy(this.path.toWorld(this.s, 0, 0));
      this.camera.position.copy(this.controls.target).add(this.path.dir(this.s, new THREE.Vector3(ox * R, oy * R, oz * R)));
      this.camera.up.copy(this.path.dir(this.s, UP));
      this.followed = this.s;
    }
    this.controls.update();
    this.clearIron();
    this.invalidate();
  }

  /** With the cut-away off, a camera inside the magnet's outline backs off along its line of sight. */
  private clearIron() {
    if (this.opts.cutaway) return;
    const t = this.controls.target;
    const off = this.camera.position.clone().sub(t);
    if (off.length() > 4 * this.R * this.viewScale) return;
    this.camera.position.copy(t).add(off.multiplyScalar(UNCUT));
    this.controls.update();
  }

  zoom(direction: 1 | -1) {
    const t = this.controls.target;
    const off = this.camera.position.clone().sub(t).multiplyScalar(direction > 0 ? 0.8 : 1.25);
    this.camera.position.copy(t).add(off);
    this.controls.update();
    this.invalidate();
  }

  get following() {
    return this.opts.follow;
  }

  setTheme() {
    this.colors = readColors();
    this.applyColors();
    this.buildStatics(); // labels are textures in the theme's colours
    this.buildMoving();
    this.invalidate();
  }

  dispose() {
    this.disposed = true;
    cancelAnimationFrame(this.frameReq);
    this.ro.disconnect();
    this.mo.disconnect();
    this.controls.removeEventListener("change", this.invalidate);
    this.controls.dispose();
    this.clear(this.statics);
    this.clear(this.moving);
    for (const m of Object.values(this.mats)) m.dispose();
    this.renderer.dispose();
    this.renderer.forceContextLoss();
    this.renderer.domElement.remove();
  }

  /* ------------------------------------------------------------ scene */
  invalidate = () => {
    if (!this.frameReq && !this.disposed) this.frameReq = requestAnimationFrame(this.frame);
  };

  private frame = () => {
    this.frameReq = 0;
    if (this.disposed) return;
    this.update();
    this.renderer.render(this.scene, this.camera);
  };

  private resize() {
    const w = Math.max(1, this.host.clientWidth);
    const h = Math.max(1, this.host.clientHeight);
    this.renderer.setSize(w, h, false);
    this.camera.aspect = w / h;
    this.camera.updateProjectionMatrix();
    this.invalidate();
  }

  private c(name: ColorVar) {
    return this.colors.get(name)!;
  }

  private applyColors() {
    this.renderer.setClearColor(this.c("--beamline-bg"), 1);
    const m = this.mats;
    m.pipe.color.copy(this.c("--el-drift"));
    m.poleN.color.copy(this.c("--pole-n"));
    m.poleS.color.copy(this.c("--pole-s"));
    m.coil.color.copy(this.c("--coil"));
    m.iron.color.copy(this.c("--iron"));
    m.steerer.color.copy(this.c("--el-steerer"));
    m.diag.color.copy(this.c("--el-diag"));
    m.flange.color.copy(this.c("--iron"));
    m.tank.color.copy(this.c("--el-rf"));
    m.particle.color.copy(this.c("--bunch"));
    m.marked.color.copy(this.c("--accent"));
    m.trail.color.copy(this.c("--bunch"));
    m.markedTrail.color.copy(this.c("--accent"));
    m.plane.color.copy(this.c("--field-arrow"));
    m.fieldLine.color.copy(this.c("--field-arrow"));
    m.fieldArrow.color.copy(this.c("--field-arrow"));
    m.forceArrow.color.copy(this.c("--force-arrow"));
  }

  private clear(group: THREE.Group) {
    for (const child of [...group.children]) {
      group.remove(child);
      child.traverse((o) => {
        const any = o as any;
        if (any.geometry && !isShared(any.geometry)) any.geometry.dispose();
        if (o instanceof THREE.Sprite) {
          o.material.map?.dispose();
          o.material.dispose();
        }
      });
    }
  }

  private rebuild() {
    const tr = this.tracks;
    const ap = tr?.elements.map((e) => e.aperture ?? 0).filter((a) => a > 0) ?? [];
    this.R = ap.length ? Math.max(...ap) / 1000 : DEFAULT_APERTURE;
    this.viewScale = Math.max(1, ...(tr?.elements ?? []).map((e) => OUTSIDE[e.key] ?? 1));
    this.forceScale = this.measureForces();
    this.buildStatics();
    this.buildMoving();
  }

  /** A group placed at the start *s* of a straight element, its local z along the orbit. */
  private placed(s: number): THREE.Group {
    const g = new THREE.Group();
    g.position.copy(this.path.toWorld(s, 0, 0));
    g.quaternion.copy(this.path.frame(s));
    return g;
  }

  private tube(pts: THREE.Vector3[], r: number, closed = false): THREE.Mesh {
    const curve = pts.length === 2 ? new THREE.LineCurve3(pts[0], pts[1]) : new THREE.CatmullRomCurve3(pts, closed, "centripetal");
    return new THREE.Mesh(new THREE.TubeGeometry(curve, pts.length === 2 ? 2 : 64, r, 6, closed), this.mats.fieldLine);
  }

  private cone(group: THREE.Group, marks: Mark[], at: THREE.Vector3, dir: THREE.Vector3 | null, size: number) {
    const cone = new THREE.Mesh(UNIT_HEAD, this.mats.fieldArrow);
    if (dir) {
      cone.quaternion.setFromUnitVectors(UP, dir.clone().normalize());
      cone.scale.set(size, 3 * size, size);
    }
    cone.position.copy(at);
    group.add(cone);
    marks.push({ cone, at: at.clone(), size });
  }

  /** The elements: pipe, magnets, cavities, BPMs. */
  private buildStatics() {
    this.clear(this.statics);
    const tr = this.tracks;
    if (!tr || !tr.s.length) return;
    const R = this.R;
    const s0 = tr.s[0];
    const s1 = tr.s[tr.s.length - 1];
    const bends = this.path.hasBends;
    const pipe = new THREE.Mesh(new THREE.TubeGeometry(new OrbitCurve(this.path, s0, s1), bends ? 240 : 2, R, 64, false), this.mats.pipe);
    pipe.renderOrder = 10;
    this.statics.add(pipe);
    for (const z of [s0, s1]) {
      // flanges at the ends of the drawn pipe
      const g = this.placed(z);
      g.add(new THREE.Mesh(new THREE.TorusGeometry(R * 1.04, R * 0.05, 8, 48), this.mats.flange));
      this.statics.add(g);
    }
    for (const e of tr.elements) {
      if (e.z0 == null || e.z1 == null) continue;
      if (e.key === "quad") this.addQuad(e);
      else if (e.key === "solenoid") this.addSolenoid(e);
      else if (e.key === "steerer") this.addSteerer(e);
      else if (e.key === "bend") this.addBend(e);
      else if (e.key === "field" && e.ez) this.addCavity(e);
    }
    for (const m of tr.monitors) {
      // a BPM: four button electrodes in the pipe wall
      const g = this.placed(m.z);
      for (let q = 0; q < 4; q++) {
        const a = (q * Math.PI) / 2;
        const b = new THREE.Mesh(new THREE.CylinderGeometry(R * 0.16, R * 0.16, R * 0.16, 20), this.mats.diag);
        b.position.set(Math.cos(a) * R, Math.sin(a) * R, 0);
        b.rotation.z = a - Math.PI / 2;
        g.add(b);
      }
      this.statics.add(g);
    }
  }

  private poleTag(g: THREE.Object3D, north: boolean, at: THREE.Vector3, text?: string) {
    const tag = label(text ?? (north ? "N" : "S"), this.c(north ? "--pole-n" : "--pole-s"), this.R * 0.3);
    tag.position.copy(at);
    g.add(tag);
  }

  private addQuad(e: MotionElement) {
    const R = e.aperture ? e.aperture / 1000 : this.R;
    const L = Math.max(e.z1! - e.z0!, 1e-4);
    const G = e.G ?? 1;
    const g = this.placed(e.z0!);
    const pole = poleGeometry(R, L);
    const w = 0.55 * R;
    const t = 0.22 * R;
    const du = 0.55 * R;
    const uc = 1.55 * R;
    for (let q = 0; q < 4; q++) {
      // cut open: the upper-right quarter (pole, coil and yoke) is left out so the camera sees into the bore
      if (this.opts.cutaway && q === 0) continue;
      const a = Math.PI / 4 + (q * Math.PI) / 2;
      const north = quadPole(q, G) === "N";
      const holder = new THREE.Group();
      holder.rotation.z = a;
      holder.add(new THREE.Mesh(pole, north ? this.mats.poleN : this.mats.poleS));
      // the coil around the pole: a rectangular loop of copper
      const side = new THREE.BoxGeometry(du, t, L + 2 * t);
      const end = new THREE.BoxGeometry(du, 2 * w + 2 * t, t);
      for (const sg of [1, -1]) {
        const b = new THREE.Mesh(side, this.mats.coil);
        b.position.set(uc, sg * (w + t / 2), L / 2);
        holder.add(b);
      }
      for (const z of [-t / 2, L + t / 2]) {
        const b = new THREE.Mesh(end, this.mats.coil);
        b.position.set(uc, 0, z);
        holder.add(b);
      }
      g.add(holder);
      this.poleTag(g, north, new THREE.Vector3(Math.cos(a) * R * 1.3, Math.sin(a) * R * 1.3, L + R * 0.45));
    }
    // the iron yoke, with the upper-right quarter cut away like the pole there
    const [a0, a1] = this.opts.cutaway ? KEEP : [0, 2 * Math.PI];
    g.add(new THREE.Mesh(shellGeometry(2.1 * R, 2.65 * R, a0, a1, L), this.mats.iron));
    this.statics.add(g);
  }

  private addSolenoid(e: MotionElement) {
    const R = this.R;
    const L = Math.max(e.z1! - e.z0!, 1e-4);
    const g = this.placed(e.z0!);
    const [a0, a1] = this.opts.cutaway ? KEEP : [0, 2 * Math.PI];
    // the coil: many turns of copper around the pipe, here one thick shell with a line between the turns
    g.add(new THREE.Mesh(shellGeometry(1.45 * R, 2.2 * R, a0, a1, L), this.mats.coil));
    const turns = Math.max(4, Math.round(L / (0.45 * R)));
    for (let i = 1; i < turns; i++) {
      for (const r of [1.44 * R, 2.21 * R]) {
        const ring = new THREE.Mesh(new THREE.TorusGeometry(r, 0.025 * R, 4, 48, a1 - a0), this.mats.flange);
        ring.rotation.z = a0;
        ring.position.z = (L * i) / turns;
        g.add(ring);
      }
    }
    // the iron around it, closed at both ends but for the bore: the field returns through it
    const t = 0.25 * R;
    const shell = new THREE.Mesh(shellGeometry(2.3 * R, 2.65 * R, a0, a1, L + 2 * t), this.mats.iron);
    shell.position.z = -t;
    g.add(shell);
    for (const z of [-t, L]) {
      const plate = new THREE.Mesh(shellGeometry(1.25 * R, 2.3 * R, a0, a1, t), this.mats.iron);
      plate.position.z = z;
      g.add(plate);
    }
    this.statics.add(g);
  }

  /** Poles (or plates) facing each other across the bore for a uniform field (bx, by), in group *g*. */
  private facingPoles(g: THREE.Group, bx: number, by: number, L: number, electric: boolean, curved?: { s0: number; s1: number }) {
    const R = this.R;
    const comps: [number, boolean][] = [];
    if (by || !bx) comps.push([by || 1, true]);
    if (bx) comps.push([bx, false]);
    const box = (xa: number, xb: number, ya: number, yb: number, mat: THREE.Material) => {
      if (curved) {
        this.statics.add(new THREE.Mesh(sweptBox(this.path, curved.s0, curved.s1, xa, xb, ya, yb, 48), mat));
      } else {
        const m = new THREE.Mesh(new THREE.BoxGeometry(xb - xa, yb - ya, L), mat);
        m.position.set((xa + xb) / 2, (ya + yb) / 2, L / 2);
        g.add(m);
      }
    };
    for (const [comp, vertical] of comps) {
      // the field runs from the N pole (+ plate) to the S pole (− plate): By > 0 → N below
      for (const side of [-1, 1]) {
        const north = (comp > 0 ? -1 : 1) === side;
        const mat = electric ? this.mats.steerer : north ? this.mats.poleN : this.mats.poleS;
        const [p0, p1] = side > 0 ? [1.1 * R, 1.75 * R] : [-1.75 * R, -1.1 * R];
        if (vertical) box(-1.25 * R, 1.25 * R, p0, p1, mat);
        else box(p0, p1, -1.25 * R, 1.25 * R, mat);
        if (!electric) {
          // the coil around the pole
          const [c0, c1] = side > 0 ? [1.15 * R, 1.65 * R] : [-1.65 * R, -1.15 * R];
          for (const sg of [-1, 1]) {
            const [x0, x1] = sg > 0 ? [1.25 * R, 1.6 * R] : [-1.6 * R, -1.25 * R];
            if (this.opts.cutaway && vertical && side > 0 && sg > 0) continue;
            if (this.opts.cutaway && !vertical && side > 0 && sg > 0) continue;
            if (vertical) box(x0, x1, c0, c1, this.mats.coil);
            else box(c0, c1, x0, x1, this.mats.coil);
          }
        }
        const at = vertical ? new THREE.Vector3(-1.45 * R, side * 1.42 * R, 0) : new THREE.Vector3(side * 1.42 * R, -1.45 * R, 0);
        const tagGroup = curved ? this.placed((curved.s0 + curved.s1) / 2) : g;
        if (curved) this.statics.add(tagGroup);
        else at.z = L / 2;
        this.poleTag(tagGroup, electric ? side > 0 === comp < 0 : north, at, electric ? (side > 0 === comp < 0 ? "+" : "−") : undefined);
      }
    }
    // the iron frame around both: top and bottom bars and the side walls (cut open at the upper right)
    const cut = this.opts.cutaway;
    box(-2.3 * R, cut ? 0 : 2.3 * R, 1.75 * R, 2.3 * R, this.mats.iron);
    box(-2.3 * R, 2.3 * R, -2.3 * R, -1.75 * R, this.mats.iron);
    box(-2.3 * R, -1.8 * R, -1.75 * R, 1.75 * R, this.mats.iron);
    box(1.8 * R, 2.3 * R, -1.75 * R, cut ? 0 : 1.75 * R, this.mats.iron);
  }

  private addSteerer(e: MotionElement) {
    const span = actingSpan(e);
    if (!span) {
      const g = this.placed(e.z0!);
      g.add(new THREE.Mesh(new THREE.TorusGeometry(this.R * 1.08, this.R * 0.07, 10, 48), this.mats.steerer));
      this.statics.add(g);
      return;
    }
    const L = Math.max(span[1] - span[0], 1e-4);
    const g = this.placed(span[0]);
    this.facingPoles(g, e.Bx ?? 0, e.By ?? 0, L, e.kind === 1);
    this.statics.add(g);
  }

  private addBend(e: MotionElement) {
    const b = e.B ?? 0;
    const vertical = e.hv === 1;
    // the magnet follows the arc; a vertical bend has its poles left and right
    this.facingPoles(new THREE.Group(), vertical ? -b : 0, vertical ? 0 : b, 0, false, { s0: e.z0!, s1: e.z1! });
  }

  /** Gap centres (m from the element's start) and width of an RF map: where |Ke·Ez| peaks on each side of its middle. */
  private gaps(e: MotionElement): { centres: number[]; width: number } {
    const L = e.z1! - e.z0!;
    const p = e.ez;
    if (!p || p.z.length < 3) return { centres: [0.3 * L, 0.7 * L], width: 0.15 * L };
    let iMax = 0;
    let iMin = 0;
    for (let i = 0; i < p.e.length; i++) {
      if (p.e[i] > p.e[iMax]) iMax = i;
      if (p.e[i] < p.e[iMin]) iMin = i;
    }
    const centres = [p.z[iMax], p.z[iMin]].sort((a, b) => a - b);
    const width = Math.max(0.4 * (centres[1] - centres[0]), 0.5 * this.R);
    return { centres, width };
  }

  /** A half-wave resonator: a vertical tank, the inner conductor across the beam with its drift tube, two gaps. */
  private addCavity(e: MotionElement) {
    const R = this.R;
    const g = this.placed(e.z0!);
    const { centres, width } = this.gaps(e);
    const [z1, z2] = centres;
    const mid = (z1 + z2) / 2;
    const H = 5 * R;
    const rt = (z2 - z1) / 2 + width * 1.2; // the tank wall lies beyond the gaps
    const ri = Math.max((z2 - z1 - width) / 2, 0.6 * R); // the inner conductor fills the space between them
    const tank = new THREE.Mesh(new THREE.CylinderGeometry(rt, rt, 2 * H, 64, 1, false), this.mats.tank);
    tank.position.set(0, 0, mid);
    tank.renderOrder = 11;
    g.add(tank);
    // the inner conductor: a vertical post above and below the beam, cut open on the camera's side
    const [t0, tl] = this.opts.cutaway ? [Math.PI / 2, 1.5 * Math.PI] : [0, 2 * Math.PI];
    for (const sg of [1, -1]) {
      const post = new THREE.Mesh(new THREE.CylinderGeometry(ri, ri, H - 1.3 * R, 48, 1, false, t0, tl), this.mats.iron);
      post.position.set(0, sg * (1.3 * R + (H - 1.3 * R) / 2), mid);
      g.add(post);
    }
    // the drift tube through it: the beam is shielded from the field between the gaps
    const [a0, a1] = this.opts.cutaway ? KEEP : [0, 2 * Math.PI];
    const tube = new THREE.Mesh(shellGeometry(1.05 * R, 1.35 * R, a0, a1, z2 - z1 - width), this.mats.iron);
    tube.position.z = z1 + width / 2;
    g.add(tube);
    // the beam ports: the tank wall continues as a tube to the element's ends
    for (const [za, zb] of [[0, mid - rt], [mid + rt, e.z1! - e.z0!]]) {
      if (zb - za <= 1e-4) continue;
      const port = new THREE.Mesh(shellGeometry(1.05 * R, 1.3 * R, a0, a1, zb - za), this.mats.iron);
      port.position.z = za;
      g.add(port);
    }
    this.statics.add(g);
  }

  /** Particles, their paths, the field pictures and the force arrows. */
  private buildMoving() {
    this.clear(this.moving);
    this.dots = this.marks = null;
    this.trails = [];
    this.pictures = [];
    this.forceArrows = [];
    const tr = this.tracks;
    if (!tr || !tr.n) return;
    const ns = tr.s.length;
    const marked = new Set(tr.marked);
    this.dots = new THREE.InstancedMesh(new THREE.SphereGeometry(1, 12, 10), this.mats.particle, Math.max(1, tr.n - marked.size));
    this.marks = new THREE.InstancedMesh(new THREE.SphereGeometry(1, 18, 14), this.mats.marked, Math.max(1, marked.size));
    for (const m of [this.dots, this.marks]) {
      m.frustumCulled = false;
      this.moving.add(m);
    }
    if (this.opts.trails) {
      const p = new THREE.Vector3();
      for (let i = 0; i < tr.n; i++) {
        const pos = new Float32Array(ns * 3);
        for (let k = 0; k < ns; k++) {
          this.path.toWorld(tr.s[k] + (tr.z ? tr.z[i * ns + k] / 1000 : 0), tr.x[i * ns + k] / 1000, tr.y[i * ns + k] / 1000, p);
          pos[3 * k] = p.x;
          pos[3 * k + 1] = p.y;
          pos[3 * k + 2] = p.z;
        }
        const geom = new THREE.BufferGeometry();
        geom.setAttribute("position", new THREE.BufferAttribute(pos, 3));
        const line = new THREE.Line(geom, marked.has(i) ? this.mats.markedTrail : this.mats.trail);
        line.frustumCulled = false;
        this.moving.add(line);
        this.trails.push(line);
      }
    }
    for (const e of tr.elements) {
      const pic = this.fieldPicture(e);
      if (pic) {
        pic.group.visible = false;
        this.moving.add(pic.group);
        this.pictures.push(pic);
      }
    }
    for (let i = 0; i < marked.size; i++) {
      const arrow = new SolidArrow(this.mats.forceArrow);
      arrow.traverse((o) => (o.renderOrder = 15));
      const tag = label("F", this.c("--force-arrow"), this.R * 0.14);
      arrow.visible = tag.visible = false;
      this.moving.add(arrow, tag);
      this.forceArrows.push({ arrow, tag });
    }
  }

  /** The field picture of one element (null: no field to draw). */
  private fieldPicture(e: MotionElement): FieldPicture | null {
    const R = this.R;
    const group = new THREE.Group();
    const marks: Mark[] = [];
    const disc = () => group.add(new THREE.Mesh(new THREE.CircleGeometry(R * 0.98, 64), this.mats.plane));
    if (e.key === "quad" && e.G) {
      disc();
      const G = e.G;
      for (const pts of quadLinePoints(R * 0.95)) {
        group.add(this.tube(pts, R * 0.004));
        for (const f of [0.2, 0.5, 0.8]) {
          const p = pts[Math.round(f * (pts.length - 1))];
          // B = G·(y, x), along the field line; a little larger away from the axis, where |B| = G·r is stronger
          this.cone(group, marks, p, new THREE.Vector3(G * p.y, G * p.x, 0), R * (0.014 + 0.014 * (p.length() / R)));
        }
      }
      return { e, kind: "plane", group, marks };
    }
    if ((e.key === "steerer" && e.kind !== 1 && (e.Bx || e.By)) || (e.key === "bend" && e.B)) {
      disc();
      const [bx, by] = e.key === "bend" ? (e.hv === 1 ? [-(e.B ?? 0), 0] : [0, e.B ?? 0]) : [e.Bx ?? 0, e.By ?? 0];
      const m = Math.hypot(bx, by);
      for (const pts of uniformLinePoints(R * 0.95, bx / m, by / m)) {
        group.add(this.tube(pts, R * 0.004));
        for (const f of [0.25, 0.75]) this.cone(group, marks, pts[0].clone().lerp(pts[1], f), new THREE.Vector3(bx, by, 0), R * 0.022);
      }
      return { e, kind: "plane", group, marks };
    }
    if (e.key === "steerer" && e.kind === 1 && (e.Bx || e.By)) {
      disc();
      const [ex, ey] = [e.Bx ?? 0, e.By ?? 0];
      const m = Math.hypot(ex, ey);
      for (const pts of uniformLinePoints(R * 0.95, ex / m, ey / m)) {
        group.add(this.tube(pts, R * 0.004));
        for (const f of [0.25, 0.75]) this.cone(group, marks, pts[0].clone().lerp(pts[1], f), new THREE.Vector3(ex, ey, 0), R * 0.022);
      }
      return { e, kind: "plane", group, marks };
    }
    if (e.key === "solenoid" && e.B) {
      // along the axis inside the coil, spreading at the ends and back through the iron (drawn in the four planes
      // at 45°); the cones point along +z for B > 0
      const L = Math.max(e.z1! - e.z0!, 1e-4);
      const sg = Math.sign(e.B);
      const back = 2.47 * R;
      const local = this.placed(e.z0!);
      for (let q = 0; q < 4; q++) {
        const a = Math.PI / 4 + (q * Math.PI) / 2;
        const ca = Math.cos(a);
        const sa = Math.sin(a);
        const P = (r: number, z: number) => new THREE.Vector3(r * ca, r * sa, z);
        for (const r of [0.35 * R, 0.8 * R]) {
          const m = (r + back) / 2;
          const loop = [P(r, 0.1 * L), P(r, 0.5 * L), P(r, 0.9 * L), P(m, L + 0.45 * R), P(back, 0.85 * L), P(back, 0.15 * L), P(m, -0.45 * R)];
          local.add(this.tube(loop, R * 0.008, true));
          for (const f of [0.25, 0.5, 0.75]) this.cone(local, marks, P(r, f * L), new THREE.Vector3(0, 0, sg), R * 0.04);
          this.cone(local, marks, P(back, 0.5 * L), new THREE.Vector3(0, 0, -sg), R * 0.04);
        }
      }
      group.add(local);
      return { e, kind: "static", group, marks };
    }
    if (e.key === "field" && e.ez && e.phase) {
      // the electric field across each gap, on five lines; its cones follow the RF phase of the moment
      const local = this.placed(e.z0!);
      const { centres, width } = this.gaps(e);
      for (const zc of centres) {
        for (const [x, y] of [[0, 0], [0.55, 0], [-0.55, 0], [0, 0.55], [0, -0.55]]) {
          local.add(this.tube([new THREE.Vector3(x * R, y * R, zc - width / 2), new THREE.Vector3(x * R, y * R, zc + width / 2)], R * 0.006));
          for (const f of [-0.25, 0.25]) this.cone(local, marks, new THREE.Vector3(x * R, y * R, zc + f * width), null, R * 0.05);
        }
      }
      group.add(local);
      return { e, kind: "rf", group, marks, gaps: centres };
    }
    return null;
  }

  /** The largest force on a highlighted particle in each element over the whole line: arrows are drawn relative to it. */
  private measureForces(): Map<MotionElement, number> {
    const out = new Map<MotionElement, number>();
    const tr = this.tracks;
    if (!tr || !tr.mass) return out;
    const ns = tr.s.length;
    for (let k = 0; k < ns; k += 2) {
      const els = activeElements(tr, tr.s[k]);
      if (!els.length) continue;
      const loc: Loc = { lo: k, hi: k, f: 0 };
      const beta = tr.energy ? betaOf(tr.energy[k], tr.mass) : 0.05;
      for (const i of tr.marked) {
        const F = this.forceAt(tr, i, loc, tr.s[k], els, beta);
        const m = Math.hypot(...F);
        for (const e of els) if (Number.isFinite(m)) out.set(e, Math.max(out.get(e) ?? 0, m));
      }
    }
    return out;
  }

  private forceAt(tr: MotionTracks, i: number, loc: Loc, s: number, els: MotionElement[], beta: number) {
    const ns = tr.s.length;
    const at = (a: ArrayLike<number>) => a[i * ns + loc.lo] + (a[i * ns + loc.hi] - a[i * ns + loc.lo]) * loc.f;
    const dz = tr.z ? at(tr.z) / 1000 : 0;
    return forceOn(els, tr.charge || 1, at(tr.x) / 1000, at(tr.y) / 1000, s + dz, at(tr.xp) / 1000, at(tr.yp) / 1000, beta, (e) =>
      e.phase ? curveAt(e.phase, loc) : NaN,
    );
  }

  /** Move the particles, trails, field pictures and force arrows to the current s. */
  private update() {
    const tr = this.tracks;
    if (!tr || !this.dots || !this.marks) return;
    const ns = tr.s.length;
    const loc = locate(tr.s, this.s);
    const at = (a: ArrayLike<number>, i: number) => {
      const v0 = a[i * ns + loc.lo];
      return v0 + (a[i * ns + loc.hi] - v0) * loc.f;
    };
    const m = new THREE.Matrix4();
    const zero = new THREE.Matrix4().makeScale(0, 0, 0);
    // metres per CSS pixel at the beam's distance from the camera
    const centre = this.path.toWorld(this.s, 0, 0);
    const dist = this.camera.position.distanceTo(centre);
    const perPx = (2 * dist * Math.tan(THREE.MathUtils.degToRad(this.camera.fov) / 2)) / Math.max(1, this.host.clientHeight);
    const rDot = Math.max(PARTICLE_R, PARTICLE_PX * perPx);
    const rMark = Math.max(MARKED_R, MARKED_PX * perPx);
    const marked = new Set(tr.marked);
    let d = 0;
    let k = 0;
    const markedPos: { p: THREE.Vector3; s: number; i: number }[] = [];
    const p = new THREE.Vector3();
    for (let i = 0; i < tr.n; i++) {
      const x = at(tr.x, i) / 1000;
      const y = at(tr.y, i) / 1000;
      // a real bunch has a length: each particle sits ahead of or behind the centre by its place in it
      const s = this.s + (tr.z ? at(tr.z, i) / 1000 : 0);
      const ok = Number.isFinite(x) && Number.isFinite(y) && Number.isFinite(s);
      const r = marked.has(i) ? rMark : rDot;
      if (ok) {
        this.path.toWorld(s, x, y, p);
        m.makeScale(r, r, r).setPosition(p);
      }
      if (marked.has(i)) {
        this.marks.setMatrixAt(k++, ok ? m : zero);
        markedPos.push({ p: ok ? p.clone() : new THREE.Vector3(NaN, NaN, NaN), s, i });
      } else this.dots.setMatrixAt(d++, ok ? m : zero);
    }
    this.dots.instanceMatrix.needsUpdate = true;
    this.marks.instanceMatrix.needsUpdate = true;
    for (const line of this.trails) line.geometry.setDrawRange(0, loc.lo + 1);

    // field pictures
    const q = this.path.frame(this.s);
    for (const pic of this.pictures) {
      const span = actingSpan(pic.e);
      if (pic.kind === "plane") {
        // the plane of the particles, while they are in the element
        const inside = !!span && this.s >= span[0] && this.s <= span[1];
        pic.group.visible = inside && this.opts.field;
        if (pic.group.visible) {
          pic.group.position.copy(centre);
          pic.group.quaternion.copy(q);
        }
      } else if (pic.kind === "static") {
        pic.group.visible = this.opts.field;
      } else {
        pic.group.visible = this.opts.field;
        if (!pic.group.visible || !pic.e.phase) continue;
        // a standing wave: everywhere in the cavity the field is Ke·Ez(z)·cos(phase of the moment)
        const c = Math.cos((curveAt(pic.e.phase, loc) * Math.PI) / 180);
        const peak = Math.max(...Array.from(pic.e.ez?.e ?? [1], (v) => Math.abs(v)), 1e-9);
        for (const mk of pic.marks) {
          const ez = rfProfile(pic.e, pic.e.z0! + mk.at.z) * c;
          const f = Math.abs(ez) / peak;
          mk.cone.visible = f > 0.06;
          if (!mk.cone.visible) continue;
          mk.cone.quaternion.setFromUnitVectors(UP, new THREE.Vector3(0, 0, Math.sign(ez)));
          const size = mk.size * (0.35 + 0.65 * f);
          mk.cone.scale.set(size, 3 * size, size);
        }
      }
    }

    // forces on the highlighted particles
    const R = this.R;
    const beta = tr.energy && tr.mass ? betaOf(curveAt(tr.energy, loc), tr.mass) : 0.05;
    this.forceArrows.forEach(({ arrow: a, tag }, j) => {
      const mp = markedPos[j];
      const els = mp && Number.isFinite(mp.s) ? activeElements(tr, mp.s) : [];
      const show = this.opts.forces && els.length > 0 && !!mp && Number.isFinite(mp.p.x);
      a.visible = tag.visible = show;
      if (!show) return;
      const F = this.forceAt(tr, mp.i, loc, this.s, els, beta);
      const mag = Math.hypot(...F);
      const scale = Math.max(...els.map((e) => this.forceScale.get(e) ?? 0));
      if (!(mag > 1e-12) || !(scale > 0) || mag < 0.02 * scale) {
        a.visible = tag.visible = false;
        return;
      }
      // starts at the ball's surface and is a few particle sizes long: it points, it does not cover the bunch
      const dir = this.path.dir(mp.s, new THREE.Vector3(F[0], F[1], F[2]).normalize());
      const len = R * (0.08 + 0.2 * Math.min(mag / scale, 1));
      const from = mp.p.clone().addScaledVector(dir, MARKED_R);
      a.aim(from, dir, len, R * 0.005);
      tag.position.copy(from).addScaledVector(dir, len + R * 0.06);
    });
  }
}
