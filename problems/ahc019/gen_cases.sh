#!/usr/bin/env bash
set -euo pipefail
gen=$1
seeds=$2
out=$3
mkdir -p "$out"
tmp=$(mktemp -d "${out%/}/.gen.XXXXXX")
trap 'rm -rf "$tmp"' EXIT
for d in {5..14}; do
  awk -v dim="$d" '$1==dim {print $2}' "$seeds" > "$tmp/seeds.txt"
  if [[ ! -s "$tmp/seeds.txt" ]]; then continue; fi
  "$gen" --D "$d" --dir "$tmp/$d" "$tmp/seeds.txt"
  for p in "$tmp/$d/"*.txt; do cp "$p" "$out/$d-${p##*/}"; done
done
