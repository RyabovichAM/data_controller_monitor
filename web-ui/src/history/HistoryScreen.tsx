import { useCallback, useEffect, useState } from "react";
import { fetchCollectors } from "../api";
import { fetchHistory } from "./api";
import type { History } from "./api";
import { Chart } from "./Chart";

const RANGES: Array<[string, number]> = [
  ["Час", 3600],
  ["6 часов", 6 * 3600],
  ["Сутки", 24 * 3600],
  ["Неделя", 7 * 24 * 3600],
];

export function HistoryScreen() {
  const [collectors, setCollectors] = useState<string[]>([]);
  const [collectorId, setCollectorId] = useState("");
  const [rangeSeconds, setRangeSeconds] = useState(RANGES[0][1]);
  const [history, setHistory] = useState<History | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    fetchCollectors()
      .then((list) => {
        setCollectors(list);
        setCollectorId((current) => current || list[0] || "");
      })
      .catch((reason: Error) => setError(reason.message));
  }, []);

  const load = useCallback(async () => {
    if (!collectorId) {
      return;
    }

    setBusy(true);
    try {
      const to = Math.floor(Date.now() / 1000);
      setHistory(await fetchHistory(collectorId, to - rangeSeconds, to));
      setError(null);
    } catch (reason) {
      setError((reason as Error).message);
      setHistory(null);
    } finally {
      setBusy(false);
    }
  }, [collectorId, rangeSeconds]);

  useEffect(() => {
    void load();
  }, [load]);

  const parameters = history ? Object.keys(history.series).sort() : [];

  return (
    <section>
      <div className="toolbar">
        <select value={collectorId} onChange={(event) => setCollectorId(event.target.value)}>
          {collectors.map((collector) => (
            <option key={collector} value={collector}>
              {collector}
            </option>
          ))}
        </select>

        <div className="switch">
          {RANGES.map(([label, seconds]) => (
            <button
              key={label}
              type="button"
              className={rangeSeconds === seconds ? "active" : ""}
              onClick={() => setRangeSeconds(seconds)}
            >
              {label}
            </button>
          ))}
        </div>

        <button type="button" onClick={() => void load()} disabled={busy}>
          {busy ? "Загрузка…" : "Обновить"}
        </button>

        {history && (
          <span className="muted">
            {history.t.length} точек
            {history.t.length >= 5000 && " · показаны первые, сузьте диапазон"}
          </span>
        )}
      </div>

      {error && <p className="error">{error}</p>}

      {!error && parameters.length === 0 && !busy && (
        <p className="muted">За выбранный период данных нет.</p>
      )}

      <section className="cards wide">
        {history &&
          parameters.map((name) => (
            <Chart
              key={name}
              title={name}
              times={history.t}
              values={history.series[name]}
            />
          ))}
      </section>
    </section>
  );
}
