// Which local dataset the file-API reads from.
//
// Two datasets are built side by side so GBIF can be tested against Mushroom
// Observer without losing either:
//   - Mushroom Observer  -> public/data/   (scripts/build-data.py)
//   - GBIF               -> public/gbif/   (scripts/build-gbif-data.py)
//
// Both emit the same bucket layout, so the only per-source differences are the
// base directory, the image handling, and the "view original" link. The active
// source is resolved ONCE at module load (URL ?source= > localStorage > default)
// and switching it reloads the page, so the data caches in localData.js never
// mix records from two sources.

const SOURCES = {
  mo: {
    id: 'mo',
    label: 'Mushroom Observer',
    short: 'MO',
    dir: 'data/',
    // Obs tuples store an integer image id; the URL is built in localData.
    observationUrl: (id) => `https://mushroomobserver.org/${id}`,
  },
  gbif: {
    id: 'gbif',
    label: 'GBIF',
    short: 'GBIF',
    dir: 'gbif/',
    // Obs tuples store a full image URL string already.
    observationUrl: (id) => `https://www.gbif.org/occurrence/${id}`,
  },
}

const DEFAULT = 'mo'
const STORAGE_KEY = 'fungi.source'

function resolve() {
  try {
    const q = new URLSearchParams(window.location.search).get('source')
    if (q && SOURCES[q]) return SOURCES[q]
    const saved = window.localStorage.getItem(STORAGE_KEY)
    if (saved && SOURCES[saved]) return SOURCES[saved]
  } catch {
    /* SSR / no DOM — fall through */
  }
  return SOURCES[DEFAULT]
}

export const sources = SOURCES
export const source = resolve()

/** Switch dataset and reload (caches are per-source, so a reload is cleanest). */
export function setSource(id) {
  if (!SOURCES[id] || id === source.id) return
  try {
    window.localStorage.setItem(STORAGE_KEY, id)
    const url = new URL(window.location.href)
    url.searchParams.delete('source') // let localStorage win from here on
    window.location.href = url.toString()
  } catch {
    /* ignore */
  }
}
