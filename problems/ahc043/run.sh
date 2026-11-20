#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run.sh OUT_DIR GEN VIS [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
vis=$(realpath "${3:?Provide the full path to official vis}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC043_ONE_LINE problems/ahc043/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 problems/ahc043/main.cpp -o "$out/extensions"
python3 scripts/bundle.py problems/ahc043/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
python3 problems/ahc043/replay_test.py
g++ -std=c++23 -O3 tests/paired_coverage_reward_test.cpp -o "$out/coverage-test"
"$out/coverage-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc043/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc043/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/baseline" "$out/extensions"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc043/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$vis" "$out/extensions" "$out/bundled"
