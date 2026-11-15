// The kernel a run used, as a small chip ("New kernel · Metal fp32"); nothing for records written
// before AVAS recorded it (see engine.ts engineLabel).
import { engineLabel } from "../engine";
import { useT } from "../i18n";
import { cx, Icon } from "./ui";

export function EngineBadge({ engine, className }: { engine: unknown; className?: string }) {
  useT();
  const label = engineLabel(engine);
  if (!label) return null;
  const predicted = !!(engine as { predicted?: boolean } | null)?.predicted;
  return (
    <span className={cx("engine-badge", label.fallback && "fallback", predicted && "predicted", className)} data-tip={label.tip}>
      <Icon name={label.fallback ? "warning" : "server-process"} />
      <span className="ellipsis">{label.text}</span>
    </span>
  );
}
