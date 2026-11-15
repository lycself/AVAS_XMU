import { describe, expect, it } from "vitest";
import { DEFAULT_KERNEL_SETTING } from "../engine";
import { KERNEL_NOTICE_SETTING, kernelNoticeState } from "./kernelNoticeState";

describe("note about the new default kernel", () => {
  it("is shown once where the default changed and the user has not chosen", () => {
    expect(kernelNoticeState({}, true)).toBe("show");
    expect(kernelNoticeState({ [DEFAULT_KERNEL_SETTING]: "" }, true)).toBe("show");
    expect(kernelNoticeState({ [KERNEL_NOTICE_SETTING]: "2026-11" }, true)).toBe("done");
  });
  it("is skipped where nothing changed for the user", () => {
    expect(kernelNoticeState({}, false)).toBe("skip"); // macOS: always the new kernel
    expect(kernelNoticeState({ [DEFAULT_KERNEL_SETTING]: "original" }, true)).toBe("skip");
    expect(kernelNoticeState({ [DEFAULT_KERNEL_SETTING]: "new" }, true)).toBe("skip");
  });
  it("waits for the kernel information", () => {
    expect(kernelNoticeState({}, null)).toBe("wait");
  });
});
