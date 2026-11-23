# Frozen Map1 physical nine-body passive first-passage diagnostic

Use precisely the 19 official practice Map1 and 25 fresh Map1 signed
a237 tapes in oversized_chase/{practice,fresh}-map1/recordings.json.
Hash each input/action/reply, action vs trace correspondence and replay.
Scan actual pre-action snapshots of true configured Map1 nine-body
rooms captured by their own two gates (at least one newly constructed
this turn, actual both shut after command, exact physical nine-body,
head accessible before but not after). One entry per distinct
(pinned panel, board, real animal ID). Passive only, all outcomes
reported by cohort/type and phase where material. No solver or tester.

Freeze the physically open pre-action wall graph at the event. Use the
current otherwise empty single-animal chain and only existing intact
stock nominal rooms, physical sizes 1–4, exact determined physical
footprints with their own gate(s) simulated shut. All gates must be
open, room and gate free of workers, stock nominal role and body
intact. Retain every currently reachable exterior/service cell outside
the tested closure footprint; currently occupied workers may not
lose any exterior connectivity. Gate-safe target positions are actual
body tiles outside the closed neighborhood of their own gate(s).
Instantaneous staffing is optimistic; ignore every other animal
including all other potential blockers. Do not apply any factual
current/future build or remove any past wall. Sum legal target cells
as an absorbing union at **whole user-turn endpoints**, not substeps.
Apply 1/2/3 uniform local movements per turn with degree-zero stays.
Exact Markov recurrence, binary64 computation; no random estimates,
no predictions of counterfactual human motion or official outcomes.

Horizons 10/25/50 with separately truncated versions. At snapshot
t (0-based) final legal closure action t299, so maximum remaining
whole movements before another possible closure is 299−t. Success
at h=0 counts. Diagnostic supports delaying no actual capture by
itself; specifically reject it as evidence if ANY cohort has fewer
than 60% of distinct passive IDs reaching first-passage ≥0.80 by
min(25,299−t). Threshold fixed before loading outcomes. No changes
to admission rules, maps, or stock code. Nominal complete topology
atlas and prior four-step internal Map1 crop do not measure this
state-matched redirection to a different existing narrow room.
