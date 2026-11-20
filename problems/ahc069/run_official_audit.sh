#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=$(realpath -m "${1:?Usage: run_official_audit.sh OUT OFFICIAL_ZIP SEEDS INPUT_CSV RESULT_CSV [CPU]}")
zip=$(realpath "${2:?Official generator and tester zip}")
seeds=$(realpath "${3:?Published final seeds.txt}")
metadata=$(realpath "${4:?Official input.csv}")
result=$(realpath "${5:?Official result.csv}")
pin=()
if [[ ${6-} ]]; then pin=(taskset -c "$6"); fi
cd "$root"
mkdir -p "$out/official" "$out/metadata"
exec 9>"$out/experiment.lock"
flock -n 9
echo "4a602b749a31f570f797c4b454479c7b9a6e31b25714a83ef7b05cbb287c1069  $zip" | sha256sum -c -
echo "f63752a346a388e810574d22c9ee271f4f77d0b5f9e353084f2c335417c96482  $seeds" | sha256sum -c -
echo "caddaae040abb1ad9ff336f3dc02e7dc5b6b7b16316db667292314f4b5a5f9c7  problems/ahc069/main.cpp" | sha256sum -c -
unzip -oq "$zip" -d "$out/official"
unzip -oq "$zip" -d "$out/metadata"
python3 scripts/bundle.py problems/ahc069/main.cpp -o "$out/submission.cpp"
echo "3bce00c01c16967e0c1123e8fea7ace89fec428e16248c49d000142282f8fdc2  $out/submission.cpp" | sha256sum -c -
"${pin[@]}" g++ -std=c++23 -O3 "$out/submission.cpp" -o "$out/solver"
export CARGO_TARGET_DIR="$out/rust-stock"
(cd "$out/official/tools"; "${pin[@]}" cargo build --release --locked -j 1 --bin gen --bin tester)
gen="$CARGO_TARGET_DIR/release/gen"
tester="$CARGO_TARGET_DIR/release/tester"
"${pin[@]}" "$gen" "$seeds" --dir "$out/in"
python3 problems/ahc069/instrument_generator.py "$out/metadata/tools"
export CARGO_TARGET_DIR="$out/rust-meta"
gen="$CARGO_TARGET_DIR/release/gen"
(cd "$out/metadata/tools"; "${pin[@]}" cargo build --release --locked -j 1 --bin gen)
"${pin[@]}" "$gen" "$seeds" --dir "$out/metadata-in" 2> "$out/metadata.csv"
"${pin[@]}" python3 problems/ahc069/audit_official.py "$seeds" "$metadata" "$result" \
    "$out/in" "$out/metadata-in" "$out/metadata.csv" "$tester" "$out/solver" "$out/audit"
