# Map3 one-bank literal phase shift: analytic obstruction

Map3's narrow paired banks have body columns 2..4 and 15..17,
door columns 1/5 and 14/18, and guard columns 0/6 and 13/19.
The nominal rows are 1,3,…,29. Exact catalogue entries for the
first rooms (IDs 0 and 2) include an axial notch at (0,3) and
(0,16), respectively. Every lower room is straight through.
Inputs: map3 stock row patterns in
../ahc008_terry_exact_maps/terrain_data.inc (SHA-256
8d1f703d67b9d16fb0ba6d3b20cd2bb4a2cf20aea33e4a7351d45fd129e256cf);
physical room catalogue ../ahc008_terry_small_rooms/catalog.tsv
(SHA-256 2d697e1b2cbf0cd9d754d38a18059e2fbcd484b7440868da8016f8310132ba06).

For either narrow bank:
* +1 row clips the low r29 body/mouth/stance to r30.
* -1 row places nominal centers at 0,2,…,28, but clips the
  first body's true notch to r−1. Dropping the notch does not
  preserve its exact body.
* Even allowing a clipped notch to be dropped to make all rooms
  exactly three tiles, rigid through tubes at 0,2,…,28 need whole
  three-X brace rows 1,3,…,29. This is 45 X versus original
  14×3+2=44 in this slab. Other walls stay fixed: global 234, not
  ≤233. The last brace is r29, because the body is at r28 and
  outside the board cannot replace its actual neighbor row.
* Moving the top notch to the lower end is a morphological
  relocation, not a bodily translation. It was not attempted.

Existing three-cell tube safe centers use distinct colors:
(r odd, c3 odd) even in the first narrow bank, and
(r odd, c16 even) odd in the second. A shift of just one
whole bank reverses that bank but gives the two banks identical
center colors, and cannot introduce per-bank parity diversity.

Follow-up logical checks were not pursued after the shape/count
failure. Reproduce catalogue/mask observations from workspace:

    awk '$1==3 && ($3%30==1 || $3%30==14)' ahc008_terry_small_rooms/catalog.tsv
    grep -B1 'map 3' ahc008_terry_exact_maps/terrain_data.inc

No scorer, solver, runtime benchmark or alternative cell edits.
