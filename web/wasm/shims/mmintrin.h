#pragma once
// Shim for the WASM build.
//
// ofxPDSP does `#include <mmintrin.h>` (MMX) in math/simd/core/activate.h and
// math/simd/functions/simd_wrapper.h, but it never uses MMX (__m64) types — only
// SSE2 (__m128 / __m128i). clang's real <mmintrin.h> hard-errors on non-x86
// targets ("This header is only meant to be used on x86 and x64 architecture"),
// which breaks the wasm build. We put this directory first on the include path
// so pdsp's `#include <mmintrin.h>` resolves here (empty) instead.
//
// SSE2 types/intrinsics still come from <xmmintrin.h> / <emmintrin.h>, which
// Emscripten supports and lowers to wasm SIMD (-msimd128 -msse2).
