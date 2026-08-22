// Collectors that have data in storage. Not the same list as the one
// config-service keeps: that one holds the configured ones, this one the ones
// that actually sent something.
export async function fetchCollectors(): Promise<string[]> {
  const response = await fetch("/api/collectors");
  if (!response.ok) {
    throw new Error(`/api/collectors: ${response.status}`);
  }

  return (await response.json()) as string[];
}
