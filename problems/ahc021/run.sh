#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run.sh OUT_DIR GEN VIS [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
vis=$(realpath "${3:?Provide the full path to official vis}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
g++ -std=c++23 -O3 -DAHC021_BEAM_ONLY problems/ahc021/main.cpp -o "$out/reference"
g++ -std=c++23 -O3 problems/ahc021/two_ended.cpp -o "$out/two-ended"
python3 scripts/bundle.py problems/ahc021/main.cpp -o "$out/reference-submission.cpp"
python3 scripts/bundle.py problems/ahc021/two_ended.cpp -o "$out/candidate-submission.cpp"
g++ -std=c++23 -O3 -DAHC021_BEAM_ONLY "$out/reference-submission.cpp" -o "$out/reference-bundled"
g++ -std=c++23 -O3 "$out/candidate-submission.cpp" -o "$out/candidate-bundled"
g++ -std=c++23 -O2 tests/two_sided_dag_frontier_test.cpp -o "$out/frontier-test"
"$out/frontier-test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc021/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc021/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$vis" "$out/reference" "$out/two-ended"
done
