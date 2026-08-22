import { useCallback, useState } from "react";
import { ConfigsScreen } from "./config/ConfigsScreen";
import { LiveScreen, STATUS_LABEL } from "./LiveScreen";
import type { FeedStatus } from "./types";

type Tab = "live" | "configs";

export default function App() {
  const [tab, setTab] = useState<Tab>("live");
  const [status, setStatus] = useState<FeedStatus>("connecting");

  // Passed down so the live screen can report the state of its socket without
  // the shell owning the connection.
  const onStatus = useCallback((next: FeedStatus) => setStatus(next), []);

  return (
    <main>
      <header className="top">
        <h1>Data Controller Monitor</h1>

        <nav className="tabs">
          <button
            type="button"
            className={tab === "live" ? "active" : ""}
            onClick={() => setTab("live")}
          >
            Живые значения
          </button>
          <button
            type="button"
            className={tab === "configs" ? "active" : ""}
            onClick={() => setTab("configs")}
          >
            Настройки
          </button>
        </nav>

        <span className={`status ${status}`}>{STATUS_LABEL[status]}</span>
      </header>

      {/* The live screen keeps rendering while hidden, so the socket and the
          values it has collected survive a trip to the settings and back. */}
      <div hidden={tab !== "live"}>
        <LiveScreen onStatus={onStatus} />
      </div>

      {tab === "configs" && <ConfigsScreen />}
    </main>
  );
}
