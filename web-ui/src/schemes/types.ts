// The shape of Scheme as protobuf writes it to JSON, same convention as the
// collector configs: field names from scheme_service.proto, enum values by
// name, int64 as a string.

export type ShapeKind =
  | "SHAPE_KIND_RECTANGLE"
  | "SHAPE_KIND_ELLIPSE"
  | "SHAPE_KIND_LINE"
  | "SHAPE_KIND_POLYLINE";

export interface Point {
  x: number;
  y: number;
}

export interface Shape {
  kind: ShapeKind;
  points: Point[];
  stroke?: string;
  fill?: string;
  stroke_width?: number;
}

// A live value placed on the canvas. `parameter` is the JSON key the
// controller sends, which is what ties a scheme to one collector.
export interface Label {
  position: Point;
  parameter: string;
  caption?: string;
  units?: string;
  font_size?: number;
  color?: string;
}

export interface Scheme {
  scheme_id: string;
  title: string;
  collector_id: string;
  width: number;
  height: number;
  shapes: Shape[];
  labels: Label[];
  version?: string;
}

// What ListSchemes returns: enough to fill a menu, without the drawing.
export interface SchemeSummary {
  scheme_id: string;
  title: string;
  collector_id: string;
  version?: string;
}

// The tool the pointer is currently holding. "select" is the one that does not
// draw: it moves labels and picks shapes to delete.
export type Tool = ShapeKind | "select" | "label";

export const TOOLS: Array<[Tool, string]> = [
  ["select", "Выбор"],
  ["SHAPE_KIND_RECTANGLE", "Прямоугольник"],
  ["SHAPE_KIND_ELLIPSE", "Эллипс"],
  ["SHAPE_KIND_LINE", "Линия"],
  ["SHAPE_KIND_POLYLINE", "Кривая"],
  ["label", "Значение"],
];

export function emptyScheme(schemeId: string): Scheme {
  return {
    scheme_id: schemeId,
    title: "Новая схема",
    collector_id: "",
    width: 900,
    height: 600,
    shapes: [],
    labels: [],
  };
}
