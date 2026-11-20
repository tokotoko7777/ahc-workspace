from benchmark import score

inp=b"6 1 11000 5\n0 0 0 5\n"
line=b"0 0 1\n1 0 2\n1 0 3\n0 0 4\n-1\n"
assert score(inp,line)==810
assert score(inp,line.replace(b"1 0 2",b"2 0 2"))==800
for case,out in [(inp.replace(b"11000",b"10100"),line),
                 (inp,line.replace(b"1 0 2",b"1 0 1")),
                 (inp,line[:-3])]:
    try:
        score(case,out)
    except ValueError:
        continue
    raise AssertionError("accepted invalid railway output")
