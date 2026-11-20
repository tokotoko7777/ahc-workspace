#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath "${1:?Usage: run_greedy_control.sh OUT VIS [CPU]}")
vis=$(realpath "${2:?official vis required}")
cd "$root"
exec 9>"$out/greedy.lock"
flock -n 9
pin=()
if [[ ${3-} ]]; then pin=(taskset -c "$3"); fi
g++ -std=c++23 -O3 -DAHC038_SEARCH_TIME_MS=0.0 examples/search/ahc038_variable_cost_beam.cpp -o "$out/greedy-only"
for suite in dev holdout; do
    "${pin[@]}" python3 problems/ahc038/benchmark.py "$out/$suite/in" "$out/greedy-$suite-results" \
        "$vis" "$out/greedy-only" "$out/stations"
done
