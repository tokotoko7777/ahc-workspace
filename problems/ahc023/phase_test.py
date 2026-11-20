"""Per-day validation must distinguish same phases from opposite phases."""
from pathlib import Path
from tempfile import TemporaryDirectory

from benchmark import score


def run():
    with TemporaryDirectory() as directory:
        case = Path(directory) / "input.txt"
        case.write_text("5 1 4 0\n000\n4\n2 4\n2 4\n4 5\n1 2\n")
        # A deeper planting is taken before its parent's planting. At the
        # same harvest: remove the parent before reaching the child.
        assert score(case, "2\n1 0 1 2\n2 0 2 2\n") == (300000, 2)
        for output in ("2\n1 0 1 2\n3 0 2 4\n",  # plant before parent harvest
                       "2\n1 0 1 2\n4 0 2 1\n"): # harvest after parent plant
            try:
                score(case, output)
            except ValueError:
                pass
            else:
                raise AssertionError("a planted ancestor was crossed")


if __name__ == "__main__":
    run()
