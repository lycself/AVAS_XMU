import { describe, expect, it } from "vitest";
import {
  autoReason,
  compatProblem,
  deviceChoices,
  effectiveDefaultKernel,
  engineLabel,
  engineShort,
  ENGINE_KEYS,
  formatGpus,
  formKernel,
  formProblems,
  iniLines,
  kernelLines,
  kernelsDiffer,
  kernelsDifferAmong,
  normalizeEngine,
  parseGpus,
  precisionProblem,
  preferenceIgnored,
  secondaryNeedsBeamFile,
  seedDefect,
  type EngineForm,
  type EngineInfo,
} from "./engine";

const form = (over: Partial<EngineForm> = {}): EngineForm => ({ kernel: "new", device: "auto", precision: "auto", gpus: "all", threads: "0", compat: "none", ...over });

function info(cuda: number, metal: boolean): EngineInfo {
  return {
    platform: "test",
    originalAvailable: true,
    defaultKernel: "original",
    original: { file: "AVAS.dll", exists: true, sha1: null },
    new: { available: true, path: "x", sha1: null, version: "v", sourceHash: null, compiler: null, error: null, build: null },
    devices: {
      cuda: Array.from({ length: cuda }, (_, i) => ({ index: i, name: `GPU ${i}`, compute: "8.9", fp64Full: false, memoryGb: 48 })),
      cudaCompiled: cuda > 0,
      cudaNote: null,
      metal: metal ? [{ name: "Apple M2", usable: true }] : [],
      metalCompiled: metal,
      metalNote: null,
      cpuThreads: 8,
    },
    auto: null,
    error: null,
  };
}

describe("kernel options", () => {
  it("lists only the devices this computer has", () => {
    expect(deviceChoices(null)).toEqual(["auto", "cpu"]);
    expect(deviceChoices(info(0, false))).toEqual(["auto", "cpu"]);
    expect(deviceChoices(info(2, false))).toEqual(["auto", "cpu", "cuda"]);
    expect(deviceChoices(info(0, true))).toEqual(["auto", "cpu", "metal"]);
  });

  it("rejects the combinations the new kernel cannot run", () => {
    expect(precisionProblem("metal", "fp64")).toMatch(/fp32 only/);
    expect(precisionProblem("cpu", "fp32")).toMatch(/fp64 only/);
    expect(precisionProblem("cuda", "fp32")).toBeNull();
    expect(precisionProblem("auto", "fp64")).toBeNull();
    expect(compatProblem("cuda", "dll")).toMatch(/CPU only/);
    expect(compatProblem("metal", "dll")).toMatch(/CPU only/);
    expect(compatProblem("auto", "dll")).toBeNull();
    expect(compatProblem("cpu", "dll")).toBeNull();
    expect(compatProblem("cuda", "none")).toBeNull();
  });

  it("checks the whole form; the original kernel ignores the other keys", () => {
    expect(formProblems(form(), true)).toEqual([]);
    expect(formProblems(form({ device: "metal", precision: "fp64" }), true)).toHaveLength(1);
    expect(formProblems(form({ threads: "-1" }), true)).toHaveLength(1);
    expect(formProblems(form({ gpus: "0,x" }), true)).toHaveLength(1);
    expect(formProblems(form({ kernel: "original", device: "metal", precision: "fp64" }), true)).toEqual([]);
    expect(formProblems(form({ kernel: "original" }), false)[0]).toMatch(/does not run on this platform/);
    // a form without a kernel is checked as the default kernel
    expect(formProblems(form({ kernel: "", device: "metal", precision: "fp64" }), true, "original")).toEqual([]);
    expect(formProblems(form({ kernel: "", device: "metal", precision: "fp64" }), true, "new")).toHaveLength(1);
  });

  it("resolves the default kernel from the global preference like the back end", () => {
    expect(effectiveDefaultKernel("", "original", true)).toBe("original");
    expect(effectiveDefaultKernel(undefined, "new", false)).toBe("new");
    expect(effectiveDefaultKernel("new", "original", true)).toBe("new");
    expect(effectiveDefaultKernel("Original", "new", true)).toBe("original");
    expect(effectiveDefaultKernel("original", "new", false)).toBe("new");       // not on this platform: built-in
    expect(effectiveDefaultKernel("bogus", "original", true)).toBe("original");
    expect(preferenceIgnored("original", false)).toBe(true);
    expect(preferenceIgnored("original", true) || preferenceIgnored("new", false) || preferenceIgnored(null, false)).toBe(false);
    expect(formKernel({ kernel: "" }, "new")).toBe("new");
    expect(formKernel({ kernel: "original" }, "new")).toBe("original");
  });

  it("warns about the fixed-seed defect only for the original kernel and compat = dll", () => {
    expect(seedDefect("original", null, "12345")).toBe(true);
    expect(seedDefect("new", "dll", 7)).toBe(true);
    expect(seedDefect("new", "none", "12345")).toBe(false);
    expect(seedDefect("original", null, "0")).toBe(false);
    expect(seedDefect("original", null, "")).toBe(false);
    expect(seedDefect("original", null, "abc")).toBe(false);
    expect(seedDefect(null, null, "5")).toBe(false);
    expect(secondaryNeedsBeamFile("original", null) && secondaryNeedsBeamFile("new", "dll")).toBe(true);
    expect(secondaryNeedsBeamFile("new", "none")).toBe(false);
  });

  it("tells when two runs used different kernels or modes", () => {
    const orig = { kernel: "original" };
    const cpu = { kernel: "new", device: "cpu", precision: "fp64", compat: "none" };
    const metal = { kernel: "new", device: "metal", precision: "fp32", requested: { device: "auto", compat: "none" } };
    const dll = { kernel: "new", device: "cpu", precision: "fp64", compat: "dll" };
    expect(kernelsDiffer(orig, cpu)).toBe(true);
    expect(kernelsDiffer(cpu, dll)).toBe(true);
    expect(kernelsDiffer(orig, dll)).toBe(true);
    expect(kernelsDiffer(cpu, metal)).toBe(false);                            // same kernel and mode, other device
    expect(kernelsDiffer(orig, { kernel: "original", device: "gpu" })).toBe(false);
    expect(kernelsDiffer(null, cpu)).toBe(false);                             // older record: unknown
    expect(kernelsDifferAmong([cpu, metal, null])).toBe(false);
    expect(kernelsDifferAmong([cpu, null, metal, orig])).toBe(true);
    expect(kernelsDifferAmong([])).toBe(false);
  });

  it("maps the form to ini.ini [engine] keys like the back end saves it", () => {
    expect(ENGINE_KEYS).toEqual(["kernel", "device", "precision", "gpus", "threads", "compat"]);
    expect(iniLines(form({ kernel: "original" }))).toEqual(["kernel = original"]);
    expect(iniLines(form({ kernel: "" }))).toEqual([]);                      // follows the default kernel
    expect(iniLines(form({ kernel: "", threads: "2" }))).toEqual(["threads = 2"]);
    expect(iniLines(form())).toEqual(["kernel = new"]);
    expect(iniLines(form({ device: "cuda", precision: "fp32", gpus: "0,1", threads: "4", compat: "dll" }))).toEqual([
      "kernel = new",
      "device = cuda",
      "precision = fp32",
      "gpus = 0,1",
      "threads = 4",
      "compat = dll",
    ]);
  });

  it("parses and formats the GPU list", () => {
    expect(parseGpus("all")).toEqual([]);
    expect(parseGpus("")).toEqual([]);
    expect(parseGpus("1,0")).toEqual([1, 0]);
    expect(formatGpus([])).toBe("all");
    expect(formatGpus([2, 0, 2])).toBe("0,2");
  });
});

describe("kernel lines of the About window", () => {
  it("lists both kernels and the devices", () => {
    const i = info(1, false);
    i.original.sha1 = "3d6c3e089f895d267ab61aec0ba24a013e290443";
    i.new.sha1 = "3faeb7fb077a2ce7aff4a0c0605543f80c07a00f";
    i.new.build = { cuda: true, metal: false, mpi: false, cudaArchs: ["89"], buildType: "Release", platform: "linux-x86_64" };
    i.auto = { sc: { device: "cuda", precision: "fp32", gpus: [0], reason: "auto", error: null }, nosc: { device: "cuda", precision: "fp32", gpus: [0], reason: "auto", error: null } };
    const text = kernelLines(i).join("\n");
    expect(text).toContain("Original kernel: AVAS.dll · SHA-1 3d6c3e0");
    expect(text).toContain("New kernel (preview): version v · SHA-1 3faeb7f · CPU + CUDA (sm 89)");
    expect(text).toContain("GPU 0 (CUDA 0, sm 8.9, 48 GB, slow fp64)");
    expect(text).toContain("with space charge → CUDA fp32");
  });

  it("says when the original kernel or the new library is missing", () => {
    const i = info(0, true);
    i.originalAvailable = false;
    i.platform = "macOS arm64";
    i.new = { ...i.new, available: false, error: "not found here" };
    const text = kernelLines(i).join("\n");
    expect(text).toContain("not available on macOS arm64");
    expect(text).toContain("New kernel (preview): not found (not found here)");
    expect(kernelLines(null, "boom")[0]).toContain("boom");
  });
});

describe("kernel labels", () => {
  it("shows nothing for records written before the kernel was recorded", () => {
    expect(engineLabel(undefined)).toBeNull();
    expect(engineLabel(null)).toBeNull();
    expect(engineLabel({})).toBeNull();
    expect(engineLabel({ kernel: "other" })).toBeNull();
  });

  it("names the original kernel", () => {
    expect(engineLabel({ kernel: "original", sha1: "abc", tier: "original" })?.text).toBe("Original kernel");
    expect(engineLabel({ kernel: "original", device: "gpu" })?.text).toBe("Original kernel · libPIC.so GPU");
  });

  it("names the device and precision the new kernel used", () => {
    const raw = { kernel: "new", device: "metal", precision: "fp32", gpus: [], compat: "none", tier: "new-fp32", requested: { device: "auto" }, version: "5762f2c2" };
    const label = engineLabel(raw)!;
    expect(label.text).toBe("New kernel (preview) · Metal fp32");
    expect(label.fallback).toBe(false);
    expect(label.tip).toContain("new-fp32");
    expect(engineLabel({ kernel: "new", device: "cuda", precision: "fp64", gpus: [0, 1], requested: { device: "cuda" } })!.text).toBe("New kernel (preview) · CUDA fp64 · 2 GPUs");
    expect(engineLabel({ kernel: "new", device: "cpu", precision: "fp64", compat: "dll", requested: { device: "cpu" } })!.text).toBe("New kernel (preview) · CPU fp64 · AVAS.dll compatible");
  });

  it("explains when auto fell back to the CPU", () => {
    const label = engineLabel({ kernel: "new", device: "cpu", precision: "fp64", requested: { device: "auto" }, reason: "auto: no CUDA driver" })!;
    expect(label.fallback).toBe(true);
    expect(label.text).toBe("New kernel (preview) · actual: CPU (auto: no usable GPU on this computer)");
    expect(label.tip).toContain("no CUDA driver");
    expect(autoReason("auto: no space charge, Metal is used for runs with space charge")).toMatch(/no space charge/);
    expect(autoReason("auto: compat=dll runs on the CPU only")).toMatch(/compatibility/);
    expect(autoReason("auto: this engine was built without CUDA")).toMatch(/no GPU support/);
    expect(autoReason("auto: metal could not be used at run time (see the log)")).toMatch(/failed at run time/);
  });

  it("translates the kernel's reason codes, whatever the text says", () => {
    expect(autoReason("auto: anything", "no_space_charge")).toMatch(/no space charge/);
    expect(autoReason("auto: anything", "few_particles")).toMatch(/half the space-charge mesh cells/);
    expect(autoReason("auto: 5000 particles for 262144 space-charge mesh cells (fewer than 0.5 per cell): the CPU is faster than Metal")).toMatch(/half/);
    expect(autoReason("auto: anything", "compat_dll")).toMatch(/compatibility/);
    expect(autoReason("auto: anything", "gpu_memory")).toMatch(/memory/);
    expect(autoReason("auto: anything", "no_kernel_image")).toMatch(/machine code/);
    expect(autoReason("auto: anything", "driver_too_old")).toMatch(/driver is too old/);
    expect(autoReason("auto: no space charge", "no_gpu")).toMatch(/no usable GPU/);
    const label = engineLabel({ kernel: "new", device: "cpu", precision: "fp64", requestedDevice: "auto", reason: "x", reasonCode: "no_driver" })!;
    expect(label.text).toMatch(/no NVIDIA driver/);
    expect(label.fallback).toBe(true);
  });

  it("shows the request while the run has not recorded its device", () => {
    expect(engineLabel({ kernel: "new", device: null, requested: { device: "auto" } })!.text).toBe("New kernel (preview) · requested: auto");
    expect(engineLabel({ kernel: "new", device: null, requestedDevice: "metal" })!.text).toBe("New kernel (preview) · requested: Metal");
    const predicted = engineLabel({ kernel: "new", device: "metal", precision: "fp32", requestedDevice: "auto", predicted: true })!;
    expect(predicted.text).toBe("New kernel (preview) · Metal fp32");
    expect(predicted.tip).toMatch(/Expected/);
  });

  it("gives a short form for legends", () => {
    expect(engineShort(null)).toBeNull();
    expect(engineShort({ kernel: "original" })).toBe("original kernel");
    expect(engineShort({ kernel: "new", device: "metal", precision: "fp32" })).toBe("new kernel Metal fp32");
    expect(engineShort({ kernel: "new", device: "cpu", precision: "fp64", compat: "dll" })).toBe("new kernel CPU fp64 dll");
  });

  it("accepts both the back end's summary and a raw avas_run.json entry", () => {
    const raw = { kernel: "new", device: "cpu", precision: "fp64", requested: { device: "cpu", compat: "none" } };
    const summary = { kernel: "new", device: "cpu", precision: "fp64", requestedDevice: "cpu", compat: "none" };
    expect(normalizeEngine(raw)).toEqual(normalizeEngine(summary));
  });
});
