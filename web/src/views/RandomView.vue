<template>
  <div class="random">
    <div class="random__stage">
      <transition name="fade" mode="out-in">
        <!-- Loading -->
        <div v-if="loading" key="loading" class="random__center">
          <div class="spinner" />
          <p class="random__hint">summoning a fungus…</p>
        </div>

        <!-- Error -->
        <div v-else-if="error" key="error" class="random__center">
          <p class="error">{{ error }}</p>
          <button class="btn" @click="observe">try again</button>
        </div>

        <!-- Result -->
        <div v-else-if="fungus" key="result" class="random__center">
          <div class="random__result">
            <MushroomCard :fungus="fungus" class="random__card" />
            <SonifyDebug
              v-if="debugMatrix"
              :matrix="debugMatrix"
              :position-fn="positionFn"
              class="random__debug"
            />
          </div>
          <div class="random__actions">
            <button
              class="btn btn--sonify"
              :class="{ 'btn--playing': sonifying }"
              :disabled="sonifyLoading"
              @click="toggleSonify"
            >
              {{ sonifyLoading ? '… rendering' : sonifying ? '◼ stop' : '♫ sonify' }}
            </button>
            <button class="btn" :disabled="loading" @click="observe">observe another</button>
          </div>

          <div class="random__engine">
            <span class="random__engine-label">synth</span>
            <button
              v-for="e in engines"
              :key="e.id"
              class="random__engine-btn"
              :class="{ 'random__engine-btn--on': engine === e.id }"
              :disabled="sonifying || sonifyLoading"
              :title="e.title"
              @click="engine = e.id"
            >
              {{ e.short }}
            </button>
            <button
              class="random__engine-btn"
              :class="{ 'random__engine-btn--on': isolate }"
              :disabled="sonifying || sonifyLoading"
              title="Isolate the mushroom from the background with a local ML segmentation model (runs in your browser, no API)"
              @click="isolate = !isolate"
            >
              {{ isolate ? '◉' : '○' }} isolate (ML)
            </button>
          </div>

          <p v-if="sonifyError" class="random__sonify-hint">{{ sonifyError }}</p>
          <p v-else-if="sonifyLoading" class="random__sonify-hint">
            {{ modelStatus || (isolate ? 'isolating the mushroom + rendering…' : 'rendering…') }}
          </p>
          <p v-else-if="sonifying" class="random__sonify-hint">
            {{ engine === 'pdsp'
              ? 'playing its contour through your ofxPDSP synth (WebAssembly)'
              : 'akebono scan of the contour' }}
          </p>
        </div>

        <!-- Idle -->
        <div v-else key="idle" class="random__center">
          <p class="random__intro">
            A random fungus, observed somewhere on Earth.
          </p>
          <button class="btn btn--big" @click="observe">observe</button>
        </div>
      </transition>
    </div>
  </div>
</template>

<script setup>
import { ref, onBeforeUnmount } from 'vue'
import MushroomCard from '../components/MushroomCard.vue'
import { fetchRandomObservation } from '../api/localData'
import { Sonifier } from '../audio/sonifier'
import { PdspSonifier } from '../audio/pdspSonifier'
import SonifyDebug from '../components/SonifyDebug.vue'

const loading = ref(false)
const error = ref('')
const fungus = ref(null)

// Debug view: the processed 64x64 matrix + live playhead (set once a sonifier
// has prepared it). positionFn polls the active sonifier each animation frame.
const debugMatrix = ref(null)
function positionFn() {
  return sonifier && sonifier.position ? sonifier.position() : null
}

// Sonification engines: the real ofxPDSP synth compiled to WASM (default), and
// the interim Web Audio one. AudioContext starts from the click (user gesture).
const engines = [
  { id: 'pdsp', short: 'ofxPDSP ▸ wasm', title: 'Your original ofxPDSP synth compiled to WebAssembly' },
  { id: 'js', short: 'web audio', title: 'Interim Web Audio engine' },
]
const engine = ref('pdsp')
const isolate = ref(true) // ML mushroom isolation (local segmentation model)
const sonifying = ref(false)
const sonifyLoading = ref(false)
const sonifyError = ref('')
const modelStatus = ref('') // vision-model download progress, first run only
let sonifier = null
let sonifierEngine = null

function onModelProgress(p) {
  if (p && p.status === 'progress' && p.progress != null) {
    modelStatus.value = `downloading vision model… ${Math.round(p.progress)}%`
  } else if (p && (p.status === 'done' || p.status === 'ready')) {
    modelStatus.value = ''
  }
}

function makeSonifier(id) {
  const s = id === 'pdsp' ? new PdspSonifier() : new Sonifier()
  s.onended = () => {
    sonifying.value = false // PDSP renders a finite buffer; clear when it ends
  }
  return s
}

async function toggleSonify() {
  if (sonifying.value || sonifyLoading.value) return stopSonify()
  if (!fungus.value) return
  sonifyError.value = ''
  if (!sonifier || sonifierEngine !== engine.value) {
    if (sonifier) sonifier.dispose()
    sonifier = makeSonifier(engine.value)
    sonifierEngine = engine.value
  }
  sonifyLoading.value = true
  const opts = { segment: isolate.value, onProgress: onModelProgress }
  try {
    await sonifier.start(fungus.value, opts)
    sonifying.value = true
  } catch (e) {
    console.error('sonify failed', e)
    // Fall back to Web Audio if the WASM engine couldn't load/render.
    if (engine.value === 'pdsp') {
      sonifyError.value = 'wasm synth unavailable — switched to web audio'
      engine.value = 'js'
      if (sonifier) sonifier.dispose()
      sonifier = makeSonifier('js')
      sonifierEngine = 'js'
      try {
        await sonifier.start(fungus.value, opts)
        sonifying.value = true
      } catch {
        sonifyError.value = 'sonification failed'
      }
    } else {
      sonifyError.value = 'sonification failed'
    }
  } finally {
    sonifyLoading.value = false
    modelStatus.value = ''
    // expose the processed matrix to the debug view once it's been prepared
    if (sonifier && sonifier.data) debugMatrix.value = sonifier.data.matrix
  }
}

function stopSonify() {
  if (sonifier) sonifier.stop()
  sonifying.value = false
  sonifyLoading.value = false
  modelStatus.value = ''
}

async function observe() {
  stopSonify()
  debugMatrix.value = null
  loading.value = true
  error.value = ''
  try {
    fungus.value = await fetchRandomObservation()
  } catch (e) {
    error.value = e.message || 'Something went wrong.'
  } finally {
    loading.value = false
  }
}

onBeforeUnmount(() => {
  if (sonifier) sonifier.dispose()
})
</script>

<style scoped>
.random {
  height: 100%;
  overflow-y: auto;
}
.random__stage {
  min-height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 32px 16px;
}
.random__center {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 22px;
}
.random__result {
  display: flex;
  gap: 22px;
  align-items: flex-start;
  justify-content: center;
  flex-wrap: wrap;
}
.random__card {
  width: min(440px, 92vw);
}
.random__debug {
  margin-top: 6px;
}
.random__actions {
  display: flex;
  gap: 12px;
  flex-wrap: wrap;
  justify-content: center;
}
.btn--sonify {
  letter-spacing: 1px;
}
.btn--playing {
  background: var(--accent);
  color: #04140a;
  box-shadow: 0 0 24px rgba(57, 255, 20, 0.35);
  animation: pulse 1.4s ease-in-out infinite;
}
@keyframes pulse {
  0%,
  100% {
    box-shadow: 0 0 18px rgba(57, 255, 20, 0.3);
  }
  50% {
    box-shadow: 0 0 30px rgba(57, 255, 20, 0.6);
  }
}
.random__sonify-hint {
  color: var(--muted);
  font-size: 12px;
  letter-spacing: 1px;
  text-align: center;
  margin: 0;
}
.random__engine {
  display: flex;
  align-items: center;
  gap: 6px;
}
.random__engine-label {
  color: var(--muted);
  font-size: 11px;
  letter-spacing: 1px;
  text-transform: uppercase;
  margin-right: 2px;
}
.random__engine-btn {
  background: transparent;
  color: var(--muted);
  border: 1px solid #1a201a;
  border-radius: 2px;
  padding: 5px 10px;
  font-size: 11px;
  letter-spacing: 1px;
  text-transform: uppercase;
  transition: color 0.15s, border-color 0.15s;
}
.random__engine-btn--on {
  color: var(--accent);
  border-color: var(--accent-dim);
}
.random__engine-btn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}
.random__intro {
  color: var(--muted);
  font-size: 16px;
  text-align: center;
  max-width: 360px;
}
.random__hint {
  color: var(--muted);
  letter-spacing: 1px;
}

.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.2s ease;
}
.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}
</style>
