import { useEffect, useRef } from "react";
import uPlot from "uplot";
import "uplot/dist/uPlot.min.css";

// One parameter per chart. Several parameters on one pair of axes would mean
// two scales — pressure around 101 next to flow around 12 — and a second y
// scale is the surest way to make a chart lie.
// uPlot formats dates in en-US by default. The range decides how much of the
// date has to be shown: within a day the time is enough, across days it is not.
function formatTick(seconds: number, spanSeconds: number): string {
  const date = new Date(seconds * 1000);
  const time = date.toLocaleTimeString("ru-RU", { hour: "2-digit", minute: "2-digit" });

  if (spanSeconds <= 24 * 3600) {
    return time;
  }

  const day = date.toLocaleDateString("ru-RU", { day: "2-digit", month: "2-digit" });
  return `${day} ${time}`;
}

export function Chart({
  title,
  times,
  values,
}: {
  title: string;
  times: number[];
  values: Array<number | null>;
}) {
  const holder = useRef<HTMLDivElement>(null);
  const plot = useRef<uPlot | null>(null);

  useEffect(() => {
    if (!holder.current) {
      return;
    }

    const style = getComputedStyle(document.documentElement);
    const line = style.getPropertyValue("--series-1").trim() || "#3987e5";
    const grid = style.getPropertyValue("--line").trim() || "#26313d";
    const ink = style.getPropertyValue("--muted").trim() || "#7e8c9c";

    const options: uPlot.Options = {
      width: holder.current.clientWidth,
      height: 180,
      // The value at the cursor is the whole legend: one series needs no key,
      // the heading above the chart names it.
      legend: { show: true },
      cursor: { y: false, points: { size: 8 } },
      scales: { x: { time: true } },
      axes: [
        {
          stroke: ink,
          grid: { stroke: grid, width: 1 },
          ticks: { stroke: grid },
          values: (self, splits) => {
            const scale = self.scales.x;
            const span = (scale.max ?? 0) - (scale.min ?? 0);
            return splits.map((seconds) => formatTick(seconds, span));
          },
        },
        {
          stroke: ink,
          grid: { stroke: grid, width: 1 },
          ticks: { stroke: grid },
          size: 60,
        },
      ],
      series: [
        {
          label: "Время",
          value: (_self, seconds) =>
            seconds === null
              ? "--"
              : new Date(seconds * 1000).toLocaleString("ru-RU", {
                  day: "2-digit",
                  month: "2-digit",
                  hour: "2-digit",
                  minute: "2-digit",
                  second: "2-digit",
                }),
        },
        // The heading above the chart already names the parameter; this row is
        // the value under the cursor, not a key to the colour.
        { label: "значение", stroke: line, width: 2 },
      ],
    };

    const chart = new uPlot(options, [times, values], holder.current);
    plot.current = chart;

    // The card is in a grid that reflows with the window.
    const observer = new ResizeObserver(() => {
      if (holder.current) {
        chart.setSize({ width: holder.current.clientWidth, height: 180 });
      }
    });
    observer.observe(holder.current);

    return () => {
      observer.disconnect();
      chart.destroy();
      plot.current = null;
    };
    // Rebuilt only when the parameter changes; new data goes through setData.
  }, [title]);

  useEffect(() => {
    plot.current?.setData([times, values]);
  }, [times, values]);

  return (
    <article className="card chart">
      <header>
        <h2>{title}</h2>
      </header>
      <div ref={holder} />
    </article>
  );
}
