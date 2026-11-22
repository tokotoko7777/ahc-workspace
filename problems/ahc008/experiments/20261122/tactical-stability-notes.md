# Tactical gate prep risk-veto ablation

This is only a veto of a move-redirection plan already selected by the late tactical gate solver. It does not change the baseline action, actual same-turn legality check, score evaluation, or terminal-only optimizer. Apply tactical-stability.patch to a scratch copy of the tactical_gate_submission.cpp standalone.

For each pet that can reach a prospective gate or its adjacency within the modeled horizon, the veto propagates a small exact distribution of uniform elementary moves through currently open cells, conditioned on the intended first-turn walls. The product estimates the probability that all pets stay on the predicted side and leave the gate legal to build. Dogs and cats are approximated as unbiased walkers, not modeled with their hidden destinations. This is a sensitivity proxy, not a probabilistic guarantee. After closure, pet motion cannot change the connected-component score.

- -DGATE_STABILITY=55: species' nominal elementary-step count, aggregate stability threshold .55.
- -DGATE_STABILITY=75 -DGATE_EXTRA_STEPS=1: an additional elementary step (capped at four), threshold .75.
- With default flags the veto is off.

Direct pilot on 60 newly generated cases (ahc008/habitat/1/tactical-risk-new/{i}, first 8 SHA-256 bytes little-endian): control 776,924,794; 55% 769,630,350 (−7,294,444); 75% 760,953,189 (−15,971,605). All legal. The pilot is not one of the previously used development sets. No distributed practice or original system cases were used for this ablation.

Independent replay cohort: 100 official-generator seed values from the same SHA conversion of ahc008/20261122/tactical-risk-v1/fresh100/{i}, i=0–99. The official tester and independent recorded-action replay agree, all cases are legal.

| Arm | Total | wins / losses vs control | worst tester wall |
|---|---:|---:|---:|
| Tactical control | 1,630,324,682 | 0 / 0 | 0.527470 s |
| 55% nominal | 1,631,670,362 | 1 / 1 | 0.607570 s |
| 75% stressed | 1,615,373,933 | 1 / 6 | 0.566835 s |

The nominal veto shows a tiny gain on this cohort and a larger loss on the prior pilot; the stressed veto loses both. Neither is promoted. Approximate movement forecast can over-veto opportunities that already have a sound next-observation legality check; forecasting hidden-target pets as unbiased walkers is an additional limitation.
