// Simulation settings → Computing kernel: ini.ini [engine] of the project (kernel, device, precision, GPUs,
// threads, compatibility mode).  Only the devices this computer has are offered; with device Auto the group
// shows what a run of this project will use.  Saved with the page (settings.save), only when it changed.
// A project without "kernel" in [engine] follows the default computing kernel (Settings menu → Default
// computing kernel, else the built-in default); the form's kernel is "" then.
import { useEffect, useMemo, useState } from "react";
import { call } from "../bridge";
import { Button, Checkbox, Collapsible, Icon, IconButton, Radio, Section, Spinner, TextInput } from "../components/ui";
import {
  autoReason,
  compatProblem,
  DEFAULT_KERNEL_SETTING,
  deviceChoices,
  deviceName,
  effectiveDefaultKernel,
  formatGpus,
  formKernel,
  iniLines,
  parseGpus,
  precisionProblem,
  preferenceIgnored,
  type EngineForm,
  type Kernel,
  type Plan,
} from "../engine";
import { useApp } from "../store/app";
import { useT } from "../i18n";
import { useEngineInfo } from "../store/engine";
import { FormRow } from "./common";

/** settings.load → engine */
export type EngineSettingsData = {
  form: EngineForm;
  /** [engine] entries that are not valid (the defaults are used instead) */
  invalid: string[];
  explicitKernel: boolean;
  /** AVAS_ENGINE* variables that override ini.ini for runs started here */
  env: string[];
  originalAvailable: boolean;
  /** the kernel the project follows when the form's kernel is "" (when the page was loaded) */
  defaultKernel: Kernel;
  /** the built-in default of this platform, and the global preference ("" = not set) */
  builtinKernel: Kernel;
  preference: Kernel | "";
  platform: string;
};

/** The global preference: the app's current setting (it changes while the page is open), else the loaded one. */
export function currentPreference(data: EngineSettingsData, settings: Record<string, unknown>): unknown {
  return DEFAULT_KERNEL_SETTING in settings ? settings[DEFAULT_KERNEL_SETTING] : data.preference;
}

/** The kernel a project without [engine] kernel runs with, now. */
export function defaultKernelOf(data: EngineSettingsData, settings: Record<string, unknown>): Kernel {
  return effectiveDefaultKernel(currentPreference(data, settings), data.builtinKernel ?? data.defaultKernel, data.originalAvailable);
}

/** The kernel the page's form runs with (its own choice, else the default kernel). */
export function engineKernelOf(data: EngineSettingsData | null | undefined, settings: Record<string, unknown>): Kernel | null {
  return data ? formKernel(data.form, defaultKernelOf(data, settings)) : null;
}

type Preview = { kernel: string; spaceCharge?: boolean; plan: Plan | null; other: Plan | null; error: string | null };

function planText(plan: Plan | null | undefined, t: ReturnType<typeof useT>): string {
  if (!plan) return "–";
  if (plan.error) return t("cannot run: {reason}", { reason: plan.error });
  const gpus = plan.gpus.length ? ` (GPU ${plan.gpus.join(", ")})` : "";
  const text = `${deviceName(plan.device)} ${plan.precision ?? ""}${gpus}`.trim();
  // auto on the CPU: say why (the kernel's reason code)
  if (plan.device === "cpu" && plan.reasonCode && plan.reasonCode !== "requested")
    return `${text} (${autoReason(plan.reason, plan.reasonCode)})`;
  return text;
}

export function EngineSettings({
  data,
  onChange,
  spaceCharge,
  locked,
}: {
  data: EngineSettingsData;
  onChange: (form: EngineForm) => void;
  spaceCharge: boolean;
  locked: boolean;
}) {
  const t = useT();
  const { info, loading, error: infoError, load } = useEngineInfo();
  const f = data.form;
  const set = <K extends keyof EngineForm>(k: K, v: EngineForm[K]) => onChange({ ...f, [k]: v });
  useEffect(() => {
    load();
  }, [load]);

  const settings = useApp((s) => s.settings);
  const preference = currentPreference(data, settings);
  const defaultKernel = defaultKernelOf(data, settings);
  const kernel = formKernel(f, defaultKernel);
  const kernelName = (k: Kernel) => (k === "original" ? t("Original kernel (being retired)") : t("New kernel"));
  const isNew = kernel === "new";
  const libraryOk = !!info?.new.available;
  const devices = deviceChoices(info);
  const cuda = info?.devices?.cuda ?? [];

  // what a run of this project would use with the form's choice (asked from the back end, debounced)
  const [preview, setPreview] = useState<Preview | null>(null);
  const key = JSON.stringify([f, kernel, spaceCharge, isNew && libraryOk]);
  useEffect(() => {
    if (!isNew || !libraryOk) {
      setPreview(null);
      return;
    }
    let alive = true;
    const timer = window.setTimeout(() => {
      call<Preview>("engine.preview", { choice: { ...f, kernel }, spaceCharge })
        .then((p) => alive && setPreview(p))
        .catch(() => alive && setPreview(null));
    }, 250);
    return () => {
      alive = false;
      window.clearTimeout(timer);
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [key]);

  const deviceLabel: Record<string, string> = {
    auto: t("Automatic"),
    cpu: "CPU",
    cuda: t("NVIDIA GPU (CUDA)"),
    metal: t("Mac GPU (Metal)"),
  };
  const shownDevices = devices.includes(f.device as never) ? devices : [...devices, f.device as never];
  const deviceMissing = isNew && !!info && !devices.includes(f.device as never);
  const selectedGpus = parseGpus(f.gpus);
  const toggleGpu = (index: number, on: boolean) => {
    const list = selectedGpus.length ? selectedGpus : [];
    set("gpus", formatGpus(on ? [...list, index] : list.filter((g) => g !== index)));
  };
  const newOnly = !isNew ? t("Only the new kernel uses this option.") : undefined;
  const lines = useMemo(() => iniLines(f).join("\n"), [f]);

  return (
    <Section
      title={t("Computing kernel")}
      icon="server-process"
      actions={<IconButton icon="refresh" tip={t("Detect the devices again")} disabled={loading} onClick={() => load(true)} />}
    >
      <div className="form">
        <FormRow label={t("Kernel")} tip={t("Stored in ini.ini [engine] of this project. 'avas run --engine' and the AVAS_ENGINE variable override it; without it the project follows Settings → Default computing kernel.")}>
          <div className="radio-row">
            <Radio
              checked={kernel === "original"}
              disabled={!data.originalAvailable && kernel !== "original"}
              onChange={() => set("kernel", "original")}
              label={t("Original kernel (being retired)")}
              tip={data.originalAvailable ? t("AVAS.dll / libAVAS.so, the kernel AVAS used before the new kernel; it will be removed in a later release") : t("The original kernel does not run on {platform}", { platform: data.platform })}
            />
            <Radio
              checked={isNew}
              disabled={!!info && !libraryOk && !isNew}
              onChange={() => set("kernel", "new")}
              label={t("New kernel")}
              tip={t("The rebuilt kernel: CPU, NVIDIA GPU or Mac GPU; supports the PICNIC solver")}
            />
          </div>
        </FormRow>
        <FormRow>
          {f.kernel ? (
            <span className="engine-note">
              <Icon name="pin" /> {t("This project chooses its kernel in ini.ini [engine].")}{" "}
              <Button variant="ghost" small disabled={locked} onClick={() => set("kernel", "")} tip={t("Remove the project's choice; it then follows Settings → Default computing kernel")}>
                {t("Follow the default ({kernel})", { kernel: kernelName(defaultKernel) })}
              </Button>
            </span>
          ) : (
            <span className="engine-note">
              <Icon name="link" />{" "}
              {preference && !preferenceIgnored(preference, data.originalAvailable)
                ? t("Not set in this project: follows the global default (currently {kernel}, Settings → Default computing kernel).", { kernel: kernelName(defaultKernel) })
                : t("Not set in this project: follows the global default (currently {kernel}, the built-in default).", { kernel: kernelName(defaultKernel) })}
            </span>
          )}
        </FormRow>
        {preferenceIgnored(preference, data.originalAvailable) && (
          <FormRow>
            <span className="engine-note warning-text">
              <Icon name="warning" /> {t("The default computing kernel is set to the original kernel, which does not run on {platform}; the new kernel is used instead.", { platform: data.platform })}
            </span>
          </FormRow>
        )}
        {!data.originalAvailable && (
          <FormRow>
            <span className="engine-note">
              <Icon name="info" /> {t("This platform ({platform}) only has the new kernel; it is used by default.", { platform: data.platform })}
            </span>
          </FormRow>
        )}
        {f.kernel === "original" && !data.originalAvailable && (
          <FormRow>
            <span className="engine-note warning-text">
              <Icon name="warning" /> {t("ini.ini selects the original kernel, which cannot run here. Choose the new kernel.")}
            </span>
          </FormRow>
        )}
        {info && !libraryOk && (
          <FormRow>
            <span className="engine-note warning-text" data-tip={info.new.error ?? info.error ?? undefined}>
              <Icon name="warning" />{" "}
              {t("The new kernel's library was not found. Installed versions include it; a source checkout uses the prebuilt library in avas/engine/new/ or one built with CMake (see README).")}
            </span>
          </FormRow>
        )}
        {infoError && (
          <FormRow>
            <span className="engine-note danger-text">{infoError}</span>
          </FormRow>
        )}

        <FormRow label={t("Device")} tip={newOnly}>
          <div className="radio-row">
            {shownDevices.map((d) => (
              <Radio
                key={d}
                checked={f.device === d}
                disabled={!isNew || (!devices.includes(d) && f.device !== d) || !!compatProblem(d, f.compat)}
                onChange={() => set("device", d)}
                label={deviceLabel[d] ?? d}
                tip={compatProblem(d, f.compat) ?? undefined}
              />
            ))}
            {loading && <Spinner size={14} />}
          </div>
        </FormRow>
        {deviceMissing && (
          <FormRow>
            <span className="engine-note warning-text">
              <Icon name="warning" /> {t("This computer has no {device}; runs would fail. Choose Automatic or CPU.", { device: deviceLabel[f.device] ?? f.device })}
            </span>
          </FormRow>
        )}
        {isNew && preview && (
          <FormRow>
            <span className="engine-note" data-tip={preview.plan?.reason ?? undefined}>
              <Icon name="arrow-right" />{" "}
              {preview.error
                ? t("cannot run: {reason}", { reason: preview.error })
                : f.device === "auto"
                  ? t("Automatic for this project ({sc}): {plan}; {other_sc}: {other}", {
                      sc: preview.spaceCharge ? t("with space charge") : t("without space charge"),
                      plan: planText(preview.plan, t),
                      other_sc: preview.spaceCharge ? t("without space charge") : t("with space charge"),
                      other: planText(preview.other, t),
                    })
                  : t("Runs of this project: {plan}", { plan: planText(preview.plan, t) })}
            </span>
          </FormRow>
        )}

        <FormRow label={t("Precision")} tip={newOnly}>
          <div className="radio-row">
            {(["auto", "fp64", "fp32"] as const).map((p) => {
              const problem = precisionProblem(f.device, p);
              return (
                <Radio
                  key={p}
                  checked={f.precision === p}
                  disabled={!isNew || (!!problem && f.precision !== p)}
                  onChange={() => set("precision", p)}
                  label={p === "auto" ? t("Automatic") : p}
                  tip={problem ?? (p === "auto" ? t("fp64 on the CPU and on GPUs with full-rate fp64, fp32 on other GPUs and on Metal") : undefined)}
                />
              );
            })}
          </div>
        </FormRow>
        {isNew && precisionProblem(f.device, f.precision) && (
          <FormRow>
            <span className="engine-note warning-text">
              <Icon name="warning" /> {precisionProblem(f.device, f.precision)}
            </span>
          </FormRow>
        )}

        {cuda.length > 1 && (f.device === "auto" || f.device === "cuda") && (
          <FormRow label={t("GPUs")} tip={newOnly}>
            <div className="radio-row">
              <Checkbox checked={!selectedGpus.length} disabled={!isNew} onChange={(v) => v && set("gpus", "all")} label={t("All")} />
              {cuda.map((g) => (
                <Checkbox
                  key={g.index}
                  checked={selectedGpus.includes(g.index)}
                  disabled={!isNew}
                  onChange={(v) => toggleGpu(g.index, v)}
                  label={`${g.index}: ${g.name}`}
                  tip={`sm ${g.compute} · ${g.memoryGb.toFixed(0)} GB · ${g.fp64Full ? t("full-rate fp64") : t("slow fp64")}`}
                />
              ))}
            </div>
          </FormRow>
        )}

        {!isNew && (
          <FormRow>
            <span className="engine-note muted">{t("Device, precision and the advanced options apply only to the new kernel.")}</span>
          </FormRow>
        )}
        {data.invalid.length > 0 && (
          <FormRow>
            <span className="engine-note warning-text">
              <Icon name="warning" /> {t("ini.ini [engine] has invalid entries ({entries}); the defaults are used. Saving replaces them.", { entries: data.invalid.join(", ") })}
            </span>
          </FormRow>
        )}
        {data.env.length > 0 && (
          <FormRow>
            <span className="engine-note warning-text">
              <Icon name="warning" /> {t("The environment variables {names} override these settings for runs started from this window.", { names: data.env.join(", ") })}
            </span>
          </FormRow>
        )}
      </div>
      <div className="engine-advanced">
        <Collapsible title={t("Advanced")} open={f.threads.trim() !== "0" || f.compat === "dll"}>
          <div className="form">
            <FormRow
              label={t("CPU threads")}
              tip={isNew
                ? t("0 = automatic (all cores). Only affects the speed: results of the new kernel do not depend on the thread count.")
                : t("0 = automatic (all cores). Passed to the original kernel as OMP_NUM_THREADS. With 1 thread it avoids the original kernel's data race in the space charge of matrix elements (results then repeat exactly), but runs slower.")}
            >
              <TextInput
                style={{ width: 80 }}
                disabled={locked}
                value={f.threads}
                invalid={!/^\d+$/.test(f.threads.trim())}
                onChange={(e) => set("threads", e.target.value)}
              />
              <span className="hint">{t("0 = automatic")}</span>
            </FormRow>
            <FormRow label={t("Compatibility")} top tip={newOnly}>
              <div className="radio-col">
                <Radio checked={f.compat !== "dll"} disabled={!isNew} onChange={() => set("compat", "none")} label={t("Fix the original kernel's defects (default)")} />
                <Radio
                  checked={f.compat === "dll"}
                  disabled={!isNew || (!!compatProblem(f.device, "dll") && f.compat !== "dll")}
                  onChange={() => set("compat", "dll")}
                  label={t("Reproduce AVAS.dll (for comparisons)")}
                  tip={compatProblem(f.device, "dll") ?? t("Reproduces the original kernel bit for bit, its defects included; CPU only.")}
                />
              </div>
            </FormRow>
            <FormRow label="ini.ini">
              <pre className="engine-ini mono selectable">[engine]{"\n"}{lines}</pre>
            </FormRow>
          </div>
        </Collapsible>
      </div>
    </Section>
  );
}
