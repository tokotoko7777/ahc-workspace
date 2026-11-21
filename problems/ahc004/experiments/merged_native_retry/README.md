# Native-anchor retries for failed merged geometry
This opt-in arm starts from the measured rescue + k5 merge solver. On the saved
100-case experiment, 15 geometry-attempted cases did not get full coverage; 11
had an augmented merged bank, and none of the 15 improved the ordinary start.
The new branch applies only to an adopted merged bank whose primary geometry
search did not beat the ordinary seed. In particular it does not retry the
formerly successful geometry starts.

Alternative quintets use observed closed k-mer cycles rather than assuming the
merged evidence ordering can identify consistent anchors. Merged cycles remain
allowed donors; exact crossings are enforced. Donor-first augmentations maximize matching cardinality, then maximize the
number of native donors among those matchings. Native lines are also tried first
during extension. Actual read coverage ranks the distinct realized boards; only a
material covered-word advantage over the ordinary board can replace that board.
The retry phases end by about 1990 ms, and the normal optimization limit is
unchanged. Failed retries keep the ordinary seed and RNG state, though time
spent on a retry shortens ordinary annealing in those cases.

Bundle the local `submission_entry.cpp` and compile with `g++ -std=c++23` only.
