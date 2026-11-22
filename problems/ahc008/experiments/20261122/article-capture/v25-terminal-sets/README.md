# Exact five-worker final wall-set enumeration

Pinned v25-fast2, unchanged through action t298. Applies only M=5 at t299 unless already globally clear. Run the old proposal, repair/late tactical and original terminal paths first; retain their actual final string on ties, bad input or idle budget. The new stage never alters a wall until after its final strict comparison and validation. Uses the same existing process CPU cutoff at 2.05 s, checked every 32 evaluated sets; any truncated search keeps its best proven candidate. At most 20 potentially legal cells and 21,699 nonempty candidate subsets of size at most five even before feasibility pruning. In typical cases feasible simultaneous sets are much fewer.

For a chosen sorted wall set, a bit over each of the 32 available worker subsets indicates whether the walls can be covered with distinct original-adjacent workers. Transition extends only with an unused real worker capable of building the pending cell. Unassignable prefixes can't extend. After finding strict improvement, reconstruct a builder matching once and verify its exact objective/legality independently. The set score is evaluated once per subset, not per matching. Other workers wait. With a legal old simultaneous command, an uppercase walk goes from one open cell to an adjacent open cell in the *same post-wall grid*; replacing that move by wait leaves component and all counts unchanged. This is why best build-only enumeration covers every legal terminal objective. Pets also cannot leave their post-wall components before scoring, regardless of their last move substeps. Exact objective sums area × people × 2^(all pets - component pets), so no floating rounding is used to decide strict improvement (a gain can round to the same official score). No teleportation or wall on original/after-move human or on/adjacent to an actual pet.

Reproduce:
    python ahc008_v25_terminal_sets/make.py ahc008_v25_fast_static/solve.cpp ahc008_v25_terminal_sets/solve.cpp
    cd ahc008_v25_terminal_sets
    g++ -std=c++23 solve.cpp -o solve
    g++ -std=c++23 unit.cpp -o unit && ./unit

Optional AHC008_TERMINAL_SETS_LOG records count / completed / changed; globally cleared skipped cases have all zero. The unit independently checks 2,000 random matching sets against brute, 120 random small boards against exhaustive original real lowercase/wait commands, and 9,600 mixed moves/builds against an independent set/queue oracle, verifying also walk replacement and candidate nonregression under early interruption. No official wall-time claims here; require exact prefix and official/replay legality in the scored run.

## Verified outcome

Same M5 development 30, fast2 vs expanded terminal. All60 legal official
and independently replayed. All30 prefixes through t298, every final
command and every score match. Candidate evaluated 5,276 distinct feasible
sets but did not improve any observed last action. Score 536,821,892 both
arms, full11/useful≥3004/residual79 unchanged. Worst local tester .626 s.
As preregistered no held panel was run, and v25-fast2 remains selected.
Proposal gains theoretical enumeration breadth, not an observed gain.

Source builder, unit and exact logs are in `evidence.tar.gz`. See
[pinned speed baseline](../v25-fast2/README.md) for the solver and official
tool setup.
