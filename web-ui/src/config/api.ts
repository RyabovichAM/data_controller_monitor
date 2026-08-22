import type { CollectorConfig } from "./types";

// The backend answers errors with {"error": "..."} and the status code that
// matters: 409 means someone else saved while this form was open.
export class ApiError extends Error {
  constructor(
    message: string,
    readonly status: number,
  ) {
    super(message);
  }
}

async function failure(response: Response): Promise<ApiError> {
  let message = `HTTP ${response.status}`;
  try {
    const body = (await response.json()) as { error?: string };
    if (body.error) {
      message = body.error;
    }
  } catch {
    // an empty or non-JSON body leaves the status as the only thing to say
  }

  return new ApiError(message, response.status);
}

export async function fetchConfigs(): Promise<CollectorConfig[]> {
  const response = await fetch("/api/configs");
  if (!response.ok) {
    throw await failure(response);
  }

  return (await response.json()) as CollectorConfig[];
}

// The version inside the config is the one the form started from: the backend
// passes it on as expected_version, so a stale form is refused rather than
// overwriting a newer config.
export async function saveConfig(config: CollectorConfig): Promise<number> {
  const response = await fetch(`/api/configs/${encodeURIComponent(config.collector_id)}`, {
    method: "PUT",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(config),
  });

  if (!response.ok) {
    throw await failure(response);
  }

  const body = (await response.json()) as { version: number };
  return body.version;
}

export async function deleteConfig(collectorId: string): Promise<void> {
  const response = await fetch(`/api/configs/${encodeURIComponent(collectorId)}`, {
    method: "DELETE",
  });

  if (!response.ok) {
    throw await failure(response);
  }
}
