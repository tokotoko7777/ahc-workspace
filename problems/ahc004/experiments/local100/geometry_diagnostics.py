#!/usr/bin/env python3
"""Extract gate/realization details from the already completed official-local run."""
import csv
import re
import sys
from pathlib import Path


def main():
    inputs, run, diagnostics = map(Path, sys.argv[1:])
    with (run / "results.csv").open() as stream:
        scored = list(csv.DictReader(stream))
    scores = {(x["case"], x["solver"]): int(x["score"]) for x in scored}
    assert len(scored) == len(scores) == 200
    rows = []
    for i in range(100):
        case = f"{i:04d}"
        log = (run / "anchor_geometry" / f"{case}.log").read_text().splitlines()[0]
        fields = dict(re.findall(r"geometry_(\w+)=(-?\d+(?:\.\d+)?)", log))
        assert len(fields) == 19, fields
        name = case + ".txt"
        words = (inputs / name).read_text().split()[2:]
        length = (min(map(len, words)) + max(map(len, words))) // 2
        eligible = int(int(fields["cycles"]) >= 28
                       and 4 * int(fields["supported"]) >= 3 * int(fields["words"]))
        delta = scores[name, "anchor_geometry"] - scores[name, "anchor_control"]
        rows.append(dict(case=name, length_parameter=length, gate_eligible=eligible,
                         delta=delta, **fields))
    with diagnostics.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    print("eligible, searched, realized cases, realized layouts:",
          sum(x["gate_eligible"] for x in rows),
          sum(int(x["sweep_visits"]) > 0 for x in rows),
          sum(int(x["realized"]) > 0 for x in rows),
          sum(int(x["realized"]) for x in rows))
    groups = (
        ("selected", [x for x in rows if int(x["selected"])]),
        ("realized-not-selected",
         [x for x in rows if int(x["realized"]) and not int(x["selected"])]),
        ("not-realized", [x for x in rows if not int(x["realized"])]),
    )
    for name, group in groups:
        diffs = [x["delta"] for x in group]
        wtl = tuple(sum((x > 0) - (x < 0) == sign for x in diffs) for sign in (1, 0, -1))
        print(name, len(group), sum(diffs), wtl)


if __name__ == "__main__":
    main()
