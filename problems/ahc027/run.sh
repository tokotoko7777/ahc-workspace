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
g++ -std=c++23 -O3 -DAHC027_TREE_ONLY problems/ahc027/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 -DAHC027_FIRST_SPLICES problems/ahc027/main.cpp -o "$out/splice"
python3 scripts/bundle.py problems/ahc027/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC027_FIRST_SPLICES "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O2 tests/cyclic_visit_energy_test.cpp -o "$out/energy-test"
"$out/energy-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc027/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc027/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/baseline" "$out/splice"
done
