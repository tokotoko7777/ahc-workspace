"""Small paired synthetic-only smoke; all grids and reads are generated here."""
import random
import subprocess
import sys
import time
from pathlib import Path


def example(length, count, seed):
    rng = random.Random(seed)
    board = [[rng.choice("ABCDEFGH") for _ in range(20)] for _ in range(20)]
    reads = []
    for _ in range(count):
        r, c, d = rng.randrange(20), rng.randrange(20), rng.randrange(2)
        n = rng.randint(length - 2, length + 2)
        reads.append("".join(board[(r + d * j) % 20][(c + (1-d) * j) % 20]
                             for j in range(n)))
    return reads


def evaluate(rows, reads):
    assert len(rows) == 20 and all(len(s)==20 and set(s)<=set(".ABCDEFGH") for s in rows)
    lines = [s+s for s in rows + ["".join(c) for c in zip(*rows)]]
    return sum(any(w in line for line in lines) for w in reads)


def main():
    exes = [str(Path(x).resolve()) for x in sys.argv[1:3]]
    offset = int(sys.argv[3]) if len(sys.argv)>3 else 0
    configs = [(length,count,offset+seed*173+391) for seed,(length,count) in enumerate(
        [(6,690),(7,580),(7,725),(8,425),(8,510),(8,690),(8,785),
         (9,410),(9,450),(9,495),(9,535),(9,610),(9,720),(9,815)])]
    for i, (length, count, seed) in enumerate(configs):
        if len(sys.argv)>4 and i not in {int(x) for x in sys.argv[4:]}:
            continue
        reads = example(length,count,seed)
        results = {}
        for j in [i % 2, 1-i%2]:
            at = time.monotonic()
            run = subprocess.run([exes[j]], input=f"20 {count}\n"+"\n".join(reads)+"\n",
                                 text=True,capture_output=True,timeout=3,check=True)
            sec = time.monotonic()-at
            stats = next((s for s in run.stderr.splitlines() if s.startswith("strip_tested=")), "-")
            rows = run.stdout.splitlines()
            covered = evaluate(rows, reads)
            blanks = sum(row.count(".") for row in rows)
            # All values reported on synthetic input, rounded like the published formula.
            score = (covered * 100_000_000 * 2 + count) // (2 * count)
            if covered == count:
                denominator = 800-blanks
                score = (160_000_000_000 + denominator) // (2*denominator)
            results[j] = (covered, blanks, score, round(sec,6), stats)
        print(length,count,seed,results[0],results[1],flush=True)


if __name__=="__main__":
    main()
