import { describe, expect, it } from "vitest";
import * as THREE from "three";
import { BeamPath } from "./closeupPath";
import type { MotionElement } from "./motionDraw";
import { fieldOf, forceOn } from "./fields";

const el = (o: Partial<MotionElement>): MotionElement => ({ line: 1, key: "drift", name: "", z0: 0, z1: 1, aperture: 20, ...o });

describe("BeamPath", () => {
  it("is the identity without bends", () => {
    const p = new BeamPath([el({})]);
    const v = p.toWorld(0.7, 0.01, -0.02);
    expect([v.x, v.y, v.z].map((a) => +a.toFixed(12))).toEqual([0.01, -0.02, 0.7]);
  });

  it("turns a horizontal bend with α > 0 towards −x and keeps the arc length", () => {
    const rho = 0.5;
    const a = Math.PI / 2;
    const p = new BeamPath([el({ key: "bend", z0: 1, z1: 1 + rho * a, alpha: 90, rho, hv: 0 })]);
    const end = p.toWorld(1 + rho * a, 0, 0);
    expect(end.x).toBeCloseTo(-rho, 9);
    expect(end.z).toBeCloseTo(1 + rho, 9);
    // after the bend the orbit runs along −x
    const later = p.toWorld(1 + rho * a + 0.3, 0, 0);
    expect(later.x).toBeCloseTo(-rho - 0.3, 9);
    expect(later.z).toBeCloseTo(1 + rho, 9);
    // x is measured towards the outside: at the end the outside is +z
    const out = p.toWorld(1 + rho * a, 0.01, 0);
    expect(out.z - end.z).toBeCloseTo(0.01, 9);
    expect(p.dir(1 + rho * a, new THREE.Vector3(0, 0, 1)).x).toBeCloseTo(-1, 9);
  });

  it("turns a vertical bend with α > 0 towards −y", () => {
    const p = new BeamPath([el({ key: "bend", z0: 0, z1: 0.2, alpha: 30, rho: 0.4, hv: 1 })]);
    expect(p.toWorld(0.2, 0, 0).y).toBeLessThan(0);
  });
});

describe("fields", () => {
  it("bends a positive particle the way the model does", () => {
    // a bend with α > 0 has B0 > 0 and pushes towards −x
    const F = forceOn([el({ key: "bend", B: 0.3, rho: 0.5 })], 1, 0, 0, 0.5, 0, 0, 0.05);
    expect(F[0]).toBeLessThan(0);
    // steerer By > 0 → −x, Bx > 0 → +y
    expect(forceOn([el({ key: "steerer", By: 0.003, Bx: 0 })], 1, 0, 0, 0.5, 0, 0, 0.05)[0]).toBeLessThan(0);
    expect(forceOn([el({ key: "steerer", By: 0, Bx: 0.003 })], 1, 0, 0, 0.5, 0, 0, 0.05)[1]).toBeGreaterThan(0);
    // quadrupole G > 0 focuses x, defocuses y
    const q = el({ key: "quad", G: 10 });
    expect(forceOn([q], 1, 0.001, 0, 0.5, 0, 0, 0.05)[0]).toBeLessThan(0);
    expect(forceOn([q], 1, 0, 0.001, 0.5, 0, 0, 0.05)[1]).toBeGreaterThan(0);
  });

  it("gives the RF field as Ke·Ez·cos(phase)", () => {
    const cav = el({ key: "field", z0: 0, z1: 0.2, ez: { z: [0, 0.1, 0.2], e: [0, 2, 0] } });
    expect(fieldOf(cav, 0, 0, 0.1, 0).E[2]).toBeCloseTo(2e6, 3);
    expect(fieldOf(cav, 0, 0, 0.1, 180).E[2]).toBeCloseTo(-2e6, 3);
    expect(fieldOf(cav, 0, 0, 0.05, 0).E[2]).toBeCloseTo(1e6, 3);
  });
});
