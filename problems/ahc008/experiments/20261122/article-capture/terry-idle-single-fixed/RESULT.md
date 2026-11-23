# Fixed-work practice: NO-GO

All 100 paired practice inputs and 698 fixed late simulation pairs were
reproduced from both selected-a237-based arms. Complete action streams and
all 15 telemetry fields (including both virtual sums and selected mode)
were equal, official tester and shared independent playback/score passed,
and historical command/score/telemetry differences were each zero.

Worst measured tester wall was 2.8909675 s, below frozen 2.95 s. Whole
CPU 52.915491 → 53.076911 s (+0.305%, needed at least 2% reduction).
Positive rollout 36.619259 → 36.805691 s (+0.509%). No-rollout 16.296232
→ 16.271220 s (−0.153%, no-regression gate passed). Do not claim whole
solver improvement from the previously proven detached phase savings.

Practice did not pass, so fresh100 was never run. No adjustment, rerun,
native comparison, action selection change, or production integration.
Selected a237 remains unchanged.
