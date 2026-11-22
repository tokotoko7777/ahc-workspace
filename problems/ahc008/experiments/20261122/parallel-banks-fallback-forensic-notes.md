# Read-only old rescue comparative fallback traces: practice 0068 and 0089

Retrospective diagnosis only. These were selected from previously measured
score disagreements; not used for proposal selection, thresholds, script
conditioning, or any solver code. The old stay225, rescue160, and
unchanged tactical each match the six exact frozen action hashes and
scores, official v3 recorded playback, and independent game replay.
Coordinates, turns, and people/animal indices below are zero-based.
Walls are noted as `(row,column)`. Splits describe the true board
post-action and before animal response; terminal components use
the final board.

## 0089: a discarded empty pocket, then costly separations

3-bank rescue sends all five people into tactical fallback from a
744-cell region with seven animals at t160. It drives left with ULDLD,
LLDLD and begins placing (9,5), (13,1) at t162. Its first substantive
irreversible split is t172, wall pair (4,0), (5,1): 738 cells become
711 with **all** original animals/people, plus an empty 25-cell
component. Then t227 wall (0,16) removes 134 cells with three animals,
and t264 wall (19,23) removes 212 cells with two animals. In both cases
all five people retain their own original part. At t300 though, their
296-cell component still contains dog #1 and cat #7; score 8,222,222.
Stay225 leaves three animals but keeps all five people in 633 cells,
so scores 8,791,667. One fewer residual animal was not worth the
smaller territory in this case.

Unchanged tactical allocates the stubborn animals differently much
earlier. At t60, diagonal dog gate (23,6) isolates dog #1 and five
others with a single decoy, human #2, in the 78-cell part; the other
four people retain 809 cells. Tactical later gets cat #7 with two
others in the 127-cell split at t216, wall (28,29), and seals its last
two animals from those four people with a wall at (0,10) on t260,
leaving their 375 cells sterile. Ends at 33,360,417. Rescue did not
literally destroy a pre-existing 375-cell refuge in its own trace: it
never created that clean connected component.

## 0068: larger rescue separation did not solve the dog's location

3-bank rescue's t160 component is 739 cells with six animals and all
seven people; first left/inner walls at (12,10),(15,9) on t161. At t198
wall (0,6) separates 44 cells with rabbit #1. At t247 wall (10,9) cuts
208 cells holding pig #2, cow #3 and cat #4 off a 648-cell region; all
seven retain the 439-cell side with rabbit #0 and dog #8. At t300 they
have 422 cells plus dog #8, score 23,444,444 (stay225 leaves four
animals in 624 cells and ends 4,333,333). In contrast, unchanged
tactical t79 dog gate (7,8) already puts dog #8 with the decoy
human #1 and three other animals in a 120-cell region. At t196 wall
(10,29) cuts the final two free animals from the other six humans,
giving them a 486-cell sterile component, score 46,404,762.

These two paths support a bottleneck of which individual animals share
the big retained component, not just raw captures or final residual
count. Both early tactical decisions traded one person's access for
the others before the bank handoff even occurred. This does not establish
that reserving a decoy or changing thresholds will help broadly.
The archived traces let a reader examine precise actions, splits, pet
identities, and per-person metrics without inferring solver intent
from uncommitted target choices.

```sh
D=problems/ahc008/experiments/20261122
python "$D/parallel_banks_fallback_trace.py" "$HARNESS" "$TESTER" "$DUAL_BANKS" "$OLD_NOARGS_RESCUE" "$TACTICAL" "$OFFICIAL_PRACTICE" "$D/parallel-banks-rescue-practice100.csv.gz" /tmp/fallback.json 0068.txt 0089.txt
```
