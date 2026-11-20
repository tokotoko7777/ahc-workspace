#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run.sh OUT_DIR GEN VIS [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
vis=$(realpath "${3:?Provide the full path to official vis}")
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
"${pin[@]}" g++ -std=c++23 -O3 -DAHC063_STATIC problems/ahc063/main.cpp -o "$out/static"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC063_NO_BRANCH problems/ahc063/main.cpp -o "$out/moving"
python3 scripts/bundle.py problems/ahc063/main.cpp -o "$out/submission.cpp"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC063_NO_BRANCH "$out/submission.cpp" -o "$out/bundled"
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc063/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc063/benchmark.py "$out/$suite/in" "$out/$suite-results" "$vis" "$out/static" "$out/moving"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc063/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" "$vis" "$out/moving" "$out/bundled"
"${pin[@]}" "$out/moving" < "$out/bundle-smoke/in/0000.txt" > "$out/moving.out"
"${pin[@]}" "$out/bundled" < "$out/bundle-smoke/in/0000.txt" > "$out/bundled.out"
cmp "$out/moving.out" "$out/bundled.out"
