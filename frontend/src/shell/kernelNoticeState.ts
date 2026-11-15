// When the one-time note about the new default kernel is shown (shell/kernelNotice.tsx).
import { DEFAULT_KERNEL_SETTING } from "../engine";

export const KERNEL_NOTICE_SETTING = "engine/newDefaultNotice";

type Settings = Record<string, unknown>;

/** "show": the note is due; "skip": record it as seen without showing (nothing changed for this user);
 *  "done": already seen; "wait": the kernel information is not known yet. */
export function kernelNoticeState(settings: Settings, originalAvailable: boolean | null | undefined): "show" | "skip" | "done" | "wait" {
  if (settings[KERNEL_NOTICE_SETTING]) return "done";
  if (originalAvailable == null) return "wait";
  if (!originalAvailable) return "skip";
  const pref = settings[DEFAULT_KERNEL_SETTING];
  if (pref === "original" || pref === "new") return "skip";
  return "show";
}
