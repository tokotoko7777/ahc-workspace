#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run.sh OUT_DIR GEN TESTER [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
tester=$(realpath "${3:?Provide the full path to official tester}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 problems/ahc029/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 -DAHC029_MARKET_FORECAST problems/ahc029/main.cpp -o "$out/forecast"
python3 scripts/bundle.py problems/ahc029/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O3 -DAHC029_MARKET_FORECAST "$out/submission.cpp" -o "$out/forecast-bundled"
python3 problems/ahc029/replay_test.py
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc029/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc029/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/baseline" "$out/forecast"
done
