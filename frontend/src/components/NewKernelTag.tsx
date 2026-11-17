// Tag for inputs only the new kernel's default mode reads (schema new_only: outputplane names, lpboundary V2, initpos,
// initmom, nplanes).  The original kernel and the mode "Reproduce AVAS.dll" ignore them; the tag then turns into a
// warning, but the field stays editable so that switching kernels never loses a value.  The run check warns as well
// (avas.engines.new_only_inputs).
import { useT } from "../i18n";
import { Icon } from "./ui";

/** Whether a kernel choice ignores new_only inputs (the original kernel, or the new one with compat = dll). */
export function ignoresNewOnly(kernel?: string | null, compat?: string | null): boolean {
  return kernel === "original" || (kernel === "new" && compat === "dll");
}

export function NewKernelTag({ kernel, compat }: { kernel?: string | null; compat?: string | null }) {
  const tt = useT();
  const ignored = ignoresNewOnly(kernel, compat);
  return (
    <span
      className={ignored ? "badge badge-warning new-kernel-tag" : "badge new-kernel-tag"}
      data-tip={
        ignored
          ? tt("Only the new kernel's default mode reads this; the kernel chosen for this project ignores it. Choose the new kernel in its default mode in Simulation settings → Computing kernel.")
          : tt("New kernel (default mode) only; the original kernel and the mode “Reproduce AVAS.dll” ignore it.")
      }
    >
      {ignored && <Icon name="warning" />}
      {ignored ? tt("Ignored by the current kernel") : tt("New kernel only")}
    </span>
  );
}
