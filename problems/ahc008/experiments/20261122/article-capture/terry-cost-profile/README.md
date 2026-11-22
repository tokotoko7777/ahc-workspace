# Where the current exact-short faithful solver spends time

The observer uses the accepted h123 version at fixed completed rollout
pair counts, reusing already-validated independent held inputs. Every
one of the 30 profiled complete action digests and official scores
agrees with the frozen version. No other behavior is tested. The two
predeclared groups contribute 16 no-rollout and 14 positive-rollout
boards (all 14 available) in the selected independent100. Reported
fractions are exclusive wall samples of instrumented solver stages
only, not rankings and not precise end-to-end CPU percentages.

| Cost owner | No-rollout | Positive-rollout |
|---|---:|---:|
| Initial build split planning | 55.3% | 9.8% |
| Retention MC other than exact short mass | 22.5% | 5.1% |
| Idle patrol | 8.0% | 8.8% |
| Aisle assignment | 4.7% | 0.6% |
| Exact short passive mass | 0.2% | <0.1% |
| Chase shape / max flow | 0% | 47.9% |
| Virtual pet motion | 0% | 14.9% |
| Other Chase routing | 0% | 7.7% |

The diagnostic changes only timers and uses an explicit positive or
zero fixed cap, not the original wall budget, so instrumentation
cannot discard the last intended pair. Accepted published execution
must not be built with the diagnostic. Numerical differences are
only observations on the selected serial inputs. Recommended first
exact speed target is the high-cost construction split, not smaller
per-turn representation conversions.

This archive also preserves the independently rejected actual-wall
byte cache: exact command/scoring identical on 200 serial official
runs, but wait4 CPU **68.795474s → 68.982629s** (+0.2720%) on
practice100; no-rollout 37.757069→38.158567s. No held or default
evaluation was taken for the cache. Do not change live reference.
