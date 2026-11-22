# One-empty-tube bridge: offline route and coverage audit

Read-only exact version v4. Replay used dev30 with actual per-case topology
from `AHC008_BANK_LOG` and exact active phase ownership from the paired
ledger. All 30 final action hashes and official/independent scores are
byte-for-byte matches. No changes to interactive actions.

In an active bank phase, normal worker paths exclude every currently
prepared tube body, while the one-bridge alternate can traverse all
seven edges of one different currently empty, actually prepared tube.
The pure exterior prefix and suffix never traverse the target tube.
Bridge gates must stay open until exit, and already-sealed candidate
doors and wall/move conflicts are unavailable. Once the guards arrive at
paired stances, apply factual observed future exact legality only as a
geometric ceiling. Use one canonical current shortest path per end and
one crossing actor per pair; no assertion of all dynamic alternate paths
or changing dog/cat motion. Normal walls displaced by both hypothetical
actors are separately counted; the strict category preserves every
ordinary physical build. Future predicted policy information is not
fed back into the actual solver.

Local proactive-lane supply measure: 3,497 distinct case-turn-lane states
in all30 where a presently nominally empty (no person exactly on that
aisle coordinate) lane can gain a genuinely shorter open route to a
bridge exit and has at least one other already-open prepared door within
three stance cells of exit. Median arrival8, mean nearby doors2.15.
1,267 such states spanning all30 also have true legal observed future
opportunities among those nearby doors within 16 turns of the arrival.
This is opportunity supply, not a guarantee that both actors would
align or that the future path after intervening actions is unchanged.

Direct t+7..10 closure route witnesses: 99 dispatch+finish pairs,
73 distinct observed legal tube-window events, 19 cases, four otherwise
final-missed animal IDs. There are 62 witnesses that preserve all
ordinary wall work; only three unique otherwise-final-missed animals
across three cases among them. No chase variant. Persistent relocation
to balance a genuinely empty aisle is a separate possible action,
not answered by this short-window upper-bound diagnostic.

`python bridge.py HARNESS TESTER V4 DEV30 V4_PAIRED_DEV.csv
friends.jsonl supply.jsonl summary.json`. Keep official reference
cohort and parser helpers unchanged.

## Strict proactive placement supply

Independently instrumented the exact `usedPeople` reservation immediately
before ordinary idle parking on v4. The actual observed worker is currently
outside and already on that tube's entrance stance, is not assigned a pair
(including holds) or brace job, other source-lane worker already present and
destination lane presently nominally empty, both tube ends physically open,
all four body + two gate cells animal/human-free, and destination has
at least one other genuinely prepared open stance within three cells.

All30 official action hashes/scores reproduce. In t101–152 actual full
scheduling there are755 actor candidates in18 cases. Requiring every
planned brace row already completely installed gives561 actor candidates
at276 unique case-turns in9 cases. 182 of those561 would replace the
idle-parking move (not an ordinary paid pair or brace). An additional
hindsight screen preserving future ordinary builds, gate route open and
donor-lane support throughout all seven crossing moves leaves537 candidates
in the same9 cases, including166 replaceable idle parking moves.
No live actions or future-assisted eligibility. This documents supply for a
possible proactive lane relocation, distinct from short-window chasing.

Generate `python instrument_idle.py V4.cpp audit.cpp`, compile no-args
`g++ -std=c++23 audit.cpp -o audit`, then
`python check_idle.py HARNESS THIS_DIRECTORY TESTER audit DEV30
V4_PAIRED_DEV.csv idle.jsonl idle-summary.json`.
