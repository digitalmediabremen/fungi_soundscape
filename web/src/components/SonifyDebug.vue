<template>
  <div class="dbg">
    <div class="dbg__title">computer vision · 64×64</div>
    <canvas ref="cv" class="dbg__canvas" :width="W" :height="H" />
    <div class="dbg__legend">
      <span class="dbg__chip dbg__chip--on" /> played
      <span class="dbg__chip dbg__chip--scan" /> scan row
      <span class="dbg__chip dbg__chip--sec" /> section
    </div>
  </div>
</template>

<script setup>
import { ref, watch, onMounted, onBeforeUnmount } from 'vue'
import { MATRIX, STEPS, SECTIONS, MIN_TO_FILL } from '../audio/sonifier'

// The processed 64x64 matrix that drives the synth (the "computer vision"
// result), with the live sequencer playhead drawn on top — so you can see
// whether the image processing + scan are doing what you expect.
const props = defineProps({
  matrix: { type: Array, default: null }, // 64 rows of Float32Array(64)
  positionFn: { type: Function, default: null }, // () => {row, step} | null
})

const CELL = 4
const W = MATRIX * CELL
const H = MATRIX * CELL
const cv = ref(null)
let raf = null
let off = null // offscreen canvas with the static matrix

function buildOffscreen() {
  if (!props.matrix) {
    off = null
    return
  }
  const c = document.createElement('canvas')
  c.width = W
  c.height = H
  const x = c.getContext('2d')
  for (let y = 0; y < MATRIX; y++) {
    const row = props.matrix[y]
    for (let xx = 0; xx < MATRIX; xx++) {
      const v = row[xx]
      if (v > MIN_TO_FILL) {
        // note-firing ("contour") cell -> neon green scaled by intensity
        const g = Math.round(110 + v * 145)
        x.fillStyle = `rgb(${Math.round(v * 40)},${g},${Math.round(v * 30)})`
      } else {
        const l = Math.round(v * 85) // dim grey background
        x.fillStyle = `rgb(${l},${l},${l})`
      }
      x.fillRect(xx * CELL, y * CELL, CELL, CELL)
    }
  }
  off = c
}

function draw() {
  raf = requestAnimationFrame(draw)
  const ctx = cv.value?.getContext('2d')
  if (!ctx) return
  ctx.clearRect(0, 0, W, H)
  if (off) ctx.drawImage(off, 0, 0)
  else {
    ctx.fillStyle = '#0b0e0b'
    ctx.fillRect(0, 0, W, H)
  }

  // section boundaries (every STEPS columns = one synth voice)
  ctx.strokeStyle = 'rgba(57,255,20,0.30)'
  ctx.lineWidth = 1
  for (let s = 1; s < SECTIONS; s++) {
    const px = s * STEPS * CELL + 0.5
    ctx.beginPath()
    ctx.moveTo(px, 0)
    ctx.lineTo(px, H)
    ctx.stroke()
  }

  const pos = props.positionFn ? props.positionFn() : null
  if (pos) {
    // scan row band
    ctx.fillStyle = 'rgba(255,255,255,0.16)'
    ctx.fillRect(0, pos.row * CELL, W, CELL)
    // per-section step column + bright marker on the cell that's firing
    for (let sec = 0; sec < SECTIONS; sec++) {
      const col = sec * STEPS + pos.step
      const px = col * CELL
      ctx.fillStyle = 'rgba(255,255,255,0.10)'
      ctx.fillRect(px, 0, CELL, H)
      const v = props.matrix?.[pos.row]?.[col] ?? 0
      if (v > MIN_TO_FILL) {
        ctx.fillStyle = '#eaffea'
        ctx.fillRect(px - 1, pos.row * CELL - 1, CELL + 2, CELL + 2)
      }
    }
  }
}

watch(() => props.matrix, buildOffscreen)
onMounted(() => {
  buildOffscreen()
  raf = requestAnimationFrame(draw)
})
onBeforeUnmount(() => {
  if (raf) cancelAnimationFrame(raf)
})
</script>

<style scoped>
.dbg {
  display: flex;
  flex-direction: column;
  gap: 8px;
  align-items: center;
}
.dbg__title {
  color: var(--muted);
  font-size: 11px;
  letter-spacing: 1px;
  text-transform: uppercase;
}
.dbg__canvas {
  image-rendering: pixelated;
  width: 256px;
  height: 256px;
  border: 1px solid #1a201a;
  border-radius: 2px;
  background: #0b0e0b;
}
.dbg__legend {
  display: flex;
  align-items: center;
  gap: 6px;
  color: var(--muted);
  font-size: 10px;
  letter-spacing: 0.5px;
}
.dbg__chip {
  width: 10px;
  height: 10px;
  border-radius: 2px;
  display: inline-block;
}
.dbg__chip--on {
  background: #39ff14;
}
.dbg__chip--scan {
  background: rgba(255, 255, 255, 0.4);
}
.dbg__chip--sec {
  background: transparent;
  border: 1px solid rgba(57, 255, 20, 0.6);
}
</style>
