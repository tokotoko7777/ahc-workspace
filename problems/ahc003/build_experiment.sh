#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
dir=${1:?Usage: build_experiment.sh OUT_DIR}
mkdir -p "$dir"
git show 8dd3304396405987291ad576d88e1a6194725263:problems/ahc003/main.cpp > "$dir/base.cpp"
g++ -std=c++23 -O3 "$dir/base.cpp" -o "$dir/baseline"
g++ -std=c++23 -O3 -DAHC_PROBE_DESIGN problems/ahc003/main.cpp -o "$dir/probe"
python3 scripts/bundle.py problems/ahc003/main.cpp -o "$dir/submission.cpp"
g++ -std=c++23 -O3 -DAHC_PROBE_DESIGN "$dir/submission.cpp" -o "$dir/bundled"
g++ -std=c++23 -O2 tests/gaussian_linear_information_test.cpp -o "$dir/test"
"$dir/test"
