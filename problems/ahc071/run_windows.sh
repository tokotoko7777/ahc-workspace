#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_windows.sh OUT_DIR GEN VIS [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
vis=$(realpath "${3:?Provide the full path to official vis}")
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
"${pin[@]}" g++ -std=c++23 -O3 problems/ahc071/main.cpp -o "$out/static"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC071_SWEEP_STAGGERED problems/ahc071/main.cpp -o "$out/moving"
"${pin[@]}" g++ -std=c++23 -O3 tests/ahc071_two_row_test.cpp -o "$out/test"
"${pin[@]}" "$out/test"
python3 scripts/bundle.py problems/ahc071/main.cpp -o "$out/submission.cpp"
"${pin[@]}" g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc071/window-$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc071/benchmark.py "$out/$suite/in" "$out/$suite-results" "$vis" "$out/static" "$out/moving"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc071/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" "$vis" "$out/static" "$out/bundled"
"${pin[@]}" "$out/static" < "$out/bundle-smoke/in/0000.txt" > "$out/moving.out"
"${pin[@]}" "$out/bundled" < "$out/bundle-smoke/in/0000.txt" > "$out/bundled.out"
cmp "$out/moving.out" "$out/bundled.out"

"${pin[@]}" g++ -std=c++23 -O3 -DAHC071_NO_WINDOWS problems/ahc071/main.cpp -o "$out/incumbent"
for suite in dev holdout; do
    "${pin[@]}" python3 problems/ahc071/benchmark.py "$out/$suite/in" "$out/$suite-incumbent" "$vis" "$out/incumbent" "$out/static"
done
