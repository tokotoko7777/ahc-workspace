# AHC008 wall-backed adaptive rails

Base is unchanged `mincut-speed.single.cpp` from the user's AHC008 work (snapshot copied as `base.cpp`). No official editorial, participant solution or submission code was consulted. Official v3 generator and tester evaluate SHA-256-derived local seeds under `develop/seeds` and `holdout/seeds`. These are neither distributed practice seeds nor system seeds. Tester time includes process launch and simulator CPU.

| Policy | Dev 48 | Fresh holdout 100 | Max holdout tester time |
| --- | ---: | ---: | ---: |
| mincut-speed | 664,463,564 | 1,306,453,362 | 0.414 s |
| v1 rectangular rails, two crews | 696,141,088 | not tested | — |
| v2 serialize construction, path avoiding rim and chamber, atomic final holes | 615,820,556 | not tested | — |
| v3 also reject backing holes with no exterior builder position | 735,117,204 | 1,283,929,421 | 0.414 s |
| v4 3–7-cell straight or once-bent anchored frontages, projected graph-based chamber | 739,152,414 | 1,217,649,287 | 0.496 s |
| v5 prior baseline actions first, idle crews only for bent frontages | 753,616,481 | 1,232,904,705 | 0.389 s |

Baseline and evaluated candidates emitted legal actions in every completed run. v1 planned 387 rectangles on dev48, but built 13 and registered just 4 captures. v2's outside-only construction often made holes against adjacent existing walls mathematically unreachable (no adjacent exterior stand). v3 fixed that: 17/34 chamber frontages fully ready, 3 direct closures on dev48, and 37/91 ready on holdout. The strong dev result arose from relatively few divergent trajectories. v4 had 613 chosen, 446 built, 215 closure proposals and 277 pets in the chamber at those proposals on holdout100. Closure telemetry is counted when proposed, before inherited cohesion repair, not just committed walls; do not equate the 277 with verified unique pets. Despite the plausible mechanism, v4 hired workers needed by the base strategy and may also have sacrificed too much area; the exact reason needs a final component-area and population analysis. Do not promote these versions off the dev win.

## Reproduce

Use the official v3 tools and archive from the repository root.
The archived programs compile without extra optimization flags.
The initial v1 and v2 were discarded prototypes; the later tested variants can be rebuilt:

    archive="$PWD/problems/ahc008/experiments/20261122"
    work=$(mktemp -d)
    g++ -std=gnu++23 "$archive/mincut-speed.single.cpp" -o "$work/base"
    g++ -std=gnu++23 "$archive/rails/v3.cpp" -o "$work/rail3"
    g++ -std=gnu++23 "$archive/rails/rail.cpp" -o "$work/bend"
    g++ -std=gnu++23 "$archive/rails/idle.cpp" -o "$work/idle"
    cd "$work"
    python "$archive/rails/run.py" "$tools/target/release" holdout 100 base > base-holdout.csv
    python "$archive/rails/run.py" "$tools/target/release" holdout 100 rail3 > rail3-holdout.csv
    python "$archive/rails/run.py" "$tools/target/release" holdout 100 bend 1 6 6 30 8 0 210 21 0 1 2 13 3 1 2 5 3 8 82 259 2 2 8 > bend-holdout.csv
    python "$archive/rails/run.py" "$tools/target/release" holdout 100 idle > idle-holdout.csv

Mask 8 selects bent frontages; the final four command-line args in v4 above are end-turn, open chamber limit, worker reservation maximum, and geometry mode. `rail4-dev.csv` is **not** an equivalent reproducible v4 benchmark: an old parser off-by-one accidentally gave the baseline flow logic `flowSpare=82`; `bend-dev.csv` uses corrected argument boundaries and default flowSpare 8.

v5 also failed holdout: -5.63%, 41W/2T/57L. Only 76/370 completed frontages and 11 proposed immediate closures. Idle workers aren't interchangeable across turns and their later availability and positions were often too inconsistent. Do not adopt. Keep `base.cpp` as reference.
