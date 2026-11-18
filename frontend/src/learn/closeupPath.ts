// The reference orbit of a primer animation in 3D: straight along z, turning on a circular arc in
// every bend.  Maps the beam's coordinates (s along the orbit, x, y across it) to world positions,
// so that particles, pipe, magnets and the camera all follow a bend.  For a line without bends it is
// the identity (world z = s).
//
// Direction convention of the linear model (avas/model): a bend with α > 0 turns a positive particle
// towards −x (horizontal, hv 0) or −y (vertical, hv 1); x is measured towards the outside of the bend.
import * as THREE from "three";
import type { MotionElement } from "./motionDraw";

type Seg = {
  s0: number;
  s1: number;
  p0: THREE.Vector3;
  q0: THREE.Quaternion;
  /** for an arc: centre in the segment's local frame, rotation axis (local) and turn per metre (rad/m, signed) */
  arc?: { centre: THREE.Vector3; axis: THREE.Vector3; rate: number };
};

const X = new THREE.Vector3(1, 0, 0);
const Y = new THREE.Vector3(0, 1, 0);

export class BeamPath {
  private segs: Seg[] = [];

  constructor(elements: MotionElement[]) {
    const bends = elements
      .filter((e) => e.key === "bend" && e.z0 != null && e.z1 != null && e.z1 > e.z0 && e.rho && e.alpha)
      .sort((a, b) => a.z0! - b.z0!);
    let s = 0; // the first straight starts at the origin (s < 0 extends it backwards)
    let p = new THREE.Vector3(0, 0, s);
    let q = new THREE.Quaternion();
    const push = (seg: Seg) => {
      this.segs.push(seg);
      p = this.local(seg, seg.s1, 0, 0, new THREE.Vector3());
      q = this.rotation(seg, seg.s1, new THREE.Quaternion());
      s = seg.s1;
    };
    for (const b of bends) {
      if (b.z0! < s) continue; // overlapping bends: keep the first
      push({ s0: s, s1: b.z0!, p0: p.clone(), q0: q.clone() });
      const sign = Math.sign(b.alpha!);
      const vertical = b.hv === 1;
      // towards −x: turn about +y by a negative angle; towards −y: turn about +x by a positive angle
      const arc = vertical
        ? { centre: new THREE.Vector3(0, -b.rho! * sign, 0), axis: X.clone(), rate: sign / b.rho! }
        : { centre: new THREE.Vector3(-b.rho! * sign, 0, 0), axis: Y.clone(), rate: -sign / b.rho! };
      push({ s0: b.z0!, s1: b.z1!, p0: p.clone(), q0: q.clone(), arc });
    }
    this.segs.push({ s0: s, s1: Infinity, p0: p.clone(), q0: q.clone() });
  }

  get hasBends(): boolean {
    return this.segs.some((g) => g.arc);
  }

  private seg(s: number): Seg {
    for (const g of this.segs) if (s <= g.s1) return g;
    return this.segs[this.segs.length - 1];
  }

  private rotation(g: Seg, s: number, out: THREE.Quaternion): THREE.Quaternion {
    out.copy(g.q0);
    if (g.arc) out.multiply(new THREE.Quaternion().setFromAxisAngle(g.arc.axis, g.arc.rate * (s - g.s0)));
    return out;
  }

  private local(g: Seg, s: number, x: number, y: number, out: THREE.Vector3): THREE.Vector3 {
    const u = s - g.s0;
    if (g.arc) {
      // rotate the point (x, y, 0) of the arc's start about the centre by the angle turned so far
      out.set(x, y, 0).sub(g.arc.centre).applyAxisAngle(g.arc.axis, g.arc.rate * u).add(g.arc.centre);
    } else out.set(x, y, u);
    return out.applyQuaternion(g.q0).add(g.p0);
  }

  /** World position of the point (x, y) m across the orbit at *s*. */
  toWorld(s: number, x: number, y: number, out = new THREE.Vector3()): THREE.Vector3 {
    return this.local(this.seg(s), s, x, y, out);
  }

  /** Orientation of the orbit's frame at *s* (local z along the orbit, x outwards, y up). */
  frame(s: number, out = new THREE.Quaternion()): THREE.Quaternion {
    return this.rotation(this.seg(s), s, out);
  }

  /** A vector given in the orbit's frame at *s*, in world coordinates. */
  dir(s: number, v: THREE.Vector3, out = new THREE.Vector3()): THREE.Vector3 {
    return out.copy(v).applyQuaternion(this.frame(s));
  }
}

/** The orbit (or a line parallel to it at x, y) from s0 to s1 as a three.js curve, for tubes and swept shapes. */
export class OrbitCurve extends THREE.Curve<THREE.Vector3> {
  constructor(private path: BeamPath, private s0: number, private s1: number, private x = 0, private y = 0) {
    super();
  }
  getPoint(t: number, out = new THREE.Vector3()): THREE.Vector3 {
    return this.path.toWorld(this.s0 + (this.s1 - this.s0) * t, this.x, this.y, out);
  }
}
