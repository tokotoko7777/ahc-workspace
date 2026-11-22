# Deferred square: frozen 100-case validations

Exactly the dev deferred binary and its fixed square sibling, not a new
search. Both run in the same pilot executable with arguments `2 27 1` or
`2 27 0`; each case verifies the identical initial square, gate comparison
metadata and crew. Original tactical is an independent untouched arm.
For all arms, recorded legal output played back under official v3, byte-
identical to the initial action transcript. Independent replay verifies
rounded score and exact-four-cell connected components. No cohort-dependent
source edits, shape selection, watch length or score thresholds.

Cohorts: independent 100 official-generator inputs using archived seeds
(first eight SHA-256 bytes little-endian of
`ahc008/exact4/defer-door/frozen-100-v1/{i}`, 0≤i<100), then the v3's
distributed `tools/in` practice100. Neither set is the contest's hidden
system cases. Comparison builds contain real GNU optimization pragma
and use `g++ -std=c++23`; no extra compile flags.

Binary hashes for this run: tactical
`90e0fc4fd6a39b9556630bfd88a87ebe1e0162d835dde8fa98f9e6b6cb039ae6`,
pilot `ca96e5a4bf4972f140e3a4f1d434d19724ce8baf06a3665165d0f8e1a85c3a91`,
official tester
`f95e04c6adacf82d51352ae1fcd32212ce9ba4d38bfa3e81156ca9580308ebfe`.
Per-case data with official action and input hashes is compressed
(`gzip -cd FILE.csv.gz`). Plain JSON summaries report paired wins/losses.

## Generated100

| Measure | Tactical control | Fixed square | Deferred square |
|---|---:|---:|---:|
| Official score sum | 1,688,074,201 | 1,317,138,009 | 1,579,164,236 |
| Final exact-four-cell pets | 5 | 20 | 22 |
| Final reachable pets | 519 | 565 | 500 |
| All-clear cases | 5 | 6 | 6 |
| Pilot seals / pets | n/a | 16 / 16 | 16 / 16 |
| Pilot multi-wall seals | n/a | 0 | 2 |
| Diverted commands | n/a | 6370 | 4408 |
| Assigned crew moves | n/a | 2837 | 3736 |
| Assigned waits before one gate | n/a | 500 | 127 |
| Maximum tester seconds | .457 | .679 | .517 |

Fixed→deferred casewise wins/losses: final exact-four 14:10, normal
score 58:41, fewer reachable pets 50:34. Versus tactical, exact-four
19:4, normal score 45:53, and fewer reachable pets 46:31.
Final deferred exact-four species cow/pig/rabbit/dog/cat = 13/7/2/0/0.
It improves setup efficiency (~45% more pilot captures per diverted
command at same absolute pilot captures); some released waits are spent
on additional walking because shoulder assignment is no longer fixed.

## Official distributed practice100

| Measure | Tactical control | Fixed square | Deferred square |
|---|---:|---:|---:|
| Official score sum | 1,512,932,629 | 1,250,699,651 | 1,476,533,852 |
| Final exact-four-cell pets | 3 | 32 | 26 |
| Final reachable pets | 509 | 526 | 516 |
| All-clear cases | 10 | 5 | 11 |
| Pilot seals / pets | n/a | 18 / 20 | 17 / 17 |
| Pilot multi-wall seals | n/a | 0 | 1 |
| Diverted commands | n/a | 6520 | 4334 |
| Assigned crew moves | n/a | 2844 | 3622 |
| Assigned waits before one gate | n/a | 599 | 187 |
| Maximum tester seconds | .445 | .524 | .516 |

Fixed→deferred casewise wins/losses: final exact-four 11:19, normal
score 53:46, and fewer reachable pets 45:34. Versus tactical, exact-four
21:2, score 45:55, fewer reachable 40:31 (seven worse in total).
Deferred final exact-four species 16/5/4/0/1. The sole cat in a final
four-cell cage was captured by downstream tactics, not the direct pilot.
The directly captured species were 10/3/4/0/0; fixed pilot 13/2/4/1/0.

## Decision

All 600 official outputs and replays were legal; no timeouts, peak
.679s across all variants of the two cohorts. Deferred control returns
labor to tactics and improves captures per diverted command against
the dedicated fixed gate by 45% on generated, 28% on practice, despite
higher walking. Its capture **count** relative to fixed is not robust
(final +2 on generated, −6 on practice; direct 0 then −3). Both shell
variants have more compact animals than tactical alone, but neither
wins normal official score or approaches capturing most animals. Do
not promote either. The original score-leading implementation remains
unchanged.

Use the commands in quad-deferred-notes.md with `--case-count 100` and
the two distinct input directories. Run `python quad_deferred_report.py
results.csv` for the exact summary and per-case pair totals.
