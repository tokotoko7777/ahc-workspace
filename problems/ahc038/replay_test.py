from benchmark import score

inp=b"5 1 3\n00000\n00000\n00100\n00000\n00000\n00000\n00000\n00010\n00000\n00000\n"
solution=b"3\n0 1\n1 1\n1 1\n.RL..P\nR....P\n"
assert score(inp,solution)=={"score":2,"missing":0}
for bad in (solution.replace(b".RL..P",b".RR..P"),
            solution.replace(b".RL..P",b".RL.P.")):
    try:
        score(inp,bad)
    except ValueError:
        continue
    raise AssertionError("accepted bad arm action")
