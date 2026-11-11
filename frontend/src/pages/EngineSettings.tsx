// Simulation settings → Computing kernel: ini.ini [engine] of the project (kernel, device, precision, GPUs,
// threads, compatibility mode).  Only the devices this computer has are offered; with device Auto the group
// shows what a run of this project will use.  Saved with the page (settings.save), only when it changed.
import { useEffect, useMemo, useState } from "react";
import { call } from "../bridge";
import { Checkbox, Collapsible, Icon, IconButton, Radio, Section, Spinner, TextInput } from "../components/ui";
import {
  compatProblem,
  deviceChoices,
  deviceName,
  formatGpus,
  iniLines,
  parseGpus,
  precisionProblem,
  type EngineForm,
  type Plan,
} from "../engine";
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
  defaultKernel: string;
  platform: string;
};

type Preview = { kernel: string; spaceCharge?: boolean; plan: Plan | null; other: Plan | null; error: string | null };

function planText(plan: Plan | null | undefined, t: ReturnType<typeof useT>): string {
  if (!plan) return "–";
  if (plan.error) return t("cannot run: {reason}", { reason: plan.error });
  const gpus = plan.gpus.length ? ` (GPU ${plan.gpus.join(", ")})` : "";
  return `${deviceName(plan.device)} ${plan.precision ?? ""}${gpus}`.trim();
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

  const isNew = f.kernel === "new";
  const libraryOk = !!info?.new.available;
  const devices = deviceChoices(info);
  const cuda = info?.devices?.cuda ?? [];

  // what a run of this project would use with the form's choice (asked from the back end, debounced)
  const [preview, setPreview] = useState<Preview | null>(null);
  const key = JSON.stringify([f, spaceCharge, isNew && libraryOk]);
  useEffect(() => {
    if (!isNew || !libraryOk) {
      setPreview(null);
      return;
    }
    let alive = true;
    const timer = window.setTimeout(() => {
      call<Preview>("engine.preview", { choice: f, spaceCharge })
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
        <FormRow label={t("Kernel")} tip={t("Stored in ini.ini [engine] of this project. 'avas run --engine' and the AVAS_ENGINE variable override it.")}>
          <div className="radio-row">
            <Radio
              checked={f.kernel === "original"}
              disabled={!data.originalAvailable && f.kernel !== "original"}
              onChange={() => set("kernel", "original")}
              label={t("Original kernel")}
              tip={data.originalAvailable ? t("AVAS.dll / libAVAS.so, the kernel AVAS has always used") : t("The original kernel does not run on {platform}", { platform: data.platform })}
            />
            <Radio
              checked={isNew}
              disabled={!!info && !libraryOk && !isNew}
              onChange={() => set("kernel", "new")}
              label={t("New kernel (preview)")}
              tip={t("The rebuilt kernel: CPU, NVIDIA GPU or Mac GPU; supports the PICNIC solver")}
            />
          </div>
        </FormRow>
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
            <FormRow label={t("CPU threads")} tip={t("0 = automatic (all cores). Only affects the speed: results of the new kernel do not depend on the thread count.")}>
              <TextInput
                style={{ width: 80 }}
                disabled={!isNew || locked}
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
