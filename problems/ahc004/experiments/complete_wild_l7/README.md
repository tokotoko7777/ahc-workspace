# L7-only extension of the failed-complete wildcard retry

Base `8d44ffe`, opt in with this directory's submission_entry.cpp. L8–10 still
use the same five-character overlap and minimum 15 fixed characters as the
base. New L7 inputs (longest observed read length nine) take the late branch
only if complete geometry actually ran, no seed beat ordinary, and the same
remaining-time cutoff holds. L7 uses four-character overlaps and minimum 13
fixed characters; all independent-crossing, exact score and time checks are
unchanged. This preserves the existing non-overwriting fallback and 2780 ms
normal solver deadline.

Saved archive diagnosis only: four L7 full-geometry misses meet the late
start cutoff. No official run of this arm.

CPU5 synthetic six-pair experiment, `make_case(7,M,88211951+783*j)` where
M=660,690,710,735,760,790:
- Two nonlaunches and two successful primary cases skipped the change, all full.
- M690 complete miss: late retry tried and rejected; 437/690 both.
- M760 complete miss: late retry chosen (539 verified seed reads);
  459/760 to 760/760. All resulting boards meet output shape/alphabet.
