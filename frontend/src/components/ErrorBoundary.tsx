import { Component, type ReactNode } from "react";
import { useT } from "../i18n";
import { Button } from "./ui";

/**
 * A lazily loaded part of the interface (3D view, a page) could not be fetched: the page was opened before the
 * interface was rebuilt or updated, and the file names of the parts changed.  Trying again fetches the same old
 * name, so only reloading the page helps.
 */
export function isStaleChunkError(error: unknown): boolean {
  const msg = String((error as Error)?.message ?? error);
  return /dynamically imported module|Importing a module script failed|error loading dynamically imported module|Failed to load module script/i.test(msg);
}

/** The fallback is a function component so it re-renders when the language changes. */
function ErrorView({ error, onRetry }: { error: Error; onRetry: () => void }) {
  const t = useT();
  if (isStaleChunkError(error))
    return (
      <div className="empty-state" role="alert">
        <div style={{ fontSize: 15 }}>{t("The interface was updated after this page was opened.")}</div>
        <p className="soft" style={{ maxWidth: 560, textAlign: "center" }}>
          {t("Reload the page to load the new version. Save your edits first.")}
        </p>
        <Button onClick={() => window.location.reload()}>{t("Reload the page")}</Button>
      </div>
    );
  return (
    <div className="empty-state" role="alert">
      <div className="danger-text" style={{ fontSize: 15 }}>
        {t("This view hit an internal error.")}
      </div>
      <pre className="selectable" style={{ maxWidth: 800, whiteSpace: "pre-wrap", color: "var(--fg-muted)" }}>
        {String(error?.message ?? error)}
      </pre>
      <Button onClick={onRetry}>{t("Try again")}</Button>
    </div>
  );
}

/** Keeps one broken page from blanking the whole window. */
export class ErrorBoundary extends Component<{ children: ReactNode; name?: string }, { error: Error | null }> {
  state = { error: null as Error | null };

  static getDerivedStateFromError(error: Error) {
    return { error };
  }

  componentDidCatch(error: Error) {
    console.error(this.props.name, error);
  }

  render() {
    const { error } = this.state;
    if (!error) return this.props.children;
    return <ErrorView error={error} onRetry={() => this.setState({ error: null })} />;
  }
}
