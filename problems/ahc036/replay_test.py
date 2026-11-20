from benchmark import score

inp=b"4 4 3 4 2\n0 1\n1 2\n2 3\n3 0\n1 2 3\n"
valid=b"1 2 3 0\ns 2 0 0\nm 1\nm 2\ns 1 2 0\nm 3\n"
assert score(inp,valid)=={"score":2,"moves":3}
for bad in (valid.replace(b"s 2 0 0",b"s 2 0 1"),
            valid.replace(b"s 1 2 0",b"s 1 0 0"),
            valid.replace(b"m 1",b"m 2"),valid[:-4]):
    try:
        score(inp,bad)
    except ValueError:
        continue
    raise AssertionError("accepted bad signal plan")
