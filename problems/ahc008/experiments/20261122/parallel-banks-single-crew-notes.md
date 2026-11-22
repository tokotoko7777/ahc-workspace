# One ordinary pocket crew at early handoff (rejected)

Comparison on the same used dev30 as hard-anchor. Freeze both arms from
global-clear standalone (remote 591d608…, Git blob
9448476d8cf3571bc0519a1ec431add302ef7165, SHA256
74ef6c116d970224397bbf8c403a8b6b6c27cb9d78621fc52e189a7faa5bc728).
After unchanged higher-precedence dog/late plan calls, enter the existing
ordinary all-worker one-pocket branch during actual release160, turns
160–209 only. No target-generation, retention, escape-check, global-clear,
seam memoization, construction, bank-hunt or release changes. The control's
action hashes and scores all match the archived global-clear dev30. Both
arms have identical prefixes before handoff in all cases; 13 inactive have
complete byte identity. In both arms 17 cases (765 turns) reached dispatch
under the experimental condition that would ordinarily select two crews.

| Metric | Existing dual | Single crew |
| --- | ---: | ---: |
| Official aggregate | 746886882 | 677935654 |
| Final residual animals | 42 | 89 |
| All humans pet-free cases | 18 | 13 |
| Clean human component union, sum | 5885 | 5556 |
| All human component union, sum | 10190 | 13155 |
| Exact-four isolated animals at closure and at end | 212 | 211 |
| Animals first isolated during t160–209 | 61 | 47 |
| Of those, nonrectangular 4–48-cell pens | 38 | 32 |
| Max sampled wall seconds (direct official tester + one diagnostic log) | 0.429 | 0.433 |

Four case wins, 13 losses, 13 ties. Both exact control and candidate pass
all 30 tester/replays. Component attribution uses real wall transitions,
not approximate target metadata. JSONL retains animal input IDs, zero-based
action time and physical component masks at first isolation from all humans.
Animals isolated after t209 also fell 136→103, despite identical dispatch
rules then: the state reached by the single crew was worse. Its larger human
territory is predominantly mixed. No frozen/official holdout or promotion.

Apply the patch to the exact global-clear source, compile with
`g++ -std=c++23 global-clear.cpp -o dual` and
`g++ -std=c++23 -DAHC008_SINGLE_CREW global-clear.cpp -o single`.
Then call `parallel_banks_single_crew_audit.py HARNESS TESTER CASES dual single
NARROW_CONTROL.csv output.jsonl` with the same dev30 paths as the anchored
audit. The solver's existing pragma applies optimization to both builds.
