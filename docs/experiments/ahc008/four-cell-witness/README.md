# AHC008: exact four-cell capture witness for every species

With 1-based coordinates, the pen is `{(1,1),(1,2),(2,1),(2,2)}`.
The exterior boundary is four squares, `(1,3),(2,3),(3,1),(3,2)`.
For a pet at `(1,1)`, all four pending walls are distance at least two.
Four humans at `(1,4),(2,4),(4,1),(4,2)` simultaneously play `lluu`.
There is no change in the rule check depending on the species. All four
inside squares connect; no human is inside. The remaining 892 open squares
form one human-side connected component.

This fixture is deliberately set up. It proves exact area, legality and
identical capture mechanics in all five species; it does **not** establish
how often you can steer a pet to this position or staff four builders
on official-distribution cases. Closing uses actual pet coordinates and
matching of current distinct builders, not the input seed or species.
The policy then continues legal 300-turn interaction.

Reproduction (run from this archive folder, pointing to a built official v3 tester):

```sh
python witness_inputs.py inputs
g++ -std=c++23 atomic_corner.cpp -o atomic_corner
for f in inputs/*.txt; do /absolute/path/tester ./atomic_corner < "$f" > "$f.actions"; done
```

Official tester and independent transcript replay agreed on all five.
Every result had topology `[[5 humans,892 cells,9 pets],[0 humans,4 cells,1 pet]]`.
