from benchmark import score
inp=b"2 2\n.#\n#.\n"
assert score(inp,b"0 0\n1 1\n")==500000
assert score(inp,b"1 1\n0 0\n")==500000
for x in (b"0 0\n0 0\n",b"0 1\n1 1\n",b"0 0\n"):
    try:
        score(inp,x)
    except ValueError:
        continue
    raise AssertionError("illegal placement accepted")
