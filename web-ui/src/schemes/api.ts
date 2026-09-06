import { ApiError } from "../config/api";
import type { Scheme, SchemeSummary } from "./types";

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

export async function fetchSchemes(): Promise<SchemeSummary[]> {
  const response = await fetch("/api/schemes");
  if (!response.ok) {
    throw await failure(response);
  }

  return (await response.json()) as SchemeSummary[];
}

export async function fetchScheme(schemeId: string): Promise<Scheme> {
  const response = await fetch(`/api/schemes/${encodeURIComponent(schemeId)}`);
  if (!response.ok) {
    throw await failure(response);
  }

  const scheme = (await response.json()) as Scheme;
  // protobuf omits empty repeated fields, and the editor would rather not
  // check for undefined on every render.
  return { ...scheme, shapes: scheme.shapes ?? [], labels: scheme.labels ?? [] };
}

// The version inside the scheme is the one the canvas started from: the
// backend passes it on as expected_version, so a stale editor is refused.
export async function saveScheme(scheme: Scheme): Promise<number> {
  const response = await fetch(`/api/schemes/${encodeURIComponent(scheme.scheme_id)}`, {
    method: "PUT",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(scheme),
  });

  if (!response.ok) {
    throw await failure(response);
  }

  const body = (await response.json()) as { version: number };
  return body.version;
}

export async function deleteScheme(schemeId: string): Promise<void> {
  const response = await fetch(`/api/schemes/${encodeURIComponent(schemeId)}`, {
    method: "DELETE",
  });

  if (!response.ok) {
    throw await failure(response);
  }
}
