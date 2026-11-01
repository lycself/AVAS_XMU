// The latest result of an RPC whose parameters change while the reader drags a
// slider: requests are serialized and only the newest parameters are sent.
import { useEffect, useRef, useState } from "react";
import { call } from "../bridge";

export function useLatest<T>(method: string, params: Record<string, unknown> | null, delay = 120) {
  const [result, setResult] = useState<T | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);
  const pending = useRef<Record<string, unknown> | null>(null);
  const inflight = useRef(false);
  const alive = useRef(true);
  useEffect(() => () => void (alive.current = false), []);
  const key = JSON.stringify(params);
  useEffect(() => {
    if (!params) return;
    pending.current = params;
    const timer = setTimeout(pump, delay);
    return () => clearTimeout(timer);
  }, [key]);
  async function pump() {
    if (inflight.current || !pending.current) return;
    const p = pending.current;
    pending.current = null;
    inflight.current = true;
    setBusy(true);
    try {
      const r = await call<T>(method, p);
      if (alive.current) {
        setResult(r);
        setError(null);
      }
    } catch (e) {
      if (alive.current) setError(String((e as Error)?.message ?? e));
    } finally {
      inflight.current = false;
      if (alive.current) setBusy(false);
      if (pending.current) pump();
    }
  }
  return { result, error, busy };
}
