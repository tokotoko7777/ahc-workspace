# Soft recent first-substep cue for directing bank guards

Inspired by the directed dog/cat explanation in the user-authorized
[simanman article](https://simanman.hatenablog.com/entry/2022/02/28/074838).
Use only observed first substep in the most recent official reply, not
latent target inference. After t101, if such an animal is within one row
and three cells of either mouth, and the substep is along the tube axis
without crossing the midpoint, multiply its existing contribution to
candidate ranking by 1.6 for inward or 0.5 for outward. No hard exclusion.
Keep same staged body-only geometry, full bank staff, exact immediate
atomic closure priority, construction and t225 tactical fallback. No
irregular overlay. Ctrl mode5 0, direction mode5 1. Read from live replies.

On old dev30 all 60 legal with official replay, same geometry and
t0–100 bytes, all humans and service lanes connected through t225.
Direct exact-four 270→288 (11/16/3 paired wins/ties/losses), atomic
seals261→282. Direct species cow/pig/rabbit/dog/cat
29/42/54/83/62→33/44/63/82/66. Eligibility385→387 distinct case-pets,
residual t225 165→147, final116→94; all-clear9→7, final safe union
4144→3451, official score 541,987,952→522,083,279 (17/7/6,
unequal magnitudes), max .566/.587s. The cue fires 2121 inward/1936
outward candidate-pet ranking incidences; these are not distinct pets
or necessarily selected orders, and don't estimate predictive accuracy.
Observed compact gain is mostly not in directed species and score loses.
Archive separately, no promotion/heldout, no threshold sweeps.

Apply parallel-banks-pilot.patch, parallel-banks-staged.patch,
parallel-banks-zone.patch, then parallel-banks-direction.patch
to a scratch tactical copy. GNU C++23. Run
parallel_banks_direction_bench.py with the existing harness, tester,
tactical binary, new dual mode binary, dev30 case directory and CSV path.
