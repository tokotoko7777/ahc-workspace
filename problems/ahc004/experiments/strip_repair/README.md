# Optional corrupted-strip rescue

Base: `178d2a0c5223db33ef7ac5f67a9eab92451cf91f`, the original
native + wild + disjoint + medium portfolio. The standalone
strip prototype was tested over `b4a6b5a`; this port retains the
original four routes and their deadlines. This is an experimental seed
arm, not a replacement for the incumbent or a tested ranking claim.

When a dense raw geometry scaffold is proven contradictory, some of
its rows/columns may have coherent but incorrect registrations. Erase
the entire one or two strongest suspect strips. Erasing even one
complete line gives every short read a trivial compatible placement,
so the search deliberately ignores those unanchored placements. It
instead counts each read once with its strongest remaining placement:
exact, one hole, or two holes, with at least three retained letters.
Ranking weights are 10/9/5, relative to the unedited scaffold, plus a
small penalty for the number of known cells erased. Scan every single
strip and each pair of top six singles; retain one best pair and the
top two singles. Palettes crossing an erased strip contribute a vote
only for letter/cell agreements across their equally strongest
placements. Try both the voted and unseeded version with the same
greedy ordering. The votes are heuristic, not logical deductions.

The stage runs after all original geometry/refills. It retains up to
two dense raw scaffolds in the original six seed-ingress sites. The
forced-letters routine is used solely to **prove contradiction in a
discarded copy**. No forced-only board can be adopted.
original seed and RNG are retained unless exact covered read count
strictly improves (at complete coverage, blanks break ties).
The stage is bounded at 128 ms, also cut off by 2.32 s (only starts before 2.28 s), with per-scaffold
ranking and per-proposal refill deadlines. The external annealer end
remains 2.78 s. A time-limited search starting from another seed, or
given a shorter residual budget, need not dominate the original
final annealer. Measure before considering adoption.

```sh
python3 scripts/bundle.py problems/ahc004/experiments/strip_repair/submission_entry.cpp -o candidate.cpp
python3 scripts/bundle.py problems/ahc004/experiments/medium_diverse/submission_entry.cpp -o control.cpp
g++ -std=c++23 candidate.cpp -o candidate
g++ -std=c++23 control.cpp -o control
g++ -std=c++23 tests/ahc004_strip_repair_test.cpp -o strip_unit
./strip_unit
python3 problems/ahc004/experiments/strip_repair/synthetic.py ./control ./candidate
```

48 planted fixtures reveal 81% of a freshly generated truth board and
overwrite every retained letter on one or two strips with a different
letter. On 192 random deletion-set comparisons the packed result
matches a literal toroidal text/blank scan. The correct deletion
appears on the shortlist in 24/24 single-strip and 24/24 double-strip
cases. The resulting seeded repair predicts 909 correct and 2 wrong
erased letters; none repeat its corrupted predecessor. These fixtures
are not a measurement on generated geometry. The separate paired
runner generates independent synthetic toroidal inputs and verifies
the actual solver outputs with literal matching and the rounded score
formula. No official cases were run.
