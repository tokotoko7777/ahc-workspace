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
echo "24432b101519407fc0e5c2f92f3b939089115b79fd2e71ab3ee737ebc53e0601  $seeds" | sha256sum -c -
echo "43e9fd574575314ea7b3eea63ca23b32a15bf8a6872dc31625d6227097d0eb89  problems/ahc061/main.cpp" | sha256sum -c -
python3 scripts/bundle.py problems/ahc061/main.cpp -o "$out/submission.cpp"
echo "01b83165de719d86f8f8c3e996c1db52e02c6a415a3518fd8a09f15ad5047d4f  $out/submission.cpp" | sha256sum -c -
"${pin[@]}" g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/solver"
(cd "$out"; "${pin[@]}" "$gen" "$seeds")
"${pin[@]}" python3 problems/ahc061/audit_official.py "$seeds" "$metadata" "$result" "$out/in" "$tester" "$out/solver" "$out/audit"
