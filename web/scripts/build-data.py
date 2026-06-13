#!/usr/bin/env python3
"""
Build the local "file API" for the Fungi Soundscape web app from Mushroom
Observer's nightly CSV dumps.

Instead of hitting the live API at runtime, we download the public CSV dumps
once, join them, and emit compact static JSON the site reads directly. Only
the images themselves are fetched from MO at runtime.

Dumps used (https://mushroomobserver.org/<name>.csv):
  observations      id, name_id, when, location_id, lat, lng, alt,
                    vote_cache, is_collection_location, thumb_image_id
  names             id, text_name, author, deprecated, ...
  locations         id, name, north, south, east, west, high, low
  name_descriptions id, name_id, source_type, source_name,
                    general_description, ...

This emits a relation-preserving (graph-ready) dataset: location nodes,
species nodes (with taxonomy + synonym group), and observation edges that
link species <-> location. Future network/graph views can build:
  - species <-> location bipartite graph (the observation edges)
  - species co-occurrence (species sharing a location)
  - taxonomic hierarchy (kingdom/phylum/class/order/family per species)
  - synonym clusters (shared synonym_id)

Dumps used (https://mushroomobserver.org/<name>.csv):
  observations         id, name_id, when, location_id, ..., thumb_image_id
  names                id, text_name, author, deprecated, ..., synonym_id, rank
  locations            id, name, north, south, east, west, high, low
  name_classifications name_id, domain, kingdom, phylum, class, order, family
  name_descriptions    id, name_id, ..., general_description, ...

Output (web/public/data/):
  meta.json        {observations, locations, species, obsBuckets, nameBuckets, ...}
  locations.json   [{id, name, lat, lng, count}]  -- location nodes (image obs only)
  obs/<0..N>.json  { "<location_id>": [[id, name, date, imageId, conf, nameId], ...] }
                   -- observation edges, bucketed by location_id
  names/<0..M>.json { "<name_id>": {n, rank, syn, dep, tax:[k,p,c,o,f], desc} }
                   -- species nodes, bucketed by name_id

Buckets keep each file small and let the browser load only what it needs.
Keep OBS_BUCKETS / NAME_BUCKETS in sync with src/api/localData.js.

Usage:  python3 scripts/build-data.py
"""

import csv
import json
import os
import re
import sys
import subprocess
from datetime import datetime, timezone

csv.field_size_limit(1 << 24)

HERE = os.path.dirname(os.path.abspath(__file__))
WEB = os.path.dirname(HERE)
CACHE = os.path.join(WEB, ".data-cache")
OUT = os.path.join(WEB, "public", "data")

OBS_BUCKETS = 256
NAME_BUCKETS = 64


def download(name):
    path = os.path.join(CACHE, f"{name}.csv")
    if os.path.exists(path) and os.path.getsize(path) > 0:
        print(f"  cached: {name}.csv ({os.path.getsize(path)//1048576} MB)")
        return path
    os.makedirs(CACHE, exist_ok=True)
    url = f"https://mushroomobserver.org/{name}.csv"
    print(f"  downloading {url} ...")
    subprocess.run(["curl", "-sSL", "-o", path, url], check=True)
    print(f"    -> {os.path.getsize(path)//1048576} MB")
    return path


def rows(name):
    """Yield rows from a tab-separated dump (quoting disabled — MO escapes)."""
    with open(download(name), newline="", encoding="utf-8", errors="replace") as f:
        reader = csv.reader(f, delimiter="\t", quoting=csv.QUOTE_NONE)
        header = next(reader)
        for r in reader:
            yield r, len(header)


def f(v):
    try:
        return float(v)
    except (ValueError, TypeError):
        return None


TAG_RE = re.compile(r"<[^>]+>")
TEXTILE_LINK_RE = re.compile(r'"([^"]+)":\S+')   # "label":url -> label
URL_RE = re.compile(r"https?://\S+")
WS_RE = re.compile(r"\s+")


def clean_desc(text, limit=480):
    if not text:
        return ""
    t = text.replace("\\n", " ").replace("\\t", " ")
    t = TEXTILE_LINK_RE.sub(r"\1", t)
    t = TAG_RE.sub(" ", t)
    t = URL_RE.sub("", t)
    t = WS_RE.sub(" ", t).strip()
    if len(t) > limit:
        t = t[:limit].rsplit(" ", 1)[0] + "…"
    return t


def main():
    print("1/6  names")
    names = {}      # id -> text_name
    name_meta = {}  # id -> (rank, synonym_id, deprecated)
    for r, _ in rows("names"):
        # id, text_name, author, deprecated, correct_spelling_id, synonym_id, rank
        if len(r) < 2:
            continue
        names[r[0]] = r[1]
        if len(r) >= 7:
            rank = int(r[6]) if r[6] not in ("", "NULL") else None
            syn = int(r[5]) if r[5] not in ("", "NULL") else None
            dep = r[3] == "1"
            name_meta[r[0]] = (rank, syn, dep)
    print(f"     {len(names):,} names")

    print("2/6  locations")
    locs = {}  # id -> (name, lat, lng)
    for r, _ in rows("locations"):
        if len(r) < 7:
            continue
        n, s, e, w = f(r[2]), f(r[3]), f(r[4]), f(r[5])
        if None in (n, s, e, w):
            continue
        lat, lng = round((n + s) / 2, 4), round((e + w) / 2, 4)
        if not (-90 <= lat <= 90 and -180 <= lng <= 180):
            continue
        locs[r[0]] = (r[1], lat, lng)
    print(f"     {len(locs):,} locations with coords")

    print("3/6  observations (image-bearing, placeable)")
    by_loc = {}     # location_id -> list of obs tuples  (the species<->location edges)
    used_names = set()
    n_obs = 0
    for r, _ in rows("observations"):
        # id, name_id, when, location_id, lat, lng, alt, vote_cache, is_coll, thumb_image_id
        if len(r) < 10:
            continue
        oid, name_id, when, loc_id, thumb = r[0], r[1], r[2], r[3], r[9]
        if thumb in ("", "NULL") or loc_id in ("", "NULL"):
            continue
        if loc_id not in locs:
            continue
        conf = f(r[7])
        has_name = name_id not in ("", "NULL")
        if has_name:
            used_names.add(name_id)
        tup = [
            int(oid),
            names.get(name_id, "Unknown species"),
            when,
            int(thumb),
            round(conf, 2) if conf is not None else None,
            int(name_id) if has_name else None,
        ]
        by_loc.setdefault(loc_id, []).append(tup)
        n_obs += 1
    print(f"     {n_obs:,} observations across {len(by_loc):,} locations; "
          f"{len(used_names):,} distinct species")

    print("4/6  taxonomy (name_classifications)")
    tax = {}  # name_id -> [kingdom, phylum, class, order, family]
    for r, _ in rows("name_classifications"):
        # name_id, domain, kingdom, phylum, class, order, family
        if len(r) < 7 or r[0] not in used_names:
            continue
        lineage = [r[2], r[3], r[4], r[5], r[6]]
        if any(lineage):
            tax[r[0]] = lineage
    print(f"     {len(tax):,} classified species")

    print("5/6  descriptions")
    desc = {}  # name_id -> text
    for r, ncols in rows("name_descriptions"):
        if len(r) != ncols or len(r) < 5 or r[1] not in used_names:
            continue
        nid, body = r[1], r[4]
        text = clean_desc(body)
        if text and nid not in desc:
            desc[nid] = text
    print(f"     {len(desc):,} species descriptions")

    print("6/6  writing JSON")
    os.makedirs(os.path.join(OUT, "obs"), exist_ok=True)
    os.makedirs(os.path.join(OUT, "names"), exist_ok=True)

    # locations.json — only places that actually have image observations
    loc_list = []
    for loc_id, obs in by_loc.items():
        name, lat, lng = locs[loc_id]
        loc_list.append({"id": int(loc_id), "name": name, "lat": lat, "lng": lng, "count": len(obs)})
    loc_list.sort(key=lambda d: d["id"])
    with open(os.path.join(OUT, "locations.json"), "w") as fh:
        json.dump(loc_list, fh, separators=(",", ":"))

    # observation buckets, keyed by location_id
    obs_buckets = [dict() for _ in range(OBS_BUCKETS)]
    for loc_id, obs in by_loc.items():
        obs_buckets[int(loc_id) % OBS_BUCKETS][str(loc_id)] = obs
    for b, bucket in enumerate(obs_buckets):
        with open(os.path.join(OUT, "obs", f"{b}.json"), "w") as fh:
            json.dump(bucket, fh, separators=(",", ":"))

    # species-node buckets, keyed by name_id: taxonomy + synonym group + description
    name_buckets = [dict() for _ in range(NAME_BUCKETS)]
    for nid in used_names:
        rank, syn, dep = name_meta.get(nid, (None, None, False))
        node = {"n": names.get(nid, "Unknown species")}
        if rank is not None:
            node["rank"] = rank
        if syn is not None:
            node["syn"] = syn      # synonym group id -> links synonymous species
        if dep:
            node["dep"] = 1        # deprecated name
        if nid in tax:
            node["tax"] = tax[nid]  # [kingdom, phylum, class, order, family]
        if nid in desc:
            node["desc"] = desc[nid]
        name_buckets[int(nid) % NAME_BUCKETS][str(nid)] = node
    for b, bucket in enumerate(name_buckets):
        with open(os.path.join(OUT, "names", f"{b}.json"), "w") as fh:
            json.dump(bucket, fh, separators=(",", ":"))

    meta = {
        "observations": n_obs,
        "locations": len(loc_list),
        "species": len(used_names),
        "classified": len(tax),
        "described": len(desc),
        "obsBuckets": OBS_BUCKETS,
        "nameBuckets": NAME_BUCKETS,
        "generatedAt": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
    }
    with open(os.path.join(OUT, "meta.json"), "w") as fh:
        json.dump(meta, fh, indent=2)

    total = sum(
        os.path.getsize(os.path.join(dp, fn))
        for dp, _, fns in os.walk(OUT) for fn in fns
    )
    print(f"\nDone. {meta['observations']:,} obs · {meta['locations']:,} sites · "
          f"{meta['species']:,} species ({meta['classified']:,} classified) · "
          f"{meta['obsBuckets']}+{meta['nameBuckets']} bucket files · "
          f"{total//1048576} MB total in public/data/")


if __name__ == "__main__":
    sys.exit(main())
