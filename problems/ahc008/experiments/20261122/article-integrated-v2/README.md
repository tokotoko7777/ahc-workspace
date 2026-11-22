# Integrated article-inspired v2, restored measured handoff

This is v1's identical predictive construction/hunter/cleanup bundle,
with one causal change: restore archived global-clear's actual rescue
handoff at t160 iff current aisle-connected residual >4, otherwise t225.
The live last-pet planner still has priority from t180 when exactly 1–2
animals actually remain human-reachable. v1 source and literature provenance
are in the adjacent archive.

Restore and compile GNU C++23 standalone:
```sh
gzip -dc ../article-integrated-v1/integrated.cpp.gz > integrated.cpp
patch integrated.cpp < handoff.patch
sha256sum integrated.cpp # 73f96e3af51d3bdba65165daf4685385f722d1b47e849275ebaf3978fcfb2dbd
g++ -std=c++23 integrated.cpp -o integrated
```
No arguments required, inherited optimization pragma. Same official
independently checked playback runner and old dev30.

Dev30 common latest global-clear→v2: 746,886,882→834,282,356 (18 wins,
12 losses); all-clear 18→13 (10 both, 8 lost, 3 gained); final residual
42→44; exact-four at capture and at end 212→236; nonrectangular 4–48
53→60; clean union5885→5478; t100/t160/t225/t275 residual
353/239/161/111→321/203/140/77; all60 legal and playback-matched, max wall
.442→.930 s. Exact-four first captures by species cow/pig/rabbit/dog/cat
22/41/38/71/40→27/31/35/79/64.

Final lastcap module owned 309 turns in 19 cases, exactly 3 isolates on
those turns. Final pet-only masks for those captures had 41, 35 and 6
cells and were all nonrectangular. Five of eight all-clear losses never
engaged lastcap; one cannot assign the reliability difference solely to
that module. In source, when the final distinct gate matching exists,
all remaining gates wait until the complete matching is both staffed and
legally buildable. Thus wall-preparation opportunities can be lost before
a legal last gap, but the present ledger has no per-turn action/abort
reason to establish that account for an individual trajectory.
