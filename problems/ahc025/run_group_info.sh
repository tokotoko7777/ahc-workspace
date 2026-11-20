#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_group_info.sh OUT_DIR GEN TESTER [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
tester=$(realpath "${3:?Provide the full path to official tester}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 problems/ahc025/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 -DAHC025_GROUP_INFORMATION problems/ahc025/main.cpp -o "$out/balance"
python3 scripts/bundle.py problems/ahc025/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC025_GROUP_INFORMATION "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O2 tests/budgeted_order_buckets_test.cpp -o "$out/bucket-test"
"$out/bucket-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc025/group-$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc025/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/baseline" "$out/balance"
done

"${pin[@]}" g++ -std=c++23 -O2 tests/ahc025_sign_scheduler_test.cpp -o "$out/sign-scheduler-test"
"${pin[@]}" "$out/sign-scheduler-test"
mkdir -p "$out/bundle-smoke/in"
cp "$out/holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc025/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$tester" "$out/balance" "$out/bundled"
"${pin[@]}" "$tester" "$out/balance" < "$out/bundle-smoke/in/0000.txt" > "$out/bundle-smoke/plain.out"
"${pin[@]}" "$tester" "$out/bundled" < "$out/bundle-smoke/in/0000.txt" > "$out/bundle-smoke/bundle.out"
cmp "$out/bundle-smoke/plain.out" "$out/bundle-smoke/bundle.out"
