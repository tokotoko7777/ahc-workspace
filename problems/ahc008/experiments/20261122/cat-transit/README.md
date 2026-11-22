# AHC008: observed-only cat opportunities in small two-door corridors

**No solver promoted.** For this narrow mechanism, existing layouts almost never support staffed passages, and even a separate policy that actually built and staffed such passages did not see cat entry in its observed traces.

## Method

Replay own frozen tactical and capture4 solver actions on the same official-generator development 30. Separately replay existing dog-edge standalone capture proof on the first 30 of its distinct pilot seeds. No timed A/B, other contestants' code, target oracle, new solver, distributed practice cases or retrospective future-derived candidate selection. Official score matched independent replay for all 90 case/arm runs. Raw transcripts are not committed.

At each public turn-start, enumerate all straight, one-cell-wide spans with 3–12 interior tiles and two separately open end gates. Every lateral tile adjoining interior or gates must already be a wall or off-board. Exterior stances beyond the two gates must be open. Count geometry-turns, both stances occupied, and both stances reachable in <=1 move by two distinct humans with no interior human. Current exact closing legality requires both guards, no human in the interior or on either gate, and no animal on or adjacent to either gate. A legal window must have a live cat in the interior, not merely open legal gates.

The Bayesian filter uses only actual walls and observed first directed substeps (plus target reset if the target is touched by either substep). For an outside, still human-connected cat near a span (Manhattan within 5 of a stance and graph-distance <=6 to a gate), normalize remaining target mass including uniform reset over reachable cells. Count only targets outside the span on routes requiring the span interior: full-grid shortest distance must equal gate-to-gate route length, and deleting the interior strictly worsens the shortest distance. This excludes equal-length bypasses and goals within the span. Record event and distinct-cat flags at >=25/50/75% mass and whether a subsequent exact legal window on the **same span** appears. The one-move staff category is not a guarantee that a legal placement will exist after next pet motion.

The audit is conditional on frozen trajectories: no claim about a newly positioned corridor that changes the cat's path. The strict transit metric also deliberately omits non-straight cages or destinations within a pen.

## Result

| Policy and seed set | Distinct cats | Supported span-turns | Staffed | One-step staffed | Both gates legal | Cat inside / legal window |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| tactical / dev30 | 90 | 63 | 0 | 0 | 0 | 0 / 0 |
| capture4 / dev30 | 90 | 0 | 0 | 0 | 0 | 0 / 0 |
| constructed dog edge / distinct pilot30 | 90 | 2,922 | 670 | 2,046 | 288 | 0 / 0 |

Span-turn totals count overlapping spans/turns and are not independent trials. One tactical cat was already isolated by the baseline in an unrelated <=12-cell no-human component, and remained there at turn 300. Zero such cats for the other two arms. Cats already isolated in large refuges are **not** counted.

On tactical/capture4 there were zero distinct cats at >=25% conditional route mass (tactical maximum 2.91%). On physically constructed dog edge, 18 distinct cats came near; 14 had nonzero transit mass, **two** reached >=25% with one-step staff, and **one** exceeded 50% while staffed (maximum 56.88%). Neither gave a later same-span legal capture window; no cats entered **any** of these supports. This separates lack of geometry from low availability even with geometry, in these tested runs. No evidence here justifies building or staffing cat-specific edge traps.

## Reproduce

Build `audit.cpp` with GNU C++23. Use official generator v3 and the own baseline artifacts. Input seeds: `dev-seeds.txt`; dog edge uses first 30 of `../dog-transit/seeds.txt`. The analyzer receives only normal initial visible data plus alternating human/animal move lines. Example:

    g++ -std=c++23 audit.cpp -o audit
    python replay_audit.py "$TESTER" "$REPO/problems/ahc008/benchmark.py" "$DEV_INPUT" ./audit dev tactical="$TACTICAL" cap4="$CAP4"
    python replay_audit.py "$TESTER" "$REPO/problems/ahc008/benchmark.py" "$DOG_PILOT_INPUT" ./audit edge dog_edge="$DOG_STANDALONE"

This archive contains exact case/cat CSVs and the complete observed-only analyzer. Dog standalone source is `../dog-transit/standalone_bundled.cpp`; frozen capture4 source is `../dog-transit/cap4_original.cpp`. Tactical used the own `lock-tactical` artifact (SHA256 `90e0fc4fd6a39b9556630bfd88a87ebe1e0162d835dde8fa98f9e6b6cb039ae6`).

Tactical reproducibility: from the parent directory of a checkout named `ahc008_focus_solver`, compile `g++ -std=c++23 ahc008_focus_solver/problems/ahc008/tactical_gate_submission.cpp -o lock-tactical`. This reproduced the exact artifact SHA above in the measurement environment. The source is the unchanged repo file at `../../tactical_gate_submission.cpp` relative to this archive's parent `experiments/20261122` (or `problems/ahc008/tactical_gate_submission.cpp` from repo root).
