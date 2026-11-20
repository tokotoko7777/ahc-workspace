from benchmark import score

case=b"4 2 1 3 0\n3 1\n0 0 0 0\n3 3 4 4\n3 3 6 6\n4 4 6 6\n0 0\n3 4\n3 6\n4 6\n"
answer=b"? 3 0 1 2\n!\n0 1 2\n0 1\n1 2\n3\n"
result=score(case,answer)
assert result["score"]==7 and result["queries"]==1
for bad in (answer.replace(b"? 3 0 1 2",b"? 3 0 1 1"),
            answer.replace(b"1 2\n3",b"0 1\n3"),
            answer[:-2]+b"0\n",b"? 2 0 3\n"+answer):
    try:
        score(case,bad)
    except ValueError:
        continue
    raise AssertionError("accepted invalid query or grouping")
