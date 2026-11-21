"""Build paired AHC008 score and capture breakdowns by visible input features."""
import argparse
import csv
import json
from pathlib import Path
import statistics


def features(p):
    lines = p.read_text().splitlines()
    pets = int(lines[0])
    humans = int(lines[pets + 1])
    dogs = sum(int(row.split()[2]) == 4 for row in lines[1:pets + 1])
    return {
        "all": "all",
        "humans": "5–6" if humans <= 6 else "7–8" if humans <= 8 else "9–10",
        "pets": "10–13" if pets <= 13 else "14–16" if pets <= 16 else "17–20",
        "dogs": "0–1" if dogs <= 1 else "2–3" if dogs <= 3 else "4+",
    }


def main():
    a = argparse.ArgumentParser(description=__doc__)
    a.add_argument("cases", type=Path)
    a.add_argument("results", type=Path)
    a.add_argument("control")
    a.add_argument("candidate")
    p = a.parse_args()
    bins = {}
    rows = {}
    with p.results.open(newline="") as f:
        for r in csv.DictReader(f):
            if r["model"] in (p.control, p.candidate):
                key = r["case"], r["model"]
                if key in rows:
                    raise ValueError(f"duplicate {key!r}")
                rows[key] = r
    if not rows or p.control == p.candidate:
        a.error("need paired rows for two distinct variants")
    for case in sorted({k[0] for k in rows}):
        before, after = rows[case, p.control], rows[case, p.candidate]
        for metric, bucket in features(p.cases / case).items():
            bins.setdefault(metric, {}).setdefault(bucket, []).append((before, after))
    report = {}
    for metric, groups in bins.items():
        report[metric] = {}
        for bucket, pairs in groups.items():
            rows = [[int(entry["score"]) for entry in pair] for pair in pairs]
            diffs = [new - prev for prev, new in rows]
            report[metric][bucket] = {
                "cases": len(pairs), "mean_before": statistics.mean(x[0] for x in rows),
                "mean_after": statistics.mean(x[1] for x in rows),
                "mean_delta": statistics.mean(diffs), "median_delta": statistics.median(diffs),
                "wins": sum(d > 0 for d in diffs), "losses": sum(d < 0 for d in diffs),
                "pet_free_before": sum(int(p[0]["all_humans_pet_free"]) for p in pairs),
                "pet_free_after": sum(int(p[1]["all_humans_pet_free"]) for p in pairs),
                "errors": sum(r["status"] != "ok" for p in pairs for r in p),
            }
    print(json.dumps(report, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
