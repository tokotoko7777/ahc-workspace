from benchmark import score
case=b"3\n2 4 5\n2 1 1\n1 2 3\n1 1 5\n1 1 1\n"
valid=b"-1 0\n-1 0\n0 1\n0 1\n1 2\n"
assert score(case,valid)["score"]==7
for bad in (b"0 1\n"+valid,valid[:-4],valid+b"-1 0\n",
            valid.replace(b"1 2",b"0 2")):
    try:
        score(case,bad)
    except ValueError:
        continue
    raise AssertionError("accepted invalid opening/charge")
