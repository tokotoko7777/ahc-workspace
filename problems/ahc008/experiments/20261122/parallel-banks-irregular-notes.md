# Keep paired tube banks; shape a few late cages from real local walls

The user-approved references were [TERRY’s AHC008 write-up](https://blog.terry-u16.net/entry/ahc008),
which describes late pet-side/staff-side vertex cuts and assigning builders,
and [simanman’s AHC008 write-up](https://simanman.hatenablog.com/entry/2022/02/28/074838),
which discusses species-dependent capture and irregular fences as a possibility;
its irregular score observation does not establish a broad benchmark.
This implementation uses the existing independent bank/pilot code and existing
MaxFlow library, not either entrant's implementation or submitted code.

Freeze the stable staged3 zoning controller and its normal t225 fallback.
The two variants only touch late actions. Local vertex-split mincut within
Manhattan radius five around an actual reachable pet uses current walls
for free, forbids splitting service aisle cells or human occupied positions,
and proposes at most three additional wall cells. Require a truly connected,
non-rectangular area4–24 after closure, exact distinct exterior builder
assignment, no replacement of proposed ordinary wall actions, retain normal
connectedness of every human group, and positive true current score proxy.
All geometry, guards and previous 270 direct bank pets remain identical
on this 30-case used development cohort.

- **Immediate**: only use humans already adjacent, at most two normal
  movement commands replaced. All 30 full action transcripts unchanged
  (score 541,987,952, final residual116, safe union4144 distinct case
  cells summed). Out of 5,418 actual local mincut solves, 5,416 lack
  a finite cheap candidate below cutoff401; the two passing shapes are
  rectangular. This only tests the chosen mincut objective/radius/constraints,
  not exhaustive irregular geometry. Deterministic fixture verifies
  a supported five-cell L closed by two actual builders, and a pet blocking
  either gate vetoes.
- **Short assigned setup**: try one live plan at a time around visible
  cow/pig/rabbit, assign distinct outside builders at ≤3 walking steps,
  preserve ordinary wall actions, follow only while actual target stays
  in proposed pocket. At most five turns after start; no added wall until
  true commit rechecks exact legal door conditions, actual component
  4–24 nonrectangular cells, human groups, score. Two starts, one expires,
  one seals. 29/30 whole action hashes remain unchanged. On 0027, t249
  assigns the parapet, t252 builds two gates zero-based (12,23),(17,24),
  and a rabbit is truly isolated at (13,23),(14,23),(15,23),(16,23),
  (17,22),(17,23). Legal actual score there 4,506,944→4,555,556;
  overall 541,987,952→542,036,564, final residual116→116, safe union
  unchanged. Maximum wall .568s reference/.595s short; every replay legal.

No promotion or heldout update; one live positive cage demonstrates the
mechanism but not systematic endgame benefit. Optimistic current readiness
does not create body/guard co-occurrence at scale. Keep alternative work
separate from the early parallel capture stage.

Apply parallel-banks-pilot.patch, parallel-banks-staged.patch,
parallel-banks-zone.patch and parallel-banks-irregular.patch for immediate.
Then apply parallel-banks-irregular-short.patch for short. Build in GNU
C++23. Run matching Python files passing harness, official tester, tactical,
pilot binary, old dev30 input folder and output CSV. Mode `5 0` control,
`5 1` immediate, `5 2` short. Fixture include expects assembled
`parallel_banks_irregular.cpp` via include directory. For immediate
screening numbers, set `AHC008_IRREGULAR_DIAG` to a distinct scratch
sidecar for each case; compare action digests to the archived match.
