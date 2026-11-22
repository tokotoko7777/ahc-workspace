# Exact virtual cat memo: NO-GO

Sole detached measured run, frozen signatures in ANCHORS.sha256 and
FIXTURES.sha256. No official evaluation, no solver score and no rerun.

All hashes and signed stock replays matched. Both implementations matched
all pet coordinates/types/latent goals and all ChaCha fields on each of
14,000 turns: 400 timelines from the 100 signed factual t275 anchors
(376 cat / 24 no cat) and 160 independent synthetic trajectories. Explicit
singleton, wall/disconnected old goal, both arrival substeps and live
epoch invalidation boundaries passed.

| Complete motion aggregate CPU, eight order-alternating passes | Stock | Memo | Reduction |
|---|---:|---:|---:|
| Factual anchors with cats | 1.3852443s | 1.3138856s | 5.15% |
| Factual anchors without cats | 0.0862599s | 0.0798318s | 7.45% |
| All factual anchors | 1.4715042s | 1.3937174s | 5.29% |
| Independent synthetic | 0.9045222s | 0.6096217s | 32.60% |

Observed real cat turns 30,500, redraws 20,270, cache row hits 15,477,
physical topology changes 289, component index builds 603. Factual
component topology differs from synthetic: near-closed cages likely
make many originally local BFS expansions cheap while the replacement
index floods the entire open topology when cold. This is an explanation
consistent with counts, not a separate timed attribution. The small
no-cat total is reported, but no meaningful claim of improved no-cat
performance follows from such a short timing base.

Factual cat and overall CPU reductions miss the frozen 20% and 8% gates.
Do not integrate, rerun, split this aggregate retrospectively, or infer
native-clock score from its synthetic speedup. Only independent own
cat memo code and this report are suitable for external sharing;
stock source, borrowed input data and generated solver/binary remain local.
