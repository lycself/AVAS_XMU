// The one-time note after the update that made the new kernel the built-in default (M8b, 2026-11-15).
// Shown once on platforms with the original kernel (Windows, Linux x86-64: there the default changed) to users
// who have not chosen Settings → Default computing kernel themselves; macOS always defaulted to the new kernel.
import { useEffect } from "react";
import { DEFAULT_KERNEL_SETTING } from "../engine";
import { choiceDialog } from "../components/overlays";
import { useT } from "../i18n";
import { persist, useApp } from "../store/app";
import { useEngineInfo } from "../store/engine";
import { KERNEL_NOTICE_SETTING, kernelNoticeState } from "./kernelNoticeState";

export function useKernelNotice() {
  const t = useT();
  const settings = useApp((s) => s.settings);
  const info = useEngineInfo((s) => s.info);
  useEffect(() => {
    const state = kernelNoticeState(settings, info ? info.originalAvailable ?? true : null);
    if (state === "done" || state === "wait") return;
    persist({ [KERNEL_NOTICE_SETTING]: "2026-11" });
    if (state === "skip") return;
    void choiceDialog(
      <>
        <p>{t("Projects that do not choose a computing kernel now run with the new kernel. It computes on the CPU, NVIDIA GPUs and Mac GPUs and corrects defects of the original kernel, so results can differ slightly from earlier runs, for example:")}</p>
        <ul>
          <li>{t("with a fixed random seed the original kernel makes x and the phase of the initial bunch fully correlated;")}</li>
          <li>{t("with multithreading, its space charge in matrix elements (quadrupoles, solenoids, bends) is too strong and differs from run to run;")}</li>
          <li>{t("SPICNIC cuts off the field of long bunches.")}</li>
        </ul>
        <p>{t("To use the original kernel again: Settings → Default computing kernel → Original kernel (all projects), or Simulation settings → Computing kernel for one project. The mode “Reproduce AVAS.dll” of the new kernel gives the original kernel's results bit for bit.")}</p>
      </>,
      [
        { key: "keep", label: t("Keep the new kernel"), variant: "primary" },
        { key: "original", label: t("Use the original kernel by default") },
      ],
      { title: t("New default computing kernel") },
    ).then((choice) => {
      if (choice === "original") persist({ [DEFAULT_KERNEL_SETTING]: "original" });
    });
  }, [settings, info, t]);
}
