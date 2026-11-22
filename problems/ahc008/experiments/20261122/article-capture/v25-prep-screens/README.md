# AHC008 v25: reject two supplemental geometries before live tests

The main solution still uses the existing two-ended four-cell rails. These
are two read-only replays on the same fixed M5-dev30 using selected v25.
Input/command hashes and official scores agree exactly for all 30 cases in
each audit, including independent playback. No live solver changes.

- The [perimeter bulkhead](bulkhead/README.md) would leave the three service
  aisle doors open until the final close, but across 2,460 eligible-time
  row states it required 17–19 incremental non-rail walls. Preregistered
  maximum eight never passed.
- For the [one-prep script](oneprep/audit.py), choose a spare non-rail
  build from an observed dot or a one-step approach at t160/180/200.
  Require prep connectivity of actual people, service and live rail
  stances, then one later gate creates a physically connected 4–64 cell
  enclosure and retains ≥300 human cells and ≥75% of current area.
  There were zero eligible topologies for either prep ETA, so zero
  observed visitors or staffed lawful closure scenarios. We did not
  implement recruitment or modify pet movement.

These failures describe the tested geometry and preserved infrastructure,
not a general impossibility. For one-prep, run `audit.py` from the same
workspace layout or inspect the archived gzip records; `summary.py`
reads decompressed JSON. See [pinned v25](../v25-early-seam/README.md)
for input and output verification.
