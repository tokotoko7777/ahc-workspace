#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_flow.sh OUT_DIR GEN VIS [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
vis=$(realpath "${3:?Provide the full path to official vis}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 problems/ahc023/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 -DAHC023_SIBLING_FLOW problems/ahc023/main.cpp -o "$out/flow"
python3 scripts/bundle.py problems/ahc023/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC023_SIBLING_FLOW "$out/submission.cpp" -o "$out/bundled"
python3 problems/ahc023/phase_test.py
g++ -std=c++23 -O3 tests/parallel_interval_packing_test.cpp -o "$out/interval-test"
"$out/interval-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in flow-dev flow-holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc023/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc023/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/baseline" "$out/flow" --candidate-label flow
done
