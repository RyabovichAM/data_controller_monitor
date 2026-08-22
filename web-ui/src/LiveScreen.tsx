import { useEffect, useState } from "react";
import { fetchCollectors } from "./api";
import { useSensorFeed } from "./useSensorFeed";
import type { CollectorState, FeedStatus } from "./types";

const STATUS_LABEL: Record<FeedStatus, string> = {
  connecting: "подключение",
  online: "на связи",
  offline: "нет связи",
};

function formatValue(value: unknown): string {
  if (value === null || value === undefined) {
    return "—";
  }
  if (typeof value === "object") {
    return JSON.stringify(value);
  }
  return String(value);
}

function CollectorCard({ state }: { state: CollectorState }) {
  const parameters = Object.entries(state.parameters);

  return (
    <article className="card">
      <header>
        <h2>{state.collectorId}</h2>
        <span className="muted">
          {state.messageCount} сообщ. · {state.updatedAt.toLocaleTimeString()}
        </span>
      </header>

      {parameters.length === 0 ? (
        <p className="muted">параметров нет</p>
      ) : (
        <table>
          <tbody>
            {parameters.map(([name, value]) => (
              <tr key={name}>
                <td className="name">{name}</td>
                <td className="value">{formatValue(value)}</td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </article>
  );
}

export function LiveScreen({ onStatus }: { onStatus: (status: FeedStatus) => void }) {
  const { status, collectors } = useSensorFeed();
  const [stored, setStored] = useState<string[]>([]);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    fetchCollectors()
      .then(setStored)
      .catch((reason: Error) => setError(reason.message));
  }, []);

  // The header belongs to the shell, and the status comes from the socket the
  // screen owns.
  useEffect(() => {
    onStatus(status);
  }, [status, onStatus]);

  const live = Object.values(collectors).sort((left, right) =>
    left.collectorId.localeCompare(right.collectorId),
  );

  // A collector may have history and be silent right now — it belongs on the
  // screen just as much, only without values.
  const silent = stored.filter((collectorId) => !(collectorId in collectors));

  return (
    <section>
      {error && <p className="error">Список из хранилища недоступен: {error}</p>}

      {live.length === 0 && silent.length === 0 ? (
        <p className="muted">
          Данных пока нет. Как только collector опубликует первое сообщение, оно появится
          здесь.
        </p>
      ) : (
        <section className="cards">
          {live.map((state) => (
            <CollectorCard key={state.collectorId} state={state} />
          ))}

          {silent.map((collectorId) => (
            <article className="card silent" key={collectorId}>
              <header>
                <h2>{collectorId}</h2>
                <span className="muted">есть история, сейчас молчит</span>
              </header>
            </article>
          ))}
        </section>
      )}
    </section>
  );
}

export { STATUS_LABEL };
