import { useEffect, useRef, useState } from "react";
import type { CollectorState, FeedStatus, SensorMessage } from "./types";

const RECONNECT_DELAY_MS = 2000;

// Subscribes to the live feed and keeps the latest values of every collector.
//
// The socket outlives renders, so it is held in a ref: putting it in state
// would reconnect on every message.
export function useSensorFeed(collectorId?: string) {
  const [status, setStatus] = useState<FeedStatus>("connecting");
  const [collectors, setCollectors] = useState<Record<string, CollectorState>>({});
  const socketRef = useRef<WebSocket | null>(null);

  useEffect(() => {
    let closedByUs = false;
    let reconnectTimer: number | undefined;

    const connect = () => {
      const protocol = window.location.protocol === "https:" ? "wss" : "ws";
      const query = collectorId ? `?collector_id=${encodeURIComponent(collectorId)}` : "";
      const socket = new WebSocket(`${protocol}://${window.location.host}/ws/sensor-data${query}`);
      socketRef.current = socket;

      socket.onopen = () => setStatus("online");

      socket.onmessage = (event: MessageEvent<string>) => {
        let message: SensorMessage;
        try {
          message = JSON.parse(event.data) as SensorMessage;
        } catch {
          return;   // the backend drops non-JSON, but the type says nothing
        }

        setCollectors((previous) => {
          const known = previous[message.collector_id];
          return {
            ...previous,
            [message.collector_id]: {
              collectorId: message.collector_id,
              // Merged rather than replaced: a controller may send a subset of
              // its parameters in one message.
              parameters: { ...known?.parameters, ...message.payload },
              updatedAt: new Date(),
              messageCount: (known?.messageCount ?? 0) + 1,
            },
          };
        });
      };

      socket.onclose = () => {
        setStatus("offline");
        if (!closedByUs) {
          reconnectTimer = window.setTimeout(connect, RECONNECT_DELAY_MS);
        }
      };

      socket.onerror = () => socket.close();
    };

    connect();

    return () => {
      closedByUs = true;
      window.clearTimeout(reconnectTimer);
      socketRef.current?.close();
    };
  }, [collectorId]);

  return { status, collectors };
}
