# One-line narrowing of integrated v7 at M=7 (rejected)

Use the existing v7 two-bank 2×13/three-lane construction for actual
M7, previously only M5/6 in v7. This replaces36 tubes/156 brace cells
with26/112, retaining all scheduling, bridges, observed legality,
handoff and cleanup settings. No change at any other M. Longer
uncovered periphery can cost late coverage despite easier staffing.

Apply `m7-two-banks.patch` to exact v7 standalone, GNU C++23 and no
args. Source SHA256
`22aba7365b6618f6f017a845b39685ac24ed49a33493d09a1cb6e8332e61735c`.
`g++ -std=c++23 integrated.cpp -o integrated`; included geometry fixture
checks M5–10 lane count, tube count and unique shoulders.

Use already-used earlier permanent-pairs cohort, 30 M7 cases
filtered by visible M only, plus eight inactive M5/8/9/10 identities
from that same source pool. Generate `selection-seeds.txt` via official
v3 gen; the paired CSV has exact input filenames and SHA after selection.
Run adjacent v9 `bench_v9.py` for real geometry/staging telemetry plus
official legal replay. All76 case-runs exact, eight inactive complete
hash identities. Not an unseen adoption holdout.

M7 v7→v10 earliest physical median: five ready 38→27.5,
first staffed65→50.5, first real exact4 81→63. Before state100
exact4 captures117→138 (cats15→29), total exact4 221→242. However
all animals isolated13→7 (5 new,11 lost); final residual40→51;
irregular4–48 93→72; score706,256,943→701,046,754. At state275
residual90→76, but last25 turns actual isolation50→25. First
<=2 cases25→23 (conditional median270→248), first exactly1
cases15→19 but by260 7→6. Sample max .923→.832s. No official
holdout, no promotion or geometry/phase tuning.
