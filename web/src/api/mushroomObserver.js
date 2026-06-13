// Remote helpers for Mushroom Observer.
//
// The app's *data* now comes from local JSON built from MO's CSV dumps
// (see src/api/localData.js and scripts/build-data.py). The only thing still
// fetched from mushroomobserver.org at runtime is the images themselves —
// plus a link back to each observation's page.

const IMAGE_HOST = 'https://mushroomobserver.org/images'

// Available image sizes on MO: thumbnail, 320, 640, 960, 1280, orig.
export const IMAGE_SIZE = 640

/** Full URL for an observation's primary image at a given size. */
export function imageUrl(imageId, size = IMAGE_SIZE) {
  return `${IMAGE_HOST}/${size}/${imageId}.jpg`
}

/** Public Mushroom Observer page for an observation. */
export function observationUrl(id) {
  return `https://mushroomobserver.org/${id}`
}
