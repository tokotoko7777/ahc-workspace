# AHC008 v25: preserve early seam area through revalidation

An early short seam in v25 is admitted only when its prospective human
side retains at least 300 cells and 75% of the current root. Once an early
seam crosses turn 205 its continuation normally reverts to the usual late
floor. This change preserves the earlier floor during revalidation and
proposed closure of early-born seams only. It does not alter selection
of ordinary new lines, physical movement, or the original rails.

Both versions GNU C++23. Paired fixed M5-dev30 on the exact previous v25
input set, 60 legal official and independently played back.
Score 536,821,892 → 531,009,393, raw whole 11 → 10,
whole with area ≥300 stays 4, final residual 79 → 76. Five changed
action histories, max local tester 0.621 s. Rejected; do not tune on
a holdout or move baseline. Scope remains v25 research branch.

Regenerate from the exact [v25 source](../../v25-early-seam/README.md) with
`python make.py /path/to/solve-v25.cpp solve.cpp`, check hash and compile
`g++ -std=gnu++23 solve.cpp -o solve`. Same paired bench harness as v25.
