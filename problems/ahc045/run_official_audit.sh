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
elif git rev-parse --verify --quiet '301bc7c8cdfa05763e724e57ed50965e81e8ca56^{commit}' >/dev/null; then
    ref=$(git rev-parse '301bc7c8cdfa05763e724e57ed50965e81e8ca56^{commit}')
else
    ref=$(git rev-parse 'ef908953cb79d12f7f5f71ef4aaef53d45fc4c04^{commit}')
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
"${pin[@]}" g++ -std=c++23 -O3 problems/ahc045/main.cpp -o "$src/frozen-solver"
cd "$root"
if ! [[ -e "$out/cases/2999.txt" && -e "$out/generated-metadata.csv" ]]; then
    "${pin[@]}" "$gen" "$seeds" -v -d "$out/cases" > "$out/generated-metadata.csv"
fi
"${pin[@]}" python3 problems/ahc045/audit_official.py \
    "$out/cases" "$out/results" "$src/frozen-solver" "$tester" "$gen" \
    "$seeds" "$inputs" "$history" "$out/generated-metadata.csv" --source "$ref"
