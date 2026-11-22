# AHC008: replace tactical's dog stage only for exactly one dog

This is a precommitted replacement of tactical's old dog handling, not the rejected prefix before capture4 and not a second dog fence. The control is the unchanged standalone tactical solver. The variant switches only when exactly one dog is visible in the normal initial input. It runs the previously frozen up-to-75-turn four-cell two-door board-edge capture controller, disables tactical's large corner dog stage and human decoy, then continues normal tactical non-dog planning at the real global turn/state with no reserved human. On zero or multiple dogs it runs unchanged tactical commands. It does not infer or access the official seed, target or future moves.

The pen is the eight-wall dog-edge mechanism (six side-wall builds and two simultaneous gate builds) archived in sibling `dog-transit`. Both human stances are outside. Exact-four metrics use independently replayed final orthogonal connected components of area four with pets and no humans; residuals count distinct pets in any human-containing component (not a per-human repeated count). All-clear requires no pet in **any** human component and includes everyone, never an excluded decoy. Safe-area numerator is the sum, over all humans, of area of a pet-free component, zero for a human still with a pet.

## Development (enriched, not an overall score claim)

Fresh generator pool 450 seeds, Python Random(967840059629); selected in sequence solely by visible dog count, before scoring. First 36 with exactly one dog and first 18 with other counts. Both arms official tester and independent transcript score replay, no invalid outputs.

| Dev exact-one subset | Tactical | Replacement |
| --- | ---: | ---: |
| Official sum | 630,513,547 | 824,694,444 |
| Exact-four dogs at turn 300 | 0 | 36 |
| Residual animals [cow,pig,rabbit,dog,cat] | [25,22,16,34,13] | [2,1,3,0,2] |
| Exact-four animals [cow,pig,rabbit,dog,cat] | [1,1,1,0,0] | [3,2,2,36,0] |
| All human groups pet-free | 2/36 | 31/36 |
| Pet-free humans | 174/275 | 248/275 |
| Sum safe area across humans | 36,603 | 55,911 |
| Sum distinct safe area across cases | 5,309 | 6,983 |

23/0/13 score win/tie/loss; paired bootstrap aggregate gain about +3.7% to +67.2% (point +30.8%). All 36 dogs were captured by the prefix, and still in ≤4-cell exact group at finish (in fact exactly 4). Prefix total active 1,665 turns (mean 46.25, median 45, max 69), 288 wall actions, 6,493 walks and 5,991 waits counting every human's action. Over all 300 turns, total walk count fell 48,120→44,537, but waits rose 30,011→33,348 and builds 4,369→4,615. This bookkeeping identifies action use rather than assuming every added wait was a causal loss. Other-count dev controls matched action stream SHA256, score and replay metrics in all 18 cases.

## Reproduction

`tactical_original.cpp`, `transit.hpp`, `make.py`, `tactical_edge.cpp` are the frozen sources. Compile both with `g++ -std=c++23` and no extra flags. The bundled variant has no extra includes from local files. `validation.sha256` preserves freeze hashes. Use official AHC008 tools v3 with the untouched distributed package `in/` or generate independent `fresh100/in/` from `fresh100/seeds.txt` (first eight bytes little-endian of SHA256 of `ahc008/20261122/one-dog-tactical-replace/frozen100/{i}`).

`compare.py` is the dev runner. `compare_frozen.py` runs official tester directly, then independently via transparent protocol relay, asserts byte-identical tester output and replay score, and extracts terminal per-species topology/human area. It writes only per-case CSV and action digests, not raw transcripts. `summarize.py` aggregates overall/one/other metrics without enrichment on the frozen sets.

For a portable run, set `AHC008_TESTER` to the official tester executable and `AHC008_RELAY` to this repository's `problems/ahc008/benchmark.py`. Compile the two matching filenames `tactical_original` and `tactical_edge` in this directory, then run `python compare_frozen.py fresh100/in trial-fresh.csv` and `python compare_frozen.py /path/to/official/distributed/in trial-distributed.csv`. The local benchmark script asserts identical direct-vs-relay output; on a slow/loaded machine a CPU guard can affect action hashes, so compare measured runtime, failures, and traces before interpreting score.

## Frozen validation: unselected fresh100

The enriched development gain did not reproduce in normal score. On the untouched full fresh100, **1,859,040,695→1,839,946,910** (−1.03%). Eligible16: **356,121,564→337,027,779** (−5.36%, 8 wins and 8 losses); ineligible84 were byte-identical. No invalid outputs, no direct-vs-relay discrepancy, maximum official tester wall 0.467/0.505 s (control/variant).

The capture change did reproduce: exact-four dogs 0→16 (all 16 eligible dogs). All-human all-clear 3→13 across all100, 1→11 among the eligible16. Distinct eligible residual species [cow,pig,rabbit,dog,cat] **[10,9,15,14,6]→[2,1,3,0,6]**. Yet mean usable area across all eligible humans (zero for those still sharing with pets) was **191.73→151.99 cells**, a material area–exclusion tradeoff. This variant is not the normal-score leader on fresh data and was not merged.

## Frozen validation: official distributed100

With precisely the same frozen executable, full distributed score **1,512,932,629→1,556,844,226** (+2.90%). Eligible13: **258,505,070→302,416,667** (+16.98%, 9 wins and 4 losses); ineligible87 were byte-identical. All outputs valid with exact independent replay and matching direct output. Maximum official tester wall 0.439/0.448 s. Exact-four dogs 0→12 of 13; the missed dog in case 0097 reached the 75-turn cutoff with side walls complete and no gate closure.

All-human all-clear 10→20 overall, 0→10 among eligible. Eligible residual species [cow,pig,rabbit,dog,cat] **[11,10,7,13,6]→[0,1,0,1,3]**. Mean usable area across all eligible humans **171.10→193.87** on this set, opposite direction from fresh100. Overall individual pet-free counts were 419→418 on distributed and 504→503 fresh: many all-clear cases improved, but aggregate per-person clear count did not. This is why all-clear, exact-four captivity and safe area are recorded separately.

Across both unselected validation sets: **28/29** eligible dogs finished in four-cell captivity (compared to 0/29), **+20 all-clear cases**, **171/171** ineligible stdout streams matched byte-for-byte and **0** invalid case/arm executions out of 400. The score gain is not stable across sets, so retain as an exact-four mechanism alternative, **not** a new normal-score default. No changes to main, existing leading solver, or AtCoder submissions.
