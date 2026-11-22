# Multi-bank first wave build + opposing physical aisle egress (rejected)

Independent implementation of the construction-to-aisle assignment idea
from the authorized [TERRY description](https://blog.terry-u16.net/entry/ahc008).
Keep v7's exact board geometry. For each bank choose five contiguous
tubes at t0, split the six 4-cell shared shoulder rows into two 12-wall
routes, and assign separate crews ending at the two center aisle stances.
Each three paired U/D sweep route uses projected passable terrain and
takes real starting approach, wall commands and final egress into
account. Across all banks a disjoint worker-mask DP minimizes maximum
route completion+egress followed by travel-sum. No permanent positions
after this wave. All observed immediately legal original locks preempt
the new jobs. Builders walk only in actual open terrain excluding future
brace sites; no blocked walls, temporarily skip stalled jobs, cancel wave
reservation at the sole fixed deadline min(75, nominal optimum+12).
Original unassigned builders continue ordinary work, and genuine
ready pairs resume ordinary dynamic capture after wave release.

Apply `paired-egress-waves.patch` to exact v7 standalone. SHA256
`11abd512239885ecd9603a3505545abde0c64331b0091c6d937e89ef2756770a`.
GNU C++23 no args `g++ -std=c++23 integrated.cpp -o integrated`.
Fixture beside candidate standalone `g++ -std=c++23 wave-unit.cpp -o unit && ./unit`
checks disjoint 72 jobs, exact legal physical egress for all three banks,
prior paying guards untouched, and a truly blocked wall never written
while the abort returns to ordinary scheduling. For exact paired dev
`bench_v9.py` uses the outer standard official harness, plus
`stage_metrics.py` and the adjacent bridge audit geometry helper.

Dev30 all60 legal independent recorded replay, all30 exact v7 baseline
full traces. 74 of80 planned wave pairs complete (median t45).
First observed median exact stance staffing57→46, after five ready
gap19→3; first exact4 lock median76→64 (ever exact4 banks75→70).
But time of five-ready itself35.5→41, and useful open tubes at
t50/75/100 total572/666/725→497/616/675. At t100 physically paired
tubes25→14, on any prepared stance workers98→68. Early exact4
114→111; cats31→17. v7→v9 all-head separation14→13 (7 gains,
8 losses), residual49→42 but t160202→213 and t225132→151;
exact4 236→233, irregular4–48 63→57, total829,740,689→815,271,950.
First≤2 cases23→26 (med248→248.5); first exactly1 cases18→12,
conditional med265→253.5. Recorded max .896→.794 s. This co-design
works mechanically, but concentrated early staffing trades away
later tube coverage and directed species throughput; do not promote.
No changed parameters or unseen/official holdout.
