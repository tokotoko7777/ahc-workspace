# AHC008 faithful method: ordered open-neighbor forecast cache

Follows the already verified faithful C++ solver and the ETA-zero speed
shortcut, not a new capture or room-selection rule. Before the first
eligible forecast pet on a projected board, calculate its open neighbor
lists once, in the original U,R,D,L order. Each 1/2/3 random substep and
dog directed choice reads the same eligible neighbors; the type of
random draw, order, and consuming behavior are unchanged. Lazy creation
keeps no-eligible-pet paths free of extra work. No change to exact layout,
construction, staff, objective, actual motion or virtual-jury simulator.
Unit: 128 random complete count matrices and resulting PCG states match
the old simulator exactly.

GNU C++23 standalone independent implementation contains private
coefficient data from the user's authorized input; keep generated source
and binary local. This archive contains only the generating transform,
unit, and evidence ledgers. Derive source d14f6471e5f64bf4a936f8cf27a0f585136a53e13f3e4a27af9fa21f6427b49c
from previous ETA-zero source 71b0edebbf87e873543f6438786d68d8b1130ff69975605318ed8b18507fdad7.

    python ahc008_terry_motion_cache/make.py ahc008_terry_speed/submission.private.cpp ahc008_terry_motion_cache/submission.private.cpp
    g++ -std=c++23 ahc008_terry_motion_cache/submission.private.cpp -o ahc008_terry_motion_cache/solve
    g++ -std=c++23 -O2 ahc008_terry_motion_cache/unit.cpp -o /tmp/ahc008-cache-unit && /tmp/ahc008-cache-unit
    python ahc008_terry_speed/bench.py ahc008_focus_assets/tools/target/release/tester ahc008_focus_assets/tools/in ahc008_terry_exact_root/practice100.csv ahc008_terry_speed/solve ahc008_terry_motion_cache/solve practice.speed.csv

## Paired fixed-work comparison

Use the same factual per-board terminal cap in both arms *only in this
equivalence test*. All 400 timed serial official solver runs matched their
exact prior command SHA and official score, identical on the two 100
panels. Independently scored traces for these hashes are in the faithful
archive. Relative to the already faster ETA-zero version:

| Panel | ETA-zero solver CPU | Add cache CPU | Reduction |
|---|---:|---:|---:|
| Official practice100 | 82.240947 s | 71.499169 s | 13.061% |
| Independent generated100 | 96.194780 s | 84.492097 s | 12.166% |
| Practice without late rollout (86) | 47.361620 s | 39.384849 s | 16.842% |
| Independent without late rollout (81) | 48.825658 s | 40.619991 s | 16.806% |

Local max tester (fixed work): 2.7063→2.5154 s practice; 2.7471→2.7457 s
independent. Scores unchanged at 6,082,666,664 and 6,037,488,888 on these
fixed-work proofs. Do not combine separately measured percentages or
treat this speed result as a higher default-clock score. The separate
default-clock trial is preregistered in DEFAULT_PRESET.md.
