// End-to-end offline-render check (node): buildSchedule -> WASM synth -> buffer.
// Mirrors pdspSonifier._render without a browser (fake matrix instead of an image).
import initSynth from '../src/audio/wasm/pdspSynth.js'
import { buildSchedule, MATRIX } from '../src/audio/sonifier.js'

// fake "contour" matrix: vertical stripes so the scan fires notes
const matrix = []
let filled = 0
for (let y = 0; y < MATRIX; y++) {
  const row = new Float32Array(MATRIX)
  for (let x = 0; x < MATRIX; x++) {
    const v = x % 8 < 3 ? 0.85 : 0.1
    row[x] = v
    if (v > 0.4) filled++
  }
  matrix.push(row)
}
const sched = buildSchedule({ matrix, filledPercentage: filled / (MATRIX * MATRIX) }, 4)
console.log(`schedule: ${sched.events.length} events, ${sched.duration.toFixed(1)}s, ${sched.bpm.toFixed(0)} bpm`)

const Module = await initSynth()
const sr = 44100
const BLOCK = 1024
Module._synth_init(sr, BLOCK, 6)
const ptr = Module._malloc(BLOCK * 2 * 4)
const total = Math.ceil((sched.duration + 1.6) * sr)
const ons = sched.events
  .map((e) => ({ frame: (e.t * sr) | 0, voice: e.voice, pitch: e.pitch, vel: e.velocity }))
  .sort((a, b) => a.frame - b.frame)
const offs = sched.events
  .map((e) => ({ frame: ((e.t + e.dur) * sr) | 0, voice: e.voice }))
  .sort((a, b) => a.frame - b.frame)
let oi = 0,
  fi = 0,
  frame = 0,
  peak = 0,
  sumSq = 0
while (frame < total) {
  const n = Math.min(BLOCK, total - frame)
  const end = frame + n
  while (oi < ons.length && ons[oi].frame < end) {
    Module._synth_note_on(ons[oi].voice, ons[oi].pitch, ons[oi].vel)
    oi++
  }
  while (fi < offs.length && offs[fi].frame < end) {
    Module._synth_note_off(offs[fi].voice)
    fi++
  }
  Module._synth_process(ptr, n)
  const buf = new Float32Array(Module.HEAPF32.buffer, ptr, n * 2)
  for (let i = 0; i < buf.length; i++) {
    sumSq += buf[i] * buf[i]
    if (Math.abs(buf[i]) > peak) peak = Math.abs(buf[i])
  }
  frame += n
}
Module._free(ptr)
const rms = Math.sqrt(sumSq / (total * 2))
console.log(`render: ${total} frames · RMS=${rms.toFixed(4)} · peak=${peak.toFixed(4)}`)
console.log(rms > 1e-3 ? 'PASS render pipeline' : 'FAIL silent')
process.exit(rms > 1e-3 ? 0 : 1)
