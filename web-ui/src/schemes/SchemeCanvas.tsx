import { useRef, useState } from "react";
import type { Label, Point, Scheme, Shape, ShapeKind, Tool } from "./types";

// SVG rather than canvas: a scheme is a few dozen shapes that have to be
// clickable and selectable, and the DOM already does hit-testing. A canvas
// would mean maintaining a scene graph by hand for no gain at this size.

function shapeElement(shape: Shape, key: number, extra: Record<string, unknown> = {}) {
  const stroke = shape.stroke || "var(--text)";
  const width = shape.stroke_width || 2;
  const fill = shape.fill || "none";
  const [first, last] = [shape.points[0], shape.points[shape.points.length - 1]];

  switch (shape.kind) {
    case "SHAPE_KIND_RECTANGLE":
      return (
        <rect
          key={key}
          x={Math.min(first.x, last.x)}
          y={Math.min(first.y, last.y)}
          width={Math.abs(last.x - first.x)}
          height={Math.abs(last.y - first.y)}
          stroke={stroke}
          strokeWidth={width}
          fill={fill}
          {...extra}
        />
      );
    case "SHAPE_KIND_ELLIPSE":
      return (
        <ellipse
          key={key}
          cx={(first.x + last.x) / 2}
          cy={(first.y + last.y) / 2}
          rx={Math.abs(last.x - first.x) / 2}
          ry={Math.abs(last.y - first.y) / 2}
          stroke={stroke}
          strokeWidth={width}
          fill={fill}
          {...extra}
        />
      );
    case "SHAPE_KIND_LINE":
      return (
        <line
          key={key}
          x1={first.x}
          y1={first.y}
          x2={last.x}
          y2={last.y}
          stroke={stroke}
          strokeWidth={width}
          {...extra}
        />
      );
    case "SHAPE_KIND_POLYLINE":
      return (
        <polyline
          key={key}
          points={shape.points.map((point) => `${point.x},${point.y}`).join(" ")}
          stroke={stroke}
          strokeWidth={width}
          fill="none"
          {...extra}
        />
      );
  }
}

export function SchemeCanvas({
  scheme,
  tool,
  values,
  editable,
  onChange,
}: {
  scheme: Scheme;
  tool: Tool;
  // Live values by parameter name; undefined while nothing has arrived yet.
  values: Record<string, unknown>;
  editable: boolean;
  onChange: (scheme: Scheme) => void;
}) {
  const surface = useRef<SVGSVGElement>(null);
  const [drawing, setDrawing] = useState<Shape | null>(null);
  const [draggedLabel, setDraggedLabel] = useState<number | null>(null);

  // Pointer coordinates in the scheme's own units, not the screen's: the SVG
  // is scaled to fit, so the two differ whenever the window is not exactly the
  // canvas size.
  const toCanvas = (event: React.PointerEvent): Point => {
    const box = surface.current?.getBoundingClientRect();
    if (!box) {
      return { x: 0, y: 0 };
    }

    return {
      x: ((event.clientX - box.left) / box.width) * scheme.width,
      y: ((event.clientY - box.top) / box.height) * scheme.height,
    };
  };

  const onPointerDown = (event: React.PointerEvent) => {
    if (!editable || tool === "select") {
      return;
    }

    const point = toCanvas(event);

    if (tool === "label") {
      const parameter = window.prompt("Имя параметра, например temperature");
      if (parameter) {
        onChange({
          ...scheme,
          labels: [...scheme.labels, { position: point, parameter, font_size: 16 }],
        });
      }
      return;
    }

    setDrawing({ kind: tool as ShapeKind, points: [point, point], stroke_width: 2 });
  };

  const onPointerMove = (event: React.PointerEvent) => {
    if (draggedLabel !== null) {
      const point = toCanvas(event);
      const labels = scheme.labels.map((label, index) =>
        index === draggedLabel ? { ...label, position: point } : label,
      );
      onChange({ ...scheme, labels });
      return;
    }

    if (!drawing) {
      return;
    }

    const point = toCanvas(event);
    // A freehand stroke keeps every point; the others are defined by two, so
    // the second one simply follows the pointer.
    setDrawing(
      drawing.kind === "SHAPE_KIND_POLYLINE"
        ? { ...drawing, points: [...drawing.points, point] }
        : { ...drawing, points: [drawing.points[0], point] },
    );
  };

  const onPointerUp = () => {
    if (draggedLabel !== null) {
      setDraggedLabel(null);
      return;
    }

    if (!drawing) {
      return;
    }

    // A click that never moved is not a shape.
    const [first, last] = [drawing.points[0], drawing.points[drawing.points.length - 1]];
    if (Math.abs(last.x - first.x) > 2 || Math.abs(last.y - first.y) > 2) {
      onChange({ ...scheme, shapes: [...scheme.shapes, drawing] });
    }

    setDrawing(null);
  };

  const removeShape = (index: number) => {
    if (editable && tool === "select") {
      onChange({ ...scheme, shapes: scheme.shapes.filter((_, i) => i !== index) });
    }
  };

  const removeLabel = (index: number) => {
    onChange({ ...scheme, labels: scheme.labels.filter((_, i) => i !== index) });
  };

  const formatValue = (label: Label): string => {
    const value = values[label.parameter];
    if (value === undefined || value === null) {
      return "—";
    }
    return typeof value === "object" ? JSON.stringify(value) : String(value);
  };

  return (
    <svg
      ref={surface}
      className={`scheme ${editable ? "editable" : ""}`}
      viewBox={`0 0 ${scheme.width} ${scheme.height}`}
      preserveAspectRatio="xMidYMid meet"
      onPointerDown={onPointerDown}
      onPointerMove={onPointerMove}
      onPointerUp={onPointerUp}
      onPointerLeave={onPointerUp}
    >
      {scheme.shapes.map((shape, index) =>
        shapeElement(shape, index, {
          onClick: () => removeShape(index),
          style: editable && tool === "select" ? { cursor: "pointer" } : undefined,
        }),
      )}

      {drawing && shapeElement(drawing, -1, { opacity: 0.7 })}

      {scheme.labels.map((label, index) => (
        <g
          key={index}
          transform={`translate(${label.position.x} ${label.position.y})`}
          onPointerDown={(event) => {
            if (editable && tool === "select") {
              event.stopPropagation();
              setDraggedLabel(index);
            }
          }}
          onDoubleClick={() => editable && removeLabel(index)}
          style={editable && tool === "select" ? { cursor: "move" } : undefined}
        >
          {/* The caption sits a whole line above the value, measured in the
              value's own font size — a fixed offset collides as soon as the
              operator picks a larger one. */}
          <text
            className="scheme-caption"
            y={-(label.font_size || 16) * 0.9}
            fontSize={(label.font_size || 16) * 0.6}
          >
            {label.caption || label.parameter}
          </text>
          <text
            className="scheme-value"
            fontSize={label.font_size || 16}
            fill={label.color || "var(--series-1)"}
          >
            {formatValue(label)}
            {label.units ? ` ${label.units}` : ""}
          </text>
        </g>
      ))}
    </svg>
  );
}
