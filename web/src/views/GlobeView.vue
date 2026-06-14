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
import {
  BufferGeometry,
  Float32BufferAttribute,
  ShaderMaterial,
  Points,
  AdditiveBlending,
  Color,
} from 'three'
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
let pointCloud = null // THREE.Points — the whole dataset in one buffer
let resizeHandler = null
let clickToken = 0

// three-globe places the sphere at the origin with this radius; matching it
// lets us drop our own geometry straight onto the globe surface.
const GLOBE_RADIUS = 100

function latLngToVec3(lat, lng, alt = 0.007) {
  const phi = ((90 - lat) * Math.PI) / 180
  const theta = ((90 - lng) * Math.PI) / 180
  const r = GLOBE_RADIUS * (1 + alt)
  return [
    r * Math.sin(phi) * Math.cos(theta),
    r * Math.cos(phi),
    r * Math.sin(phi) * Math.sin(theta),
  ]
}

// Render every site as ONE gl.POINTS draw call. A merged sphere-per-point layer
// (globe.gl's default) chokes well before this many markers; a single buffered
// point cloud with a custom shader scales to millions and lets dense regions
// glow additively, which doubles as a density map.
const VERT = `
  attribute float aSize;
  attribute vec3 aColor;
  uniform float uScale;
  varying vec3 vColor;
  void main() {
    vColor = aColor;
    vec4 mv = modelViewMatrix * vec4(position, 1.0);
    gl_Position = projectionMatrix * mv;
    gl_PointSize = clamp(aSize * uScale / -mv.z, 1.0, 26.0);
  }
`
const FRAG = `
  varying vec3 vColor;
  void main() {
    vec2 uv = gl_PointCoord - vec2(0.5);
    float d = length(uv);
    if (d > 0.5) discard;            // round dots, not squares
    float alpha = smoothstep(0.5, 0.12, d);
    gl_FragColor = vec4(vColor, alpha);
  }
`

function updatePointScale() {
  if (!pointCloud || !globe) return
  const cam = globe.camera()
  const fov = cam && cam.fov ? cam.fov : 50
  const h =
    (globe.renderer() && globe.renderer().domElement.height) ||
    globeEl.value.clientHeight * (window.devicePixelRatio || 1)
  // world-unit point size -> pixels, with perspective attenuation in the shader
  pointCloud.material.uniforms.uScale.value =
    h / (2 * Math.tan((fov * Math.PI) / 180 / 2))
}

function buildPointCloud(list) {
  const n = list.length
  const positions = new Float32Array(n * 3)
  const sizes = new Float32Array(n)
  const colors = new Float32Array(n * 3)

  // Scale size/brightness by log(count) so a 1-obs site and a 30k-obs site are
  // both visible but distinguishable.
  let maxLog = 1
  for (let i = 0; i < n; i++) {
    const l = Math.log1p(list[i].count || 1)
    if (l > maxLog) maxLog = l
  }
  const low = new Color('#1f8a3b')
  const high = new Color('#e8ffe0')
  const c = new Color()

  for (let i = 0; i < n; i++) {
    const site = list[i]
    const [x, y, z] = latLngToVec3(site.lat, site.lng)
    positions[i * 3] = x
    positions[i * 3 + 1] = y
    positions[i * 3 + 2] = z
    const t = Math.log1p(site.count || 1) / maxLog
    sizes[i] = 0.5 + t * 1.4
    c.copy(low).lerp(high, t)
    colors[i * 3] = c.r
    colors[i * 3 + 1] = c.g
    colors[i * 3 + 2] = c.b
  }

  const geom = new BufferGeometry()
  geom.setAttribute('position', new Float32BufferAttribute(positions, 3))
  geom.setAttribute('aSize', new Float32BufferAttribute(sizes, 1))
  geom.setAttribute('aColor', new Float32BufferAttribute(colors, 3))

  const material = new ShaderMaterial({
    uniforms: { uScale: { value: 800 } },
    vertexShader: VERT,
    fragmentShader: FRAG,
    transparent: true,
    depthTest: true, // the opaque globe still occludes back-facing sites
    depthWrite: false, // but points blend with each other (additive glow)
    blending: AdditiveBlending,
  })

  pointCloud = new Points(geom, material)
  pointCloud.renderOrder = 1
  pointCloud.frustumCulled = false
  globe.scene().add(pointCloud)
  updatePointScale()
}

function disposePointCloud() {
  if (!pointCloud) return
  if (globe) globe.scene().remove(pointCloud)
  pointCloud.geometry.dispose()
  pointCloud.material.dispose()
  pointCloud = null
}

function sizeGlobe() {
  if (!globe || !globeEl.value) return
  globe.width(globeEl.value.clientWidth)
  globe.height(globeEl.value.clientHeight)
  updatePointScale()
}

// The cloud is one merged buffer, so there are no per-marker click targets —
// instead we find the nearest site to wherever the globe was clicked (the same
// trick the original app used).
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
    disposePointCloud()
    buildPointCloud(locations.value)
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
    // Sites are drawn as our own GPU point cloud (see buildPointCloud), not the
    // built-in points layer. Rings are still used for the selection highlight.
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
  disposePointCloud()
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
