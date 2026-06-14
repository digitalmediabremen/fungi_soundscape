#!/usr/bin/env bash
# Compile the ofxPDSP-based synth to a single-file ES6 WASM module.
# Reuses the real ofxPDSP DSP sources (compiled here, minus the OF-coupled bits).
set -e
set -o pipefail

source "$HOME/emsdk/emsdk_env.sh" >/dev/null 2>&1 || {
  echo "emsdk not found at \$HOME/emsdk — install it first"; exit 1; }

HERE="$(cd "$(dirname "$0")" && pwd)"
ADDONS="$HERE/../../of_app/of_v0.11.2_osx_release/addons"
PDSP="$ADDONS/ofxPDSP"
AUDIOFILE="$ADDONS/ofxAudioFile"
SRC="$PDSP/src"
OUT="$HERE/../src/audio/wasm"
mkdir -p "$OUT"

OPT="${OPT:--O2}"

# All pdsp core .cpp EXCEPT the openFrameworks integration layer (ofx/) and
# ValueControl.cpp (the only core unit that truly needs openFrameworks: ofMain).
# SampleBuffer.cpp is fine — it only needs ofxAudioFile (a standalone decoder
# addon, added to the include path below), NOT openFrameworks.
CORE=$(find "$SRC" -name '*.cpp' -not -path '*/ofx/*' -not -name 'ValueControl.cpp')

# ofxPDSP uses relative includes like "../pdspCore.h" that only resolve when each
# src subdirectory is on the include path (the OF addon build does the same).
# shims/ goes FIRST so our empty <mmintrin.h> shadows clang's x86-only one.
INCLUDES="-I$HERE/shims"
for d in $(find "$SRC" -type d); do INCLUDES="$INCLUDES -I$d"; done
INCLUDES="$INCLUDES -I$PDSP/libs/audiofft -I$AUDIOFILE/src -I$AUDIOFILE/libs"

echo "compiling $(echo "$CORE" | wc -l | tr -d ' ') pdsp files + ofxAudioFile + synth.cpp ($OPT) ..."

# pdsp's DSP kernels are SSE2/NEON intrinsics (see math/simd/core/flags.h); there
# is no scalar fallback. Emscripten can lower SSE2 intrinsics to wasm SIMD, and
# -msse2 makes clang define __SSE2__ so pdsp selects its real SSE backend.
SIMD="-msimd128 -msse2"

emcc -std=c++17 $OPT $SIMD \
  -include "$HERE/shims/wasm_compat.h" \
  $INCLUDES \
  $CORE "$PDSP/libs/audiofft/AudioFFT.cpp" "$AUDIOFILE/src/ofxAudioFile.cpp" \
  "$HERE/synth.cpp" \
  -o "$OUT/pdspSynth.js" \
  -sMODULARIZE=1 -sEXPORT_ES6=1 -sSINGLE_FILE=1 \
  -sENVIRONMENT=web,worker,node \
  -sALLOW_MEMORY_GROWTH=1 \
  -sEXPORTED_FUNCTIONS='["_synth_init","_synth_process","_synth_note_on","_synth_note_off","_malloc","_free"]' \
  -sEXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPF32"]' \
  2>&1 | tail -25

echo "built -> $OUT/pdspSynth.js"
