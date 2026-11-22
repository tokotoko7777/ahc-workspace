# Rejected: bounded multi-source reach for Chase

From the selected faster initial-construction solver, replace only
the repeated crew start flood and member-distance scans in Chase
shape by one truncated multi-source graph BFS. Same occupied mask,
bounded minimum distances, cut-arc order and capacities. Synthetic
blocked/duplicate starting positions and empty/radius-zero cases
were checked. 10,054 reach masks and 220 complete flow shapes and
ordered arcs matched independent original fixtures.

Practice100 fixed-work official: all 200 complete arms match the
stored independently validated action streams and scores. Combined
vs split-only, summed CPU all cases 48.164885→47.849331 s (−0.655%),
positive-rollout 12 cases 27.896050→27.569434 (−1.171%), no-rollout
88 20.268835→20.279897. Maximum tester wall2.69145→2.68892s.
Reject against the predeclared ≥2.0% positive-rollout cut. This does
not imply the reachable-mask result is approximate or never useful
on a different input scale. No held/default test or adoption.
Candidate source SHA-256
`ed954da6b57e4b0084d2993499b86394edb42ae5cb426d0cc3753f60c8ce6426`.

Rebuild with the accepted split-DP predecessor (pinned) and run the
source transform, standalone/unit build and normal fixed-work harness.
Archive includes the helper, fixture and logs. No local C++ containing
private selector data or executable is included.
