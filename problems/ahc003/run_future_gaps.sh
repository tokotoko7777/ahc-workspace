#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_future_gaps.sh OUT GEN TESTER [CPU]}")
gen=$(realpath "${2:?Official gen}")
tester=$(realpath "${3:?Official tester}")
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
git show 8dd3304396405987291ad576d88e1a6194725263:problems/ahc003/main.cpp > "$out/incumbent.cpp"
"${pin[@]}" g++ -std=c++23 -O3 "$out/incumbent.cpp" -o "$out/incumbent"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC_FUTURE_EXPERIMENT problems/ahc003/main.cpp -o "$out/local"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC_FUTURE_EXPERIMENT -DAHC_FUTURE_GAPS problems/ahc003/main.cpp -o "$out/future"
python3 scripts/bundle.py problems/ahc003/main.cpp -o "$out/submission.cpp"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC_FUTURE_EXPERIMENT -DAHC_FUTURE_GAPS "$out/submission.cpp" -o "$out/bundled"
"${pin[@]}" g++ -std=c++23 -O2 tests/ahc003_future_projection_test.cpp -o "$out/test"
"${pin[@]}" "$out/test"
evaluate() {
    local suite=$1
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc003/future-$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc003/benchmark_future.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/incumbent" "$out/local" "$out/future"
}
evaluate dev
evaluate holdout
if python3 - "$out" <<'PY'
import json
from pathlib import Path
import sys
d=Path(sys.argv[1])
a,b=[json.loads((d/f"{x}-results/summary.json").read_text()) for x in ("dev","holdout")]
def clears_gap(x):
    return x["against_incumbent"]["delta"]*1_000_000 >= x["incumbent"]["score"]*337
sys.exit(not (clears_gap(a) and clears_gap(b) and b["against_local"]["delta"]>0
              and b["against_incumbent"]["wins"]>=16))
PY
then
    echo 'Preregistered promotion gate passed; evaluating fresh confirmation only.'
    evaluate confirm
else
    echo 'Gate not met; no confirmation or official run.'
fi
mkdir -p "$out/bundle-smoke/in"
cp "$out/dev/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc003/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$tester" "$out/future" "$out/bundled"
cmp "$out/bundle-smoke/results/0000-baseline.routes" "$out/bundle-smoke/results/0000-probe.routes"
