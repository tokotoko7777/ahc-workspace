#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath -m "${1:?Usage: run.sh OUT GEN TESTER [CPU]}")
gen=$(realpath "${2:?official gen required}")
tester=$(realpath "${3:?official tester required}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC045_DISCOUNT_ONLY problems/ahc045/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 -DAHC045_ORDINAL problems/ahc045/main.cpp -o "$out/ordinal"
python3 scripts/bundle.py problems/ahc045/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC045_DISCOUNT_ONLY "$out/submission.cpp" -o "$out/baseline-bundled"
g++ -std=c++23 -O3 -DAHC045_ORDINAL "$out/submission.cpp" -o "$out/ordinal-bundled"
python3 problems/ahc045/replay_test.py
g++ -std=c++23 -O3 tests/bounded_order_projection_test.cpp -o "$out/projection-test"
"$out/projection-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc045/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc045/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/baseline" "$out/ordinal"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc045/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$tester" "$out/baseline-bundled" "$out/ordinal-bundled"
