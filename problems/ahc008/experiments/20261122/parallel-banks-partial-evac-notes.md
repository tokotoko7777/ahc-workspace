# Exact no-sacrifice partial evacuation opportunity audit

Read-only on the 30 frozen narrow-rescue development executions.
All full action hashes and scores matched the frozen baseline and
passed official recorded playback and independent replay. Reuse the
previous evacuation observer with no bank, line or policy changes.

Screen previously selected late lines with one mobile person outside
the intended main and one or two gates unfinished. Candidate must leave
all ordinary commands intact except replacing idle builders with distinct
lawful same-turn gate placements from the main. No relocation. Check
all original pet-adjacency and human occupancy restrictions and conflict
with actual ordinary moves. Require an actually occupied sterile main
of >=300 cells, no loss of majority, and at least one actual pet
separated. A means every person is in an animal-free component
(including people in other connected components), and the exact
score at closure could be retained by emitting no more moves/walls.
B has a clean majority/main but still leaves at least one person with
animals, diagnostic only. Compare prospective all-human total to the
frozen actual final score; this comparison is retrospective, not a
future signal for a live policy.

* 210 qualifying observations in 27 cases.
* 151 fail exact gate legality or conflict with ordinary movement
  (84 pet proximity, 123 current human, 23 ordinary move; overlap).
* The other 59 fail sterile main size/occupancy: 24 mains below
  300 and 45 still contain pets (overlap). Largest sterile main 207.
* Zero A and zero B with the same main gate, so zero positive
  constructive cases. Not a claim for arbitrary topology or smaller
  refuges. No live run, promotion or main change.

Synthetic test first verifies an actually completable one-gate
A split with a 331-cell safe majority, a second safe occupied part,
and separate pet component. Adding one animal to the second part
classifies it B. The same fixture sanity-checks builder and gate IDs.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/evac-observer.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-evac-audit.patch"
g++ -std=c++23 /tmp/evac-observer.cpp -o /tmp/evac-observer
python "$D/parallel_banks_partial_evac_audit.py" --fixture
python "$D/parallel_banks_partial_evac_audit.py" "$HARNESS" "$TESTER" /tmp/evac-observer "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/partial.jsonl /tmp/summary.json
```
