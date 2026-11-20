#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_prune.sh OUT_DIR GEN VIS [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
vis=$(realpath "${3:?Provide the full path to official vis}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC021_BEAM_ONLY problems/ahc021/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 problems/ahc021/main.cpp -o "$out/prune"
python3 scripts/bundle.py problems/ahc021/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O2 tests/swap_suffix_pruning_test.cpp -o "$out/prune-test"
"$out/prune-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in prune-dev prune-holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc021/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc021/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/baseline" "$out/prune" --candidate-label prune
done
