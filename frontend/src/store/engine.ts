// The kernels and devices of this computer (engine.info), shared by the Simulation settings page and the About
// window.  The back end asks a child process the first time (about a second) and caches the answer; refresh()
// asks again (a new driver, an external GPU, a rebuilt library).
import { useEffect, useState } from "react";
import { create } from "zustand";
import { call } from "../bridge";
import { DEFAULT_KERNEL_SETTING, type EngineInfo, type Kernel } from "../engine";
import { useApp } from "./app";

type EngineInfoState = {
  info: EngineInfo | null;
  loading: boolean;
  error: string | null;
  load: (refresh?: boolean) => Promise<EngineInfo | null>;
};

let pending: Promise<EngineInfo | null> | null = null;

export const useEngineInfo = create<EngineInfoState>((set, get) => ({
  info: null,
  loading: false,
  error: null,
  load: (refresh = false) => {
    if (!refresh && get().info) return Promise.resolve(get().info);
    if (pending && !refresh) return pending;
    set({ loading: true });
    pending = call<EngineInfo>("engine.info", { refresh })
      .then((info) => {
        set({ info, loading: false, error: null });
        return info;
      })
      .catch((e: any) => {
        set({ loading: false, error: e?.message ?? String(e) });
        return null;
      })
      .finally(() => {
        pending = null;
      });
    return pending;
  },
}));

/** engine.effective: the kernel a run of the open project resolves to (ini.ini [engine] under the environment, the
 *  global default kernel, the built-in default); null while unknown. */
export type EffectiveKernel = { kernel: Kernel | null; compat: string | null; source: string | null; note: string | null; error: string | null };

/** The open project's effective kernel, asked again when the project, the global default kernel or the shown page
 *  changes (the Simulation settings page may have changed ini.ini meanwhile). */
export function useEffectiveKernel(enabled = true): EffectiveKernel | null {
  const path = useApp((s) => s.project.path);
  const pref = useApp((s) => s.settings[DEFAULT_KERNEL_SETTING]);
  const page = useApp((s) => s.page);
  const [value, setValue] = useState<EffectiveKernel | null>(null);
  useEffect(() => {
    if (!enabled || !path) {
      setValue(null);
      return;
    }
    let alive = true;
    // the preference is saved a moment after the menu sets it (persist): ask after that
    const timer = window.setTimeout(() => {
      call<EffectiveKernel>("engine.effective")
        .then((v) => alive && setValue(v))
        .catch(() => alive && setValue(null));
    }, 400);
    return () => {
      alive = false;
      window.clearTimeout(timer);
    };
  }, [enabled, path, pref, page]);
  return value;
}
