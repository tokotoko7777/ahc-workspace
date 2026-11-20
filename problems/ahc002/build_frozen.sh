#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
work=${1:?Usage: build_frozen.sh OUT_DIR}
mkdir -p "$work"
base=8dd3304396405987291ad576d88e1a6194725263
git show "$base:problems/ahc002/main.cpp" > "$work/v10.cpp"
g++ -std=c++23 -O3 "$work/v10.cpp" -o "$work/v10"
g++ -std=c++23 -O3 -DAHC_SHORT_REPAIR problems/ahc002/main.cpp -o "$work/short-repair"
python3 scripts/bundle.py problems/ahc002/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC_SHORT_REPAIR "$work/submission.cpp" -o "$work/bundled"
g++ -std=c++23 -O2 tests/bounded_colored_path_test.cpp -o "$work/test"
"$work/test"
