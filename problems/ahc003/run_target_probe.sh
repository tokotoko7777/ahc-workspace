#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_target_probe.sh OUT_DIR GEN TESTER [CPU]}")
gen=$(realpath "${2:?Pass absolute path to the official gen binary}")
tester=$(realpath "${3:?Pass absolute path to the official tester binary}")
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
git show 8dd3304396405987291ad576d88e1a6194725263:problems/ahc003/main.cpp > "$out/base.cpp"
g++ -std=c++23 -O3 "$out/base.cpp" -o "$out/baseline"
g++ -std=c++23 -O3 -DAHC_TARGET_PROBE problems/ahc003/main.cpp -o "$out/target"
python3 scripts/bundle.py problems/ahc003/main.cpp -o "$out/submission.cpp"
g++ -std=c++23 -O3 -DAHC_TARGET_PROBE "$out/submission.cpp" -o "$out/bundled"
g++ -std=c++23 -O2 tests/gaussian_linear_information_test.cpp -o "$out/test"
"$out/test"
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
for suite in target-dev target-confirm; do
  mkdir -p "$out/$suite"
  (cd "$out/$suite"; "$gen" "$root/problems/ahc003/$suite-seeds.txt")
  "${pin[@]}" python3 problems/ahc003/benchmark.py "$out/$suite/in" "$out/$suite-results" \
    "$tester" "$out/baseline" "$out/target"
done
