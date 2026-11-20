#!/usr/bin/env python3
"""Summarize completed AHC004 official local 0..99 benchmark result CSVs."""
import argparse
from collections import defaultdict
import csv
from pathlib import Path
import statistics


def main():
    p = argparse.ArgumentParser()
    p.add_argument("inputs", type=Path)
    p.add_argument("csvs", nargs="+", type=Path)
    p.add_argument("--control", default="incumbent")
    p.add_argument("--target", type=int, default=7_321_305_762)
    args = p.parse_args()
    cases = [f"{i:04d}.txt" for i in range(100)]
    if {p.name for p in args.inputs.glob("*.txt")} != set(cases):
        raise ValueError("official local input set must contain exactly 0000..0099")
    bands = {}
    for case in cases:
        words = (args.inputs / case).read_text().split()[2:]
        central = (min(map(len, words)) + max(map(len, words))) // 2
        bands[case] = "short 4–5" if central <= 5 else "medium 6–7" if central <= 7 else "long 8–10"
    data = defaultdict(dict)
    for path in args.csvs:
        with path.open(newline="") as f:
            for row in csv.DictReader(f):
                label, case = row["solver"], row["case"]
                if case in data[label]:
                    raise ValueError(f"duplicate {label}/{case}")
                data[label][case] = (int(row["score"]), float(row["elapsed_s"]))
    if args.control not in data:
        raise ValueError(f"missing control {args.control}")
    for label, rows in data.items():
        if set(rows) != set(cases):
            raise ValueError(f"incomplete {label}: {len(rows)} of 100")
    base = data[args.control]
    for name, rows in data.items():
        total = sum(s for s, _ in rows.values())
        elapsed = [t for _, t in rows.values()]
        diffs = {case: rows[case][0] - base[case][0] for case in cases}
        bandsum = {}
        for case, diff in diffs.items():
            bandsum[bands[case]] = bandsum.get(bands[case], 0) + diff
        splits = ", ".join(f"{band}: {v:+,}" for band, v in sorted(bandsum.items()))
        print(f"{name}: {total:,}; vs target {total - args.target:+,}; "
              f"vs {args.control} {sum(diffs.values()):+,} "
              f"({sum(d>0 for d in diffs.values())}W/{sum(d==0 for d in diffs.values())}T/"
              f"{sum(d<0 for d in diffs.values())}L), "
              f"wall median={statistics.median(elapsed):.6f}s max={max(elapsed):.6f}s; {splits}")


if __name__ == "__main__":
    main()
