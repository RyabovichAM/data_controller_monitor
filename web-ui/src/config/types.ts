// The shape of CollectorConfig as protobuf writes it to JSON. Field names are
// the ones from config_service.proto, enum values are their names, and int64
// travels as a string — that is protobuf's JSON mapping, not our choice.

export type BaudRate =
  | "BAUD_RATE_1200"
  | "BAUD_RATE_2400"
  | "BAUD_RATE_4800"
  | "BAUD_RATE_9600"
  | "BAUD_RATE_19200"
  | "BAUD_RATE_38400"
  | "BAUD_RATE_57600"
  | "BAUD_RATE_115200";

export type DataBits = "DATA_BITS_5" | "DATA_BITS_6" | "DATA_BITS_7" | "DATA_BITS_8";

export type Parity =
  | "PARITY_NONE"
  | "PARITY_EVEN"
  | "PARITY_ODD"
  | "PARITY_SPACE"
  | "PARITY_MARK";

export type StopBits = "STOP_BITS_ONE" | "STOP_BITS_ONE_AND_HALF" | "STOP_BITS_TWO";

export type FlowControl =
  | "FLOW_CONTROL_NONE"
  | "FLOW_CONTROL_HARDWARE"
  | "FLOW_CONTROL_SOFTWARE";

export interface SerialTransfer {
  port_name: string;
  baud_rate: BaudRate;
  data_bits: DataBits;
  parity: Parity;
  stop_bits: StopBits;
  flow_control: FlowControl;
}

export interface TcpIpTransfer {
  host: string;
  port: number;
}

// Exactly one of the two is set — that is the oneof of the contract.
export interface TransferSettings {
  tcp_ip?: TcpIpTransfer;
  serial?: SerialTransfer;
}

export interface KafkaSettings {
  brokers: string;
  topic: string;
}

export interface CollectorConfig {
  collector_id: string;
  transfer: TransferSettings;
  kafka: KafkaSettings;
  version?: string;
}

export type TransferKind = "tcp_ip" | "serial";

// Labels for the selects: the contract keeps the physical values, the operator
// should see them rather than the enum names.
export const BAUD_RATES: Array<[BaudRate, string]> = [
  ["BAUD_RATE_1200", "1200"],
  ["BAUD_RATE_2400", "2400"],
  ["BAUD_RATE_4800", "4800"],
  ["BAUD_RATE_9600", "9600"],
  ["BAUD_RATE_19200", "19200"],
  ["BAUD_RATE_38400", "38400"],
  ["BAUD_RATE_57600", "57600"],
  ["BAUD_RATE_115200", "115200"],
];

export const DATA_BITS: Array<[DataBits, string]> = [
  ["DATA_BITS_5", "5"],
  ["DATA_BITS_6", "6"],
  ["DATA_BITS_7", "7"],
  ["DATA_BITS_8", "8"],
];

export const PARITIES: Array<[Parity, string]> = [
  ["PARITY_NONE", "нет"],
  ["PARITY_EVEN", "чётность"],
  ["PARITY_ODD", "нечётность"],
  ["PARITY_SPACE", "пробел"],
  ["PARITY_MARK", "маркер"],
];

export const STOP_BITS: Array<[StopBits, string]> = [
  ["STOP_BITS_ONE", "1"],
  ["STOP_BITS_ONE_AND_HALF", "1.5"],
  ["STOP_BITS_TWO", "2"],
];

export const FLOW_CONTROLS: Array<[FlowControl, string]> = [
  ["FLOW_CONTROL_NONE", "нет"],
  ["FLOW_CONTROL_HARDWARE", "аппаратное"],
  ["FLOW_CONTROL_SOFTWARE", "программное"],
];
