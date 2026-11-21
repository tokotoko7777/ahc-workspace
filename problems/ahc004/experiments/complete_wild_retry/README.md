# Late exact-intersection fallback after an unsuccessful complete-bank search

The base is `178d2a0`. Nothing changes if complete geometry beat the ordinary
seed or never ran, and the fallback is limited to longest read 10–12. An early
cutoff requires enough remaining time after ordinary, complete, alternate, and
native stages. The fallback rebuilds a k5 partial bank, with up to three
genuinely open anchor lines, actual closed lines as donors, and deduplication.
It uses offset-diverse packed wildcard triples, fourth-anchor matching and the
same exact realization rules as the incumbent wildcard route. Candidates are
corroborated with independent intersections, deduplicated under translations
and transposition, and ranked using exact covered-word counts before filling
up to three. The ordinary board is never replaced by a worse seed. Failure
restores the RNG state, but time spent may reduce ordinary annealing.

Absolute stage check cutoffs: 2100/2290/2360/2410 ms, with final anneal limit
unchanged at 2780 ms. No changes to incumbent thresholds or previously selected
geometry are intended. This is experimental and has not been run on the
official benchmark by its author.

CPU5 paired synthetic check, g++ -std=c++23, synthetic make_case seed
881229117+303*j for j=0..5:
- L8 M450: complete not attempted; 277/450 both.
- L8 M550: complete missed; fallback tried but rejected; 327/550 both.
- L9 M430: complete missed; fallback tried but rejected; 271/430 both.
- L9 M525: complete succeeded; fallback skipped; 525/525 both.
- L10 M440: complete missed; fallback seed 367, final 299/440 -> 440/440.
- L10 M520: complete succeeded; fallback skipped; 520/520 both.

Build by bundling the local submission_entry.cpp, then use only g++ -std=c++23.
