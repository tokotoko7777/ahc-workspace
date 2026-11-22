# Selected lastcap shape ETA vs real certified exterior (read-only audit)

Take the exact v4 standalone and apply the diagnostic patch. GNU C++23
no arguments, same official v3 replay controls, same previously used dev30.
No policy, objective, route or eligibility changes.

```sh
gzip -dc ../article-integrated-v4/integrated.cpp.gz > integrated.cpp
patch integrated.cpp < instrumentation.patch
g++ -std=c++23 integrated.cpp -o audit
gzip -dc ../article-integrated-v4/dev30.csv.gz > reference.csv
python audit_shapes.py HARNESS TESTER audit DEV30 reference.csv events.jsonl summary.json
```

All 30 action hashes and official+independently verified scores exactly
match official dev v4. 114 selected attempts across 19 cases; 102 had at
most as many walls as workers when selected and all 102 had a complete
exterior distinct assignment. This is necessary by the current
certificate: all workers reach all exterior wall stances, occupation of
the same stance by different workers is permitted, and individual walls
are unique. Exact minmax ETA tracks the existing executor's optimum
bottleneck, with its travel-sum tie heuristic. Action histories log all
actual planner-owned turns and connect them to independently replayed
capture IDs. No downstream trace changes.

Among the 102, median nearest pre-cut ETA8, median nearest certified
exterior ETA12, median bottleneck distinct exterior ETA19.5, and median
actual old duration proxy11. Match ETA plus one exceeded duration proxy
by ≥5 on 67 attempts across 15 cases, including 19 attempts in five
cases eventually not all clear. Six exceeded remaining time until own
attempt deadline if requiring a single distinct full assignment,
including three attempts in two cases that ended with animals reachable.
Ten attempts yielded 11 real captures across 331 owned actions; among
successful K≤M attempts, median match ETA11.5 and five still had a
forecast gap ≥5. Thus a high ETA gap is a prioritization clue, not
proof of failure or a hard rejection rule. Partial building and a more
flexible multi-job assignment may finish faster than the single-pose
distinct bottleneck, and stochastic escape remains unmodeled.
