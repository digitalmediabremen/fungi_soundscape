// Sonify a mushroom image in the browser, following the original openFrameworks
// pipeline: ImageProcessor -> 64x64 BitMatrix -> CustomSequencer scan -> akebono
// pentatonic pitches -> PolySynth voices.
//
// This file holds the engine-independent FRONT-END (image -> matrix -> note
// schedule), shared by two back-ends:
//   - Sonifier      (this file)        : interim Web Audio voices
//   - PdspSonifier  (./pdspSonifier.js): the REAL ofxPDSP synth compiled to WASM
// Keeping the schedule shared means both play the same "piece".

// --- constants mirrored from the OF app (ofApp.h / CustomSequencer) -----------
export const MATRIX = 64
export const STEPS = 16
export const SECTIONS = MATRIX / STEPS // NUMSYNTHS (=4)
export const MIN_BPM = 60
export const MAX_BPM = 100
export const ABS_MIN_PITCH = 40
export const ABS_MAX_PITCH = 100
export const MIN_TO_FILL = 0.4
// akebono — Japanese pentatonic scale, MIDI notes (verbatim from ofApp.cpp)
export const AKEBONO = [72, 74, 75, 79, 80, 84, 86, 87]

import { segmentMask } from './segmenter.js'

export const midiToFreq = (m) => 440 * Math.pow(2, (m - 69) / 12)
const clamp = (v, lo, hi) => Math.max(lo, Math.min(hi, v))

/**
 * Playhead position from an audio clock — shared by both engines so the debug
 * view stays in sync with what's audible. Returns {row, step, elapsed} using the
 * same bottom->top scan formula as buildSchedule(), or null if not playing.
 */
export function computePosition(ctx, startedAt, stepDur, duration) {
  if (!ctx || startedAt == null) return null
  const elapsed = ctx.currentTime - startedAt
  if (elapsed < 0) return null
  if (duration && isFinite(duration) && elapsed > duration) return null
  const step = Math.floor(elapsed / stepDur)
  return {
    row: MATRIX - 1 - (Math.floor(step / STEPS) % MATRIX),
    step: step % STEPS,
    elapsed,
  }
}

// --- image -> 64x64 matrix (the ImageProcessor stage) -------------------------

const WORK = 256 // CV working resolution (also the ML mask resolution)
// Most mushroom image hosts (mushroomobserver.org, many GBIF sources) don't send
// Access-Control-Allow-Origin, which taints the canvas so getImageData() throws.
// We read pixels through a CORS-adding image proxy (also downsizes => less data).
const corsProxy = (url) =>
  `https://images.weserv.nl/?url=${encodeURIComponent(url.replace(/^https?:\/\//, ''))}&w=${WORK}&h=${WORK}&fit=inside`

function loadImage(url) {
  return new Promise((resolve, reject) => {
    const img = new Image()
    img.crossOrigin = 'anonymous'
    img.onload = () => resolve(img)
    img.onerror = () => reject(new Error('image load failed'))
    img.src = url
  })
}

function seededMatrix(seed) {
  let s = (Math.abs(seed | 0) || 1) >>> 0
  const rnd = () => ((s = (s * 1664525 + 1013904223) >>> 0) / 4294967296)
  const m = []
  let filled = 0
  for (let y = 0; y < MATRIX; y++) {
    const row = new Float32Array(MATRIX)
    for (let x = 0; x < MATRIX; x++) {
      const v = 0.5 + 0.5 * Math.sin(x * 0.3 + rnd() * 0.6) * Math.cos(y * 0.22 + rnd() * 0.6)
      row[x] = v
      if (v > MIN_TO_FILL) filled++
    }
    m.push(row)
  }
  return { matrix: m, filledPercentage: filled / (MATRIX * MATRIX) }
}

// --- robust contour CV (the ImageProcessor stage, OpenCV-style in pure JS) -----

function toGray(rgba, n) {
  const g = new Float32Array(n)
  for (let i = 0; i < n; i++)
    g[i] = (0.299 * rgba[i * 4] + 0.587 * rgba[i * 4 + 1] + 0.114 * rgba[i * 4 + 2]) / 255
  return g
}

// separable 5-tap gaussian blur (kernel 1 4 6 4 1)
function gaussBlur(src, W) {
  const k = [1, 4, 6, 4, 1]
  const tmp = new Float32Array(src.length)
  const out = new Float32Array(src.length)
  for (let y = 0; y < W; y++)
    for (let x = 0; x < W; x++) {
      let s = 0
      for (let t = -2; t <= 2; t++) s += src[y * W + clampi(x + t, W)] * k[t + 2]
      tmp[y * W + x] = s / 16
    }
  for (let y = 0; y < W; y++)
    for (let x = 0; x < W; x++) {
      let s = 0
      for (let t = -2; t <= 2; t++) s += tmp[clampi(y + t, W) * W + x] * k[t + 2]
      out[y * W + x] = s / 16
    }
  return out
}
const clampi = (v, n) => (v < 0 ? 0 : v >= n ? n - 1 : v)

// Sobel gradient magnitude, normalized 0..1 (high at edges/contours)
function sobelMag(src, W) {
  const mag = new Float32Array(src.length)
  let max = 1e-6
  for (let y = 1; y < W - 1; y++)
    for (let x = 1; x < W - 1; x++) {
      const i = y * W + x
      const gx =
        src[i - W + 1] + 2 * src[i + 1] + src[i + W + 1] - src[i - W - 1] - 2 * src[i - 1] - src[i + W - 1]
      const gy =
        src[i + W - 1] + 2 * src[i + W] + src[i + W + 1] - src[i - W - 1] - 2 * src[i - W] - src[i - W + 1]
      const m = Math.hypot(gx, gy)
      mag[i] = m
      if (m > max) max = m
    }
  for (let i = 0; i < mag.length; i++) mag[i] /= max
  return mag
}

// Otsu threshold (0..1) — auto edge/background split, robust to lighting
function otsu(vals) {
  const hist = new Int32Array(256)
  for (let i = 0; i < vals.length; i++) hist[Math.min(255, (vals[i] * 255) | 0)]++
  const total = vals.length
  let sum = 0
  for (let t = 0; t < 256; t++) sum += t * hist[t]
  let sumB = 0,
    wB = 0,
    best = -1,
    thr = 51
  for (let t = 0; t < 256; t++) {
    wB += hist[t]
    if (!wB) continue
    const wF = total - wB
    if (!wF) break
    sumB += t * hist[t]
    const d = wB / total * (wF / total) * Math.pow(sumB / wB - (sum - sumB) / wF, 2)
    if (d > best) {
      best = d
      thr = t
    }
  }
  return thr / 255
}

// 3x3 binary dilate (connect contour fragments, like the OF dilate())
function dilate(bin, W) {
  const out = new Uint8Array(bin.length)
  for (let y = 0; y < W; y++)
    for (let x = 0; x < W; x++) {
      let on = 0
      for (let dy = -1; dy <= 1 && !on; dy++)
        for (let dx = -1; dx <= 1; dx++) {
          const yy = y + dy,
            xx = x + dx
          if (yy >= 0 && yy < W && xx >= 0 && xx < W && bin[yy * W + xx]) {
            on = 1
            break
          }
        }
      out[y * W + x] = on
    }
  return out
}

// drop connected components below minArea (removes speckle noise)
function denoise(bin, W, minArea) {
  const label = new Int32Array(bin.length)
  const stack = []
  for (let i = 0; i < bin.length; i++) {
    if (!bin[i] || label[i]) continue
    const cells = [i]
    label[i] = 1
    stack.length = 0
    stack.push(i)
    while (stack.length) {
      const p = stack.pop()
      const py = (p / W) | 0,
        px = p % W
      for (let dy = -1; dy <= 1; dy++)
        for (let dx = -1; dx <= 1; dx++) {
          const yy = py + dy,
            xx = px + dx
          if (yy < 0 || yy >= W || xx < 0 || xx >= W) continue
          const q = yy * W + xx
          if (bin[q] && !label[q]) {
            label[q] = 1
            stack.push(q)
            cells.push(q)
          }
        }
    }
    if (cells.length < minArea) for (const p of cells) bin[p] = 0
  }
  return bin
}

// max-pool WxW -> OUTxOUT (so thin contours survive the downsample)
function maxPool(vals, W, OUT) {
  const f = W / OUT
  const m = []
  for (let oy = 0; oy < OUT; oy++) {
    const row = new Float32Array(OUT)
    for (let ox = 0; ox < OUT; ox++) {
      let mx = 0
      for (let y = (oy * f) | 0; y < ((oy + 1) * f) | 0; y++)
        for (let x = (ox * f) | 0; x < ((ox + 1) * f) | 0; x++) {
          const v = vals[y * W + x]
          if (v > mx) mx = v
        }
      row[ox] = mx
    }
    m.push(row)
  }
  return m
}

/**
 * Robust contour extraction: square RGBA (side W) -> {matrix(64), filledPercentage}.
 * If `fgMask` (a W*W foreground probability map from the ML segmenter) is given,
 * the contours come ONLY from the mushroom: internal edges inside the mask plus
 * the mask's silhouette, with all background edges zeroed.
 */
export function processImageData(rgba, W, fgMask = null) {
  const gray = gaussBlur(toGray(rgba, W * W), W)
  const mag = sobelMag(gray, W)

  // build the "contour field" to threshold
  const field = new Float32Array(mag.length)
  if (fgMask) {
    const silhouette = sobelMag(fgMask, W) // gradient of the mask = mushroom outline
    for (let i = 0; i < field.length; i++) {
      const inside = fgMask[i] > 0.5 ? 1 : 0
      field[i] = Math.max(mag[i] * inside, silhouette[i]) // internal edges + outline
    }
  } else {
    // no segmentation: gradient gently weighted toward the centre subject
    const cx = W / 2,
      cy = W / 2,
      maxd = Math.hypot(cx, cy)
    for (let y = 0; y < W; y++)
      for (let x = 0; x < W; x++) {
        const i = y * W + x
        field[i] = mag[i] * (1 - 0.35 * (Math.hypot(x - cx, y - cy) / maxd))
      }
  }

  const thr = Math.max(0.1, otsu(field) * 0.85)
  const bin = new Uint8Array(field.length)
  for (let i = 0; i < field.length; i++) bin[i] = field[i] >= thr ? 1 : 0
  let mask = dilate(bin, W)
  mask = denoise(mask, W, Math.round(W * W * 0.0006))

  const strength = new Float32Array(field.length)
  for (let i = 0; i < field.length; i++) strength[i] = mask[i] ? Math.min(1, field[i] * 1.7) : 0

  const matrix = maxPool(strength, W, MATRIX)
  let filled = 0
  for (let y = 0; y < MATRIX; y++)
    for (let x = 0; x < MATRIX; x++) if (matrix[y][x] > MIN_TO_FILL) filled++
  return { matrix, filledPercentage: filled / (MATRIX * MATRIX) }
}

/**
 * Image -> contour matrix. Reads pixels via a CORS proxy (then a direct CORS
 * attempt). With `segment: true`, runs the local ML segmenter (transformers.js)
 * to isolate the mushroom first. Falls back to a stable seeded matrix only if
 * the pixels can't be read at all.
 */
export async function prepareMatrix(fungus, { segment = false, onProgress } = {}) {
  if (fungus?.imageUrl) {
    for (const url of [corsProxy(fungus.imageUrl), fungus.imageUrl]) {
      try {
        const img = await loadImage(url)
        const c = document.createElement('canvas')
        c.width = WORK
        c.height = WORK
        const ctx = c.getContext('2d', { willReadFrequently: true })
        ctx.drawImage(img, 0, 0, WORK, WORK)
        const rgba = ctx.getImageData(0, 0, WORK, WORK).data // throws if tainted

        let fgMask = null
        if (segment) {
          try {
            fgMask = await segmentMask({ data: rgba, width: WORK, height: WORK }, WORK, WORK, onProgress)
          } catch (e) {
            console.warn('[sonify] ML segmentation failed — using plain contours', e)
          }
        }
        return processImageData(rgba, WORK, fgMask)
      } catch {
        /* try next source */
      }
    }
    console.warn('[sonify] could not read image pixels (CORS) — using fallback matrix')
  }
  return seededMatrix(fungus?.id || 1)
}

/** Tempo + pitch ceiling from how "filled" the fungus is (ofApp.cpp logic). */
export function tempoRange(data) {
  const f = clamp(data.filledPercentage, 0, 1)
  return {
    bpm: clamp(MAX_BPM - f * (MAX_BPM - MIN_BPM), MIN_BPM, MAX_BPM),
    maxPitch: clamp(
      ABS_MAX_PITCH - f * 1.5 * (ABS_MAX_PITCH - ABS_MIN_PITCH),
      ABS_MIN_PITCH + 8,
      ABS_MAX_PITCH
    ),
  }
}

/** Map a matrix cell value -> MIDI pitch (or null), following ofApp.cpp:
 *  value*maxPitch, floored at ABS_MIN_PITCH, "filled" cells snap to akebono. */
export function pitchForValue(value, maxPitch) {
  if (value <= MIN_TO_FILL) return null
  let pitch = value * maxPitch
  if (pitch < ABS_MIN_PITCH) pitch = ABS_MIN_PITCH + Math.random() * 5
  else if (pitch >= maxPitch - 1) {
    const diff = ABS_MAX_PITCH - maxPitch
    pitch = AKEBONO[(Math.random() * AKEBONO.length) | 0] - diff
  }
  return clamp(pitch, ABS_MIN_PITCH, ABS_MAX_PITCH)
}

/**
 * Build a finite note schedule by scanning the matrix bottom->top (looping if
 * needed) for ~durationTarget seconds. Returns events for offline rendering.
 *   event = { t, voice(=section), pitch(MIDI), velocity, dur }
 */
export function buildSchedule(data, durationTarget = 22) {
  const { bpm, maxPitch } = tempoRange(data)
  const stepDur = 60 / bpm / 4 // 16th notes
  const totalSteps = Math.max(1, Math.floor(durationTarget / stepDur))
  const events = []
  for (let s = 0; s < totalSteps; s++) {
    const row = MATRIX - 1 - (Math.floor(s / STEPS) % MATRIX) // bottom -> top, wraps
    const step = s % STEPS
    const t = s * stepDur
    for (let sec = 0; sec < SECTIONS; sec++) {
      const value = data.matrix[row][sec * STEPS + step]
      const pitch = pitchForValue(value, maxPitch)
      if (pitch == null) continue
      events.push({ t, voice: sec, pitch, velocity: 0.4 + 0.5 * value, dur: stepDur * 3 })
    }
  }
  return { events, duration: totalSteps * stepDur, bpm, maxPitch }
}

// ---------------------------------------------------------------------------
// Interim engine: Web Audio voices (no WASM). Realtime lookahead scheduler.
// ---------------------------------------------------------------------------

export class Sonifier {
  constructor() {
    this.ctx = null
    this.master = null
    this.playing = false
    this._timer = null
    this._row = MATRIX - 1
    this._step = 0
    this._next = 0
    this._data = null
    this._bpm = MIN_BPM
    this._maxPitch = ABS_MAX_PITCH
  }

  _ensureContext() {
    if (this.ctx) return
    const AC = window.AudioContext || window.webkitAudioContext
    this.ctx = new AC()
    const comp = this.ctx.createDynamicsCompressor()
    this.master = this.ctx.createGain()
    this.master.gain.value = 0.0
    const verb = this.ctx.createConvolver()
    verb.buffer = this._impulse(2.4, 2.6)
    const verbGain = this.ctx.createGain()
    verbGain.gain.value = 0.32
    this.master.connect(comp)
    this.master.connect(verbGain)
    verbGain.connect(verb)
    verb.connect(comp)
    comp.connect(this.ctx.destination)
  }

  _impulse(seconds, decay) {
    const rate = this.ctx.sampleRate
    const len = Math.floor(rate * seconds)
    const buf = this.ctx.createBuffer(2, len, rate)
    for (let ch = 0; ch < 2; ch++) {
      const d = buf.getChannelData(ch)
      for (let i = 0; i < len; i++) d[i] = (Math.random() * 2 - 1) * Math.pow(1 - i / len, decay)
    }
    return buf
  }

  async start(fungus, opts = {}) {
    this.stop()
    this._ensureContext()
    if (this.ctx.state === 'suspended') await this.ctx.resume()
    this._data = await prepareMatrix(fungus, opts)
    this.data = this._data // public, for the debug view
    const { bpm, maxPitch } = tempoRange(this._data)
    this._bpm = bpm
    this._maxPitch = maxPitch
    this._stepDur = 60 / bpm / 4
    this._row = MATRIX - 1
    this._step = 0
    this._next = this.ctx.currentTime + 0.08
    this._startedAt = this._next
    this.playing = true
    this.master.gain.setValueAtTime(0.0001, this.ctx.currentTime)
    this.master.gain.exponentialRampToValueAtTime(0.9, this.ctx.currentTime + 0.4)
    this._timer = setInterval(() => this._schedule(), 25)
  }

  stop() {
    this.playing = false
    if (this._timer) {
      clearInterval(this._timer)
      this._timer = null
    }
    if (this.ctx && this.master) {
      const t = this.ctx.currentTime
      this.master.gain.cancelScheduledValues(t)
      this.master.gain.setValueAtTime(this.master.gain.value, t)
      this.master.gain.linearRampToValueAtTime(0.0001, t + 0.15)
    }
  }

  _schedule() {
    if (!this.playing) return
    const stepDur = 60 / this._bpm / 4
    const ahead = this.ctx.currentTime + 0.12
    while (this._next < ahead) {
      this._emitStep(this._next)
      this._next += stepDur
      if (++this._step >= STEPS) {
        this._step = 0
        if (--this._row < 0) this._row = MATRIX - 1
      }
    }
  }

  _emitStep(when) {
    const m = this._data.matrix[this._row]
    for (let sec = 0; sec < SECTIONS; sec++) {
      const pitch = pitchForValue(m[sec * STEPS + this._step], this._maxPitch)
      if (pitch == null) continue
      this._voice(midiToFreq(pitch), when, m[sec * STEPS + this._step])
    }
  }

  _voice(freq, when, amp) {
    const ctx = this.ctx
    const osc = ctx.createOscillator()
    osc.type = 'triangle'
    osc.frequency.value = freq
    const filt = ctx.createBiquadFilter()
    filt.type = 'lowpass'
    filt.Q.value = 6
    filt.frequency.setValueAtTime(clamp(freq * 6, 400, 9000), when)
    filt.frequency.exponentialRampToValueAtTime(clamp(freq * 1.5, 200, 6000), when + 0.5)
    const g = ctx.createGain()
    const peak = 0.1 + 0.12 * amp
    g.gain.setValueAtTime(0.0001, when)
    g.gain.exponentialRampToValueAtTime(peak, when + 0.012)
    g.gain.exponentialRampToValueAtTime(peak * 0.5, when + 0.18)
    g.gain.exponentialRampToValueAtTime(0.0001, when + 1.1)
    osc.connect(filt)
    filt.connect(g)
    g.connect(this.master)
    osc.start(when)
    osc.stop(when + 1.2)
  }

  /** Current scan position for the debug view (loops forever). */
  position() {
    if (!this.playing) return null
    return computePosition(this.ctx, this._startedAt, this._stepDur, Infinity)
  }

  dispose() {
    this.stop()
    if (this.ctx) {
      this.ctx.close()
      this.ctx = null
    }
  }
}
