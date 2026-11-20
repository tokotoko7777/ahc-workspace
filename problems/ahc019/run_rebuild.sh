#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_rebuild.sh OUT_DIR GEN VIS [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
vis=$(realpath "${3:?Provide the full path to official vis}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC019_FIRST_PASS problems/ahc019/main.cpp -o "$out/baseline"
g++ -std=c++23 -O3 problems/ahc019/main.cpp -o "$out/rebuild"
g++ -std=c++23 -O3 -DAHC019_EXTRA_RESTARTS problems/ahc019/main.cpp -o "$out/restarts"
python3 scripts/bundle.py problems/ahc019/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O2 problems/ahc019/rebuild_test.cpp -o "$out/rebuild-test"
"$out/rebuild-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in rebuild-dev rebuild-holdout; do
    "${pin[@]}" bash problems/ahc019/gen_cases.sh "$gen" \
        "problems/ahc019/$suite-seeds.txt" "$out/$suite/in"
    "${pin[@]}" python3 problems/ahc019/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/baseline" "$out/rebuild" --restarts "$out/restarts"
done
