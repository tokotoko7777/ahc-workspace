# Last-only variant of integrated article-inspired pilot (rejected)

User-authorized [siman](https://simanman.hatenablog.com/entry/2022/02/28/074838)
reports restricting special pursuit to the last animal rather than taking
over with two. The sole change vs integrated v2 is to enter the final
whole-crew planner at t>=180 only with exactly one currently human-reachable
animal, not one or two. No changes to construction, predictive paired
assignment, fallback or geometry. New external ledger columns for
score ceilings don't change any game actions.

Extract the standalone from v1, apply adjacent v2 handoff patch, then apply
`last-one-only.patch`. SHA256
`fc6e17073ca503d807be548737b56cc386d6f3b537e8537e544a71a896a970e1`;
`g++ -std=c++23 integrated.cpp -o integrated` and no args. All 60
official dev30 runs with independent recorded replay passed.

Common dev30 global-clear / integrated v2 / integrated v3:
score 746,886,882 / 834,282,356 / 753,886,522;
all-clear 18 / 13 / 11; residual 42 / 44 / 49; real exact4
212 / 236 / 235; distinct irregular4–48 53 / 60 / 58;
clean component union sum 5885 / 5478 / 4774.
At t225 residual 161 / 140 / 139 and at t275 111 / 77 / 78.
v3 max official tester wall 0.908s under sample workload. No promotion,
official holdout, or threshold sweep.
