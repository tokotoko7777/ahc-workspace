# Fixed staged cordon (v15), not adopted

Exact accepted v7 replay comparator. Starting turn 168 when one to three
animals are still reachable, including at least one passive species, try
one fixed shell around the passive animal region. Keep a connected human
exterior of at least 300 cells. Allocate one or two genuine mouths to
nearby fixed guards; the remaining humans build other shell walls first.
A short excursion does not trigger pursuit: hold for up to seven outside
turns, then give control back. Seal only with an actual animal inside.
The fixture exercises shell construction, waiting through an excursion,
simultaneous legal closing, and exact exterior/animal separation.

Exact paired dev30: all 60 official runs and independent replays were legal and
all 30 v7 controls matched archived actions and scores. All action prefixes
through t100 are unchanged. Relative to v7, full clears fell 14→10 (1 gained,
5 lost), with >=300 usable cells 9→8, residual animals 49→58, score
829,740,689→778,050,208, true four-cell catches flat at 236. Max tester
wall time .937 s. There were 32 attempts with 110 real shell walls built and
248 outside-turns; only three windows actually sealed, none converting a v7
miss to a full clear. No holdout and no adoption.

No setup can shrink an animal-only enclosure after final closure: any open
adjacent builder stance would itself connect the animal to a human. Updated
replay checks exact first-to-final pocket footprints in future runs.

Build with GNU C++23: `g++ -std=c++23 solve.cpp -o solve`. Compile and run
the fixture with the same compiler and standard. Reproduce `dev30.csv` via
`bench.py`, passing the workspace's normal official tester, v7 executable,
this executable and dev30 input directory, with the problem and experiment
harness modules on PYTHONPATH. The captured CSV is compressed losslessly;
`summarize.py` compares its parent actions to the previous exact v7 ledger.

The user's newer direction intentionally keeps the broad repeated 1×4
equals-shaped layout as the primary mechanism rather than using a separate
arbitrary cordon. Therefore this remains diagnostic only; the tight choke
draft was not evaluated.
