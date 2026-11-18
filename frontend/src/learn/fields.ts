// The fields of the primer animation's elements and the force they put on a particle, for the
// pictures (2D cross-section, 3D close-up).  The trajectories themselves come from the back end
// (the linear model); these formulas only say which way the arrows point and how long they are,
// with the same conventions as the model:
//   quadrupole  B = G·(y, x)                               (x, y in m)
//   solenoid    B = (0, 0, B) inside the coil (hard edges)
//   steerer     B = (Bx, By) uniform over the element after it; kind 1: E = (Ex, Ey) in V/m
//   bend        By = B0·(1 − N·x/ρ), Bx = −B0·N·y/ρ        (B0 > 0 bends a positive particle to −x)
//   RF map      Ez = Ke·Ez(z)·cos(φ) on axis, φ the RF phase at that moment (MV/m)
// Coordinates are the beam's: x right, y up, z along the beam (a cross-section is seen from downstream).
import type { MotionElement, MotionTracks } from "./motionDraw";

export type Vec3 = [number, number, number];

/** The span an element acts over: a steerer acts over the element after it. */
export function actingSpan(e: MotionElement): [number, number] | null {
  if (e.key === "steerer") return e.span ?? null;
  if (e.z0 == null || e.z1 == null || e.z1 <= e.z0) return null;
  return [e.z0, e.z1];
}

/** Elements with a field at *s* (drifts and zero-length markers left out). */
export function activeElements(tr: Pick<MotionTracks, "elements">, s: number): MotionElement[] {
  return tr.elements.filter((e) => {
    if (e.key === "drift" || e.key === "edge") return false;
    const span = actingSpan(e);
    return !!span && s >= span[0] && s <= span[1];
  });
}

/** On-axis Ke·Ez (MV/m) of an RF map element at *s*. */
export function rfProfile(e: MotionElement, s: number): number {
  const p = e.ez;
  if (!p || e.z0 == null) return 0;
  const u = s - e.z0;
  const z = p.z;
  if (!z.length || u < z[0] || u > z[z.length - 1]) return 0;
  let lo = 0;
  let hi = z.length - 1;
  while (hi - lo > 1) {
    const mid = (lo + hi) >> 1;
    if (z[mid] <= u) lo = mid;
    else hi = mid;
  }
  const f = z[hi] > z[lo] ? (u - z[lo]) / (z[hi] - z[lo]) : 0;
  return p.e[lo] + (p.e[hi] - p.e[lo]) * f;
}

/** Magnetic field (T) and electric field (V/m) of element *e* at (x, y) m and *s*; *phase* the RF phase (°). */
export function fieldOf(e: MotionElement, x: number, y: number, s: number, phase = NaN): { B: Vec3; E: Vec3 } {
  const B: Vec3 = [0, 0, 0];
  const E: Vec3 = [0, 0, 0];
  switch (e.key) {
    case "quad":
      B[0] = (e.G ?? 0) * y;
      B[1] = (e.G ?? 0) * x;
      break;
    case "solenoid":
      B[2] = e.B ?? 0;
      break;
    case "steerer":
      if (e.kind === 1) {
        E[0] = e.Bx ?? 0;
        E[1] = e.By ?? 0;
      } else {
        B[0] = e.Bx ?? 0;
        B[1] = e.By ?? 0;
      }
      break;
    case "bend": {
      const b0 = e.B ?? 0;
      const k = e.rho ? (e.N ?? 0) / e.rho : 0;
      // a vertical bend (hv 1) is the horizontal one turned by 90°
      if (e.hv === 1) {
        B[0] = -b0 * (1 - k * y);
        B[1] = -b0 * k * x;
      } else {
        B[1] = b0 * (1 - k * x);
        B[0] = -b0 * k * y;
      }
      break;
    }
    case "field":
      if (Number.isFinite(phase)) E[2] = rfProfile(e, s) * 1e6 * Math.cos((phase * Math.PI) / 180);
      break;
  }
  return { B, E };
}

/** Speed of light, m/s. */
const C = 299792458;

/**
 * Force on a particle of charge sign *q* at (x, y, s) with slopes (xp, yp) (rad), divided by q·v (*phaseOf*: the
 * RF phase of an RF element at this moment):
 * E/v + v̂ × B, in tesla.  Only its direction and relative size are used.  *beta*: v/c.
 */
export function forceOn(
  els: MotionElement[],
  q: number,
  x: number,
  y: number,
  s: number,
  xp: number,
  yp: number,
  beta: number,
  phaseOf: (e: MotionElement) => number = () => NaN,
): Vec3 {
  const F: Vec3 = [0, 0, 0];
  const n = Math.hypot(xp, yp, 1);
  const v: Vec3 = [xp / n, yp / n, 1 / n];
  for (const e of els) {
    const { B, E } = fieldOf(e, x, y, s, phaseOf(e));
    const inv = 1 / Math.max(beta * C, 1);
    F[0] += q * (E[0] * inv + v[1] * B[2] - v[2] * B[1]);
    F[1] += q * (E[1] * inv + v[2] * B[0] - v[0] * B[2]);
    F[2] += q * (E[2] * inv + v[0] * B[1] - v[1] * B[0]);
  }
  return F;
}

/** v/c from the kinetic energy (MeV) and the rest mass (MeV). */
export function betaOf(energy: number, mass: number): number {
  const g = 1 + energy / mass;
  return Math.sqrt(Math.max(1 - 1 / (g * g), 0));
}
