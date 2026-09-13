import type { Label } from "./types";

// The colour a value gets when none is set, read from the stylesheet so the
// picker starts from what the canvas actually shows.
function defaultColor(): string {
  const value = getComputedStyle(document.documentElement)
    .getPropertyValue("--series-1")
    .trim();
  return value || "#3987e5";
}

// Properties of the selected value. Every field of Label is here: the
// contract has had them from the start, and until now only the parameter
// could be set from the browser.
export function LabelPanel({
  label,
  parameters,
  onChange,
  onRemove,
}: {
  label: Label;
  // Names seen in the live feed. Offered, not imposed: a collector silent
  // right now still has parameters worth binding to.
  parameters: string[];
  onChange: (label: Label) => void;
  onRemove: () => void;
}) {
  return (
    <div className="toolbar label-panel">
      <label className="inline">
        <span>Параметр</span>
        {/* A value just dropped on the canvas has no parameter yet, so the
            cursor goes straight to it. */}
        <input
          list="scheme-parameters"
          value={label.parameter}
          placeholder="temperature"
          autoFocus={!label.parameter}
          onChange={(event) => onChange({ ...label, parameter: event.target.value })}
        />
        <datalist id="scheme-parameters">
          {parameters.map((parameter) => (
            <option key={parameter} value={parameter} />
          ))}
        </datalist>
      </label>

      <label className="inline">
        <span>Подпись</span>
        <input
          value={label.caption ?? ""}
          placeholder={label.parameter || "как параметр"}
          onChange={(event) => onChange({ ...label, caption: event.target.value })}
        />
      </label>

      <label className="inline">
        <span>Единицы</span>
        <input
          className="narrow"
          value={label.units ?? ""}
          placeholder="°C"
          onChange={(event) => onChange({ ...label, units: event.target.value })}
        />
      </label>

      <label className="inline">
        <span>Размер</span>
        {/* Empty means the default size, so the field can be cleared and
            retyped rather than snapping back to 16 on the first keystroke. */}
        <input
          className="narrow"
          type="number"
          min={8}
          max={96}
          value={label.font_size || ""}
          placeholder="16"
          onChange={(event) => {
            const size = Number(event.target.value);
            onChange({ ...label, font_size: size > 0 ? size : undefined });
          }}
        />
      </label>

      <label className="inline">
        <span>Цвет</span>
        <input
          type="color"
          value={label.color || defaultColor()}
          onChange={(event) => onChange({ ...label, color: event.target.value })}
        />
        {/* Empty colour is the viewer's default ink, not a colour that
            happens to match it today. */}
        <button
          type="button"
          disabled={!label.color}
          onClick={() => onChange({ ...label, color: undefined })}
        >
          сброс
        </button>
      </label>

      {/* Named in full: the scheme's own "Удалить" sits one row above. */}
      <button type="button" onClick={onRemove}>
        Удалить значение
      </button>
    </div>
  );
}
