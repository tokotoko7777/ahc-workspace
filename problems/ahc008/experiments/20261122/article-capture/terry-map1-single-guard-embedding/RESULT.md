# Map 1: six-cell one-guard fallback does not fit an isolated room patch

Freeze the 17 minimum-eight-brace classes of area six from the adjacent
one-guard fallback experiment. Generate all 136 orientations. At each
of Map 1's 14 physical original nine-cell rooms, test all nearby
translations whose body, door, fixed stance, or brace touches the
retired body or its original two doors. Reuse old X when possible.
Do not move or remove any existing planned X; new braces are allowed
only in the old room or its doors. Physical board edges can stand
in for a wall. Nothing is allowed to collide with stock planned X
that is not itself a proposed brace.

| Stage | Count |
|---|---:|
| Nearby in-board body, doors and stance | 112,572 |
| No collision with existing planned walls | 6,320 |
| All added braces wholly inside old room and doors | 0 |

The stronger variant requiring all braces physically in-board also
finds zero (108,684 and 6,116 at the first two stages). Each old room
was independently verified for original full aisle reach and
11-cell floor cost when alone closed. To gain at least one future
worker-floor cell, an exact six-cell replacement with two final
closing walls could afford at most two added braces. Every candidate
fails the earlier locality condition, so no candidate proceeds to
connection, staff stance or final floor checks for its replacement.

From the workspace, run `python3 ahc008_terry_map1_single_guard_embedding/check.py`
with the adjacent fallback JSON, earlier AHC008 catalog and grid helpers
in the locations specified at the top of the script. The catalog's
exact SHA-256 is verified by the script. Private assembled reference
program and any input/trace are not included here.

This excludes a local patch that preserves the immutable original
scaffold, not an initial design with rearranged walls. No solver,
official tester, or policy score was run.
