#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath -m "${1:?Usage: run_swap.sh OUT GEN TESTER [CPU]}")
gen=$(realpath "${2:?official gen required}")
tester=$(realpath "${3:?official tester required}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 problems/ahc045/main.cpp -o "$out/ordinal"
g++ -std=c++23 -O3 -DAHC045_POST_QUERY_SWAP problems/ahc045/main.cpp -o "$out/swap"
python3 scripts/bundle.py problems/ahc045/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC045_POST_QUERY_SWAP "$out/submission.cpp" -o "$out/bundled"
python3 problems/ahc045/replay_test.py
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in swap-dev swap-holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc045/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc045/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/ordinal" "$out/swap" --queries-only
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/swap-holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc045/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$tester" "$out/swap" "$out/bundled"
