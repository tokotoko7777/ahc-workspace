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
g++ -std=c++23 -O3 -DAHC025_RANK_ONLY problems/ahc025/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 problems/ahc025/main.cpp -o "$out/balance"
python3 scripts/bundle.py problems/ahc025/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O2 tests/budgeted_order_buckets_test.cpp -o "$out/bucket-test"
"$out/bucket-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc025/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc025/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/baseline" "$out/balance"
done
