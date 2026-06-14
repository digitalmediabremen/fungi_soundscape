// End-to-end (node): real image -> ML mask -> combined contour matrix.
// Prints the final 64x64 matrix that drives the synth, so we can see it's
// mushroom-only contours (cap outline + gills), background dropped.
import { RawImage } from '@huggingface/transformers'
import { segmentMask } from '../src/audio/segmenter.js'
import { processImageData, MATRIX, MIN_TO_FILL } from '../src/audio/sonifier.js'

const URL = process.argv[2] || 'https://images.mushroomobserver.org/640/51.jpg'
const W = 256
const img = await (await RawImage.fromURL(URL)).resize(W, W)
const rgba = img.rgba().data

const mask = await segmentMask({ data: rgba, width: W, height: W }, W, W)
const withML = processImageData(rgba, W, mask)
const noML = processImageData(rgba, W, null)

function frac(m) {
  let n = 0
  for (let y = 0; y < MATRIX; y++) for (let x = 0; x < MATRIX; x++) if (m[y][x] > MIN_TO_FILL) n++
  return ((n / (MATRIX * MATRIX)) * 100).toFixed(1)
}
function art(m, title) {
  const ramp = ' .:-=+*#%@'
  let s = `\n${title} (active ${frac(m)}%)\n`
  for (let y = 0; y < MATRIX; y += 2) {
    for (let x = 0; x < MATRIX; x += 1) s += ramp[Math.min(9, (m[y * MATRIX + x !== undefined ? y : y][x] * 9) | 0)]
    s += '\n'
  }
  return s
}
// matrix rows are Float32Array; index [y][x]
function art2(m, title) {
  const ramp = ' .:-=+*#%@'
  let s = `\n${title} (active ${frac(m)}%)\n`
  for (let y = 0; y < MATRIX; y += 2) {
    let line = ''
    for (let x = 0; x < MATRIX; x += 1) line += ramp[Math.min(9, (m[y][x] * 9) | 0)]
    s += line + '\n'
  }
  return s
}
console.log(art2(noML.matrix, 'WITHOUT ML (all edges incl. background)'))
console.log(art2(withML.matrix, 'WITH ML segmentation (mushroom contours only)'))
