# Certified-exterior bottleneck ETA reranking of lastcap cuts (rejected)

Change only small selected-shape ranking in integrated v4's 1–2 remaining
animal cleanup. For candidate cuts with no more walls than available
people, compute every person's distance to each candidate build cell's
adjacent stance on its certified fully cut exterior. Exactly minimize
maximum ETA under distinct worker matching. Feed max(old duration,
1 + bottleneck ETA) into the existing score denominator with identical
weights. It is not a hard viability/deadline rule; geometry, cutoff,
t160/225 handoff, partial legal gate staging and exterior certificate
are unchanged. A nonnegative old ranking bound prunes only shapes that
could not beat the already retained candidate.

Restore v4's standalone, apply `integrated.cpp.patch`, then compile via
`g++ -std=c++23 integrated.cpp -o integrated` without args.
Standalone SHA256
`f0e35cef5727e4ec094d65f6c560d63c00fbd7125811e219de613c12cc141db0`.
For the unit, copy the v1 header and apply v4 header patch, then this
header patch, and `g++ -std=c++23 eta-unit.cpp -o unit && ./unit`.
3,000 random cases checked against permutation oracle; physical
wrong-side fixture unrestricted ETA1 versus certified ETA26.
No policy changes outside this ranking signal.

Official used dev30, exact v4 baseline action recurrence and all60
independent tester/playback legal. v4→v5 866,421,245→846,893,468
(4 wins, 22 ties, 4 losses and 22 identical complete traces); all-clear
13→12, residual45→46, exact4 236→236, irregular4–48 63→61.
Lastcap-owned actual animal isolations11→10 and full clears6→5.
Final hypothetical sterile area overall slightly increased despite fewer
complete isolations. Max tester wall .914→1.014s and medians .644→.644s.
Underestimated assignment time is a measured real defect in the proxy,
but changing this shape ordering alone does not improve executed capture.
No holdout or promotion; v4 remains the research default.
