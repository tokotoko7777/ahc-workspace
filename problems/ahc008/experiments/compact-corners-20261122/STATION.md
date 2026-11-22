# Guard already at a prepared one-door pocket

This follow-up avoids chasing a moving animal and does not wait for a pet to be inside before selecting its pocket. One worker is assigned to a short tour that uses existing lattice walls and builds at most two missing non-door cells. The worker then stands at the gate before occupancy and closes that gate only if an animal has entered and the placement is currently legal. There is no human refuge prerequisite. The runner keeps other nonconflicting scaffold jobs active.

On 30 independent generated development cases, the official v3 tester and exact recorded-action replay found two actual occupied three-cell pens, one each in cases 0010 and 0020. Both were pet-only at the end. The frozen capture4 baseline confined no animals to regions of at most 32 squares. All-clear counts were 30 versus baseline 29, and official score 285,333,331 versus 216,223,497. These were development figures, not holdout. There were 55 cage starts, 41 reaching completed preparation, and 480 turns with a worker on the mouth's actual stance. Only two firings, despite 812 worker-turns reserved: about 406 worker-turns per compact capture. This is a valid but low-throughput route; keep as a witness, not a replacement policy or broad guarantee. No new test block spent on it.

Diagnostics columns in station-dev30.csv (no header): 0-based case index, score, tester wall seconds, started cages, prepared cages, closure firings, worker-turns, planned prep gaps, physically staffed mouth turns.

    patch -o station-prototype.single.cpp compact-corner.single.cpp < station-prototype.patch
    g++ -std=gnu++23 station-prototype.single.cpp -o station-prototype
    /path/to/official/tester ./station-prototype < dev/in/0010.txt
    /path/to/official/tester ./station-prototype < dev/in/0020.txt

For the exact benchmark wrapper, compile a diagnostic-free copy by removing the single solver stderr line beginning with `cerr<<"rails "`; the wrapper requires the remaining stderr to contain only the tester's score.
