from benchmark import score


def check():
    # Coincident sides count once, while shifted partial sides are XORed.
    case = b"4 2 2\n4 9\n5 1\n"
    same = b"0 0 2 4\n2 0 4 4\n0 0 2 4\n2 0 4 4\n"
    assert score(case,same) == 101
    changed = b"0 0 2 4\n2 0 4 4\n0 0 2 3\n2 0 4 2\n"
    # Lost horizontal length 1, two new vertical lengths of 2 each.
    assert score(case,changed) == 106
    for bad in [same+b"1", same.replace(b"2 0 4 4",b"1 0 4 4")]:
        try:
            score(case,bad)
        except ValueError:
            pass
        else:
            raise AssertionError("invalid assignment accepted")


if __name__ == "__main__":
    check()
