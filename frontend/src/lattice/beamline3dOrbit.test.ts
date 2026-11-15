import { describe, expect, it, vi } from "vitest";
import { Orbit, edgeTilt, type Elem } from "./beamline3dModel";

vi.mock("./types", () => ({ elementColorVar: () => "--el-other" }));

const elem = (shape: Elem["shape"], s0: number, len: number, deg: number, hv: 0 | 1 = 0): Elem =>
  ({ shape, s0, s1: s0 + len, len, angle: (deg * Math.PI) / 180, hv }) as Elem;

describe("reference orbit", () => {
  // engine (compat mode = AVAS.dll), 2026-11-15: after alpha = +20 deg, d<x>/dW > 0, i.e.
  // R16 > 0 and +x is the outside of the bend, as in the linear preview's bend matrix
  it("turns a positive angle towards -x, and towards -y when vertical", () => {
    const h = new Orbit([elem("bend", 0.1, 0.5, 30)]).pose(1);
    expect(h.p[0]).toBeLessThan(0);
    expect(h.z[0]).toBeLessThan(0);
    expect(h.p[1]).toBeCloseTo(0, 12);
    const v = new Orbit([elem("bend", 0.1, 0.5, 30, 1)]).pose(1);
    expect(v.p[1]).toBeLessThan(0);
    expect(v.p[0]).toBeCloseTo(0, 12);
  });

  it("turns a negative angle the other way and keeps the arc length", () => {
    const o = new Orbit([elem("bend", 0, 1, -90)]);
    const p = o.pose(1).p;
    // quarter circle of radius 2/pi towards +x
    expect(p[0]).toBeCloseTo(2 / Math.PI, 12);
    expect(p[2]).toBeCloseTo(2 / Math.PI, 12);
  });
});

describe("edge drawing tilt", () => {
  const bend = elem("bend", 1, 0.5, 30);
  const entry = elem("edge", 1, 0, 10);
  const exit = elem("edge", 1.5, 0, 10);
  // the face is drawn along B cos t - Z sin t: t > 0 moves its +B side upstream; +B is the
  // outside of a positive bend, and beta > 0 shortens the outside path at both ends
  it("moves the outside of an entrance face downstream and of an exit face upstream", () => {
    expect(edgeTilt(entry, [bend])).toBeCloseTo((-10 * Math.PI) / 180, 12);
    expect(edgeTilt(exit, [bend])).toBeCloseTo((10 * Math.PI) / 180, 12);
  });

  it("follows the bend direction and plane", () => {
    const neg = elem("bend", 1, 0.5, -30);
    expect(edgeTilt(entry, [neg])).toBeCloseTo((10 * Math.PI) / 180, 12);
    expect(edgeTilt(exit, [neg])).toBeCloseTo((-10 * Math.PI) / 180, 12);
    // a vertical bend does not orient a horizontal edge: drawn as the entrance of a positive bend
    expect(edgeTilt(exit, [elem("bend", 1, 0.5, -30, 1)])).toBeCloseTo((-10 * Math.PI) / 180, 12);
  });
});
