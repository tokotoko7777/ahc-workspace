# Exact relaxed opposite-rail width-three packing profile

Freeze a height H strip of three interior columns. Opposing axial
doors lie one column outside each side and guards one more column
outside, as on the narrow bank. Allow exact connected bodies of
three or four orthogonal cells, disjoint within the strip, no
cardinal body/body contact. Each body spans all three columns and
has one distinct usable door on each side. Optional physical
board edges at the outer rows substitute for bracing. Classify
both-pig-color bodies by actual gate-safe cells of both colors.

Enumerate every normalized 3- and 4-cell body+gate template. Record
the shortest vertical span to pack exactly 1..15 rooms and exactly
0..k of them both-color via relative-shift DP (construct witnesses).
Record maximum both-color at fixed H=30 and k=15,14,13,12;
also largest packed k with all rooms both-color. No solve/menu
expansion if the computed maxima are poor. Guard rails and complete
built wall/floor reach beyond this strip not tested. This is a
layout-density relaxation, not full Map3 dynamics or evidence of
adoption. No test case, official tester, or score.
