# a237 adaptive skip-once: native practice rejects

Pure C++ preflight matched all 16,000 older signed snapshots, including
first 82 omission turns/cells and 18 absent, all actor masks exact.
Signed observations did not enter generated solver. No source used from
the supplied reference. Baseline a237, candidate f76fd57f…, binary
hashes fixed before native tests.

One CPU-exclusive serial alternating paired official practice100.
All 200 legs legal, <2.95s external native wall; every official
playback and independent score agreed and all pre-trigger commands and
observations matched. Results:

| Metric | a237 | Skip once |
|---|---:|---:|
| Official score sum | 6,152,805,559 | 6,070,247,621 |
| Full/useful | 100/100 | 98/98 |
| Max external native wall (s) | 2.760375 | 2.799198 |

The candidate triggered on 81 boards. None reinstalled the target.
Nineteen untriggered boards had byte-identical commands. Among the
98 both-full iterations floor added −5499/20 (−274.95 total), with
21 losing >4 individually; 43 both-full boards had target retained
open and gained ≥1 floor. Quality exit 3 as defined by the frozen
gate, not a harness/legality failure. No independent generation,
benchmark, retries, or parameter changes. This rejects only the exact
causal skip-once rule/adaptive contrast, not every fence deferral.
