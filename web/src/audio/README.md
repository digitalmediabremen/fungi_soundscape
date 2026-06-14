# Sonification

Turns a mushroom image into sound, following the original openFrameworks app
(`of_app/.../new_test`): `ImageProcessor` → 64×64 `BitMatrix` → `CustomSequencer`
scan → akebono-scale `PolySynth` voices (ofxPDSP).

## Two stages, one contract

```
image ──▶ [ vision front-end ]──▶ 64×64 matrix + note schedule ──▶ [ synth voices ]──▶ audio
          (JS, sonifier.js)        <-- the shared hand-off -->        (Web Audio | ofxPDSP-WASM)
```

The **front-end** (image → matrix → which pitch fires when) lives in `sonifier.js`
and is engine-independent: `prepareMatrix()`, `tempoRange()`, `pitchForValue()`,
`buildSchedule()`. Two back-ends consume the same schedule:

- **`Sonifier`** (`sonifier.js`) — interim Web Audio voices (osc→filter→ADSR + reverb).
- **`PdspSonifier`** (`pdspSonifier.js`) — **your actual ofxPDSP synth, compiled to WebAssembly.**

The Observe screen has a **synth selector** (ofxPDSP▸wasm / web audio); the WASM
engine is the default and falls back to Web Audio if it can't load.

## The ofxPDSP-WASM engine (the real code)

`wasm/synth.cpp` compiles the **real ofxPDSP DSP** (from `addons/ofxPDSP/src`) to
WASM and replaces `pdsp::Engine`'s RtAudio output with manual block processing
(`pdsp::Processor::processAndCopyInterleaved`). `WasmPolySynth` reproduces
`PolySynth::Voice`'s exact patch graph — `pdsp::TableOscillator → VAFilter → Amp`,
`ADSR`, `LFO`, `DimensionChorus` — with the OF GUI controls (`pdsp::Parameter`)
replaced by the same constant values from `PolySynth.cpp`. `PdspSonifier`
**offline-renders** the schedule (drive `synth_note_on/off` + `synth_process` block
by block) into an `AudioBuffer` and plays it — single-threaded, so no AudioWorklet
and pdsp's dynamic pitch re-patch is safe.

### Build it

```bash
# one-time: install Emscripten (https://emscripten.org)
git clone https://github.com/emscripten-core/emsdk ~/emsdk
cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest

# build the WASM synth -> src/audio/wasm/pdspSynth.js  (gitignored)
cd web && npm run build:wasm
# verify it makes audio, in node (no browser needed):
node wasm/test.mjs          # tone/PolySynth: audio + in-tune dynamic pitch
node wasm/test_render.mjs    # full offline render pipeline
```

Run `npm run build:wasm` before `npm run build` when deploying (the artifact is
gitignored). It's bundled into the app as a single-file ES module.

### Getting pdsp to compile to wasm

ofxPDSP's DSP is SSE2/NEON intrinsics with no scalar fallback. Three fixes
(`wasm/build.sh` + `wasm/shims/`), no edits to the original OF source:

1. `-msse2 -msimd128` — Emscripten lowers SSE2 intrinsics to wasm SIMD.
2. empty `shims/mmintrin.h` — pdsp `#include <mmintrin.h>` (MMX) but never uses
   MMX; clang's real one `#error`s on wasm, so we shadow it.
3. `shims/wasm_compat.h` (force-included) — no-ops the x86 denormal-flush macros
   (`_MM_SET_FLUSH_ZERO_MODE`) that have no wasm equivalent.

Only `ValueControl.cpp` (needs ofMain) and the `ofx/` layer are excluded; the rest
of the pdsp core (~139 files) + `ofxAudioFile` compile as-is.

## Computer vision (the `ImageProcessor` stage)

`prepareMatrix()` in `sonifier.js` produces the 64×64 contour matrix. Two stages:

**1. ML segmentation (optional, default on — the "isolate (ML)" toggle).**
`segmenter.js` runs **RMBG-1.4** (background removal) via **transformers.js +
onnxruntime-web** — entirely in the browser, **no API**. It returns a soft
foreground mask that isolates the mushroom from a cluttered background. The
onnxruntime WASM runtime is bundled into the app; the model weights (~44 MB)
download once from the Hugging Face CDN and are browser-cached (self-host them
under `public/models/` + set `env.localModelPath` for fully-offline use).
transformers.js is dynamically imported, so none of it loads until you sonify
with isolation on. Verified in node: `wasm/test_seg.mjs`, `wasm/test_segcv.mjs`.

**2. Contour pipeline** (`processImageData`, pure JS, node-testable via `wasm/test_cv.mjs`):

```
grayscale → gaussian blur → Sobel gradient → Otsu threshold → dilate → denoise → max-pool 64×64
```

Edge/gradient based (polarity- and lighting-robust). With the ML mask it takes
**only the mushroom's contours** — internal edges inside the mask plus the mask
silhouette, background edges zeroed (≈34% noisy → ≈9% clean on a cluttered shot).
Without the mask it falls back to centre-weighted edges of the whole frame. If
the model can't load, it degrades gracefully to the plain pipeline.

## Caveats

- **CORS** — most image hosts (mushroomobserver.org, many GBIF sources) don't send
  `Access-Control-Allow-Origin`, which taints the canvas. Pixels are read through
  a CORS-adding proxy (`images.weserv.nl`); a direct load is tried next, and only
  if both fail does it fall back to a stable seeded matrix. The displayed card
  image always loads directly (full quality).
- The Vite build warns that `pdspSynth.js` imports `node:module` — benign: it's a
  runtime-gated (`ENVIRONMENT_IS_NODE`) dynamic import for the node test path,
  never evaluated in the browser.

## Why not compile the whole OF app to wasm?

It links ~18 addons (Poco networking, Flite TTS, RtMidi, OSC, HTTP server,
OpenCV…) — most don't target Emscripten and none are needed for sound. Extracting
just the DSP is far smaller and more robust.
