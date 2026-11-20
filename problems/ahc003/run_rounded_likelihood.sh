#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_rounded_likelihood.sh OUT GEN TESTER [CPU]}")
gen=$(realpath "${2:?Official gen}")
tester=$(realpath "${3:?Official tester}")
pin=()
if [[ ${4-} ]]; then pin=(taskset -c "$4"); fi
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
git show 8dd3304396405987291ad576d88e1a6194725263:problems/ahc003/main.cpp > "$out/base.cpp"
"${pin[@]}" g++ -std=c++23 -O3 "$out/base.cpp" -o "$out/baseline"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC_ROUNDED_LIKELIHOOD problems/ahc003/main.cpp -o "$out/rounded"
python3 scripts/bundle.py problems/ahc003/main.cpp -o "$out/submission.cpp"
"${pin[@]}" g++ -std=c++23 -O3 -DAHC_ROUNDED_LIKELIHOOD "$out/submission.cpp" -o "$out/bundled"
"${pin[@]}" g++ -std=c++23 -O3 tests/rounded_multiplicative_normal_test.cpp -o "$out/test"
"${pin[@]}" "$out/test"
evaluate() {
    local suite=$1
    mkdir -p "$out/$suite"
    (cd "$out/$suite"; "${pin[@]}" "$gen" "$root/problems/ahc003/rounded-$suite-seeds.txt")
    "${pin[@]}" python3 problems/ahc003/benchmark.py "$out/$suite/in" "$out/$suite-results" \
        "$tester" "$out/baseline" "$out/rounded"
}
evaluate dev
evaluate holdout
if python3 - "$out" <<'PY'
import csv
import json
from pathlib import Path
import sys
d=Path(sys.argv[1])
def clears_gap(s):
    p=d/f"{s}-results"
    x=json.loads((p/"summary.json").read_text())
    if any(r["valid"]!=r["cases"] for r in x.values()): return False
    b,n=x["baseline"]["total"],x["probe"]["total"]
    with (p/"results.csv").open() as f:
        csv_scores={(row["case"],row["solver"]):int(row["score"]) for row in csv.DictReader(f)}
    wins=sum(v>csv_scores[(t,"baseline")] for (t,k),v in csv_scores.items() if k=="probe")
    return (n-b)*1_000_000>=b*337 and wins*2>=x["baseline"]["cases"]
sys.exit(not(clears_gap("dev") and clears_gap("holdout")))
PY
then
    echo 'Preregistered promotion gate passed; evaluating disjoint confirmation only.'
    evaluate confirm
else
    echo 'Gate not met; no confirmation or official run.'
fi
mkdir -p "$out/bundle-smoke/in"
cp "$out/dev/in/0000.txt" "$out/bundle-smoke/in/0000.txt"
"${pin[@]}" python3 problems/ahc003/benchmark.py "$out/bundle-smoke/in" "$out/bundle-smoke/results" \
    "$tester" "$out/rounded" "$out/bundled"
cmp "$out/bundle-smoke/results/0000-baseline.routes" "$out/bundle-smoke/results/0000-probe.routes"
