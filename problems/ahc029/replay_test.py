"""Check the independent simulator on a short, hand-calculated game."""
from pathlib import Path
from tempfile import TemporaryDirectory

from benchmark import score


def run():
    with TemporaryDirectory() as d:
        case = Path(d) / "case.txt"
        case.write_text(
            "2 2 2 3\n"
            "3 7\n5 11\n"             # initial projects
            "1 20\n5 8\n4 9\n4 9\n4 9\n4 9\n"
            "4 0\n1 5\n"              # initial cards
            "0 1 0\n0 20 99\n"        # only free card is legal here
            "0 1 0\n0 3 3\n"
            "0 1 0\n0 4 4\n"
        )
        # Investment doubles only future offers and replacement projects.
        legal = "0 0\n0\n1 0\n1\n1 0\n0\n"
        assert score(case, legal) == (52, 1, 2, 2)
        for wrong in (legal.replace("1 0", "1 1", 1),
                      legal.replace("0 0\n0", "0 0\n1", 1),
                      legal + "0\n"):
            try:
                score(case, wrong)
            except ValueError:
                pass
            else:
                raise AssertionError("invalid protocol unexpectedly accepted")


if __name__ == "__main__":
    run()
