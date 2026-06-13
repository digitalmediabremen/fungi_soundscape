<template>
  <div class="globe">
    <div ref="globeEl" class="globe__canvas" />

    <!-- Overlay status -->
    <div class="globe__hud">
      <div class="globe__count">
        <span class="globe__count-n">{{ locations.length.toLocaleString() }}</span> sites
      </div>
      <p v-if="loadError" class="error globe__error">{{ loadError }}</p>
    </div>

    <!-- Selected location panel -->
    <transition name="slide">
      <aside v-if="selectedLocation" class="globe__panel">
        <button class="globe__close" @click="clearSelection" aria-label="close">×</button>
        <h2 class="globe__loc">{{ selectedLocation.name }}</h2>

        <div v-if="locLoading" class="globe__loading">
          <div class="spinner" />
          <span>fetching observations…</span>
        </div>

        <p v-else-if="locError" class="error">{{ locError }}</p>

        <template v-else>
          <p class="globe__loc-sub">
            {{ locTotal }} observation{{ locTotal === 1 ? '' : 's' }} here
            <span v-if="locObs.length < locTotal">· showing first {{ locObs.length }}</span>
          </p>
          <div class="globe__cards">
            <MushroomCard v-for="f in locObs" :key="f.id" :fungus="f" />
          </div>
        </template>
      </aside>
    </transition>

    <p v-if="!selectedLocation" class="globe__hint">
      drag to spin · click a glowing site to reveal its fungi
    </p>
  </div>
</template>

<script setup>
import { ref, shallowRef, onMounted, onBeforeUnmount } from 'vue'
import Globe from 'globe.gl'
import MushroomCard from '../components/MushroomCard.vue'
import { getLocations, fetchObservationsByLocation } from '../api/localData'

const globeEl = ref(null)
const locations = shallowRef([])
const loadError = ref('')

const selectedLocation = ref(null)
const locObs = shallowRef([])
const locTotal = ref(0)
const locLoading = ref(false)
const locError = ref('')

let globe = null
let resizeHandler = null
let clickToken = 0

function sizeGlobe() {
  if (!globe || !globeEl.value) return
  globe.width(globeEl.value.clientWidth)
  globe.height(globeEl.value.clientHeight)
}

// Points are merged into one geometry for performance (23k of them), so we
// can't rely on per-point click events — instead we find the nearest site to
// wherever the globe was clicked (the same trick the original app used).
function findNearest(lat, lng) {
  const cosLat = Math.cos((lat * Math.PI) / 180)
  let best = null
  let bestD = Infinity
  for (const p of locations.value) {
    let dLng = p.lng - lng
    if (dLng > 180) dLng -= 360
    else if (dLng < -180) dLng += 360
    const dLat = p.lat - lat
    const d = dLat * dLat + (dLng * cosLat) * (dLng * cosLat)
    if (d < bestD) {
      bestD = d
      best = p
    }
  }
  return best
}

async function selectLocation(loc) {
  if (!loc) return
  selectedLocation.value = loc
  locObs.value = []
  locError.value = ''
  locLoading.value = true
  const token = ++clickToken

  // Ripple highlight + fly to the site.
  globe.ringsData([{ lat: loc.lat, lng: loc.lng }])
  globe.pointOfView({ lat: loc.lat, lng: loc.lng, altitude: 1.5 }, 800)

  try {
    const { results, total } = await fetchObservationsByLocation(loc.id)
    if (token !== clickToken) return // a newer click superseded this one
    locObs.value = results
    locTotal.value = total
    if (!results.length) locError.value = 'No image observations found here.'
  } catch (e) {
    if (token !== clickToken) return
    locError.value = e.message || 'Failed to load observations.'
  } finally {
    if (token === clickToken) locLoading.value = false
  }
}

function clearSelection() {
  selectedLocation.value = null
  globe.ringsData([])
}

async function loadLocations() {
  try {
    locations.value = await getLocations()
    globe.pointsData(locations.value)
  } catch (e) {
    loadError.value = 'Could not load the site map.'
    console.error(e)
  }
}

onMounted(() => {
  globe = new Globe(globeEl.value)
    .globeImageUrl('//unpkg.com/three-globe/example/img/earth-night.jpg')
    .bumpImageUrl('//unpkg.com/three-globe/example/img/earth-topology.png')
    .backgroundColor('#050608')
    .showAtmosphere(true)
    .atmosphereColor('#39ff14')
    .atmosphereAltitude(0.16)
    .pointLat('lat')
    .pointLng('lng')
    .pointAltitude(0.005)
    .pointRadius(0.16)
    .pointColor(() => '#39ff14')
    .pointsMerge(true)
    .ringColor(() => '#ffffff')
    .ringMaxRadius(4)
    .ringPropagationSpeed(1.4)
    .ringRepeatPeriod(700)
    .onGlobeClick(({ lat, lng }) => selectLocation(findNearest(lat, lng)))

  sizeGlobe()
  // No auto-rotation — the globe stays still until the user drags it.
  globe.controls().autoRotate = false

  resizeHandler = () => sizeGlobe()
  window.addEventListener('resize', resizeHandler)

  loadLocations()
})

onBeforeUnmount(() => {
  if (resizeHandler) window.removeEventListener('resize', resizeHandler)
  if (globe && typeof globe._destructor === 'function') globe._destructor()
  globe = null
})
</script>

<style scoped>
.globe {
  position: absolute;
  inset: 0;
  overflow: hidden;
}
.globe__canvas {
  position: absolute;
  inset: 0;
}

.globe__hud {
  position: absolute;
  top: 16px;
  left: 16px;
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 10px;
  z-index: 5;
}
.globe__count {
  color: var(--muted);
  font-size: 13px;
  letter-spacing: 1px;
  text-transform: uppercase;
}
.globe__count-n {
  color: var(--accent);
  font-size: 18px;
}
.globe__error {
  font-size: 12px;
}

.globe__hint {
  position: absolute;
  bottom: 16px;
  width: 100%;
  text-align: center;
  color: var(--muted);
  font-size: 12px;
  letter-spacing: 1px;
  pointer-events: none;
  margin: 0;
}

.globe__panel {
  position: absolute;
  top: 0;
  right: 0;
  height: 100%;
  width: min(440px, 94vw);
  background: rgba(5, 6, 8, 0.92);
  border-left: 1px solid #1a201a;
  backdrop-filter: blur(6px);
  padding: 46px 18px 18px;
  overflow-y: auto;
  z-index: 6;
}
.globe__close {
  position: absolute;
  top: 10px;
  right: 14px;
  background: transparent;
  border: none;
  color: var(--muted);
  font-size: 28px;
  line-height: 1;
}
.globe__close:hover {
  color: var(--fg);
}
.globe__loc {
  margin: 0 0 4px;
  font-size: 18px;
  color: var(--fg);
}
.globe__loc-sub {
  margin: 0 0 16px;
  color: var(--muted);
  font-size: 12px;
  letter-spacing: 1px;
  text-transform: uppercase;
}
.globe__loading {
  display: flex;
  align-items: center;
  gap: 12px;
  color: var(--muted);
  padding: 20px 0;
}
.globe__cards {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.slide-enter-active,
.slide-leave-active {
  transition: transform 0.25s ease, opacity 0.25s ease;
}
.slide-enter-from,
.slide-leave-to {
  transform: translateX(20px);
  opacity: 0;
}
</style>
