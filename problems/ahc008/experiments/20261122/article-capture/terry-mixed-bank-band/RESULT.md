# Fixed three-crossing-band strip: no accepted witness

Frozen source and stock header hashes matched. The sole Python invocation
exited 1 on the mixed closed-exterior connectivity assertion after the
stock control passed. Geometry, sequence and thresholds were not edited
or rerun. No accepted 57-wall strip, no live construction or score.

Fixed-mask explanation without a new run: in zero-based physical strip
coordinates, row10 x3 is unoccupied planned floor, surrounded by
r10 x2/x4 and r9/r11 x3 wall cells. It is an isolated phantom component
rather than a true common exterior connection. Each surrounding wall
belongs to a named-room shell. The two other three-X seams (r13 and r17)
abut the real band and their free cells are not the explanation. Filling
that single phantom would remove the only nominal wall saving (57 to
58), not solve the strict fewer-wall case. This is a scoped negative for
the frozen layout, not a claim about other mixed strips.
