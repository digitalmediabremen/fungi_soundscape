#pragma once
// Force-included into every translation unit of the WASM build.
//
// x86 SSE denormal-flush controls (FTZ/DAZ) have no WebAssembly equivalent.
// pdsp's ofx_activate_denormal_flush() calls _MM_SET_FLUSH_ZERO_MODE(...). We
// no-op these: denormals simply aren't flushed in wasm (a tiny perf detail on
// rare denormal values, not a correctness issue for the synth).
#ifndef _MM_FLUSH_ZERO_ON
#define _MM_FLUSH_ZERO_ON 0x8000
#endif
#ifndef _MM_FLUSH_ZERO_OFF
#define _MM_FLUSH_ZERO_OFF 0x0000
#endif
#ifndef _MM_SET_FLUSH_ZERO_MODE
#define _MM_SET_FLUSH_ZERO_MODE(x) ((void)0)
#endif
#ifndef _MM_DENORMALS_ZERO_ON
#define _MM_DENORMALS_ZERO_ON 0x0040
#endif
#ifndef _MM_SET_DENORMALS_ZERO_MODE
#define _MM_SET_DENORMALS_ZERO_MODE(x) ((void)0)
#endif
