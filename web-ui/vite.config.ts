import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// In production nginx serves the bundle and proxies /api and /ws to the
// backend, so the app always talks to its own origin. The dev server does the
// same proxying, which keeps the code free of environment-dependent URLs.
export default defineConfig({
  plugins: [react()],
  server: {
    port: 5173,
    proxy: {
      "/api": { target: "http://localhost:8080", changeOrigin: true },
      "/ws": { target: "ws://localhost:8080", ws: true },
    },
  },
});
