# First Normal action reuse with empty construction queues: reject
Pinned the admitted split-bound solver. Cache the first Normal
simulated command only if all construction queues are empty, and
restore the exact forecast RNG state from after that command. 52
fixtures agreed on pairs, forecast decisions, queues, post-call RNG
and the next actual command even with different exact topology cache
keys beforehand. All distributed100 fixed-work official action streams
and scores match. Total solver CPU 53.933230 → 53.736581 s (−0.365%).
Positive-rollout14 CPU 37.164975 → 36.966448 s (−0.534%), below the
frozen 2% gate. Max tester wall 2.7524 → 2.7484s. No held or native
test; leave accepted solver unchanged. Reproduce with make.py bound
and run_fixed.sh, which deliberately exits nonzero on a failed gate.
