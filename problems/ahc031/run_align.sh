#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath -m "${1:?Usage: run_align.sh OUT GEN VIS [CPU]}")
gen=$(realpath "${2:?official gen required}")
vis=$(realpath "${3:?official vis required}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC031_NO_ALIGNMENT problems/ahc031/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 problems/ahc031/main.cpp -o "$out/aligned"
python3 scripts/bundle.py problems/ahc031/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O3 tests/ordered_cut_placement_test.cpp -o "$out/placement-test"
"$out/placement-test"
python3 problems/ahc031/replay_test.py
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in align-dev align-holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc031/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc031/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/baseline" "$out/aligned"
    "${pin[@]}" python3 problems/ahc031/partition_components.py "$out/$suite/in" \
        "$out/baseline" "$out/aligned" | tee "$out/$suite-components.json"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/align-holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc031/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$vis" "$out/aligned" "$out/bundled"
