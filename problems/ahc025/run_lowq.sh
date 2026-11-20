#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_lowq.sh OUT_DIR GEN TESTER [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
tester=$(realpath "${3:?Provide the full path to official tester}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 problems/ahc025/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 -DAHC025_RANK_FIRST_LOW problems/ahc025/main.cpp -o "$out/lowq"
g++ -std=c++23 -O3 -DAHC025_RANK_ONLY problems/ahc025/main.cpp -o "$out/rank-only"
python3 scripts/bundle.py problems/ahc025/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC025_RANK_FIRST_LOW "$out/submission.cpp" -o "$out/bundled"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in lowq-dev lowq-holdout; do
    "${pin[@]}" bash problems/ahc025/gen_strata.sh "$gen" \
        "problems/ahc025/$suite-seeds.txt" "$out/$suite/in"
    "${pin[@]}" python3 problems/ahc025/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/baseline" "$out/lowq" --candidate-label lowq --rank-only "$out/rank-only"
done
