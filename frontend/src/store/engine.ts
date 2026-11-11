// The kernels and devices of this computer (engine.info), shared by the Simulation settings page and the About
// window.  The back end asks a child process the first time (about a second) and caches the answer; refresh()
// asks again (a new driver, an external GPU, a rebuilt library).
import { create } from "zustand";
import { call } from "../bridge";
import type { EngineInfo } from "../engine";

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
