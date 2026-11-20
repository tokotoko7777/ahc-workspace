#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_official_audit.sh OUT GEN TESTER SEEDS INPUT_CSV RESULT_CSV [CPU]}")
gen=$(realpath "${2:?Official gen}")
tester=$(realpath "${3:?Official tester}")
seeds=$(realpath "${4:?Published final seeds.txt}")
metadata=$(realpath "${5:?Official input.csv}")
result=$(realpath "${6:?Official result.csv}")
pin=()
if [[ ${7-} ]]; then pin=(taskset -c "$7"); fi
cd "$root"
mkdir -p "$out"
exec 9>"$out/experiment.lock"
flock -n 9
echo "daf8cb67f5deb3bcb35e3d34b97a3b7b7a3ee8da1fe1cbe24f39389ad64a6c85  $seeds" | sha256sum -c -
echo "6f3a90ab0ddf341bd5c17106cd914fbe46ae588ddd4c79ff0aa90558f356a830  problems/ahc063/main.cpp" | sha256sum -c -
python3 scripts/bundle.py problems/ahc063/main.cpp -o "$out/submission.cpp"
echo "eefb7845768a5a1ca23661318e0209e2bc5ffa8a66c4734ca1bfe96d9f67a552  $out/submission.cpp" | sha256sum -c -
"${pin[@]}" g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/solver"
(cd "$out"; "${pin[@]}" "$gen" "$seeds")
"${pin[@]}" python3 problems/ahc063/audit_official.py "$seeds" "$metadata" "$result" "$out/in" "$tester" "$out/solver" "$out/audit"
