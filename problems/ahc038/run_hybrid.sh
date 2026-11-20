#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath -m "${1:?Usage: run_hybrid.sh OUT GEN VIS [CPU]}")
gen=$(realpath "${2:?official gen required}")
vis=$(realpath "${3:?official vis required}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC038_WORK_CONTROL problems/ahc038/main.cpp -o "$out/control"
g++ -std=c++23 -O3 -DAHC038_HYBRID problems/ahc038/main.cpp -o "$out/hybrid"
python3 scripts/bundle.py problems/ahc038/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC038_HYBRID "$out/submission.cpp" -o "$out/bundled"
python3 problems/ahc038/replay_test.py
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in hybrid-dev hybrid-holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc038/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc038/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/control" "$out/hybrid"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/hybrid-holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc038/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$vis" "$out/hybrid" "$out/bundled"
