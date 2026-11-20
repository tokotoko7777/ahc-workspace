from benchmark import score
case=b"2 2 2\n0 0\n1 0\n0\n0\n00\n"
assert score(case,b"R R\nS S\n0\n")=={"score":11,"turns":1,"covered":4,"complete":True}
assert score(case,b"R S\nS S\n0\n")=={"score":3,"turns":1,"covered":3,"complete":False}
for bad in (b"R R\nS S\n2\n",b"R X\nS S\n0\n",b"R\nS S\n0\n"):
    try:
        score(case,bad)
    except ValueError:
        continue
    raise AssertionError("accepted bad key layout")
