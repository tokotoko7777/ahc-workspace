# Shared-mouth parallel caps: structural no-go on observed traces

Consume all exact single-target 4–24 passable-cell candidate rows on
the existing tactical dev30 traces, not just the high-probability ones.
At a given case/turn, pair distinct animals and require their two exact
post-cut components to be disjoint, with all proposed gates on the exterior
of both. Require adjacent work areas (minimum gate distance ≤2).
Replay only cases with co-occurring geometry and count union of wall
jobs against two actual nearby workers' two-turn action capacity. For
a legal-window match, require both initial pets still in their own pen on
the next observed turn, test wall legality at the observed positions, and
preserve unassigned humans' original jobs. Record changed commands and
any omitted original movement or build from assigned workers. This is
an observed-path feasibility audit, not an exact counterfactual model;
the required wall count and route check need no pet trajectory.

- 20 pairs across two distinct cases. All have disjoint interiors and
  exterior gates. Only 17 are close enough; every one is the same two
  pigs (indices 8 and 11 in case 0002) over turns 102–104 despite the
  many alternative rectangle envelopes. All need five distinct wall
  jobs; two people × two turns can build at most four even if neither
  walks and both targets stay. The remaining three pairs' mouths are
  11–12 cells apart.
- Even if two adjacent animals share one den, leaving a common gate open:
  14 pairs give a valid connected merged pen of 9–13 cells with no
  current human inside. Six need four pending walls, eight need five.
  The six nominal capacity fits still have zero actual stance routes
  for the two workers; all four worker-turns would have to be builds.
- Zero legal joint window. No worker job displacement is applicable
  because no route qualified, not because the overlay would be free.

Do not build a solver overlay on these traces. Distinct pens fail
geometrical co-occurrence and worker-turn budgets; genuinely new one-pet
witness remains only case 0002 (see stochastic-lock-notes). Any next
two-animal approach has to generate shallow shapes along a real short
common frontier earlier, or justify ≥3 distinct ready workers without
repeating the old stationary-pocket staffing.

Run after the short-lock audit has generated states.csv:

    python shared_mouth_audit.py "$HARNESS" "$TESTER" ./tactical "$CASES" states.csv > shared-mouth.csv
