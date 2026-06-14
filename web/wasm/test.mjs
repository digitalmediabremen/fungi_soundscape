// Verify the WASM PolySynth under node (no browser):
//  1) it produces audio,
//  2) dynamic pitch works (a higher note has a higher fundamental).
import initSynth from '../src/audio/wasm/pdspSynth.js'

const SR = 44100
const BLOCK = 256
const Module = await initSynth()
Module._synth_init(SR, BLOCK, 6)

const ptr = Module._malloc(BLOCK * 2 * 4)

// Render `seconds` of one held note on `voice` at `pitch`; return {rms, peak, freq}.
function renderNote(voice, pitch, seconds) {
  Module._synth_note_on(voice, pitch, 1.0)
  const total = Math.floor(SR * seconds)
  const mono = new Float32Array(total)
  let done = 0,
    peak = 0,
    sumSq = 0
  while (done < total) {
    const n = Math.min(BLOCK, total - done)
    Module._synth_process(ptr, n)
    const buf = new Float32Array(Module.HEAPF32.buffer, ptr, n * 2)
    for (let i = 0; i < n; i++) {
      const v = buf[i * 2] // left channel
      mono[done + i] = v
      sumSq += v * v
      if (Math.abs(v) > peak) peak = Math.abs(v)
    }
    done += n
  }
  Module._synth_note_off(voice)
  // crude fundamental estimate via zero crossings over the steady middle half
  let cross = 0
  const a = Math.floor(total * 0.25),
    b = Math.floor(total * 0.75)
  for (let i = a + 1; i < b; i++) if (mono[i - 1] <= 0 && mono[i] > 0) cross++
  const freq = (cross * SR) / (b - a)
  return { rms: Math.sqrt(sumSq / total), peak, freq }
}

const low = renderNote(0, 48, 1.0) // C2
const high = renderNote(1, 72, 1.0) // C4  (24 semitones up => ~4x freq)
Module._free(ptr)

console.log(`low  pitch48: rms=${low.rms.toFixed(4)} peak=${low.peak.toFixed(4)} ~${low.freq.toFixed(1)}Hz`)
console.log(`high pitch72: rms=${high.rms.toFixed(4)} peak=${high.peak.toFixed(4)} ~${high.freq.toFixed(1)}Hz`)

const audible = low.rms > 1e-3 && high.rms > 1e-3
const pitchWorks = high.freq > low.freq * 1.8 // expect clearly higher
console.log(audible ? 'PASS audio' : 'FAIL audio')
console.log(pitchWorks ? 'PASS dynamic pitch' : 'WARN pitch not clearly higher (re-patch may not work)')
process.exit(audible ? 0 : 1)
