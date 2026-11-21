"""Check that the terminal optimization only changes the final turn."""
from pathlib import Path
import re
import subprocess
import sys

def run(tester: Path, binary: Path, inp: bytes):
    proc = subprocess.run([tester, binary], input=inp, capture_output=True, check=True, timeout=3.0)
    score = re.fullmatch(rb"Score = ([1-9][0-9]*)\s*", proc.stderr)
    assert score, proc.stderr
    actions = proc.stdout.splitlines()
    assert len(actions) == 300
    return actions, int(score[1])

def main():
    tester, case_dir, baseline, terminal = map(lambda x: Path(x).resolve(), sys.argv[1:])
    totals = [0, 0]
    improved = 0
    cases = sorted(case_dir.glob("*.txt"))
    assert cases
    for case in cases:
        case_bytes = case.read_bytes()
        a, va = run(tester, baseline, case_bytes)
        b, vb = run(tester, terminal, case_bytes)
        assert a[:-1] == b[:-1], ("early command divergence", case)
        assert va <= vb, ("score regression", case, va, vb)
        totals[0] += va
        totals[1] += vb
        if a != b:
            improved += 1
            print(case.name, va, vb, a[-1].decode(), b[-1].decode())
    print(f"{len(cases)} identical prefixes, {improved} different final turns; sums = {totals}")

if __name__ == "__main__":
    main()
