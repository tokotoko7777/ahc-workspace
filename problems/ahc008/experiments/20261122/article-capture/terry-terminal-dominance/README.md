# Observer-only terminal capture dominance
See [PRESET.md](PRESET.md) for the locked two-cohort test. Use the
already accepted h123 source, separately keep all generated/private
coefficients out of archives.

```
python ahc008_terry_terminal_dominance/make.py ahc008_terry_passive_exact_short/live.private.cpp
g++ -std=c++23 ahc008_terry_terminal_dominance/shadow.private.cpp -o ahc008_terry_terminal_dominance/shadow
timeout 125s python ahc008_terry_terminal_dominance/audit.py \
 ahc008_focus_assets/tools/target/release/tester \
 ahc008_terry_terminal_dominance/shadow ahc008_terry_terminal_dominance/report.json
```

Do not execute the count-only observer as production data collection.
External contestant timeout is 5.5s per selected board, plus official
playback timeout 12s. The whole exclusive audit is capped at 125s.
Expected elapsed for 25 boards is ~75–100s, including replays and
verification; prior measured stock direct summed wall was 67.9s.
Any mismatch/timeout is incomplete, not a rejected or successful
hypothesis. No fresh inputs, practice inputs, or alternate executed.

## Verified result: rejected without a live trial

Both 100-case ledgers and all 200 inherited exact input digests verified.
On all 25 recorded stock Chase choices, the shadow reproduced the entire
300-action digest, official score, playback, exact independent score,
recorded rollout count and exact stock aggregate sums. It observed 1,150
completed pairs. On 24/25 cases Chase predicted strictly more raw and
useful full outcomes and tied on the other case. Zero suggested vetoes,
zero concordant splits, zero vetoes on factual misses. The only factual
stock miss in the selected 25 had 12/24 full/useful in Chase versus 0/24
in Normal, six versus zero in each half. No proposed rule reaches its
gate; no alternate behavior tested. Max diagnostic tester wall 2.7461s.
