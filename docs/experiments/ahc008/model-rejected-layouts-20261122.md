# AHC008 geometry screens: mixed lengths and crossbar bypasses

No live actions or extra practice cases were used to score these layouts.
Both screens use prebuilt actual board geometry, legal closure cells and
neighborhood propagation of passive paths. They omit human staffing
policy, construction delays, other-pet interference and adaptive
movement. The alternate layouts were not promoted.

- **1×3 and 1×4 mixed banks.** Length 3 leaves only a single safe center,
  so one such bank supports one sampled parity for any two-step animal.
  In 3–4–3, the two shorter banks' centers have opposite parity. Every
  parity can use 24 of 36 pens vs all 36 in 4–4–4, with wall inventory
  130 vs 156. At 120 with both sides always staffed, ideal single-pig
  absorption is 0.591 (aligned), 0.625 (staggered) per parity vs 0.819
  for aligned 4–4–4. Cow/rabbit legal dwell is also shorter; the lone
  safe cell switches parity every odd-step turn. The shorter construction
  is not enough reason to run a live alternative in current conditions.
- **Cap one exterior bypass (not a pen mouth) in each bank.** Extend a
  top or bottom shared brace into the board edge through a single
  body-span column. The opposite end keeps every pair of aisles connected
  after closing all pens. Alternating top/bottom on 3 banks adds 7 walls.
  At paired duty 0.11, independent turn epochs, t120, stationary exterior
  human targets (not the real guard controller), modeled dog absorption
  0.805688→0.807870. Cow/pig/rabbit gains are below half a percentage
  point. After the pens are hypothetically all shut, average pairwise
  distance of 8 exterior aisle anchors is 28.00→49.43. This is not worth
  the actual route and construction cost. The other fixed directions
  likewise had only small arrival gains.

Reproduce first with `python3 parity_mixed_geometry.py` (requires NumPy and
the adjacent `tube_banks_geometry.py`). For the bypass use
`python3 crossbar_bypass_model.py "$MODEL_DIR"`, where MODEL_DIR contains
[the earlier dog/passive transition reference](https://github.com/tokotoko7777/ahc-workspace/blob/6e230a8819f674d8749d64ff29d54ea4a73aad8a/problems/ahc008/experiments/20261122/bank_capped_hybrid_model.py).
The adjacent CSVs hold the full deterministic screens.
