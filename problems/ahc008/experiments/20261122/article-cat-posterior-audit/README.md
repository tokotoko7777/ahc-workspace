# Exact cat posterior on v7 bank-stage true paired ETA <=3 (read-only)

Same no-args v7 solver, with diagnostic observer only. Append actual
observed first/second movement after the actual human wall phase to
the previously unit-tested cat target-cell posterior (same stored
ShortestFirstMasks helper; source copies and hashes included).
Compare direct first-step log loss on the actual posthuman board to
the v7 heading mixture. For launch-time candidate prediction, use
the actual current prehuman graph, both old and new models using
one uniform second step, then unbiased two substeps at horizon2/3.
Only viable, currently open true 1×4 tube with nearest feasible
distinct exterior guards in the actually offered pair list, 1≤ETA≤3
(includes holding already-ready guards for one turn), and target is
a reachable cat with positive current-board support. Other-animal
gate-blocking factors remain identical model predictions in both
arms. Actual future animal occupancy, gate legality, two outside
stances, prescribed actors and applied exact two-door locks are
distinct labels; the future does not enter any input distribution.

Exact v7 score and full action hashes recur on all30 official
cases and independently verified recorded playback. NLL
.87968→.50982 (11592 observations), and still-reachable only
1.10414→.59880 (7628 observations). Evaluable 2690 candidate
rows in29 cases/90 cats. True-inner Brier .04360→.03925,
closure-legal Brier .03587→.03281 (25/29 case clusters improve,
68/90 cat-case clusters), legal AUC .88773→.91951; factual-staffed
Brier .02864→.02578. At each fixed (case,time,cat,ETA), only
nine groups offer both positive and negative tubes; rank .7778
unchanged in those direct comparisons. Of 122 lawful future labels,
101 have two actual outside stances and are caught; 91 have
the same named candidate workers. Just three distinct otherwise
final-missed cats/cases have any lawful window, none actually
staffed; conditional target capture forecasts go .0095→.0119,
.0301→.0347 and .0056→.0059. A broad prediction win is not proof
of gains in the control policy.

Diagnostic observer + extra NLL median .0242s/max .0785s, both
forecasts with logging median .0705s/max .2274s, diagnostic total
max1.175s. These are not deployed-estimator timings.
Restore v7 and run `python instrument.py V7.cpp audit.cpp` in
this directory; compile `g++ -std=c++23 audit.cpp -o audit`,
then `python check.py HARNESS BRIDGE_DIR TESTER audit DEV30
V7_DEV_LEDGER cat.jsonl nll.jsonl timings.jsonl summary.json`.
Decompress split cat/nll chunks in sorted order. No interactive
action policy or reference thesis changed.
