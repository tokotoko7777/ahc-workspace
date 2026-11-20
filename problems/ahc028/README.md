# AHC028: Lucky Words

- [Official statement](https://atcoder.jp/contests/ahc028/tasks/ahc028_a)
  and [official generator / scorer](https://img.atcoder.jp/ahc028/fWRno7xB.zip).
  Time 2 s, memory 1024 MiB, at most 5000 presses.
- Score is the sum over 150 contest cases. With all 200 words typed, each
  gives `max(10000 - total_cost, 1001)`; if only K<200 are present the score is
  `round(1000*(K+1)/200)`. There is no post-contest system test.
- Final rank 1: **1,105,471** from the rank/score CSV in the
  [AtCoder-provided score archive packaged by ALE-Bench](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc028.zip)
  (SHA-256 `7bc6df6d9f1b97c57d2a540f9c286d2acc7374775fef712b37f4923bda378035`,
  matching the publisher LFS pointer). The tools bundle includes 100 starter
  seeds, not verified as the 150 contest cases. All measurements below are on
  fresh generator inputs, so **official comparison is undetermined**.

## Baseline and rejected proxy tour

At each step choose the remaining word with cheapest optimal typing from the
current coordinate, after taking maximal suffix-prefix overlap with the last
word. Then reconstruct the exact global minimum keypress path for the full
chosen string via [Ragged Layer Shortest Path](../../library/ragged-layer-shortest-path.hpp);
the local greedy paths select order, not the final coordinate route.

With `-DAHC028_TOUR=1`, start from the same base plus shuffled restarts and
anneal overlap-aware directed tours, using swap and relocate delta scores.
Each approximate edge averages exact cost to type the remaining suffix over
all possible keys for the preceding word's last character. Check actual full
typing costs periodically and preserve the greedy answer if none improve.

| Split (24 cases each) | Greedy mean | Tour mean | Tour W/T/L |
|---|---:|---:|---:|
| Development | 7097.25 | 7097.25 | 0 / 24 / 0 |
| Untouched holdout | 7131.9583 | 7131.9583 | 0 / 24 / 0 |

All 96 outputs contained every word and matched the official scorer and an
independent replay. Max measured wall time was 0.030527 s on one pinned CPU.
Keep greedy default. These negative results suggest the average-key edge
proxy loses too much information about the coupled coordinate choices. Next
try local order edits with **exact conditioning**: prefix and suffix dynamic
frontiers at untouched word boundaries; recompute only the edited substring,
including both changed overlaps. Start with adjacent swaps/nearby relocation,
validate each delta against full typing DP, and benchmark on new splits.

The actual shared-library gap was reconstructing an optimal path with a
different set of candidate keys on each letter. Mapping all layers to fake
full-board states would add work and unavailable-key conventions. The
ragged-layer helper keeps only viable candidates with a generic cost callback;
overlap, keyboard distances, and word order stay here.

From repository root (`ATCODER_TOOLS` is the official tools' `target/release`
directory, after `cargo build --release --locked` there):

    g++ -std=c++23 -O3 problems/ahc028/main.cpp -o /tmp/ahc028-greedy
    g++ -std=c++23 -O3 -DAHC028_TOUR=1 problems/ahc028/main.cpp -o /tmp/ahc028-tour
    python problems/ahc028/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc028-greedy /tmp/ahc028-tour /tmp/ahc028.csv
    g++ -std=c++23 -O2 tests/ragged_layer_shortest_path_test.cpp -o /tmp/ahc028-dp-test
    /tmp/ahc028-dp-test
    python scripts/bundle.py problems/ahc028/main.cpp -o /tmp/ahc028-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc028-single.cpp -o /tmp/ahc028-single

Use `-DAHC028_VERIFY_DELTAS` with tour mode to check accepted move deltas
against the complete proxy objective (debug test, not submit). Per-case
outputs are summarized in [benchmark-results.csv](benchmark-results.csv).
