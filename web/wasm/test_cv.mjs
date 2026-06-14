// Validate the contour CV in node (no browser): synthetic shapes ->
// edges detected on boundaries, flat areas quiet, and different images differ.
import { processImageData, MATRIX, MIN_TO_FILL } from '../src/audio/sonifier.js'

const W = 192
function render(draw) {
  const d = new Uint8ClampedArray(W * W * 4)
  for (let y = 0; y < W; y++)
    for (let x = 0; x < W; x++) {
      const i = (y * W + x) * 4
      const v = draw(x, y)
      d[i] = d[i + 1] = d[i + 2] = v
      d[i + 3] = 255
    }
  return d
}
const noise = (x, y) => 40 + ((x * 7 + y * 13) % 22)
const disk = (cx, cy, r) => (x, y) => (Math.hypot(x - cx, y - cy) < r ? 210 : noise(x, y))

const A = processImageData(render(disk(96, 96, 55)), W)
const B = processImageData(render(disk(60, 70, 28)), W)
const flat = processImageData(render(() => 128), W)

function activeFrac(m) {
  let n = 0
  for (let y = 0; y < MATRIX; y++) for (let x = 0; x < MATRIX; x++) if (m[y][x] > MIN_TO_FILL) n++
  return n / (MATRIX * MATRIX)
}
let diff = 0
for (let y = 0; y < MATRIX; y++) for (let x = 0; x < MATRIX; x++) diff += Math.abs(A.matrix[y][x] - B.matrix[y][x])
diff /= MATRIX * MATRIX

// the disk centre cell (flat bright) should be quieter than its edge
const center = A.matrix[MATRIX >> 1][MATRIX >> 1]
const edge = A.matrix[(96 - 55) / (W / MATRIX) | 0][MATRIX >> 1] // top of the ring

console.log(`big disk active=${(activeFrac(A.matrix) * 100).toFixed(1)}%  small disk active=${(activeFrac(B.matrix) * 100).toFixed(1)}%`)
console.log(`flat image active=${(activeFrac(flat.matrix) * 100).toFixed(1)}%  (want ~0)`)
console.log(`A vs B mean abs diff=${diff.toFixed(3)}  (want > 0)`)
console.log(`disk: center=${center.toFixed(2)} edge=${edge.toFixed(2)}  (want edge > center)`)

const ok =
  activeFrac(A.matrix) > 0.01 &&
  activeFrac(flat.matrix) < 0.02 &&
  diff > 0.02 &&
  edge > center
console.log(ok ? 'PASS contour CV (edges found, images differ, flat is quiet)' : 'FAIL')
process.exit(ok ? 0 : 1)
