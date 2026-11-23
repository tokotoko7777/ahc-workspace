# a237 terminal virtual common-event variance diagnostic

Frozen exactly 22 existing complete, authenticated a237 tapes with positive
historical fixed pair cap. Deduplicated union of both archived Map1 and
actual-Chase late-large-capture selections, from both official-distributed
practice (10: 8 historical Chase, 2 Normal) and independent fresh (12:
12 historical Chase). Fixture identity, archived panels, submitted bytes and
receipt hashes pinned before any virtual result. This cohort is heavily
mode- and geometry-biased, partly outcome-selected upstream; no new
outcome selection and no representativeness or broad-score claims.
The Chase Hall snapshots without prefix motion/crew state cannot be used.

Detached stock prefix reconstruction must reproduce all 275 real human
actions, then the original decision's fixed historical pair cap and exact
both score sums. All 22 pass before any experimental arm, or stop. No
official tester or external code execution. All modes preserve 25-turn
Normal and stock Chase absorption including fixed planner turn275, exact
stock Normal controller PCG and in-arm crew progression, same independent
initial clone per method; no native time stop. Judge motion conditional
kernel, uniform accessible ID/cell/direction and order, reset and no-target
laws are unchanged. Exact stock rejection sampler is reused.

S is stock same-seeded global ChaCha. K uses a keyed uniform-word abstraction:
virtual trial seed, simulated turn, original pet index, semantic event
(target pick, directed substep, each passive hop, dog fallback as separate
slot), and local rejection retry ordinal. No branch tag in K, and no virtual
event feeds Normal controller RNG. I is two independently seeded stock-model
streams (Normal is exactly same seed as S, Chase uses fixed domain salt).
Fixed same original 16 trial seeds/method, no adaptive cap. Score-variance
comparison concerns D=(Chase−Normal)/100,000,000, a common denominator on
all boards; per-board sample Var(D) and covariance, and raw marginal means,
variance, terminal full counts. Zero/zero variance ratio counts as 1 and
positive/zero as infinity. Synthetic checks before any board: exhaustive
reduced-width rejection equal preimage counts, isolated retry slots, actual
equal-state position/latent consistency, approximately uniform interior
four-neighbor for 32,768 fixed keys. Exact marginals are kernel-level
conditional under the uniform-word abstraction, not bit equivalence or
formal PRNG independence. Conditional stock crew progression across pairs
may impair IID inference; these are sample variance diagnostics, not CIs.
Timing from serial internal CPU, cyclic balanced permutations across cases,
summing only modeled arm work, not prefix reconstruction/authentication.

Prespecified pass: at least 20 fully authenticated states; median K/S
variance ratio <=.9; absolute pooled raw mean drift <=2m separately Normal
and Chase; total K/S CPU <=1.10. Independence control descriptive. No live
adoption, official score, threshold change, extra seed block or automatic
second tuning even if positive; further study would need a broadly sampled
new cohort and independent review.

Expected serial work: 1184 historical authenticating pairs and 1056
experimental pairs; approximately 35–90 sec including compile, conservative
two minutes. Use explicit CPU clearance and never overlap other timings.

    D=ahc008_terry_virtual_crn
    (cd "$D" && sha256sum -c FROZEN.sha256)
    python "$D/make.py"
    g++ -std=c++23 -O2 "$D/unit.cpp" -o "$D/unit"
    g++ -std=c++23 -O2 "$D/observer.private.cpp" -o "$D/observer"
    "$D/unit" && "$D/observer" unit
    python "$D/run.py" "$D/observer" > "$D/result.json"

Keep the locally assembled observer and traces private.
