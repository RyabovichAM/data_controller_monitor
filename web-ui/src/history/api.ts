// The backend answers with columns rather than rows: one array of timestamps
// and one array per numeric parameter, all of the same length. Gaps — a
// parameter missing from a sample — are nulls, which uPlot draws as breaks.
export interface History {
  t: number[];
  series: Record<string, Array<number | null>>;
}

export async function fetchHistory(
  collectorId: string,
  fromSeconds: number,
  toSeconds: number,
): Promise<History> {
  const query = new URLSearchParams({
    collector_id: collectorId,
    from: String(fromSeconds),
    to: String(toSeconds),
  });

  const response = await fetch(`/api/history?${query.toString()}`);
  if (!response.ok) {
    const body = (await response.json().catch(() => ({}))) as { error?: string };
    throw new Error(body.error ?? `HTTP ${response.status}`);
  }

  return (await response.json()) as History;
}
