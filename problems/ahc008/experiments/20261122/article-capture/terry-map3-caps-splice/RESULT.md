# Map3 cap splice: frozen staffing gate fails

Same fixed cap geometry and 36 synthetic no-animal boards as the first
FIFO dry experiment. Advisory pass identifies 88 of 288 total cap
tasks with eligible splice (range1–7 per board); 48 extra travel steps
for chosen slots, lower local minimum sum 46, 200 tasks still append.
No cutoff or program altered after advisory.

Integrated frozen dry execution: all 36 build 241 goals and become
correctly staffed by t133; zero original route loss and no stranding.
Identical stock paired witness to first experiment. Added paired
staffing delay median +12, max+22 (caps +10/+20), only 0.5-turn median
reduction relative to first failed append policy. Paired original-X
latency +2 median/+17 max here vs earlier 0/+1. Paired all-X including
eight caps +16 median/+27 max. Splicing moves work earlier at the cost
of actual baseline construction, without materially fixing handoff.
CPU advisory compile+run11s, integrated3s, no official or score. Stop
this layout family; no other scheduler or live retuning.
