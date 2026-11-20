#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath -m "${1:?Usage: run_suffix_swap.sh OUT GEN VIS [CPU]}")
gen=$(realpath "${2:?official gen required}")
vis=$(realpath "${3:?official vis required}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC050_LOCAL_SWAP problems/ahc050/main.cpp -o "$out/local"
g++ -std=c++23 -O3 -DAHC050_SUFFIX_SWAP problems/ahc050/main.cpp -o "$out/suffix"
python3 scripts/bundle.py problems/ahc050/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC050_SUFFIX_SWAP "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O3 problems/ahc050/suffix_swap_test.cpp -o "$out/suffix-test"
"$out/suffix-test"
python3 problems/ahc050/replay_test.py
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in adjoint-dev adjoint-holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc050/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc050/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/local" "$out/suffix"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/adjoint-holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc050/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$vis" "$out/suffix" "$out/bundled"
