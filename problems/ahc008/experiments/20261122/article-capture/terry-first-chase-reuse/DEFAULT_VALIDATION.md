# Adopted: first simulated Chase action reuse
This is a speed improvement, not a score update. Same accepted incoming
baseline and standalone first-action memo at native 2670 ms, without
fixed pair caps, tested serially. All 400 runs matched the official
tester, playback and independent replay, and all 200 matched pairs
had identical full command SHA and score.

| 100-case panel | Score, both arms | Full / useful, both | Simulated pairs, base → reuse | Max tester wall, base → reuse |
|---|---:|---:|---:|---:|
| Official distributed | 6,152,805,559 | 100 / 100 | 620 → 657 | 2.7421 → 2.7482 s |
| Separate fresh generator | 5,998,857,137 | 97 / 97 | 874 → 956 | 2.7610 → 2.7609 s |

12/88/0 distributed and 15/84/1 fresh cases had more/equal/fewer
pairs. Fresh boards retained four accessible animals across three cases
in both arms; no arm changed a late mode decision. Wall values include
the tester and are not CPU savings. Under equally capped work see FIXED.md.
The archived fresh input manifest locks the independently generated panel,
and standard bench.py plus summarize.py checks the stored rows. From
the workspace root after rebuilding the pinned programs:
```
D=ahc008_terry_first_chase_reuse
P=ahc008_focus_solver/problems/ahc008
T=ahc008_focus_assets/tools/target/release/tester
export PYTHONPATH="$PWD/ahc008_integrated_v11:$PWD/$P/experiments/20261122:$PWD/$P:$PWD/ahc008_bridge_audit"
python "$D/prepare.py" default-fresh100
python ahc008_terry_exact_root/bench.py "$P" "$T" ahc008_terry_incoming_shadow/live "$D/solve" ahc008_focus_assets/tools/in "$D/practice.default.csv" --n 100
python ahc008_terry_exact_root/bench.py "$P" "$T" ahc008_terry_incoming_shadow/live "$D/solve" "$D/default-fresh100/in" "$D/default-fresh100/pilot.csv" --n 100
```
Pinned private source SHA256 8f50dda74f578fc8ddbf8176d15172a80df96d23d82c57db8ae51aedce4294b7.
Historical target 6,263,333,336 is on a different test panel; numerical
gap 110,527,777. No submit or main merge.
