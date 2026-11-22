# Exact posterior H1 cat motion for short bank forecasts (rejected)

Only live difference vs which v7 estimate is fed cat probabilities
to the existing tube pair ranking for ETA<=3. Condition the target-cell
posterior on actual cat steps on the posthuman graph through actual bank
handoff, use the posterior directed next step at the forecast's first
substep, then exactly as before uniform second and later substeps.
H>=4 forecast endpoints are old and bit-identical, other species and
all capture/handoff/final execution unchanged. Unlike isolation in
the prior prediction-only diagnostic, every cat's early distribution
is replaced, so the intended live forecast also updates other-cat
contributions to anticipated legal walls. No foreground rollout or
hidden RNG.

Apply `short-cat-posterior.patch` to archived v7, SHA256
`7a8c1b857e4d5d5e2d2f8387c73219772d71092df025f8d5615ce52f3176461b`.
`g++ -std=c++23 integrated.cpp -o integrated` no args. Fixture
`g++ -std=c++23 cat-unit.cpp -o unit && ./unit` checks artificial known
north target vs recent east heading, exact passive and cat h>=4
identity, and h1–3 probability conservation.

Old v7 dev30 paired official exact, all baseline traces match and
all60 independent playback/legality exact. All-clear 14→13 (6 gains,
7 losses); final residual49→42, but residual at100/160/225
321/202/132→316/212/150, exactly4 236→233, irregular63→68.
Before state100 actual captures114→119, all exactly4; cats31 unchanged.
First actual exactly1 cases18→21 yet median265→274, first<=2 cases
23→24 yet median248→254; each only one more reaching by260. Median
times are conditional on occurrence but the paired summary also reports
the full earlier/later/no-change. Score +0.49%, max tester .947→.991s
and median .637→.692s. No consistent earlier-isolation gain; v8 does not
pass the full-clear nonregression gate despite much better prediction,
no new holdout or tuning. V7 remains reference.
