// The computing kernels: the original kernel (AVAS.dll / libAVAS.so) and the new kernel (engine/), selected per
// project in ini.ini [engine] (avas/engines.py, avas/gui/services/kernels.py).  Pure helpers: the options of the
// Simulation settings page, which combinations are valid, the ini.ini lines a form gives, and the labels of run
// records ("New kernel (preview) · Metal fp32").
import { t } from "./i18n";

export type Kernel = "original" | "new";
export type Device = "auto" | "cpu" | "cuda" | "metal";
export type Precision = "auto" | "fp64" | "fp32";
export type Compat = "none" | "dll";

/** The page's form; the keys are the keys of ini.ini [engine]. */
export type EngineForm = { kernel: Kernel; device: Device | string; precision: Precision | string; gpus: string; threads: string; compat: Compat | string };
export const ENGINE_KEYS = ["kernel", "device", "precision", "gpus", "threads", "compat"] as const;
/** Defaults of the new kernel's options (avas.engines.DEFAULTS); the kernel's default depends on the platform. */
export const ENGINE_DEFAULTS = { device: "auto", precision: "auto", gpus: "all", threads: "0", compat: "none" } as const;

export type Plan = { device: string | null; precision: string | null; gpus: number[]; reason: string | null; error: string | null };
export type CudaDevice = { index: number; name: string; compute: string; fp64Full: boolean; memoryGb: number };
/** engine.info */
export type EngineInfo = {
  platform: string;
  originalAvailable: boolean;
  defaultKernel: Kernel;
  original: { file: string; exists: boolean; sha1: string | null };
  new: {
    available: boolean;
    path: string | null;
    sha1: string | null;
    version: string | null;
    sourceHash: string | null;
    compiler: string | null;
    error: string | null;
    build: { cuda: boolean; metal: boolean; mpi: boolean; cudaArchs: string[]; buildType: string | null; platform: string | null } | null;
  };
  devices: {
    cuda: CudaDevice[];
    cudaCompiled: boolean;
    cudaNote: string | null;
    metal: { name: string; usable: boolean }[];
    metalCompiled: boolean;
    metalNote: string | null;
    cpuThreads: number | null;
  } | null;
  auto: { sc: Plan; nosc: Plan } | null;
  error: string | null;
};

/** The kernel of a run record (kernels.summary on the back end, or a raw avas_run.json "engine" entry). */
export type EngineSummary = {
  kernel: Kernel;
  device?: string | null;
  precision?: string | null;
  gpus?: number[] | null;
  compat?: string | null;
  tier?: string | null;
  requestedDevice?: string | null;
  reason?: string | null;
  version?: string | null;
  /** expected before the run (engine.check); the actual values come with avas_run.json */
  predicted?: boolean;
  legacyGpu?: boolean;
};

const DEVICE_NAME: Record<string, string> = { cpu: "CPU", cuda: "CUDA", metal: "Metal", emu: "emu" };

export function deviceName(device: string | null | undefined): string {
  return device ? DEVICE_NAME[device] ?? device : "–";
}

/** Devices the page offers: auto and CPU always, NVIDIA / Mac GPU only when this computer has one. */
export function deviceChoices(info: EngineInfo | null): Device[] {
  const out: Device[] = ["auto", "cpu"];
  if (info?.devices?.cuda.length) out.push("cuda");
  if (info?.devices?.metal.some((d) => d.usable)) out.push("metal");
  return out;
}

/** Why *precision* cannot be used with *device* (null: it can).  Metal computes in fp32 only, the CPU in fp64. */
export function precisionProblem(device: string, precision: string): string | null {
  if (device === "metal" && precision === "fp64") return t("Metal computes in fp32 only");
  if (device === "cpu" && precision === "fp32") return t("The CPU computes in fp64 only");
  return null;
}

/** Why the compatibility mode *compat* cannot be used with *device* (it reproduces AVAS.dll on the CPU). */
export function compatProblem(device: string, compat: string): string | null {
  if (compat === "dll" && device !== "auto" && device !== "cpu") return t("Reproducing AVAS.dll runs on the CPU only (device Auto or CPU)");
  return null;
}

/** The problems of a form (empty: it can be saved); the back end checks the same combinations. */
export function formProblems(form: EngineForm, originalAvailable: boolean): string[] {
  const out: string[] = [];
  if (form.kernel === "original") {
    if (!originalAvailable) out.push(t("Computing kernel: the original kernel does not run on this platform; choose the new kernel"));
    return out;
  }
  const p = precisionProblem(form.device, form.precision) ?? compatProblem(form.device, form.compat);
  if (p) out.push(t("Computing kernel: {problem}", { problem: p }));
  if (!/^\d+$/.test(form.threads.trim())) out.push(t("Computing kernel: threads must be 0 (automatic) or a positive integer"));
  if (form.gpus.trim() !== "all" && !/^\d+(,\d+)*$/.test(form.gpus.trim())) out.push(t("Computing kernel: GPUs must be 'all' or numbers such as 0,1"));
  return out;
}

/** The ini.ini [engine] lines the form is saved as (the kernel explicitly, other keys only when not the default). */
export function iniLines(form: EngineForm): string[] {
  const out = [`kernel = ${form.kernel}`];
  for (const key of ENGINE_KEYS.slice(1)) {
    const value = String(form[key]).trim();
    if (value && value !== ENGINE_DEFAULTS[key as keyof typeof ENGINE_DEFAULTS]) out.push(`${key} = ${value}`);
  }
  return out;
}

/** The GPU list of the form as numbers ([] = all). */
export function parseGpus(gpus: string): number[] {
  const text = gpus.trim().toLowerCase();
  if (!text || text === "all") return [];
  return text.split(",").filter(Boolean).map(Number).filter((n) => Number.isInteger(n) && n >= 0);
}

export function formatGpus(list: number[]): string {
  return list.length ? [...new Set(list)].sort((a, b) => a - b).join(",") : "all";
}

function planLine(plan: Plan | null | undefined): string {
  if (!plan) return "–";
  if (plan.error) return plan.error;
  return `${deviceName(plan.device)} ${plan.precision ?? ""}`.trim();
}

/** The kernels and devices of this computer as text lines (About window, "Copy version information"). */
export function kernelLines(info: EngineInfo | null, error?: string | null): string[] {
  if (!info) return [`${t("Computing kernels")}: ${error ? error : t("Unknown")}`];
  const lines = [t("Computing kernels")];
  lines.push(
    info.originalAvailable
      ? `  ${t("Original kernel")}: ${info.original.file}${info.original.sha1 ? ` · SHA-1 ${info.original.sha1.slice(0, 7)}` : ` (${t("missing")})`}`
      : `  ${t("Original kernel")}: ${t("not available on {platform}", { platform: info.platform })}`,
  );
  const n = info.new;
  if (n.available) {
    const b = n.build;
    const parts = ["CPU", b?.cuda ? `CUDA${b.cudaArchs.length ? ` (sm ${b.cudaArchs.join(",")})` : ""}` : "", b?.metal ? "Metal" : "", b?.mpi ? "MPI" : ""].filter(Boolean);
    lines.push(`  ${t("New kernel (preview)")}: ${t("version")} ${n.version ?? "?"}${n.sha1 ? ` · SHA-1 ${n.sha1.slice(0, 7)}` : ""} · ${parts.join(" + ")}${n.compiler ? ` · ${n.compiler}` : ""}`);
    if (n.path) lines.push(`    ${n.path}`);
  } else {
    lines.push(`  ${t("New kernel (preview)")}: ${t("not found")}${n.error ? ` (${n.error})` : ""}`);
  }
  const d = info.devices;
  if (d) {
    const devs = [
      ...d.cuda.map((g) => `${g.name} (CUDA ${g.index}, sm ${g.compute}, ${Math.round(g.memoryGb)} GB, ${g.fp64Full ? t("full-rate fp64") : t("slow fp64")})`),
      ...d.metal.map((m) => `${m.name} (Metal${m.usable ? "" : `, ${t("not usable")}`})`),
      `CPU ${d.cpuThreads ?? "?"} ${t("threads")}`,
    ];
    lines.push(`  ${t("Devices")}: ${devs.join(" · ")}`);
  }
  if (info.auto) lines.push(`  ${t("Automatic")}: ${t("with space charge")} → ${planLine(info.auto.sc)}; ${t("without space charge")} → ${planLine(info.auto.nosc)}`);
  return lines;
}

/** A run record's kernel in the form of kernels.summary (raw avas_run.json entries are converted). */
export function normalizeEngine(raw: unknown): EngineSummary | null {
  if (!raw || typeof raw !== "object") return null;
  const r = raw as Record<string, any>;
  if (r.kernel !== "original" && r.kernel !== "new") return null;
  if (r.kernel === "original") return { kernel: "original", legacyGpu: !!r.legacyGpu || r.device === "gpu" };
  const requested = r.requested && typeof r.requested === "object" ? r.requested : {};
  return {
    kernel: "new",
    device: r.device ?? null,
    precision: r.precision ?? null,
    gpus: Array.isArray(r.gpus) ? r.gpus : [],
    compat: r.compat ?? requested.compat ?? null,
    tier: r.tier ?? null,
    requestedDevice: r.requestedDevice ?? requested.device ?? null,
    reason: r.reason ?? null,
    version: r.version ?? null,
    predicted: !!r.predicted,
  };
}

/** Why device auto ran on the CPU, in words (the engine's reason is English and technical). */
export function autoReason(reason: string | null | undefined): string {
  const r = reason ?? "";
  if (r.startsWith("auto: no space charge")) return t("auto: no space charge, the GPU is used only with space charge");
  if (r.includes("compat=dll")) return t("auto: the AVAS.dll compatibility mode runs on the CPU only");
  if (r.includes("could not be used at run time")) return t("auto: the GPU failed at run time, see the log");
  if (r.includes("built without CUDA") && !r.includes("Metal")) return t("auto: this build of the new kernel has no GPU support");
  return t("auto: no usable GPU on this computer");
}

/** A short form for plot legends: "original kernel", "new kernel Metal fp32" (null without information). */
export function engineShort(raw: unknown): string | null {
  const e = normalizeEngine(raw);
  if (!e) return null;
  if (e.kernel === "original") return t("original kernel");
  const dev = e.device ? `${deviceName(e.device)}${e.precision ? ` ${e.precision}` : ""}` : "";
  return [t("new kernel"), dev, e.compat === "dll" ? "dll" : ""].filter(Boolean).join(" ");
}

export type EngineLabel = { text: string; tip: string; fallback: boolean };

/** The label of a run's kernel, e.g. "New kernel (preview) · Metal fp32", "Original kernel",
 *  "New kernel (preview) · actual: CPU (auto: …)".  null for records without the information. */
export function engineLabel(raw: unknown): EngineLabel | null {
  const e = normalizeEngine(raw);
  if (!e) return null;
  if (e.kernel === "original") {
    const text = e.legacyGpu ? `${t("Original kernel")} · libPIC.so GPU` : t("Original kernel");
    return { text, tip: t("Original kernel (AVAS.dll / libAVAS.so)"), fallback: false };
  }
  const name = t("New kernel (preview)");
  const tips: string[] = [];
  let text: string;
  let fallback = false;
  if (!e.device) {
    text = `${name} · ${t("requested: {device}", { device: e.requestedDevice && e.requestedDevice !== "auto" ? deviceName(e.requestedDevice) : t("auto") })}`;
    tips.push(t("The device actually used is recorded when the run finishes."));
  } else if (e.requestedDevice === "auto" && e.device === "cpu") {
    fallback = true;
    text = `${name} · ${t("actual: CPU ({reason})", { reason: autoReason(e.reason) })}`;
  } else {
    text = `${name} · ${deviceName(e.device)}${e.precision ? ` ${e.precision}` : ""}`;
    if (e.gpus && e.gpus.length > 1) text += ` · ${t("{n} GPUs", { n: e.gpus.length })}`;
  }
  if (e.compat === "dll") text += ` · ${t("AVAS.dll compatible")}`;
  if (e.predicted) tips.push(t("Expected from the settings; the run records what it actually used."));
  if (e.requestedDevice) tips.push(`${t("Requested device")}: ${e.requestedDevice === "auto" ? t("auto") : deviceName(e.requestedDevice)}`);
  if (e.reason && e.requestedDevice === "auto") tips.push(`${t("Reason")}: ${e.reason}`);
  if (e.gpus && e.gpus.length) tips.push(`GPU ${e.gpus.join(", ")}`);
  if (e.tier) tips.push(`${t("Result tier")}: ${e.tier}`);
  if (e.version) tips.push(`${t("Engine version")}: ${e.version}`);
  return { text, tip: [text, ...tips].join("\n"), fallback };
}
