#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.."; pwd)
out=$(realpath -m "${1:?Usage: run_official_audit.sh OUT GEN VIS SEEDS INPUT_CSV RESULTS_CSV [CPU] [FROZEN_REF]}")
gen=$(realpath "${2:?official gen required}")
vis=$(realpath "${3:?official vis required}")
seeds=$(realpath "${4:?official seeds required}")
inputs=$(realpath "${5:?official input.csv required}")
history=$(realpath "${6:?official result.csv required}")
cd "$root"
if [[ ${8-} ]]; then
    ref=$(git rev-parse "${8}^{commit}")
elif git rev-parse --verify --quiet '5728bc1b6baf715dc7e5a23d409f5cc58ba1136e^{commit}' >/dev/null; then
    ref=$(git rev-parse '5728bc1b6baf715dc7e5a23d409f5cc58ba1136e^{commit}')
else
    ref=$(git log -1 --format=%H -- problems/ahc036/main.cpp)
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
"${pin[@]}" g++ -std=c++23 -O3 problems/ahc036/main.cpp -o "$src/frozen-solver"
cd "$root"
if ! [[ -e "$out/cases/1999.txt" && -e "$out/generated-metadata.csv" ]]; then
    "${pin[@]}" "$gen" "$seeds" -v -d "$out/cases" > "$out/generated-metadata.csv"
fi
"${pin[@]}" python3 problems/ahc036/audit_official.py \
    "$out/cases" "$out/results" "$src/frozen-solver" "$vis" "$gen" \
    "$seeds" "$inputs" "$history" "$out/generated-metadata.csv" --source "$ref"
