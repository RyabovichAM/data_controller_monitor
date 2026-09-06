import { useCallback, useEffect, useState } from "react";
import { fetchCollectors } from "../api";
import { ApiError } from "../config/api";
import { useSensorFeed } from "../useSensorFeed";
import { deleteScheme, fetchScheme, fetchSchemes, saveScheme } from "./api";
import { SchemeCanvas } from "./SchemeCanvas";
import { emptyScheme, TOOLS } from "./types";
import type { Scheme, SchemeSummary, Tool } from "./types";

export function SchemesScreen() {
  const [summaries, setSummaries] = useState<SchemeSummary[]>([]);
  const [scheme, setScheme] = useState<Scheme | null>(null);
  const [collectors, setCollectors] = useState<string[]>([]);
  const [tool, setTool] = useState<Tool>("select");
  const [editable, setEditable] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [dirty, setDirty] = useState(false);

  // The same feed the live screen uses. Subscribed to the scheme's collector,
  // so a scheme of one unit is not repainted by another one's values.
  const { collectors: live } = useSensorFeed(scheme?.collector_id || undefined);
  const values = scheme ? (live[scheme.collector_id]?.parameters ?? {}) : {};

  const reload = useCallback(async () => {
    try {
      setSummaries(await fetchSchemes());
      setError(null);
    } catch (reason) {
      setError((reason as Error).message);
    }
  }, []);

  useEffect(() => {
    void reload();
    fetchCollectors()
      .then(setCollectors)
      .catch(() => undefined);   // the list is a convenience, not a requirement
  }, [reload]);

  const open = async (schemeId: string) => {
    try {
      setScheme(await fetchScheme(schemeId));
      setEditable(false);
      setDirty(false);
      setError(null);
    } catch (reason) {
      setError((reason as Error).message);
    }
  };

  const create = () => {
    const schemeId = window.prompt("Идентификатор схемы, например boiler-1");
    if (!schemeId) {
      return;
    }

    setScheme(emptyScheme(schemeId));
    setEditable(true);
    setDirty(true);
  };

  const save = async () => {
    if (!scheme) {
      return;
    }

    try {
      const version = await saveScheme(scheme);
      // The version returned becomes the base of the next edit, otherwise the
      // second save of a session would always conflict with the first.
      setScheme({ ...scheme, version: String(version) });
      setDirty(false);
      setError(null);
      await reload();
    } catch (reason) {
      const failure = reason as ApiError;
      setError(
        failure.status === 409
          ? `${failure.message} — схему изменили, откройте её заново`
          : failure.message,
      );
    }
  };

  const remove = async (schemeId: string) => {
    if (!window.confirm(`Удалить схему ${schemeId}?`)) {
      return;
    }

    try {
      await deleteScheme(schemeId);
      if (scheme?.scheme_id === schemeId) {
        setScheme(null);
      }
      await reload();
    } catch (reason) {
      setError((reason as Error).message);
    }
  };

  const update = (next: Scheme) => {
    setScheme(next);
    setDirty(true);
  };

  return (
    <section>
      <div className="toolbar">
        <button type="button" onClick={create}>
          Новая схема
        </button>

        <select
          value={scheme?.scheme_id ?? ""}
          onChange={(event) => event.target.value && void open(event.target.value)}
        >
          <option value="">— выбрать схему —</option>
          {summaries.map((summary) => (
            <option key={summary.scheme_id} value={summary.scheme_id}>
              {summary.title}
            </option>
          ))}
        </select>

        {scheme && (
          <>
            <button
              type="button"
              className={editable ? "active" : ""}
              onClick={() => setEditable(!editable)}
            >
              {editable ? "Правка" : "Просмотр"}
            </button>
            <button type="button" onClick={() => void save()} disabled={!dirty}>
              Сохранить
            </button>
            <button type="button" onClick={() => void remove(scheme.scheme_id)}>
              Удалить
            </button>
            {dirty && <span className="muted">не сохранено</span>}
          </>
        )}
      </div>

      {error && <p className="error">{error}</p>}

      {scheme && editable && (
        <div className="toolbar">
          <div className="switch">
            {TOOLS.map(([value, label]) => (
              <button
                key={value}
                type="button"
                className={tool === value ? "active" : ""}
                onClick={() => setTool(value)}
              >
                {label}
              </button>
            ))}
          </div>

          <label className="inline">
            <span>Название</span>
            <input
              value={scheme.title}
              onChange={(event) => update({ ...scheme, title: event.target.value })}
            />
          </label>

          <label className="inline">
            <span>Collector</span>
            <select
              value={scheme.collector_id}
              onChange={(event) => update({ ...scheme, collector_id: event.target.value })}
            >
              <option value="">— нет —</option>
              {collectors.map((collector) => (
                <option key={collector} value={collector}>
                  {collector}
                </option>
              ))}
            </select>
          </label>
        </div>
      )}

      {scheme ? (
        <>
          <SchemeCanvas
            scheme={scheme}
            tool={tool}
            values={values}
            editable={editable}
            onChange={update}
          />
          {editable && (
            <p className="muted">
              Рисование: выберите инструмент и протяните по холсту. «Значение» —
              клик по холсту, затем имя параметра. В режиме «Выбор»: значения
              перетаскиваются, двойной клик удаляет значение, клик по фигуре
              удаляет фигуру.
            </p>
          )}
        </>
      ) : (
        <p className="muted">Схема не выбрана.</p>
      )}
    </section>
  );
}
