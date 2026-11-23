# Verification

GNU C++23 compiled test and example with `-Wall -Wextra -Wpedantic
-Werror`; test also separately with UBSan and
`-fno-sanitize-recover=all`. Normal and UBSan tests each passed 5,053
exhaustive and 8,000 fixed random graph cases, plus 12 boundary
scenarios: four extra brute comparisons, five rejected inputs and
three 16-worker/large-task assertions. The example returned worker
mask 1 and zero-based task IDs 0 and 2.

Only the portable independent library candidate was exercised. No
solver, native policy, official scoring, or performance comparison.
See README.md for semantics and intended use.
