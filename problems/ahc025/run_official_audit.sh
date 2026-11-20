#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath -m "${1:?Usage: run_official_audit.sh OUT GEN TESTER SEEDS INPUT_CSV RESULTS_CSV [CPU] [FROZEN_REF]}")
gen=$(realpath "${2:?official gen required}")
tester=$(realpath "${3:?official tester required}")
seeds=$(realpath "${4:?official seeds required}")
inputs=$(realpath "${5:?official input.csv required}")
history=$(realpath "${6:?official result.csv required}")
cd "$root"
if [[ ${8-} ]]; then
    ref=$(git rev-parse "${8}^{commit}")
elif git rev-parse --verify --quiet 'c8c587f292af7ba03ef6a2b94e95be5f56a7e8a0^{commit}' >/dev/null; then
    ref=$(git rev-parse 'c8c587f292af7ba03ef6a2b94e95be5f56a7e8a0^{commit}')
else
    ref=$(git rev-parse '21034a64^{commit}')
fi
pin=()
if [[ ${7-} ]]; then pin=(taskset -c "$7"); fi
mkdir -p "$out/cases"
exec 9>"$out/audit.lock"
flock -n 9
src=$(mktemp -d "$out/source.XXXXXXXX")
trap 'rm -rf -- "$src"' EXIT
git archive "$ref" | tar -x -C "$src"
cd "$src"
"${pin[@]}" g++ -std=c++23 -O3 problems/ahc025/main.cpp -o "$src/frozen-solver"
cd "$root"
if ! [[ -e "$out/cases/4999.txt" ]]; then
    "${pin[@]}" "$gen" "$seeds" -d "$out/cases"
fi
"${pin[@]}" python3 problems/ahc025/audit_official.py \
    "$out/cases" "$out/results" "$src/frozen-solver" "$tester" "$gen" \
    "$seeds" "$inputs" "$history" --source "$ref"
