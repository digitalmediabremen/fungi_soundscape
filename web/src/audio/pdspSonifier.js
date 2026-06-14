// PDSP-WASM sonifier: the REAL ofxPDSP synth (PolySynth voice graph), compiled
// to WebAssembly (wasm/synth.cpp -> wasm/pdspSynth.js), driven by the shared
// matrix->akebono schedule from sonifier.js.
//
// We render OFFLINE: step through the schedule, call the WASM synth's
// note_on/note_off + process() to fill a buffer, then play it through Web Audio
// as an AudioBuffer. Offline keeps it single-threaded (no AudioWorklet needed)
// and makes pdsp's dynamic pitch re-patch safe.

import initSynth from './wasm/pdspSynth.js'
import { prepareMatrix, buildSchedule, computePosition, SECTIONS } from './sonifier'

let modulePromise = null
function loadModule() {
  if (!modulePromise) modulePromise = initSynth()
  return modulePromise
}

const BLOCK = 1024
const TAIL = 1.6 // seconds of reverb/release tail after the last note

export class PdspSonifier {
  constructor() {
    this.ctx = null
    this.src = null
    this.playing = false
    this.onended = null
  }

  /** Render `fungus` to an AudioBuffer using the WASM synth and start playback. */
  async start(fungus, opts = {}) {
    this.stop()
    const AC = window.AudioContext || window.webkitAudioContext
    if (!this.ctx) this.ctx = new AC()
    if (this.ctx.state === 'suspended') await this.ctx.resume()

    const [Module, data] = await Promise.all([loadModule(), prepareMatrix(fungus, opts)])
    const sr = this.ctx.sampleRate
    const sched = buildSchedule(data)

    this.data = data // public, for the debug view
    this._stepDur = 60 / sched.bpm / 4
    this._duration = sched.duration

    const buffer = this._render(Module, sched, sr)

    const src = this.ctx.createBufferSource()
    src.buffer = buffer
    const g = this.ctx.createGain()
    g.gain.value = 0.9
    src.connect(g)
    g.connect(this.ctx.destination)
    src.onended = () => {
      if (this.src === src) {
        this.playing = false
        this.src = null
        if (this.onended) this.onended()
      }
    }
    src.start()
    this._startedAt = this.ctx.currentTime // playback clock origin for the debug view
    this.src = src
    this.playing = true
  }

  /** Current scan position for the debug view (matches the rendered schedule). */
  position() {
    if (!this.playing) return null
    return computePosition(this.ctx, this._startedAt, this._stepDur, this._duration)
  }

  /** Offline-render the schedule through the WASM synth into a stereo AudioBuffer. */
  _render(Module, sched, sr) {
    Module._synth_init(sr, BLOCK, Math.max(SECTIONS, 6))
    const ptr = Module._malloc(BLOCK * 2 * 4)

    const totalFrames = Math.ceil((sched.duration + TAIL) * sr)
    const L = new Float32Array(totalFrames)
    const R = new Float32Array(totalFrames)

    // event lists as sample indices, sorted by time
    const ons = sched.events
      .map((e) => ({ frame: Math.floor(e.t * sr), voice: e.voice, pitch: e.pitch, vel: e.velocity }))
      .sort((a, b) => a.frame - b.frame)
    const offs = sched.events
      .map((e) => ({ frame: Math.floor((e.t + e.dur) * sr), voice: e.voice }))
      .sort((a, b) => a.frame - b.frame)

    let oi = 0
    let fi = 0
    let frame = 0
    while (frame < totalFrames) {
      const n = Math.min(BLOCK, totalFrames - frame)
      const end = frame + n
      // fire note on/offs that fall in this block (block-granular timing)
      while (oi < ons.length && ons[oi].frame < end) {
        Module._synth_note_on(ons[oi].voice, ons[oi].pitch, ons[oi].vel)
        oi++
      }
      while (fi < offs.length && offs[fi].frame < end) {
        Module._synth_note_off(offs[fi].voice)
        fi++
      }
      Module._synth_process(ptr, n)
      // HEAPF32 view can detach on growth; re-create per block
      const buf = new Float32Array(Module.HEAPF32.buffer, ptr, n * 2)
      for (let i = 0; i < n; i++) {
        L[frame + i] = buf[i * 2]
        R[frame + i] = buf[i * 2 + 1]
      }
      frame += n
    }
    Module._free(ptr)

    // Polyphony can push the sum past 1.0 — normalize to avoid hard clipping.
    let pk = 0
    for (let i = 0; i < totalFrames; i++) {
      const a = Math.abs(L[i])
      const b = Math.abs(R[i])
      if (a > pk) pk = a
      if (b > pk) pk = b
    }
    if (pk > 0.9) {
      const g = 0.9 / pk
      for (let i = 0; i < totalFrames; i++) {
        L[i] *= g
        R[i] *= g
      }
    }

    const ab = this.ctx.createBuffer(2, totalFrames, sr)
    ab.copyToChannel(L, 0)
    ab.copyToChannel(R, 1)
    return ab
  }

  stop() {
    if (this.src) {
      try {
        this.src.onended = null
        this.src.stop()
      } catch {
        /* already stopped */
      }
      this.src = null
    }
    this.playing = false
  }

  dispose() {
    this.stop()
    if (this.ctx) {
      this.ctx.close()
      this.ctx = null
    }
  }
}
