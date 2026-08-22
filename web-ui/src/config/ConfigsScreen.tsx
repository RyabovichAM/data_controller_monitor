import { useCallback, useEffect, useState } from "react";
import { ApiError, deleteConfig, fetchConfigs, saveConfig } from "./api";
import { ConfigForm, emptyConfig } from "./ConfigForm";
import type { CollectorConfig } from "./types";

function describe(config: CollectorConfig): string {
  const tcp = config.transfer.tcp_ip;
  if (tcp) {
    return `TCP/IP ${tcp.host}:${tcp.port}`;
  }

  const serial = config.transfer.serial;
  if (serial) {
    return `Serial ${serial.port_name}`;
  }

  return "транспорт не задан";
}

export function ConfigsScreen() {
  const [configs, setConfigs] = useState<CollectorConfig[]>([]);
  const [editing, setEditing] = useState<CollectorConfig | null>(null);
  const [isNew, setIsNew] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const reload = useCallback(async () => {
    try {
      setConfigs(await fetchConfigs());
      setError(null);
    } catch (reason) {
      setError((reason as Error).message);
    }
  }, []);

  useEffect(() => {
    void reload();
  }, [reload]);

  const save = async (config: CollectorConfig) => {
    try {
      await saveConfig(config);
      setEditing(null);
      setError(null);
      await reload();
    } catch (reason) {
      const failure = reason as ApiError;
      // 409 means the config changed under the form; the list below is
      // refreshed so the operator can start from what is stored now.
      setError(
        failure.status === 409
          ? `${failure.message} — конфиг изменили, форма перечитана`
          : failure.message,
      );
      if (failure.status === 409) {
        setEditing(null);
        await reload();
      }
    }
  };

  const remove = async (collectorId: string) => {
    if (!window.confirm(`Удалить конфиг ${collectorId}?`)) {
      return;
    }

    try {
      await deleteConfig(collectorId);
      await reload();
    } catch (reason) {
      setError((reason as Error).message);
    }
  };

  return (
    <section>
      <div className="toolbar">
        <button
          type="button"
          onClick={() => {
            setEditing(emptyConfig());
            setIsNew(true);
          }}
        >
          Добавить collector
        </button>
        <span className="muted">
          Сохранение применяется на лету: collector перенастраивается без перезапуска.
        </span>
      </div>

      {error && <p className="error">{error}</p>}

      {editing && (
        <ConfigForm
          initial={editing}
          isNew={isNew}
          onSave={save}
          onCancel={() => setEditing(null)}
        />
      )}

      {configs.length === 0 && !editing ? (
        <p className="muted">Ни одного collector'а не настроено.</p>
      ) : (
        <section className="cards">
          {configs.map((config) => (
            <article className="card" key={config.collector_id}>
              <header>
                <h2>{config.collector_id}</h2>
                <span className="muted">версия {config.version ?? "0"}</span>
              </header>

              <p className="muted">{describe(config)}</p>
              <p className="muted">
                {config.kafka.brokers} · {config.kafka.topic}
              </p>

              <footer>
                <button
                  type="button"
                  onClick={() => {
                    setEditing(config);
                    setIsNew(false);
                  }}
                >
                  Изменить
                </button>
                <button type="button" onClick={() => void remove(config.collector_id)}>
                  Удалить
                </button>
              </footer>
            </article>
          ))}
        </section>
      )}
    </section>
  );
}
