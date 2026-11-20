#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run.sh OUT_DIR GEN TESTER [CPU]}")
gen=$(realpath "${2:?Provide the full path to official gen}")
tester=$(realpath "${3:?Provide the full path to official tester}")
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
"${pin[@]}" g++ -std=c++23 -O3 -DAHC074_BIT_ONLY problems/ahc074/main.cpp -o "$out/myopic"
"${pin[@]}" g++ -std=c++23 -O3 problems/ahc074/main.cpp -o "$out/access"
python3 scripts/bundle.py problems/ahc074/main.cpp -o "$out/submission.cpp"
"${pin[@]}" g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/bundled"
for suite in dev holdout; do
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc074/$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc074/benchmark.py "$out/$suite/in" "$out/$suite-results" "$tester" "$out/myopic" "$out/access"
done
mkdir -p "$out/bundle-smoke/in"
cp "$out/holdout/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc074/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" "$tester" "$out/access" "$out/bundled"
python3 - "$out/bundle-smoke/results" <<'PY'
import json
from pathlib import Path
import sys
p=Path(sys.argv[1])
assert json.loads((p/'0000-control.json').read_text())['protocol']==json.loads((p/'0000-candidate.json').read_text())['protocol']
PY
