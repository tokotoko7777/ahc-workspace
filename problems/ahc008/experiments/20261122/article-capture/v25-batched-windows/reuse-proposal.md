# Proposal: share per-object collision counts across many capture windows

The v25 guard planner evaluates dozens of windows at the same simulated time
and in the same trial. Its original API accepted one target window at a time,
so it repeatedly scanned the same animals. The verified replacement represents
membership and blockers for up to 64 windows as two masks per board cell;
each trial checks the blockers once, then counts only live target windows.
Keep the single-window kernel as the fallback for larger sets or too few
windows. Return integer numerator counts; let callers own sampling and the
one division. Preserve overlapping bodies, duplicate entity locations and
zero-sample semantics. Use deterministic exhaustive/random equality tests
before any integration; performance depends on density and count of
simultaneous windows.

This is an AHC008 research implementation, not an installed library change.
Generic API and reuse are proposed here, not claimed for other problems.
