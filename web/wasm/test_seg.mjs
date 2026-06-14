// Verify in-browser segmentation in node: load a REAL mushroom image, run the
// RMBG model, print the 64x64 foreground mask as ASCII so we can see the shape.
import { RawImage } from '@huggingface/transformers'
import { segmentMask } from '../src/audio/segmenter.js'

const URL = process.argv[2] || 'https://images.mushroomobserver.org/640/51.jpg'
console.log('loading', URL)
const img = (await RawImage.fromURL(URL)).rgba()
console.log(`image ${img.width}x${img.height}, running RMBG (first run downloads ~44MB)…`)

let lastPct = -1
const mask = await segmentMask({ data: img.data, width: img.width, height: img.height }, 64, 64, (p) => {
  if (p.status === 'progress' && p.progress != null) {
    const pct = Math.floor(p.progress / 10) * 10
    if (pct !== lastPct) {
      lastPct = pct
      process.stdout.write(`  ${p.file || ''} ${pct}%\r`)
    }
  }
})

// stats + ASCII preview (downsample 64 -> 48 wide for the terminal)
let mn = 1, mx = 0, sum = 0
for (const v of mask) {
  if (v < mn) mn = v
  if (v > mx) mx = v
  sum += v
}
const fg = mask.reduce((n, v) => n + (v > 0.5 ? 1 : 0), 0) / mask.length
console.log(`\nmask min=${mn.toFixed(2)} max=${mx.toFixed(2)} mean=${(sum / mask.length).toFixed(2)} fg>0.5=${(fg * 100).toFixed(1)}%\n`)
const ramp = ' .:-=+*#%@'
let art = ''
for (let y = 0; y < 64; y += 2) {
  for (let x = 0; x < 64; x += 1) art += ramp[Math.min(9, (mask[y * 64 + x] * 9) | 0)]
  art += '\n'
}
console.log(art)
const ok = mx > 0.6 && mn < 0.4 && fg > 0.02 && fg < 0.95
console.log(ok ? 'PASS — produced a non-trivial foreground mask' : 'WARN — mask looks trivial')
process.exit(ok ? 0 : 1)
