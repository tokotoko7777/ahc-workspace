# v25-fast2 rolling construction packet prototype, M5 only
Pinned GNU C++23 standalone alternative. Existing complete two-ended 1×4 layout, walls, movement/closure legality, original deadlines/rescue/late branches all unchanged. Only the opening construction executor changes. Initial failure to find independently feasible two-bank packet schedules falls back to original constructor permanently; after existing constructor deadline the ordinary paths take over automatically.

Each bank partitions its 13 sorted rows into 3/3/3/4 cages. Physical shoulder rows are shared, and each packet owns every currently missing required wall at one canonical neighboring stance. A build already observed is no longer a job, including across packets. A packet can use one or two workers with a contiguous split of its alternating snake stance order; one live packet per bank, no overlapping assigned workers and at most four packet builders overall. On wave dispatch jointly prefer minimum maximum/sum of estimated whole-packet readiness plus two exterior midpoint stance access times, including each assigned worker's estimated egress from its final position. Once active, crew ownership remains until packet completion/deadline, but workers whose previous forecast pair is still geometrically usable, and workers needed for known simultaneous legal closures, yield construction and enter ordinary dynamic assignment. The pair is not hard held, and original current forecast may choose different plans. The egress estimate treats other current human positions as available; it does not model workload elsewhere, forthcoming forecast yields or future pet blocks. Blocked owned jobs are retained while later legal jobs in the same packet can proceed. A worker with no current route or currently legal job is not marked busy for ordinary guard dispatch.

Existing relay remains unchanged and still estimates from old FIFO cursor/jobs while packet ownership is enabled; both see all observed walls. Existing untasked repair still runs and can complete a packet's walls first. Packet scheduling cannot mathematically guarantee unchanged future staffed windows; measure actual first legal staffed guard, catches by t50/75/100/160, brace timing, final useful/full, score and legal runtime. No test-tape dependence.

    python ahc008_v25_packets/make.py ahc008_v25_fast_static/solve.cpp ahc008_v25_packets/solve.cpp
    cd ahc008_v25_packets
    g++ -std=c++23 solve.cpp -o solve
    g++ -std=c++23 unit.cpp -o unit && ./unit

AHC008_PACKETS_LOG emits mode/starts/completed/builds/walks/yields/no-dispatch at exit (proposed-action counts, not validated claims). Unit uses deterministic physical shared shoulder dedup and concurrent routed work checks. No official cases run by author.

## Verified development result: rejected

Paired serial official tester with independent score replay, same 30 historical
development inputs filtered to five humans. No failures in either arm. Source
SHA-256 `6e1b37737f597b173c49f69fabfd70af50a26ec228e0b4a642ed4b57f7a888a7`.
Evidence archive includes exact input/action hashes, per-case score, legality
checked by the harness, timing and observations. No held cases were opened and
the selected solver is unchanged.

| Metric | v25-fast2 | packets |
|---|---:|---:|
| Exact 4-cell captures by t75 | 3 | 17 |
| Brace-complete tubes across 30 cases at t75 | 739 | 486 |
| Exact 4-cell captures by t160 | 191 | 150 |
| Animals still accessible to humans at t160 | 254 | 295 |
| All captured | 11/30 | 3/30 |
| All captured and mean human area ≥300 | 4/30 | 1/30 |
| Official aggregate | 536,821,892 | 234,057,617 |
| Max local tester wall time | 0.575 s | 0.609 s |

Seven boards improved score and 23 worsened. There were 155 packet starts and
138 completions recorded by the constructor across the panel. Earlier pockets
alone did not translate to better actual staffing or total capture; the
candidate's shell coverage was already worse at t75. Multiple dispatch/queue
changes co-occur, so their individual effects are not identified. Keep the old
construction policy. Next inspect whether the current route can reach its same
next assigned stance sooner via temporarily open fence cells without
blocking another worker or stranding anyone.

For archive reconstruction, extract `evidence.tar.gz` from the repository
root into a directory containing the eight named files, regenerate the candidate
from the pinned v25-fast2, and run:

    export PYTHONPATH="$PWD/ahc008_integrated_v11:$PWD/ahc008_focus_solver/problems/ahc008/experiments/20261122:$PWD/ahc008_focus_solver/problems/ahc008:$PWD/ahc008_bridge_audit"
    python ahc008_v25_packets/bench.py ahc008_focus_solver/problems/ahc008 ahc008_focus_assets/tools/target/release/tester ahc008_v25_fast_static/solve ahc008_v25_packets/solve ahc008_v24_five_retention/m5-dev30/in ahc008_v25_packets/reproduced.csv
    python ahc008_v25_packets/summarize.py ahc008_v25_packets/reproduced.csv
