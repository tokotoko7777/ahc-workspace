# Causal wall-source attribution on v4 used dev30, no policy change

Instrument the exact integrated v4 standalone with `instrument.py`,
compile via GNU C++23 important pragma present, and run `audit.py`
on the matched old dev30 and archived v4 paired dev reference.
All 30 entire action hashes, official scores and independent playback
scores are unchanged. This is audit, not runtime comparison.

For every physical final wall, compare its cell to the raw planner
proposal prior to repair/tactical/terminal wrapping. Pure-source wall
moves have measured direct topology changes; for mixed sources the
audit avoids arbitrary attribution (and records leave-one-source-out
marginals). The immediate loss measure is the mean per human of own
reachable component area before minus after the wall action, including
same-position humans with their full multiplicity. New animal
isolation means a pet crosses from reachable by any human to reachable
by none, regardless of score changes. Pets need not be rerolled:
movement after walls cannot cross their new connected components.

Pure late line turns: 5831 mean-floor loss summed over 30 cases, 91 real
newly isolated animals, 125 detached animal-empty cells total. Sixteen
turns have actual loss >=100 across 14 cases; only two leave a strict
majority reachable, 186 area for 2/5 and 104 for 1/3. All sixteen
finish with exactly one wall, and split no humans. Sixteen separated
animal components vary widely in area; attribution records exact IDs.
Occupancy/benefit can still be a tradeoff even without majority waste.
Actual selected episodes 476, 35 complete by final owned action
and 67 ever complete; remainder are not presumed waste. Unfinished at
ownership end accounted for 21 actual captures and 2077 of line loss.

Ten wrapper-only wall turns lose 1838.54 summed floor and produce 14
new isolates. Seven wrapper turns lose >=100; three low-yield broad
events are at t299, where tacticalGate is dormant and terminal scoring
can separate humans while capturing no new animal globally
(0011/0022/0027 losses 352.8/208.3/480.5). This is disjoint from
latePlan and is a different objective (individual score, not global clear).
Ordinary pocket turns lose1526 for39 isolates, lastcap292 for11,
banks5435 for235. No mixed-source wall turns occur in this cohort.
The audit does not measure alternate paths or justify blanket late
wall suppression.
