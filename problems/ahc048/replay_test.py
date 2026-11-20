from benchmark import score
case=b"2 2 2 4 10\n0 0 0\n1 0 0\n0.5 0 0\n0.5 0 0\n"
answer=b"0\n0\n1 1\n1 0 0 0\n1 0 1 1\n2 0 0\n2 0 1\n"
assert score(case,answer)=={"score":1,"pours":2,"ops":4}
for bad in (answer.replace(b"1 0 1 1",b"1 1 1 1"),
            answer[:-6],answer+b"3 0 0\n"):
    try:
        score(case,bad)
    except ValueError:
        continue
    raise AssertionError("accepted invalid paint operations")
