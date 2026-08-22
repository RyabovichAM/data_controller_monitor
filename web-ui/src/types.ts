// What the backend pushes over the WebSocket. The payload is whatever the
// controller sent — its schema belongs to the controller, so the parameters are
// discovered rather than declared.
export interface SensorMessage {
  collector_id: string;
  payload: Record<string, unknown>;
}

export interface CollectorState {
  collectorId: string;
  parameters: Record<string, unknown>;
  updatedAt: Date;
  messageCount: number;
}

export type FeedStatus = "connecting" | "online" | "offline";
