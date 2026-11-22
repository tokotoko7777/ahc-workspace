# Exact faithful failure tapes: immediate one/two-wall alternatives

Run `python ahc008_terry_factual_late/screen.py` from workspace root; output
is in report.json. Requires the historical ledgers, practice tapes from
ahc008_terry_scaffold_audit/tapes and the four local independent tapes.
To reproduce the latter, use scaffold audit's materialize.py with exclusive
tester timing, the frozen faithful standalone, the exact independent ledger
and input directory, and `--case 0015.txt --case 0035.txt --case 0065.txt
--case 0079.txt`, outputting here to tapes/. All four matched at the recorded
paired-rollout cap on first pass. Official replay and separate Python score
agree; no policy edit or unsolicited solver comparison.

At turns 220–299 (zero-based), generate one or two distinct currently legal
adjacent walls for distinct actual actors. Keep all unassigned original
commands including their projected moves and walls. Reject a wall on a
person/pet/orthogonally adjacent pet, out of board, already blocked, or in
any resulting person's position. Count changes to original nonwait commands
separately from assignment to original dots. Dedup by resulting board AND
human positions, preferring no displacement. Verify all actual pets are
disconnected from every resulting human position; floor is the mean own
component area across all humans, not a union or maximum. Actual normal
versus chase uses the recorded decision and exact t275 switch. The factual
pet observations before that moment suffice: once severed, future pet
motion cannot reconnect across permanent walls. No speculative relocation.

Six eventual failures, 480 snapshots, 80,643 alternative projected states:
77,316 normal and 3,327 chase. None reaches whole isolation with floor
at least 300. On independent 0079 only, two genuinely unused actors could
both seal three consecutive normal turns (277–279); those complete
isolations leave mean area 253, retaining every original move and build.
The other five boards have no whole-clear alternative even below 300,
including the only actual chase board. No stock snapshot clears. These
are selected factual trajectories, not a frequency estimate on general
boards and not evidence about intervening earlier. No live change proposed.
