#!/usr/bin/env bash
set -euo pipefail
gen=$1 seeds=$2 out=$3
mkdir -p "$out"
tmp=$(mktemp -d "${out%/}/.gen.XXXXXX")
trap 'rm -rf "$tmp"' EXIT
i=0
while read -r n d q seed; do
    printf '%s\n' "$seed" > "$tmp/seed.txt"
    "$gen" --N "$n" --D "$d" --Q "$q" --dir "$tmp" "$tmp/seed.txt"
    printf -v dest '%04d' "$i"
    mv "$tmp/0000.txt" "$out/$dest.txt"
    i=$((i+1))
done < "$seeds"
