// In-browser mushroom segmentation — fully local, NO API.
//
// Uses transformers.js (onnxruntime-web) to run a background-removal model
// (RMBG-1.4) entirely in the browser: it isolates the salient foreground (the
// mushroom) from a cluttered background and returns a soft foreground mask.
// Inference is local; the model weights download once from the Hugging Face CDN
// and are cached by the browser (they can be self-hosted for full offline use —
// see src/audio/README.md). transformers.js is dynamically imported so it (and
// onnxruntime) only load when segmentation is actually used.

const MODEL = 'briaai/RMBG-1.4'

let loadPromise = null

async function getModel(onProgress) {
  if (loadPromise) return loadPromise
  loadPromise = (async () => {
    const tf = await import('@huggingface/transformers')
    const { env, AutoModel, AutoProcessor, RawImage } = tf
    env.allowLocalModels = false // weights come from the HF CDN (cached)

    const opts = { progress_callback: onProgress }
    let model
    try {
      model = await AutoModel.from_pretrained(MODEL, { ...opts, device: 'webgpu' })
    } catch {
      model = await AutoModel.from_pretrained(MODEL, opts) // wasm fallback
    }
    const processor = await AutoProcessor.from_pretrained(MODEL, opts)
    return { model, processor, RawImage }
  })()
  return loadPromise
}

/** Kick off model loading early (optional warm-up). */
export function preloadSegmenter(onProgress) {
  return getModel(onProgress).then(() => true).catch(() => false)
}

/**
 * Foreground (mushroom) mask for an ImageData-like {data,width,height} (RGBA).
 * Returns a Float32Array(outW*outH) of foreground probabilities (0..1).
 */
export async function segmentMask(imageData, outW, outH, onProgress) {
  const { model, processor, RawImage } = await getModel(onProgress)

  const image = new RawImage(imageData.data, imageData.width, imageData.height, 4).rgb()
  const { pixel_values } = await processor(image)
  const out = await model({ input: pixel_values })

  // RMBG returns a single-channel mask tensor (1,1,H,W); be tolerant of the key.
  const tensor = out.output ?? out.alphas ?? out.logits ?? Object.values(out)[0]
  // NOTE: RawImage.resize() is async — await the whole chain (the RMBG example does).
  const maskImg = await RawImage.fromTensor(tensor[0].mul(255).to('uint8')).resize(outW, outH)

  const data = maskImg.data
  const ch = maskImg.channels || 1
  const result = new Float32Array(outW * outH)
  for (let i = 0; i < outW * outH; i++) result[i] = data[i * ch] / 255
  return result
}
