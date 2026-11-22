# Capture-first 4×4 room-backbone milestone

This targets zero reachable pets for **every human** before optimizing clean area. It does not claim to pen every pet individually; humans evacuate to completely isolated, clean connected room sets. Preserve this baseline separately from the highest-score tactical solver.

## Method

Build thin separating strips with 24 room-to-room doors left open. Doors form a living backbone: humans can still move between rooms while construction proceeds. A connected subset of currently pet-free rooms is selected by exhaustively enumerating subsets, scored by reachable area, number of exterior openings to seal, travel, and current pet proximity. Move all humans into the chosen set, then close boundary gates and any still-missing seam cells from the clean side. Recheck cleanliness throughout; existing movement and wall legality guards remain in force. Do not sacrifice a dog decoy in this arm.

## Frozen validation

| Dataset | Model | All humans pet-free | Pet-free people | Mean reachable pets/person | Mean area for clean people | Score | Maximum tester wall |
|---|---|---:|---:|---:|---:|---:|---:|
| New generated 100 | Tactical | 6/100 | 423/771 | 1.453 | 235.47 | 1,571,272,459 | 0.577445 s |
| New generated 100 | Capture-first | **99/100** | **762/771** | **0.233** | 83.23 | 896,444,521 | 0.273071 s |
| Distributed practice 100 | Tactical | 10/100 | 419/736 | 1.635 | 205.53 | 1,512,932,629 | 0.482855 s |
| Distributed practice 100 | Capture-first | **98/100** | **722/736** | **0.342** | 82.46 | 912,125,586 | 0.269227 s |

Official tester and recorded-action independent replay agree on all 400 executions, with no illegal cases or timeouts. On the development 30, capture-first was 30/30 clear versus tactical 4/30. No distributed cases were used to choose parameters. The all-clear metric concerns the final board, not temporary safety during construction.

Capture-first reaches a minimum of 100 clean cells per person in 20/100 generated and 24/100 distributed cases; minimum 150 in 4/100 and 8/100 respectively. Neither split has a case reaching a minimum of 300, 450, or 564 clean cells. Mobility is preserved while the backbone is open; final per-human area/pet profiles differ in no generated case and one distributed case (a lower bound on disconnected teams, as equally sized components can look identical). The most severe generated failure is one shared 753-cell component containing all 20 pets. It is not an invalid output.

## Reproduction

Compile the standalone with GNU C++23, no special options:

    g++ -std=c++23 problems/ahc008/capture_first_submission.cpp -o cap4
    g++ -std=c++23 problems/ahc008/tactical_gate_submission.cpp -o tactical

Use official AHC008 tester/generator v3 with benchmark_cases.py. For generated holdout, seed i is the first eight SHA-256 bytes in little-endian of ahc008/20261122/capture-first/holdout100/{i}, i=0 through 99. For the practice comparison use the official v3 archive's distributed in directory. Run:

    python problems/ahc008/benchmark_cases.py /path/to/tester /path/to/in capture.csv tactical=./tactical capture4=./cap4
    python problems/ahc008/experiments/20261122/summarize_capture.py capture.csv

With --telemetry and --interval 300 the tester wrapper also records exact group counts on the last turn. Case-level evidence is in capture-freeze100-holdout.{csv,json} and capture-freeze100-practice.{csv,json}.

## Next step

Continue compaction with the capture condition unchanged. Candidate ideas are progressively shedding occupied leaves before the final clean-side evacuation, deferring the final cut when the clean subset cannot contain enough area, and moving walls only where persistent pets require subdivision. Do not promote a higher-score compaction if it substantially sacrifices the 98–99% all-clear rate. Current score is far below the 6,263,333,336 reference, so no merge or submission.
