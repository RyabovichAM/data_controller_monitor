import { useCallback, useEffect, useState } from "react";
import { fetchCollectors } from "../api";
import { ApiError } from "../config/api";
import { useSensorFeed } from "../useSensorFeed";
import { deleteScheme, fetchScheme, fetchSchemes, saveScheme } from "./api";
import { LabelPanel } from "./LabelPanel";
import { SchemeCanvas } from "./SchemeCanvas";
import { emptyScheme, TOOLS } from "./types";
import type { Label, Scheme, SchemeSummary, Tool } from "./types";

export function SchemesScreen() {
  const [summaries, setSummaries] = useState<SchemeSummary[]>([]);
  const [scheme, setScheme] = useState<Scheme | null>(null);
  const [collectors, setCollectors] = useState<string[]>([]);
  const [tool, setTool] = useState<Tool>("select");
  const [editable, setEditable] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [dirty, setDirty] = useState(false);
  // Index into scheme.labels of the value whose properties are open.
  const [selected, setSelected] = useState<number | null>(null);

  // The same feed the live screen uses. Subscribed to the scheme's collector,
  // so a scheme of one unit is not repainted by another one's values.
  const { collectors: live } = useSensorFeed(scheme?.collector_id || undefined);
  const values = scheme ? (live[scheme.collector_id]?.parameters ?? {}) : {};
  const parameters = Object.keys(values).sort();
  const selectedLabel = selected !== null ? scheme?.labels[selected] : undefined;

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
      setSelected(null);
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
    setSelected(null);
    setDirty(true);
  };

  const save = async () => {
    if (!scheme) {
      return;
    }

    // config-service refuses it as well, but only here can the culprit be
    // pointed at rather than described.
    const unbound = scheme.labels.findIndex((label) => !label.parameter);
    if (unbound !== -1) {
      setEditable(true);
      setSelected(unbound);
      setError("У значения не выбран параметр");
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

  const updateLabel = (index: number, label: Label) => {
    if (scheme) {
      update({ ...scheme, labels: scheme.labels.map((old, i) => (i === index ? label : old)) });
    }
  };

  const removeLabel = (index: number) => {
    if (scheme) {
      update({ ...scheme, labels: scheme.labels.filter((_, i) => i !== index) });
      setSelected(null);
    }
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
              onClick={() => {
                setEditable(!editable);
                setSelected(null);
              }}
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

      {/* The row is kept while nothing is selected: a panel appearing on
          the first click would push the canvas down under the pointer. */}
      {scheme && editable &&
        (selected !== null && selectedLabel ? (
          <LabelPanel
            key={selected}
            label={selectedLabel}
            parameters={parameters}
            onChange={(label) => updateLabel(selected, label)}
            onRemove={() => removeLabel(selected)}
          />
        ) : (
          <div className="toolbar label-panel">
            <span className="muted">Выберите значение, чтобы изменить его свойства.</span>
          </div>
        ))}

      {scheme ? (
        <>
          <SchemeCanvas
            scheme={scheme}
            tool={tool}
            values={values}
            editable={editable}
            selected={selected}
            onSelect={setSelected}
            onChange={update}
          />
          {editable && (
            <p className="muted">
              Рисование: выберите инструмент и протяните по холсту. «Значение» —
              клик по холсту ставит новое значение, параметр выбирается в панели
              над холстом. В режиме «Выбор»: клик по значению открывает его
              свойства, значения перетаскиваются, клик по фигуре удаляет фигуру.
            </p>
          )}
        </>
      ) : (
        <p className="muted">Схема не выбрана.</p>
      )}
    </section>
  );
}
