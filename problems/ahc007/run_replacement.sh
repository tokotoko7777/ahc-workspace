#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_replacement.sh OUT_DIR GEN VIS [CPU]}")
gen=$(realpath "${2:?Official gen}")
vis=$(realpath "${3:?Official vis}")
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
common=(-std=c++23 -O3 -DAHC_PAIRED_ROLLOUT -DAHC_MATCHED_REPLACEMENT)
"${pin[@]}" g++ -std=c++23 -O3 problems/ahc007/main.cpp -o "$out/incumbent"
"${pin[@]}" g++ "${common[@]}" problems/ahc007/main.cpp -o "$out/uniform"
"${pin[@]}" g++ "${common[@]}" -DAHC_USE_REPLACEMENT problems/ahc007/main.cpp -o "$out/replacement"
python3 scripts/bundle.py problems/ahc007/main.cpp -o "$out/submission.cpp"
"${pin[@]}" g++ "${common[@]}" -DAHC_USE_REPLACEMENT "$out/submission.cpp" -o "$out/bundled"
"${pin[@]}" g++ -std=c++23 -O2 tests/ahc007_replacement_release_test.cpp -o "$out/test"
"${pin[@]}" "$out/test"
for suite in replacement-dev replacement-holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc007/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc007/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/uniform" "$out/replacement"
    "${pin[@]}" python3 problems/ahc007/benchmark.py "$out/$suite/in" "$out/$suite-incumbent" \
        "$vis" "$out/incumbent" "$out/replacement"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/replacement-holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc007/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$vis" "$out/replacement" "$out/bundled"
python3 - "$out/bundle-smoke/results/summary.json" <<'PY'
import json,sys
assert json.load(open(sys.argv[1]))["identical_decisions"]==1
PY
