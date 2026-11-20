#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath -m "${1:?Usage: run_reservoir.sh OUT GEN VIS [CPU]}")
gen=$(realpath "${2:?official gen required}")
vis=$(realpath "${3:?official vis required}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC048_2GRAM_POOL problems/ahc048/main.cpp -o "$out/old"
g++ -std=c++23 -O3 -DAHC048_RESERVOIR_ONLY problems/ahc048/main.cpp -o "$out/reservoir"
python3 scripts/bundle.py problems/ahc048/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC048_RESERVOIR_ONLY "$out/submission.cpp" -o "$out/bundled"
python3 problems/ahc048/replay_test.py
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in reservoir-dev reservoir-holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc048/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc048/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/old" "$out/reservoir"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/reservoir-holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc048/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$vis" "$out/reservoir" "$out/bundled"
