# Shared projected wall adjacency is exact but misses speed threshold

Both ordinary retained-region and entrant histograms use exactly
the same ordered graph. Share its table across calls keyed on the
complete projected wall mask, including arbitrary topology rollback.
The 10,000 transitions and 500 matching RNG paths show exact
storage/ordering behavior; no particle state or output is cached.

Official distributed100 at actual accepted fixed workloads
reproduces every full command and score (all 200 legal, same
caps). Overall solver CPU57.890438→56.880292s (−1.745%), and
no late rollout subset 21.127373→20.706728s; max tester wall
2.74858→2.74009s. The predeclared 2.0% whole gate fails, so
no held/native validation or promotion. This is computational
equivalence, not an end-to-end score change. Archives include
only source transform and results, not solver/model binaries.
