# One stance move before terminal: factual-tape ceiling only
Do not infer a causal outcome or propose blind pre-positioning from this screen. Use only the exact v25-fast2 M5 dev30 baseline official action/reply tapes. Existing 21-flip tapes are a different cohort and not reusable. The script never calls an official tester or solver; if old tapes were not saved root may collect and verify them separately, not substitute divergent digests.

    g++ -std=c++23 ahc008_v25_t298_audit/kernel.cpp -o ahc008_v25_t298_audit/kernel
    python ahc008_v25_t298_audit/audit.py       ahc008_v25_terminal_sets/dev30.csv ahc008_v24_five_retention/m5-dev30/in       TAPES/baseline ahc008_v25_t298_audit/kernel

Tapes per case: 0000.actions exact official stdout and 0000.trace alternating 600 exact lines from recorded-action playback. Input, full action stream, prefix t0–298, official score and legality are validated; all 30 cases must be present. The offline kernel independently validates original/altered t298 commands and exact t299 outcome. It keeps all original walls/moves except for changing one non-building worker to a single different cardinal move. New simultaneous wall feasibility at t299 is measured from that one changed person's position, all factual animal locations from real turn299 and the exact unchanged post-t298 wall mask. Original t299 is independently confirmed optimal on its unmodified state (strict nonregression stock no-op); chosen altered t299 set is exact unbounded offline optimum. Report newly detached IDs, full separation and mean floor 300 on distinct boards, plus strict objective gain. Report all compatible poses and dog-free departure-component poses separately: the affected worker cannot change a dog's reachable human target if no dog begins in that post-wall component. This still uses factual future occupancy, not a causal live decision that could know it. All dog-in-component variants are merely compatible with the recorded next snapshot; their actual animal motion may differ.

## Verified development result

30/30 exact original official input, action, and score transcripts;
independent score and legal replay matched. Evaluate 329 legal
single-worker alternative t298 poses preserving other moves and all
old walls. With factual turn299 animals there are nine strict rational
score gains in four boards, **zero additional whole clears and zero
additional useful whole clears** even in this hindsight screen.
The dog-free departure-component subset has 297 options, eight gains
in three boards, again zero added whole/useful clears. There was no
live t298 policy, new score, or held evaluation. Do not mistake the
static pose improvement for a policy that can know future occupancies.

Result and original-path scripts are bundled in `evidence.tar.gz`.
The previous pure t299 expansion is [separate](../v25-terminal-sets/README.md);
the selected v25-fast2 solver is unchanged.
