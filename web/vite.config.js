import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'

// base: './' keeps asset paths relative so the build works when hosted
// under a subpath (e.g. antoniohof.com/Fungi-soundscape).
export default defineConfig({
  base: './',
  plugins: [vue()],
  server: {
    port: 5173,
    open: true,
  },
  build: {
    // three.js makes the globe chunk inherently large; this just silences
    // the size warning for that expected chunk.
    chunkSizeWarningLimit: 2000,
  },
})
