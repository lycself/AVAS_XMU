import { describe, expect, it } from "vitest";
import { isStaleChunkError } from "./ErrorBoundary";

describe("error boundary", () => {
  it("recognises a lazily loaded part that no longer exists after a rebuild", () => {
    expect(isStaleChunkError(new TypeError("Failed to fetch dynamically imported module: http://127.0.0.1:8766/assets/Beamline3D-obJhjo34.js"))).toBe(true);
    expect(isStaleChunkError(new TypeError("Importing a module script failed."))).toBe(true);
    expect(isStaleChunkError(new Error("Cannot read properties of undefined"))).toBe(false);
  });
});
