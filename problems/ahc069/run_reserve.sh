#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_reserve.sh OUT_DIR GEN TESTER [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
tester=$(realpath "${3:?Provide the full path to official tester}")
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
"${pin[@]}" g++ -std=c++23 -O3 problems/ahc069/main.cpp -o "$out/myopic"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC069_RESERVE_RELEASES problems/ahc069/main.cpp -o "$out/access"
python3 scripts/bundle.py problems/ahc069/main.cpp -o "$out/submission.cpp"
"${pin[@]}" g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc069/reserve-$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc069/benchmark.py "$out/$suite/in" "$out/$suite-results" "$tester" "$out/myopic" "$out/access"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc069/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" "$tester" "$out/myopic" "$out/bundled"
python3 - "$out/bundle-smoke/results" <<'PY'
import json
from pathlib import Path
import sys
p=Path(sys.argv[1])
assert json.loads((p/'0000-control.json').read_text())['protocol']==json.loads((p/'0000-candidate.json').read_text())['protocol']
PY

"${pin[@]}" g++ -std=c++23 -O3 -DAHC069_NO_RESERVE problems/ahc069/main.cpp -o "$out/incumbent"
for suite in dev holdout confirm; do
    if [[ $suite == confirm ]]; then
        mkdir -p "$out/$suite"
        (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc069/reserve-$suite-seeds.txt")
    fi
    "${pin[@]}" python3 problems/ahc069/benchmark.py "$out/$suite/in" "$out/$suite-original" "$tester" "$out/incumbent" "$out/myopic"
done
