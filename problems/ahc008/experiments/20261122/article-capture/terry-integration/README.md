# AHC008: TERRY article integration on the v7 implementation

Design source: [TERRY's AHC008 article](https://blog.terry-u16.net/entry/ahc008).
No competitive submission was read. The existing selected solver is
[exact v7](https://github.com/tokotoko7777/ahc-workspace/blob/b7bb3d1bcae4c2a22dab85ad6d061841569370e8/problems/ahc008/experiments/20261122/article-capture/solve-v7.cpp);
this source is a separate experimental policy, not a replacement.

| Design section | Adaptation in the C++23 solver |
|---|---|
| Physical layout | Five figure-derived landscape/aisle choices. There are different-width passages in the picture, so this version adapts them to the requested exact 1×4 bodies and two gates. 20 orientations certify exact connected bodies and an intact outside road network after all two-gate closures. |
| Construction allocation | Prepare a deterministic snake of wall orders. Split contiguous spans over workers with an exact worker-mask minimax DP, include reach/egress and avoid walking on the eventual wall mask. A blocked assigned job is skipped and can be handled by free-worker repair. |
| Arrival at the avenues | Threshold bipartite feasibility (equivalent augmenting-path max flow), costs equal remaining work plus real final-map route distance. Idle workers are handed to distinct avenues, without forced fixed pairs in hunting. |
| Rail capture | Per-case causal 64-trajectory Monte Carlo with reusable frozen-terrain dog BFS; cats use random two steps for this forecast as in the article. For each real pair arrival score expected legal capture count / (4 × (ETA+1)). Exact worker-mask packing excludes duplicate workers or tubes; current legal closure takes priority. |
| Connectivity and idle people | LowLink bridges over the reduced service region/tube multigraph plus simultaneous recheck. Spare people are assigned to distinct pet jobs by repository min-cost flow (including a dummy for those not assigned). |
| Late all-crew capture | Vertex-split minimum cut anchored to real avenues; targets and staged wall creation checked against all pets. Protect actual worker retreat paths and use lower-bound min-cost staffing so every boundary wall receives a distinct builder job or serial assignment. Escape-safe simultaneous final gates. Staff sacrifice is **not** a default action. |
| t275 switching | Fifty maximum paired common-random-number future trials (time-capped), own dog BFS and cat approximation; forecast both continuing rails and deploying the all-crew cut. The rail continuation in these inexpensive rollouts is a greedy stance-oriented proxy, not another full run of this program. A worker-safe successful cut needs an eligible feasible shape; incomplete pairs give no vote. |
| Map model | 7 inputs (M, N, types 1–5), Dense 7→32 plus three Dense 32→32 with ReLU, Dense 32→5 sigmoid. Labels are our own official generated inputs scored under every candidate. Pilot is 80 train +20 calibration, not the article's production data size. |

Single-file build: `g++ -std=c++23 integrated.cpp -o integrated`.
Optimizations are placed in the file. No command arguments are needed for
normal play; `--terry-force-map=0` through `=4` are for reproducible data
labels and print the active forced map to stderr. Set environment variable
`AHC008_TERRY_LOG` to a writable path to inspect chosen map, planned and actual
operations and decision timing without changing action output.

On the first integrated revision, each of the five forced map settings
produced exactly 300 legal moves under the official tester on official input
0000. This only checks integration and protocol, not score improvement or
completion of training. The label binary is a later frozen revision with
transitory handoff jobs not excluding the eligible pool, and to keep the
late cut search bounded ranks up to three candidates by real worker distances
rather than disabling the cut if more than three pets remain.

Five forced trials for every training and calibration input use separate seed
ranges, exact binary identities, and no access to the practice/independent
comparison suites for training. The independent 100-case comparisons will
be run with frozen weight and policy source; document model uncertainty,
per-stage use and score with no historical placement claim.
