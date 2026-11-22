# Offline scaffold-removal bounds on faithful TERRY final states
Only use after the existing official temporal benchmarks finish. This analysis never edits the faithful algorithm, and it never calls the judge. It deliberately deletes built walls on a frozen final graph; this is an illegal game operation and not evidence that earlier skipping gives the same animal motions, captures, guard assignments or score.

Use exact per-case official current commands and observed pet tapes, with physical .actions/.trace names. Existing practice ledger has hashes but not stored action transcript; optional materialize.py is a separate FUTURE root-run harness. Supply exactly the frozen 54f2b1ec standalone and practice ledger/files. The recorded paired rollout count can be set via --pairs from each ledger telemetry to improve exact rerun stability, yet prior score/action hashes must still match. If clock differences alter commands, the materializer records omission and never writes wrong tapes. Run materialization sequentially in an exclusive timing slot, never now.
    python materialize.py TESTER FROZEN-SOLVER PRACTICE-INPUT-DIR PRACTICE-LEDGER TAPES
    python audit.py PRACTICE-LEDGER PRACTICE-INPUT-DIR TAPES
Opt into --allow-missing only if omitted boards are explicitly reported. Audit independently verifies input SHA, complete action SHA, replay legality/score, fully separated status and installed X count. Never claim estimates from omitted or unmatched cases.

On frozen terminal cleared states, consider only currently installed cells in the semantic X mask. A single open-wall vertex joins only neighboring open components, so any singleton joining a pet-used and human-used component is provably necessary under all supersets of deletion. Thus opening every other X cell yields an area UPPER bound on safe subsets for this fixed graph/position (bound graph may itself reconnect pets). Separately, exact flood singles and maximal horizontal X-runs, including those touching exclusively preexisting human-used territory. A greedy monotone sequence of safe group/single removals yields a simultaneously verified feasible *final graph* lower bound on best removal. Both bounds are on mean per-human connected area; lost human components cannot accidentally hide reachable pets. Singles/group gains are not additive and no globally optimal removal set is claimed. Report when these candidate walls were originally installed, in particular by t100 and t160; this can inform a future causal construction test only if separately authorized.

## Results on all 100 frozen practice boards

Regenerated all 100 exact input/command hashes and verified official
playback and independent scores, no omissions. Final-graph omission
certificates analyze the 98 actual all-clear boards. Leaving a subset of
planned X cells open in the fixed final snapshot raises average human area
from **552.939 to 716.194** while keeping all pets disconnected, a verified
constructive lower bound on an otherwise fictional removal operation.
The relaxed upper bound is 717.000. Mean certified removed planned walls
per clear board: 171.714; 163.520 were originally built by t100, all by
t160. This scale does not imply that early construction is optional:
omission can change walkability, room definitions, guard routing and all
pet trajectories while playing. Do not call the modeled score delta a
real or possible performance result.

An actually legal zero-risk rule would stop all commands at the first full
isolation, because walls prevent pets in separate components crossing into
human components. The exact replay finds 0/98 clear boards with any later
walls and 0 score gain, so no live change is warranted on that evidence.
`first_full.py` reproduces this from the same tapes and prints per-case data.

Next distinguish fixed layout cells that do not support any designed room
from unused actual rooms needing their own complete barriers during play.
Keep the frozen faithful implementation unmodified.
