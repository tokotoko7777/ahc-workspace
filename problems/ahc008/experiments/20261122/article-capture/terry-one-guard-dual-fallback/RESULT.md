# Six-cell one-guard fallback exists; four-cell replacement does not

See PRESET and the adjacent one- through four-cell screen. Result is an
enumeration of nominal pockets, not a score comparison. The target
guarantee is both pig starting parities and at least one rabbit start,
each under every possible next-step path of that single animal, with
no exterior animal and a fixed guard at the common stance.

| Body area | Oriented bodies | Candidate gate pairs | Qualifying orientations | Minimum prebuilt walls | Minimum classes under D4 |
|---:|---:|---:|---:|---:|---:|
| 5 | 63 | 376 | 128 | 7 | 2 |
| 6 | 216 | 1,440 | 920 | 8 | 17 |

Two more walls still have to close, on consecutive human actions.
Walls counted here are local before layout sharing or use of the
board edge. These are not complete plans or staffed trajectories.

For the minimum-wall classes, a secondary isolated traffic estimate
solves the small absorbing chain over the body with exact fractions.
Start at an entry body cell at a turn boundary. Exiting to either
gate on any substep ends that episode in failure. At a decision
boundary, a legal guaranteed capture start ends success. Reference
is a straight 1×4 with two stationary outside guards, permitted
to close both mouths in one action. It also uses eight prebuilt
walls as an isolated pocket. No re-entry or exterior/other animals.

| Pocket, entry maximum (over all minimum-wall classes/entries) | Cow | Pig | Rabbit |
|---|---:|---:|---:|
| 1×4 two-guard reference (either entry) | 1/2 | 1/3 | 2/7 |
| Five cells, one guard | 5/12 | 5/19 | 4919/38281 |
| Six cells, one guard | 10/17 | 5/12 | 9927/41693 |

The best six-cell entry attains all three maxima in that same layout,
but its other entry is only 2/17, 1/12, and 2481/41693, respectively.
Thus this does not support replacing the ordinary straight tubes. A
six-cell fallback might be worth embedding near an existing
nine-cell capture to trade one guard for the ordinary two, if it
preserves wall sharing/neighbor access and increases worker area.
Five-cell minimum-wall choices show weaker top entry exposure than
the reference for every passive animal type in this isolated model.

```
python3 audit.py > /tmp/ahc008-fallback.json
diff -u result.json /tmp/ahc008-fallback.json
python3 traffic.py > /tmp/ahc008-fallback-traffic.json
diff -u traffic.json /tmp/ahc008-fallback-traffic.json
```
