# Reproduce the compact-corner solver

These are local offline research results, not AtCoder submissions. The official AHC008 v3 interactive tester and generator are required. Compile clean single files without additional optimizations:

    g++ -std=gnu++23 compact-corner.single.cpp -o corner
    g++ -std=gnu++23 ../../capture_first_submission.cpp -o capture4
    mkdir generated
    cd generated && /path/to/official/gen ../generated100-seeds.txt && cd ..
    python benchmark_cases.py /path/to/official/tester generated/in generated-check.csv capture4=./capture4 route=./corner --count 100 --capture-topology generated-check-topology.csv
    python benchmark_cases.py /path/to/official/tester /path/to/official/in practice-check.csv capture4=./capture4 route=./corner --count 100 --capture-topology practice-check-topology.csv
    python summarize-topology.py generated-check-topology.csv practice-check-topology.csv

The parse-and-replay benchmark independently rebuilds the final connectivity from recorded interactive actions. Existing per-case summaries annotate compact pen areas as `area:pets`, with multiple distinct pens separated by semicolons. If the node executable wall time differs across hosts, verify the action SHA-256 and score instead of claiming elapsed-time equality. For the dev witnesses use the first 30 seeds and original late variant, not the route-refined candidate; the three traces show the standalone mechanism but are not holdout evidence.

For development footprint events:

    patch -o compact-late-witness.single.cpp compact-corner.single.cpp < late-witness.patch
    g++ -std=gnu++23 compact-late-witness.single.cpp -o late-witness
    mkdir dev && (cd dev && /path/to/official/gen ../dev30-seeds.txt)
    /path/to/official/tester ./late-witness < dev/in/0007.txt
    /path/to/official/tester ./late-witness < dev/in/0015.txt
    /path/to/official/tester ./late-witness < dev/in/0021.txt
