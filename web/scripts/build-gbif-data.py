#!/usr/bin/env python3
"""
Build a local "file API" for the Fungi Soundscape web app from GBIF, the
Global Biodiversity Information Facility (https://www.gbif.org).

GBIF aggregates many sources — including the Mushroom Observer dataset — under a
single normalized taxonomic backbone. This script downloads occurrence records
via GBIF's *asynchronous Download API* (NOT the paged search API, which caps at
100k records and is rate-limited) and transforms them into the same compact,
graph-ready static JSON that scripts/build-data.py emits for Mushroom Observer —
so the two datasets sit side by side and can be compared in the app.

Download workflow (https://techdocs.gbif.org/en/data-use/api-downloads):
  1. POST a predicate to /occurrence/download/request  (HTTP Basic Auth).
  2. Poll GET /occurrence/download/{key} with exponential backoff until
     status == SUCCEEDED   (420 "Enhance Your Calm" -> back off).
  3. Download the archive zip from the returned downloadLink.
  4. Parse the Darwin Core Archive and emit JSON.

We request the DWCA format (not SIMPLE_CSV) because only DWCA carries media:
its multimedia.txt holds each record's image URL (identifier). For the Mushroom
Observer dataset those URLs point back to mushroomobserver.org images, so the
app keeps working exactly as before.

Credentials come from the environment — NEVER hardcoded:
  GBIF_USER  (your GBIF.org username, not email)   [alias: GBIF_USERNAME]
  GBIF_PWD   (your GBIF.org password)               [alias: GBIF_PASSWORD]
  GBIF_EMAIL (notification address)

Scope (env GBIF_SCOPE):
  mo    (default) — only the Mushroom Observer dataset as held by GBIF.
                    This is the "does GBIF really contain MO?" test.
  fungi          — every georeferenced, imaged kingdom-Fungi occurrence in GBIF
                    (much larger; this is the "what does GBIF add?" run).

Output (web/public/gbif/) mirrors the MO layout so the client can read either:
  meta.json        counts + the GBIF download key/DOI + scope + grid precision
  locations.json   [{id, name, lat, lng, count}]   -- grid-snapped site nodes
  obs/<0..N>.json  { "<location_id>": [[id, name, date, imageUrl, null, taxonKey], ...] }
  names/<0..M>.json { "<taxonKey>": {n, rank, syn, tax:[k,p,c,o,f], keys:[...], desc} }

Graph-ready, like the MO build, but richer: every taxon rank carries a stable
GBIF integer key (`keys`), so the taxonomy backbone is a real node graph rather
than just strings — ideal for the planned network visualizations.

Usage:
  GBIF_USER=... GBIF_PWD=... GBIF_EMAIL=... python3 scripts/build-gbif-data.py
  python3 scripts/build-gbif-data.py --dry-run   # print the request, contact nothing
"""

import base64
import csv
import io
import json
import os
import shutil
import sys
import time
import urllib.error
import urllib.request
import zipfile
from datetime import datetime, timezone

try:
    import resource  # Unix only; used to raise the open-file limit for buckets.
except ImportError:
    resource = None

csv.field_size_limit(1 << 24)

HERE = os.path.dirname(os.path.abspath(__file__))
WEB = os.path.dirname(HERE)
CACHE = os.path.join(WEB, ".data-cache", "gbif")
OUT = os.path.join(WEB, "public", "gbif")

# Keep in sync with src/api/localData.js (shared with the MO build).
OBS_BUCKETS = 256
NAME_BUCKETS = 64

API = "https://api.gbif.org/v1"

# Mushroom Observer, published to GBIF (type OCCURRENCE, "Mushroom Observer, Inc.").
MO_DATASET_KEY = "d714382d-5890-4234-ae81-696eeb53658a"
FUNGI_TAXON_KEY = 5  # GBIF backbone: kingdom Fungi

FORMAT = "DWCA"  # DWCA is the only format that carries media (image URLs).


# --- request shape --------------------------------------------------------

def build_predicate():
    """Records we can actually show: georeferenced, clean, with a still image."""
    scope = os.environ.get("GBIF_SCOPE", "mo").lower()
    preds = [
        {"type": "equals", "key": "HAS_COORDINATE", "value": "true"},
        {"type": "equals", "key": "HAS_GEOSPATIAL_ISSUE", "value": "false"},
        {"type": "equals", "key": "MEDIA_TYPE", "value": "StillImage"},
    ]
    if scope == "fungi":
        preds.append({"type": "equals", "key": "TAXON_KEY", "value": FUNGI_TAXON_KEY})
    else:
        scope = "mo"
        preds.append({"type": "equals", "key": "DATASET_KEY", "value": MO_DATASET_KEY})
    return scope, {"type": "and", "predicates": preds}


def build_request(creator, email, predicate):
    return {
        "creator": creator,
        "notificationAddresses": [email],
        "sendNotification": False,
        "format": FORMAT,
        "predicate": predicate,
    }


def load_dotenv():
    """Load KEY=VALUE lines from a gitignored web/.env (or ./.env) so credentials
    can live in a file instead of the shell. Real environment variables win, so
    `GBIF_USER=… npm run build:gbif` still overrides the file. No dependency."""
    seen = set()
    for path in (os.path.join(WEB, ".env"), os.path.join(os.getcwd(), ".env")):
        if path in seen or not os.path.exists(path):
            continue
        seen.add(path)
        for raw in open(path, encoding="utf-8"):
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            if line.startswith("export "):
                line = line[len("export "):]
            if "=" not in line:
                continue
            k, v = line.split("=", 1)
            k, v = k.strip(), v.strip().strip('"').strip("'")
            if k and k not in os.environ:
                os.environ[k] = v


def credentials():
    user = os.environ.get("GBIF_USER") or os.environ.get("GBIF_USERNAME")
    pwd = os.environ.get("GBIF_PWD") or os.environ.get("GBIF_PASSWORD")
    email = os.environ.get("GBIF_EMAIL")
    missing = [n for n, v in (("GBIF_USER", user), ("GBIF_PWD", pwd),
                              ("GBIF_EMAIL", email)) if not v]
    if missing:
        sys.exit(
            "Missing required env var(s): " + ", ".join(missing) + "\n\n"
            "GBIF downloads need a free GBIF.org account. Set, then re-run:\n"
            "  export GBIF_USER=your_gbif_username   # NOT your email\n"
            "  export GBIF_PWD=your_gbif_password\n"
            "  export GBIF_EMAIL=you@example.org\n\n"
            "Register at https://www.gbif.org/user/profile . "
            "Use --dry-run to preview the request without credentials."
        )
    return user, pwd, email


# --- GBIF API (urllib + basic auth, with backoff) -------------------------

def http(method, url, user, pwd, body=None, retries=6):
    data = json.dumps(body).encode() if body is not None else None
    delay = 5
    last = ""
    for _ in range(retries):
        req = urllib.request.Request(url, data=data, method=method)
        token = base64.b64encode(f"{user}:{pwd}".encode()).decode()
        req.add_header("Authorization", f"Basic {token}")
        req.add_header("Accept", "application/json")
        if body is not None:
            req.add_header("Content-Type", "application/json")
        try:
            with urllib.request.urlopen(req, timeout=120) as r:
                return r.status, r.read().decode("utf-8", "replace")
        except urllib.error.HTTPError as e:
            last = e.read().decode("utf-8", "replace")
            # 420 = "Enhance Your Calm" (too many downloads); 429/5xx = transient.
            if e.code in (420, 429) or 500 <= e.code < 600:
                print(f"    HTTP {e.code}; backing off {delay}s …")
                time.sleep(delay)
                delay = min(delay * 2, 120)
                continue
            if e.code == 401:
                sys.exit("GBIF rejected the credentials (HTTP 401). "
                         "Check GBIF_USER (username, not email) and GBIF_PWD.")
            sys.exit(f"GBIF {method} {url} -> HTTP {e.code}: {last[:500]}")
        except urllib.error.URLError as e:
            print(f"    network error ({e.reason}); retry in {delay}s …")
            time.sleep(delay)
            delay = min(delay * 2, 120)
    sys.exit(f"Gave up on {method} {url} after {retries} attempts. {last[:300]}")


def get_or_submit(user, pwd, email, scope, predicate):
    """Reuse a cached download key for an identical request; else submit one."""
    os.makedirs(CACHE, exist_ok=True)
    req_path = os.path.join(CACHE, "request.json")
    body = build_request(user, email, predicate)

    if os.path.exists(req_path):
        saved = json.load(open(req_path))
        if saved.get("predicate") == predicate and saved.get("format") == FORMAT:
            print(f"  reusing cached download key {saved['key']} ({scope} scope)")
            return saved["key"]

    print(f"  submitting download request ({scope} scope, {FORMAT}) …")
    status, text = http("POST", f"{API}/occurrence/download/request", user, pwd, body)
    key = text.strip().strip('"')
    if not key:
        sys.exit(f"GBIF returned no download key (HTTP {status}).")
    json.dump({"key": key, "format": FORMAT, "predicate": predicate},
              open(req_path, "w"))
    print(f"    download key: {key}")
    return key


def poll(user, pwd, key):
    """Poll until SUCCEEDED. Returns the download metadata (with downloadLink)."""
    timeout = int(os.environ.get("GBIF_TIMEOUT", "7200"))  # 2h default
    delay, waited = 5, 0
    while True:
        _, text = http("GET", f"{API}/occurrence/download/{key}", user, pwd)
        meta = json.loads(text)
        st = meta.get("status")
        if st == "SUCCEEDED":
            print(f"  SUCCEEDED: {meta.get('totalRecords', '?')} records, "
                  f"{(meta.get('size') or 0)//1048576} MB")
            return meta
        if st in ("KILLED", "FAILED", "CANCELLED"):
            sys.exit(f"GBIF download {key} ended as {st}.")
        print(f"  status={st} (records so far: {meta.get('totalRecords', '?')}); "
              f"waited {waited}s")
        if waited > timeout:
            sys.exit(f"Timed out after {waited}s. The download keeps running on "
                     f"GBIF; re-run later to resume from key {key}.")
        time.sleep(delay)
        waited += delay
        delay = min(delay * 2, 60)


def download_zip(meta, key):
    link = meta.get("downloadLink") or f"{API}/occurrence/download/request/{key}.zip"
    path = os.path.join(CACHE, f"{key}.zip")
    expected = meta.get("size") or 0  # bytes, from the SUCCEEDED metadata

    have = os.path.getsize(path) if os.path.exists(path) else 0
    if expected and have >= expected:
        print(f"  cached archive: {os.path.basename(path)} ({have//1048576} MB)")
        return path

    where = f"resuming from {have//1048576} MB" if have else "fresh"
    exp = f"{expected//1048576} MB" if expected else "unknown size"
    print(f"  downloading archive ({where}; expect {exp}) {link} …")
    import subprocess
    # Resilient transfer for the multi-GB archive:
    #   -C -            resume from whatever is already on disk (server supports ranges)
    #   --retry …       retry transient failures
    #   --speed-time/-limit  abort (then retry+resume) if it stalls under 10 KB/s
    #                        for 30s, so a dead socket can't hang forever
    subprocess.run([
        "curl", "-L", "--fail", "--retry", "20", "--retry-delay", "5",
        "--retry-all-errors", "-C", "-",
        "--speed-limit", "10240", "--speed-time", "30",
        "-o", path, link,
    ], check=True)

    size = os.path.getsize(path)
    if expected and size < expected:
        sys.exit(f"Archive still incomplete ({size//1048576} of {expected//1048576} "
                 f"MB). Re-run to resume from here.")
    print(f"    -> {size//1048576} MB")
    return path


# --- DWCA parsing + transform ---------------------------------------------

def _reader(zf, member):
    stream = io.TextIOWrapper(zf.open(member), encoding="utf-8",
                              errors="replace", newline="")
    rd = csv.reader(stream, delimiter="\t", quoting=csv.QUOTE_NONE)
    header = next(rd)
    return rd, {h: i for i, h in enumerate(header)}


def _g(row, idx, key):
    i = idx.get(key)
    if i is None or i >= len(row):
        return ""
    return row[i]


def _int(v):
    try:
        return int(v)
    except (TypeError, ValueError):
        return None


def _float(v):
    try:
        return float(v)
    except (TypeError, ValueError):
        return None


def _raise_fd_limit(want):
    """Best-effort raise of the open-file soft limit (we open OBS_BUCKETS files)."""
    if resource is None:
        return
    try:
        soft, hard = resource.getrlimit(resource.RLIMIT_NOFILE)
        if soft < want:
            resource.setrlimit(resource.RLIMIT_NOFILE,
                               (min(want, hard) if hard > 0 else want, hard))
    except (ValueError, OSError):
        pass


def transform(zip_path, scope, key, meta):
    print("  parsing Darwin Core Archive …")
    prec = int(os.environ.get("GBIF_GRID", "2"))  # coord decimals (~1 km @ 2)
    os.makedirs(os.path.join(OUT, "obs"), exist_ok=True)
    os.makedirs(os.path.join(OUT, "names"), exist_ok=True)

    # The fungi scope is millions of records — far too many obs tuples to hold
    # in RAM. Stream each obs edge to a per-bucket temp file (one open handle per
    # bucket), then regroup bucket-by-bucket when writing. Only the (smaller)
    # location and species node maps stay resident.
    _raise_fd_limit(OBS_BUCKETS + 64)
    tmpdir = os.path.join(CACHE, "tmp_obs")
    shutil.rmtree(tmpdir, ignore_errors=True)
    os.makedirs(tmpdir)
    bucket_fh = [open(os.path.join(tmpdir, f"{b}.jsonl"), "w", encoding="utf-8")
                 for b in range(OBS_BUCKETS)]

    cell_id = {}     # "lat,lng" -> synthetic location id
    loc_nodes = {}   # id -> {id,name,lat,lng,count}
    species = {}     # taxonKey -> species node
    n_obs = 0

    with zipfile.ZipFile(zip_path) as zf:
        names = set(zf.namelist())
        if "multimedia.txt" not in names:
            sys.exit("Archive has no multimedia.txt — re-request with format DWCA.")

        # 1) media: gbifID -> first usable still-image URL.
        images = {}
        rd, idx = _reader(zf, "multimedia.txt")
        for row in rd:
            gid = _g(row, idx, "gbifID")
            url = _g(row, idx, "identifier")
            typ = _g(row, idx, "type")
            if not gid or not url or gid in images:
                continue
            if typ and "image" not in typ.lower():
                continue
            images[gid] = url
        print(f"     {len(images):,} records with an image")

        # 2) occurrences: location nodes, species nodes, obs edges (streamed).
        rd, idx = _reader(zf, "occurrence.txt")
        for row in rd:
            gid = _g(row, idx, "gbifID")
            url = images.get(gid)
            if not url:
                continue
            lat = _float(_g(row, idx, "decimalLatitude"))
            lng = _float(_g(row, idx, "decimalLongitude"))
            if lat is None or lng is None:
                continue
            if not (-90 <= lat <= 90 and -180 <= lng <= 180):
                continue

            clat, clng = round(lat, prec), round(lng, prec)
            ckey = f"{clat},{clng}"
            lid = cell_id.get(ckey)
            if lid is None:
                lid = len(cell_id) + 1
                cell_id[ckey] = lid
                place = (_g(row, idx, "locality") or _g(row, idx, "stateProvince")
                         or _g(row, idx, "countryCode") or f"{clat}, {clng}")
                loc_nodes[lid] = {"id": lid, "name": place,
                                  "lat": clat, "lng": clng, "count": 0}
            loc_nodes[lid]["count"] += 1

            taxon_key = _int(_g(row, idx, "taxonKey"))
            name = (_g(row, idx, "species") or _g(row, idx, "scientificName")
                    or "Unknown species")
            tup = [_int(gid) or gid, name, _g(row, idx, "eventDate"),
                   url, None, taxon_key]
            bucket_fh[lid % OBS_BUCKETS].write(
                f"{lid}\t" + json.dumps(tup, separators=(",", ":")) + "\n")
            n_obs += 1

            if taxon_key is not None and taxon_key not in species:
                accepted = _int(_g(row, idx, "acceptedTaxonKey"))
                node = {
                    "n": name,
                    "rank": _g(row, idx, "taxonRank") or None,
                    "tax": [_g(row, idx, "kingdom"), _g(row, idx, "phylum"),
                            _g(row, idx, "class"), _g(row, idx, "order"),
                            _g(row, idx, "family")],
                    # Stable GBIF backbone keys per rank -> real graph node ids.
                    "keys": [_int(_g(row, idx, "kingdomKey")),
                             _int(_g(row, idx, "phylumKey")),
                             _int(_g(row, idx, "classKey")),
                             _int(_g(row, idx, "orderKey")),
                             _int(_g(row, idx, "familyKey")),
                             _int(_g(row, idx, "genusKey"))],
                }
                if accepted is not None and accepted != taxon_key:
                    node["syn"] = accepted  # this name resolves to `accepted`
                species[taxon_key] = node

    for fh in bucket_fh:
        fh.close()
    del images

    print(f"     {n_obs:,} observations · {len(loc_nodes):,} grid sites · "
          f"{len(species):,} taxa")
    write_output(scope, key, meta, prec, loc_nodes, tmpdir, species, n_obs)
    shutil.rmtree(tmpdir, ignore_errors=True)


def write_output(scope, key, meta, prec, loc_nodes, tmpdir, species, n_obs):
    print("  writing JSON …")

    loc_list = sorted(loc_nodes.values(), key=lambda d: d["id"])
    json.dump(loc_list, open(os.path.join(OUT, "locations.json"), "w"),
              separators=(",", ":"))

    # obs buckets: regroup each bucket's streamed lines by location id. Only one
    # bucket (~n_obs/OBS_BUCKETS rows) is resident at a time.
    for b in range(OBS_BUCKETS):
        grouped = {}
        with open(os.path.join(tmpdir, f"{b}.jsonl"), encoding="utf-8") as fh:
            for line in fh:
                lid, _, payload = line.partition("\t")
                grouped.setdefault(lid, []).append(json.loads(payload))
        json.dump(grouped, open(os.path.join(OUT, "obs", f"{b}.json"), "w"),
                  separators=(",", ":"))

    name_buckets = [dict() for _ in range(NAME_BUCKETS)]
    for tkey, node in species.items():
        name_buckets[tkey % NAME_BUCKETS][str(tkey)] = node
    for b, bucket in enumerate(name_buckets):
        json.dump(bucket, open(os.path.join(OUT, "names", f"{b}.json"), "w"),
                  separators=(",", ":"))

    out_meta = {
        "source": "gbif",
        "scope": scope,
        "datasetKey": MO_DATASET_KEY if scope == "mo" else None,
        "gbifDownloadKey": key,
        "doi": meta.get("doi"),
        "gridPrecision": prec,
        "observations": n_obs,
        "locations": len(loc_list),
        "species": len(species),
        "obsBuckets": OBS_BUCKETS,
        "nameBuckets": NAME_BUCKETS,
        "generatedAt": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
    }
    json.dump(out_meta, open(os.path.join(OUT, "meta.json"), "w"), indent=2)

    total = sum(os.path.getsize(os.path.join(dp, fn))
                for dp, _, fns in os.walk(OUT) for fn in fns)
    print(f"\nDone. {n_obs:,} obs · {len(loc_list):,} sites · {len(species):,} taxa "
          f"· {OBS_BUCKETS}+{NAME_BUCKETS} bucket files · {total//1048576} MB in "
          f"public/gbif/  (GBIF key {key}, DOI {out_meta['doi']})")


def main():
    load_dotenv()
    scope, predicate = build_predicate()

    if "--dry-run" in sys.argv:
        creator = (os.environ.get("GBIF_USER") or os.environ.get("GBIF_USERNAME")
                   or "<GBIF_USER>")
        email = os.environ.get("GBIF_EMAIL") or "<GBIF_EMAIL>"
        print(f"--dry-run · scope={scope} · format={FORMAT}")
        print(f"POST {API}/occurrence/download/request")
        print(json.dumps(build_request(creator, email, predicate), indent=2))
        print("\n(no request sent; set GBIF_* env vars and drop --dry-run to run)")
        return 0

    user, pwd, email = credentials()
    print(f"GBIF download · scope={scope} · format={FORMAT}")
    key = get_or_submit(user, pwd, email, scope, predicate)
    meta = poll(user, pwd, key)
    zip_path = download_zip(meta, key)
    transform(zip_path, scope, key, meta)
    return 0


if __name__ == "__main__":
    sys.exit(main())
