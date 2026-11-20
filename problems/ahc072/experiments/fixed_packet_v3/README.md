# AHC072 fixed-support mixed packet

This opt-in experiment uses no historical cases or results. The historical
rank value remains as documented in the [problem README](../../README.md);
only a fresh generated comparison was performed. Preregistered seeds:
SHA-256 salt `ahc072/fixed-packet-v3/{split}/{i}`, 12 development, 20
untouched, exclude prior tracked and official seed IDs.

## Exact fixed-support model

Given a packet of at most eight slimes, move its whole surviving contents,
without picking up anything else. All other stacks are immutable and their
top slime, if any, must not match the underlying nest. A fixed support
therefore survives being uncovered at the source and after a packet
empties on it. A state is cell, surviving contiguous original word slice
`[lo,hi)`, and orientation; there are at most `q(q+1)` states per cell.
Before any pop, the destination must fit the whole packet above its
support within height 8. The jump length is at most source support height
plus one and all crossed cells must be floor. Landing reverses the packet
and removes any matching top suffix; that only trims an end of the interval.
Empty means success. BFS gives the exact fewest jumps for this fixed input,
including impossibility, not an optimum for arbitrary pickups or support
changes.

On the four-cell line, start with bottom-to-top `A,B` at cell zero, a
nest for A at cell two and B at cell three. With only single-step hops,
orientation is tied to checkerboard parity: the wrong slime is exposed at
each nest. One stationary support beneath the source packet enables the
length-two jump that returns A, then the length-one jump that returns B.
The literal test oracle copies the board, performs actual suffix reversal,
and runs source **and** destination automatic pops for every move. It
agrees with the compressed path on 700 additional deterministic boards
with walls, packet lengths up to eight and multi-slime supports; it also
checks a separate rejection where popping would fit but pre-pop height
would exceed eight. All recovered shortest paths are literally replayed.

## Pricing a real gather

At the same starting board as the frozen farthest-first caravan, consider
up to three other-colored singleton targets within three actual support
hops. Route the first slime onto the resident, then route the two-slime
packet with the exact BFS. Include actual gather moves in the price and
require a strict improvement over the same two slimes delivered separately
in source-first order. The comparison does **not** include the full
opportunity cost of foregoing other same-color caravan joins. The run
replays the full mixed-first itinerary and only uses it when it beats
the full frozen caravan started from the exact same input.

| Split | Caravan mean moves | Raw mixed-first mean | With full frozen fallback | Raw mixed W/T/L |
|---|---:|---:|---:|---:|
| Development 12 | 416.1667 | 535.5000 | 414.2500 | 1/0/11 |
| Untouched 20 | 425.0500 | 564.3000 | 424.7500 | 1/0/19 |

The median selected improvement is zero. Untouched raw runs chose 328
locally profitable gathers out of 1,142 trials (1,141 were reachable).
These moves saved 1,792 in the local comparison with separate routes,
but the full mixed-first solution was much worse overall. Only one
untouched case used the mixed itinerary, containing two mixed gathers;
the other 19 used the general fallback. This is a local break-even
test, not a reason to replace beneficial same-color groups.

All 64 selected and 32 raw outputs passed complete independent replay
and matched official scores. Max selected solver wall 0.018611 s, baseline
0.008730 s. The bundle, literal oracle and 22 existing regressions passed.
Keep optional and do not run a final audit. If revisiting, price one
mixed substitution against an actual two-batch **same-color** caravan
continuation after reserving its support route, rather than comparing
with two isolated single-slime trips. The new helper is problem-local.

## Reproduce

```sh
p=problems/ahc072
d="$p/experiments/fixed_packet_v3"
python3 scripts/bundle.py "$p/main.cpp" -o /tmp/072-submission.cpp
g++ -std=c++23 -O3 /tmp/072-submission.cpp -o /tmp/072-base
g++ -std=c++23 -O3 -DAHC072_MIXED=1 /tmp/072-submission.cpp -o /tmp/072-packet
python3 scripts/bundle.py "$d/raw.cpp" -o /tmp/072-raw.cpp
g++ -std=c++23 -O3 /tmp/072-raw.cpp -o /tmp/072-raw
g++ -std=c++23 -O2 tests/ahc072_fixed_packet_test.cpp -o /tmp/072-test
/tmp/072-test
taskset -c 8 python3 "$d/benchmark.py" "$GEN" "$VIS" /tmp/072-base /tmp/072-packet "$OFFICIAL_SEEDS" "$WORK"
taskset -c 8 python3 "$d/check_raw.py" "$VIS" /tmp/072-raw "$WORK"
```

Frozen baseline binary SHA
`ca05ac3425e644cd74d10d5a7689e55c466846885429049c5f2b463b8cd84249`
(local `d7a742a`), packet binary SHA
`689e80717ba6ee300591085987299ea75a17350bdd8626080f02a023cd57559e`;
bundled source `c0a55099f5288c58b2bd8fa8b70ea7816f8a3b4546f1ae8f4dca760c516fe3b3`.
