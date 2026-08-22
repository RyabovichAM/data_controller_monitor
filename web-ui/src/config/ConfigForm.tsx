import { useState } from "react";
import {
  BAUD_RATES,
  DATA_BITS,
  FLOW_CONTROLS,
  PARITIES,
  STOP_BITS,
} from "./types";
import type {
  CollectorConfig,
  SerialTransfer,
  TcpIpTransfer,
  TransferKind,
} from "./types";

const EMPTY_TCP: TcpIpTransfer = { host: "0.0.0.0", port: 2323 };

const EMPTY_SERIAL: SerialTransfer = {
  port_name: "/dev/ttyUSB0",
  baud_rate: "BAUD_RATE_115200",
  data_bits: "DATA_BITS_8",
  parity: "PARITY_NONE",
  stop_bits: "STOP_BITS_ONE",
  flow_control: "FLOW_CONTROL_NONE",
};

export function emptyConfig(): CollectorConfig {
  return {
    collector_id: "",
    transfer: { tcp_ip: { ...EMPTY_TCP } },
    kafka: { brokers: "kafka:9092", topic: "sensor-data" },
  };
}

// A select over the label pairs of the contract.
function EnumSelect<T extends string>({
  label,
  value,
  options,
  onChange,
}: {
  label: string;
  value: T;
  options: Array<[T, string]>;
  onChange: (value: T) => void;
}) {
  return (
    <label>
      <span>{label}</span>
      <select value={value} onChange={(event) => onChange(event.target.value as T)}>
        {options.map(([option, text]) => (
          <option key={option} value={option}>
            {text}
          </option>
        ))}
      </select>
    </label>
  );
}

export function ConfigForm({
  initial,
  isNew,
  onSave,
  onCancel,
}: {
  initial: CollectorConfig;
  isNew: boolean;
  onSave: (config: CollectorConfig) => Promise<void>;
  onCancel: () => void;
}) {
  const [draft, setDraft] = useState<CollectorConfig>(initial);
  const [busy, setBusy] = useState(false);

  // Both variants are kept while the form is open, so flipping the switch back
  // and forth does not lose what was typed.
  const [tcp, setTcp] = useState<TcpIpTransfer>(initial.transfer.tcp_ip ?? { ...EMPTY_TCP });
  const [serial, setSerial] = useState<SerialTransfer>(
    initial.transfer.serial ?? { ...EMPTY_SERIAL },
  );

  const kind: TransferKind = draft.transfer.serial ? "serial" : "tcp_ip";

  const setKind = (next: TransferKind) => {
    setDraft({
      ...draft,
      transfer: next === "serial" ? { serial } : { tcp_ip: tcp },
    });
  };

  const updateTcp = (patch: Partial<TcpIpTransfer>) => {
    const next = { ...tcp, ...patch };
    setTcp(next);
    setDraft({ ...draft, transfer: { tcp_ip: next } });
  };

  const updateSerial = (patch: Partial<SerialTransfer>) => {
    const next = { ...serial, ...patch };
    setSerial(next);
    setDraft({ ...draft, transfer: { serial: next } });
  };

  const submit = async (event: React.FormEvent) => {
    event.preventDefault();
    setBusy(true);
    try {
      await onSave(draft);
    } finally {
      setBusy(false);
    }
  };

  return (
    <form className="card form" onSubmit={submit}>
      <header>
        <h2>{isNew ? "Новый collector" : draft.collector_id}</h2>
        {draft.version && <span className="muted">версия {draft.version}</span>}
      </header>

      {isNew && (
        <label>
          <span>Идентификатор</span>
          <input
            value={draft.collector_id}
            onChange={(event) => setDraft({ ...draft, collector_id: event.target.value })}
            placeholder="collector-2"
            required
          />
        </label>
      )}

      <div className="switch">
        <button
          type="button"
          className={kind === "tcp_ip" ? "active" : ""}
          onClick={() => setKind("tcp_ip")}
        >
          TCP/IP
        </button>
        <button
          type="button"
          className={kind === "serial" ? "active" : ""}
          onClick={() => setKind("serial")}
        >
          Serial
        </button>
      </div>

      {kind === "tcp_ip" ? (
        <>
          <label>
            <span>Адрес прослушивания</span>
            <input
              value={tcp.host}
              onChange={(event) => updateTcp({ host: event.target.value })}
              required
            />
          </label>
          <label>
            <span>Порт</span>
            <input
              type="number"
              min={1}
              max={65535}
              value={tcp.port}
              onChange={(event) => updateTcp({ port: Number(event.target.value) })}
              required
            />
          </label>
        </>
      ) : (
        <>
          <label>
            <span>Порт</span>
            <input
              value={serial.port_name}
              onChange={(event) => updateSerial({ port_name: event.target.value })}
              required
            />
          </label>
          <EnumSelect
            label="Скорость"
            value={serial.baud_rate}
            options={BAUD_RATES}
            onChange={(baud_rate) => updateSerial({ baud_rate })}
          />
          <EnumSelect
            label="Биты данных"
            value={serial.data_bits}
            options={DATA_BITS}
            onChange={(data_bits) => updateSerial({ data_bits })}
          />
          <EnumSelect
            label="Чётность"
            value={serial.parity}
            options={PARITIES}
            onChange={(parity) => updateSerial({ parity })}
          />
          <EnumSelect
            label="Стоп-биты"
            value={serial.stop_bits}
            options={STOP_BITS}
            onChange={(stop_bits) => updateSerial({ stop_bits })}
          />
          <EnumSelect
            label="Управление потоком"
            value={serial.flow_control}
            options={FLOW_CONTROLS}
            onChange={(flow_control) => updateSerial({ flow_control })}
          />
        </>
      )}

      <label>
        <span>Брокеры Kafka</span>
        <input
          value={draft.kafka.brokers}
          onChange={(event) =>
            setDraft({ ...draft, kafka: { ...draft.kafka, brokers: event.target.value } })
          }
          required
        />
      </label>
      <label>
        <span>Топик</span>
        <input
          value={draft.kafka.topic}
          onChange={(event) =>
            setDraft({ ...draft, kafka: { ...draft.kafka, topic: event.target.value } })
          }
          required
        />
      </label>

      <footer>
        <button type="submit" disabled={busy}>
          {busy ? "Сохранение…" : "Сохранить"}
        </button>
        <button type="button" onClick={onCancel} disabled={busy}>
          Отмена
        </button>
      </footer>
    </form>
  );
}
