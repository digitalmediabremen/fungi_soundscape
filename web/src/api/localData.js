// Local "file API" for the Fungi Soundscape web app.
//
// All data is served as static JSON built from Mushroom Observer's CSV dumps
// by scripts/build-data.py — no live API calls, no rate limits. Only the
// images are fetched from mushroomobserver.org at runtime.
//
// The dataset is deliberately relation-preserving so future network/graph
// views can be built directly on it:
//   - location nodes          getLocations()                  -> [{id,name,lat,lng,count}]
//   - species nodes           getNameInfo(nameId)             -> {name,rank,synonymId,taxonomy,description}
//   - observation edges       getLocationObservations(locId)  -> species<->location links
// From these you can derive co-occurrence (species sharing a location),
// taxonomic hierarchy (lineage), and synonym clusters (shared synonymId).
//
// Bucket counts MUST match scripts/build-data.py.

import { imageUrl, observationUrl } from './mushroomObserver'

const OBS_BUCKETS = 256
const NAME_BUCKETS = 64
const BASE = `${import.meta.env.BASE_URL}data/`

// --- low-level loaders (cached) ------------------------------------------

const jsonCache = new Map()
function loadJson(path) {
  if (!jsonCache.has(path)) {
    jsonCache.set(
      path,
      fetch(`${BASE}${path}`).then((res) => {
        if (!res.ok) throw new Error(`Failed to load ${path} (HTTP ${res.status})`)
        return res.json()
      })
    )
  }
  return jsonCache.get(path)
}

let locationsPromise = null
let locationIndex = null // Map<id, location>
let cumulative = null // weighted-by-count cumulative array for random picks
let cumulativeTotal = 0

/** Load every location node (those with image observations). */
export function getLocations() {
  if (!locationsPromise) {
    locationsPromise = loadJson('locations.json').then((list) => {
      locationIndex = new Map(list.map((l) => [l.id, l]))
      // Build cumulative weights so random picks are uniform across
      // observations (not across locations).
      cumulative = new Array(list.length)
      let acc = 0
      for (let i = 0; i < list.length; i++) {
        acc += list[i].count || 0
        cumulative[i] = acc
      }
      cumulativeTotal = acc
      return list
    })
  }
  return locationsPromise
}

function obsBucket(locationId) {
  return loadJson(`obs/${locationId % OBS_BUCKETS}.json`)
}

function nameBucket(nameId) {
  return loadJson(`names/${nameId % NAME_BUCKETS}.json`)
}

// --- shaping --------------------------------------------------------------

// Observation tuple layout (see build-data.py):
//   [id, name, date, imageId, confidence, nameId]
function toFungus(tuple, location) {
  const [id, name, date, imageId, confidence, nameId] = tuple
  return {
    id,
    name,
    nameId,
    date,
    imageId,
    imageUrl: imageId ? imageUrl(imageId) : null,
    confidence,
    locationId: location?.id ?? null,
    locationName: location?.name ?? '',
    lat: location?.lat ?? null,
    lng: location?.lng ?? null,
    hasLocation: location != null,
    url: observationUrl(id),
  }
}

// --- public data API ------------------------------------------------------

/**
 * Species node: taxonomy + synonym group + description for a name_id.
 * Returns null if the species has no node (e.g. unnamed observations).
 */
export async function getNameInfo(nameId) {
  if (nameId == null) return null
  const bucket = await nameBucket(nameId)
  const node = bucket[String(nameId)]
  if (!node) return null
  const [kingdom, phylum, klass, order, family] = node.tax || []
  return {
    nameId,
    name: node.n,
    rank: node.rank ?? null,
    synonymId: node.syn ?? null,
    deprecated: !!node.dep,
    taxonomy: { kingdom, phylum, class: klass, order, family },
    description: node.desc || '',
  }
}

/**
 * All image observations recorded at a location (the species<->location
 * edges for that node). Capped for display; `total` is the true count.
 */
export async function fetchObservationsByLocation(locationId, limit = 100) {
  await getLocations()
  const location = locationIndex.get(locationId)
  const bucket = await obsBucket(locationId)
  const tuples = bucket[String(locationId)] || []
  return {
    results: tuples.slice(0, limit).map((t) => toFungus(t, location)),
    total: tuples.length,
  }
}

/** A single random observation, uniform across all observations. */
export async function fetchRandomObservation() {
  const list = await getLocations()
  if (!list.length) throw new Error('No data loaded.')

  // Pick a location weighted by its observation count, then an obs within it.
  const target = Math.random() * cumulativeTotal
  let lo = 0
  let hi = cumulative.length - 1
  while (lo < hi) {
    const mid = (lo + hi) >> 1
    if (cumulative[mid] <= target) lo = mid + 1
    else hi = mid
  }
  const location = list[lo]
  const bucket = await obsBucket(location.id)
  const tuples = bucket[String(location.id)] || []
  if (!tuples.length) throw new Error('Could not find a mushroom — try again.')
  const tuple = tuples[Math.floor(Math.random() * tuples.length)]
  return toFungus(tuple, location)
}
