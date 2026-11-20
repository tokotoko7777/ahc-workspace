from benchmark import score

inp=b"3 2 2\n5 2 1\n0 1\n1 2\n"
good=b"-1 0 1\n"
assert score(inp,good)==13
for test,out in ((inp,b"1 0 -1"),(inp,b"-1 0 0"),(b"3 2 1"+inp[5:],good)):
    try:
        score(test,out)
    except ValueError:
        continue
    raise AssertionError("accepted invalid forest")
