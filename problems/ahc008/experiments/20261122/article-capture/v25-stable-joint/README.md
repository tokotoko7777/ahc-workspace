# Stable joint rescue
Build from the original joint source:
```
python ahc008_v25_stable_joint/make.py ahc008_v25_joint_rescue/solve.cpp ahc008_v25_stable_joint/solve.cpp
cd ahc008_v25_stable_joint
g++ -std=c++23 solve.cpp -o solve
g++ -std=c++23 unit.cpp -o unit && ./unit
```
The original selection/first accepted action is unchanged when not resuming. A committed mission keeps exactly the chosen physical pocket wall/inside mask and first complementary captured-by-projection animal ID, rail animal, tube and ordered two rail workers (hence the same three pocket workers). Continuation requires the identical rail hypothesis still passes the original rail checks, the fixed complementary pet still actually falls in the certified pocket, and all actual proposed motion remains in the certified projected pocket exterior. The selected three-person proposal must contain a legal actual pocket wall command or a demonstrated short walk toward one currently legal missing wall; after more than two turns without observing an installed issued pocket wall, the mission is retired. Installation is checked on the next board because downstream action priority may override the proposal. Exact final connectivity/area and legality checks from joint still run. No continuation after an accepted immediate rail closure, a skipped hook turn, changed preconditions, expiry t205, or any invalidation (a failed continuation returns to ordinary rescue that turn, rather than reselect a new joint pocket). No change to higher-priority branches. These are conservative checks, not guaranteed animal residence or future legality. Env `AHC008_STABLE_JOINT_LOG` logs number of accepted continuations, not physical closes. Flag `--no-stable-joint` restores original joint branch logic, with possible wall-clock effects; no official timing here.

## Verified benchmark

Paired fixed M5 dev30 against the non-adopted joint parent, not against v25.
All60 runs legal and official/independent replay agree. Score
603,466,101 → 563,500,217, whole 13 → 14, whole with ≥300 cells stays 5,
residual stays 55. Four-cell physical isolation 207 → 202, t225 residue
202 → 210, t275 residue 148 → 161. Nine action histories changed, max
local tester 0.680 s. Score fell and useful full did not improve;
no held evaluation or adoption. The selected baseline remains v25.
See [the parent joint rescue checkpoint](../v25-joint-rescue/README.md).
