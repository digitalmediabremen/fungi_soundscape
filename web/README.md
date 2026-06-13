# fungi soundscape — web

A web-only reincarnation of the original openFrameworks + Vue *Fungi Soundscape*.
It browses real fungus observations from [Mushroom Observer](https://mushroomobserver.org)
and (in a later phase) will sonify them in the browser.

## What's here

- **Observe** — press the button, get a random observed fungus: image, species,
  taxonomy, location, date.
- **Globe** — every observation site on Earth (~26k) plotted on a globe; click a
  site to reveal the fungi found there.

## Data: a local "file API" (not the live API)

Per Mushroom Observer's guidance, the app does **not** hammer their live API.
Instead [`scripts/build-data.py`](scripts/build-data.py) downloads their nightly
CSV dumps once and transforms them into compact static JSON that the site reads
directly. **The only thing fetched from mushroomobserver.org at runtime is the
images.**

```bash
npm run build:data      # downloads CSV dumps -> public/data/*.json  (~38 MB, gitignored)
```

Generated layout (`public/data/`):

```
meta.json                 counts + bucket config
locations.json            location nodes: [{id, name, lat, lng, count}]   (loaded once)
obs/<0..255>.json         observation edges, bucketed by location_id:
                          { "<location_id>": [[id, name, date, imageId, conf, nameId], ...] }
names/<0..63>.json        species nodes, bucketed by name_id:
                          { "<name_id>": {n, rank, syn, dep, tax:[k,p,c,o,f], desc} }
```

Buckets keep each file small so the browser loads only what it needs (the globe
loads `locations.json`; clicking a site loads one `obs` bucket; a card loads one
`names` bucket).

### Graph-ready by design

The dataset preserves relations as IDs so future **network/graph views** can be
built directly on it — no reprocessing:

- **species ↔ location** — the observation edges (`obs/`), keyed both ways.
- **taxonomic hierarchy** — each species node carries its `[kingdom, phylum,
  class, order, family]` lineage (`names/`).
- **synonym clusters** — species sharing a `syn` (synonym group id) are synonyms.
- **co-occurrence** — species sharing a location (derivable from the edges).

The client data layer [`src/api/localData.js`](src/api/localData.js) exposes
`getLocations()`, `fetchObservationsByLocation()`, `getNameInfo()` and
`fetchRandomObservation()` — the natural query surface for those graphs.

## Stack

Vite + Vue 3 + `vue-router` + [`globe.gl`](https://github.com/vasturiano/globe.gl).

## Run

```bash
cd web
npm install
npm run build:data   # one-time (and whenever you want fresher data)
npm run dev          # http://localhost:5173
npm run build        # production build into dist/
```

> Deploying: `public/data/` is gitignored (~38 MB). Run `npm run build:data`
> before `npm run build` so the JSON is copied into `dist/`.

## Layout

```
scripts/build-data.py       CSV dumps -> local JSON (the data pipeline)
src/api/localData.js        local data access + relation/graph helpers
src/api/mushroomObserver.js remote bits that remain: image URLs + obs page links
src/components/MushroomCard.vue
src/views/RandomView.vue    "Observe" — random mushroom
src/views/GlobeView.vue     globe of worldwide encounter sites
```

## Next

- **Network/graph view** — visualize the relations above (taxonomy tree,
  species co-occurrence, synonym clusters).
- **Sonification** — port the old image→sound idea (contour / bit-matrix from
  `of_app/.../new_test`) to the Web Audio API.
