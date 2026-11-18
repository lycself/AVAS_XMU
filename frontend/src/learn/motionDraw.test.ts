import { describe, expect, it } from "vitest";
import { curveAt, focusing, locate, quadPole, valueAt, type MotionElement } from "./motionDraw";

describe("primer animation helpers", () => {
  it("locates a position between samples", () => {
    const s = [0, 0.1, 0.3, 0.6];
    expect(locate(s, -1)).toEqual({ lo: 0, hi: 0, f: 0 });
    expect(locate(s, 0.2)).toEqual({ lo: 1, hi: 2, f: expect.closeTo(0.5, 12) });
    expect(locate(s, 0.6)).toEqual({ lo: 3, hi: 3, f: 0 });
    expect(curveAt([0, 1, 3, 6], locate(s, 0.2))).toBeCloseTo(2);
    // particle-major: particle 1 of 2 with 4 samples each
    expect(valueAt([0, 0, 0, 0, 10, 20, 40, 80], 4, 1, locate(s, 0.45))).toBeCloseTo(60);
  });

  it("names the focusing plane from the gradient and the charge", () => {
    const q: MotionElement = { line: 1, key: "quad", name: "", z0: 0, z1: 0.1, aperture: 20, G: 10 };
    expect(focusing(q, "x", 1)).toBe("F");
    expect(focusing(q, "y", 1)).toBe("D");
    expect(focusing(q, "x", -1)).toBe("D");
    expect(focusing({ ...q, G: -10 }, "x", 1)).toBe("D");
    expect(focusing({ ...q, key: "drift" }, "x", 1)).toBeNull();
  });

  it("puts the north poles where the field leaves the pole (B = G·(y, x), seen from downstream)", () => {
    // quadrants 0…3 at 45°, 135°, 225°, 315°; the field near a pole points out of a north pole, i.e. towards the axis
    for (const G of [6, -6])
      for (let q = 0; q < 4; q++) {
        const ang = Math.PI / 4 + (q * Math.PI) / 2;
        const [x, y] = [Math.cos(ang), Math.sin(ang)];
        const [bx, by] = [G * y, G * x];
        const outward = bx * x + by * y > 0; // field points from the axis into the pole: a south pole
        expect(quadPole(q, G), `G=${G} q=${q}`).toBe(outward ? "S" : "N");
      }
    expect([0, 1, 2, 3].map((q) => quadPole(q, 6))).toEqual(["S", "N", "S", "N"]);
  });
});
