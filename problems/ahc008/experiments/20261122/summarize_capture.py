"""Summarize capture first and clean-area thresholds from a benchmark CSV."""
import argparse
from collections import defaultdict
import csv
import json
from statistics import mean

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("csv")
    p.add_argument("--telemetry")
    a = p.parse_args()
    rows = defaultdict(list)
    for r in csv.DictReader(open(a.csv)):
        rows[r["model"]].append(r)
    component_count = {}
    if a.telemetry:
        for r in (json.loads(x) for x in open(a.telemetry)):
            if r["turn"] == 300:
                component_count[r["model"], r["case"]] = len(r["groups"])
    for model, rr in rows.items():
        legal = [r for r in rr if r["status"] == "ok"]
        data = []
        for r in legal:
            areas = list(map(int, r["human_areas"].split()))
            pets = list(map(int, r["human_reachable_pets"].split()))
            data.append((r["case"], areas, pets))
        clear = [(name, areas) for name, areas, pets in data if not any(pets)]
        occupied = [k for _, _, pets in data for k in pets]
        clear_area = [a for _, areas, pets in data for a, k in zip(areas, pets) if k == 0]
        different = sum(len(set(zip(areas, pets))) > 1 for _, areas, pets in data)
        summary = {
            "legal": len(legal), "all_humans_clear": len(clear),
            "clear_humans": len(clear_area), "people_observed": len(occupied),
            "mean_pets_in_human_component": mean(occupied) if occupied else 0,
            "mean_area_for_clear_humans": mean(clear_area) if clear_area else 0,
            "all_clear_min_area_at_least": {t: sum(min(areas) >= t for _, areas in clear) for t in (100, 150, 300, 450, 564)},
            "different_component_sizes_or_pet_counts": different,
        }
        if a.telemetry:
            summary["disconnected_human_case_count"] = sum(component_count[model, r["case"]] > 1 for r in legal)
        print(model, json.dumps(summary))

if __name__ == "__main__": main()
