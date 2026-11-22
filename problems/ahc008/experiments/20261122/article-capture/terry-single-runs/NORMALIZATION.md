# Same-owner stance normalization fixes the five cap-layout deadlocks

Five frozen insertion-only failures reproduced with the added caps
removed: an ordinary stock-assigned cap replaces the same worker's
future stance one to two turns before it needs to place a neighboring
wall. On the original unedited map all five finish; in the failure
batch, removing just that cap restores pending stance reach. This is
not a side effect of the extra eight cap-worker assignments.

For each original planned task whose stance becomes a cap, keep the
same exact target wall, owner and order; relocate just the staging cell
to a non-door neighbor draining to a real aisle even with all X and
named gates closed. All eight possible cap-adjacent targets on the two
maps have such a neighbor. Keep the frozen explicit missing-cap
attachment unchanged and apply it after normalization.

Dry gate passes on all 35 synthetics (30 fresh plus all five failures);
every planned X/owner invariant and exact new physical room readiness
matches. Two moved stance tasks, eight stock caps and eight attached
caps per board, worst completion t124/t119 by map; measured dry CPU
10.0764 s. No score inference or user-private original files in the
archive. Only this assurance permits a separate official pilot.
