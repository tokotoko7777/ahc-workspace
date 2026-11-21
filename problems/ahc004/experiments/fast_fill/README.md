# Exact position-mask construction and scoring

The normal entry uses the same timed portfolio as base 8d44ffe. A row or column
retains 20-bit symbol occurrence masks, doubled in a 64-bit number so shifted
window intersections include the torus. Word presence is exact intersection
without wildcard dots, while a construction move includes dot compatibility.
Dot counts provide the same placement quality as direct matching. Enumerating
compatible starts from low bit to high bit preserves original iteration and
random tie calls; each placement updates both axes.

GNU C++23 only. CPU5 paired fixed-work profiling used 25 fills and 200 exact
scores per run, three alternate-order repeats of each arm for three independent
synthetic `make_case(L,M,6481278+L*761)` fixtures. Medians in milliseconds:

| L, M | Literal fill | Mask fill | Literal score | Mask score |
|---|---:|---:|---:|---:|
| 5, 550 | 127.910 | 9.730 | 413.215 | 12.924 |
| 8, 700 | 187.875 | 13.119 | 588.129 | 17.706 |
| 10, 760 | 201.868 | 12.894 | 618.732 | 17.710 |

All resulting grids, score pairs, and following RNG words were byte-for-byte
identical over the fixed workload. The mask-level independent test compares
literal starts and dot counts after 2,310 random point updates, including cyclic
windows and dimensions 1–20. The timed solver may complete more initial greedy
attempts and select different downstream boards, so score is not inferred from
the fixed-work test. This author did not run the official benchmark.

To reproduce, bundle this entry, then prepend `#define AHC004_FIXED_FILL_BENCH`
to a *copy* of the bundled source. Remove the exact line
`#define AHC004_FAST_FILL` for the control. Compile both copies with only
`g++ -std=c++23`. Feed identical generated inputs from the synthetic
`medium_diverse/partial.py` generator; compare stdout and the two fixed times
reported on stderr. The submitted entry does not enable fixed-work benchmarks.
