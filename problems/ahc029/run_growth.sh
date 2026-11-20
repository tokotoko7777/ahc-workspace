#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_growth.sh OUT_DIR GEN TESTER [CPU]}")
gen=$(realpath "${2:?Provide the absolute path to official gen}")
tester=$(realpath "${3:?Provide the absolute path to official tester}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
source=problems/ahc029/main.cpp
suite_prefix=growth
if [[ ${EXPLORATORY-} == 1 ]]; then
    patch "$source" -o "$out/exploratory-main.cpp" < problems/ahc029/growth-exploratory.diff
    source="$out/exploratory-main.cpp"
    suite_prefix=growth-exploratory
fi
g++ -std=c++23 -O3 "$source" -o "$out/baseline"
g++ -std=c++23 -O3 -DAHC029_TURNOVER "$source" -o "$out/turnover"
g++ -std=c++23 -O3 -DAHC029_GROWTH_RESERVE "$source" -o "$out/reserve"
if [[ ${EXPLORATORY-} == 1 ]]; then
    cp "$source" "$out/submission.cpp" # snapshot has no local includes
else
    python3 scripts/bundle.py "$source" -o "$out/submission.cpp"
fi
g++ -std=c++23 -O3 -DAHC029_TURNOVER "$out/submission.cpp" -o "$out/turnover-bundle"
g++ -std=c++23 -O3 -DAHC029_GROWTH_RESERVE "$out/submission.cpp" -o "$out/reserve-bundle"
python3 problems/ahc029/replay_test.py
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in "$suite_prefix-dev" "$suite_prefix-holdout"; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc029/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc029/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/baseline" "$out/turnover" --candidate-label turnover \
        --secondary "$out/reserve" --secondary-label reserve
done
