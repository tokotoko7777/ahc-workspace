# Collective orthogonal gate feasibility and shared staffing

This is an untimed, retrospective replay diagnostic, not an actionable solver or guarantee. Use the same 30 official-generated developmental cases documented in capture-first-notes.md. Re-run the frozen capture4 solver with the official tester, record commands, replay independently, and require identical official score. Sample its actual animal, human, and wall positions at turns 40,50,…,210.

At each sample, classify tiles containing an animal or adjacent to an animal on a portal as dirty. For uniform 2×3, 3×3 and 4×4 virtual tile arrangements, record largest connected clean union in the ideal tile graph, and actual currently connected safe cells on the board for candidates (maximal clean union, its single-tile deletions, and singletons). Build-only walls cannot be removed, including seams not matching the virtual geometry. Allow prospective new orthogonal border cells to close the region collectively, counting number missing. The staffing filter then requires legal-to-build boundaries in the observed state, an exact matching of one final target per human, and every human able to approach the retained region in 0–2 moves through current open cells. Preparatory pet motion is not simulated; for this reason, even passing the 2-step filter is not a forecast. Candidate enumeration is intentionally limited; these are conditional opportunity samples and not universal upper bounds across all policies.

| Layout | Mean best actual connected area ignoring staff | Cases at ≥300 area and ≤12 missing walls | Any area/staff 2 moves | Area≥150/staff 2 moves | Area≥300/staff 2 moves |
|---|---:|---:|---:|---:|---:|
| 2×3 | 155.5 | 0/30 | 0/30 | 0/30 | 0/30 |
| 3×3 | 189.0 | 0/30 | 0/30 | 0/30 | 0/30 |
| 4×4 | 305.9 | 14/30 | 12/30 | 3/30 | 1/30 |

Zero cases of any sampled layout reached 564 even in the geometry-only ideal. The coarser virtual layouts are evaluated on realized capture4 trajectories, not a simulation of different wider entrances. No coarse algorithm promoted.

With --guard-audit, keep only 4×4 and collect legal ≥150-cell collective gate sets served by two fixed exterior guard stances. The same fixed stance pair geometrically covers at least two distinct sets at sampled times in 20/30 cases (mean per-case maximum 5.47 unique sets). An optimistic oracle picks among the ten most reusable pairs, teleports any two humans into these fixed stances for a candidate, and checks actual-board exact ≤2-move matching and access for everyone else. Only 5/30 cases meet ≥150 and 3/30 meet ≥300 even then. The five respective best areas are 192, 461, 229, 317, and 329; these are retrospective oracle substitutions, not reachable achievements. This points beyond fixed pair staffing to formation/evacuation of the rest of the team.

Run for reproduction from repository root, passing the folder containing benchmark.py and benchmark_cases.py:

    python problems/ahc008/experiments/20261122/collective_cut_audit.py problems/ahc008 /path/to/tester ./capture4 /path/to/generated30
    python problems/ahc008/experiments/20261122/collective_cut_audit.py --guard-audit problems/ahc008 /path/to/tester ./capture4 /path/to/generated30

Source depends on guarded_entry_trace.py in the same directory. Both outputs are stored as CSVs. Keep frozen capture4 and score-leading tactical untouched; no distributed cases were used in the audit.
