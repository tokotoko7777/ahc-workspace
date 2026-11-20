import argparse
import csv
import json
from pathlib import Path
import re
import subprocess
import time

def score_text(case, route):
    nums = list(map(int, case.split()))
    assert len(nums) == 5002, 'wrong input length'
    x, y = nums[:2]
    tiles, values = nums[2:2502], nums[2502:]
    seen = {tiles[50*x+y]}
    score = values[50*x+y]
    for ch in route.strip():
        if ch == 'U': x -= 1
        elif ch == 'D': x += 1
        elif ch == 'L': y -= 1
        elif ch == 'R': y += 1
        else: raise ValueError('bad direction')
        if not (0 <= x < 50 and 0 <= y < 50):
            raise ValueError('out of bounds')
        i = 50*x+y
        if tiles[i] in seen:
            raise ValueError('duplicate tile')
        score += values[i]
        seen.add(tiles[i])
    return score

def bench(inputs, out, executables, vis):
    out.mkdir(parents=True, exist_ok=False)
    record = []
    with (out/'results.csv').open('w', newline='') as log:
        w = csv.writer(log)
        w.writerow(('case', 'solver', 'score', 'elapsed_s', 'status'))
        for i, case in enumerate(sorted(inputs.glob('*.txt'))):
            case_text = case.read_text()
            for label, binary in executables[i%2:] + executables[:i%2]:
                start = time.perf_counter()
                try:
                    r = subprocess.run([str(binary)], input=case_text, text=True,
                                       capture_output=True, timeout=2.0, check=True)
                    elapsed = time.perf_counter() - start
                    if elapsed > 2.0:
                        raise RuntimeError('wall clock over 2.0 seconds')
                    own = score_text(case_text, r.stdout)
                    output = out/f'{case.stem}-{label}.txt'
                    output.write_text(r.stdout)
                    (out/f'{case.stem}-{label}.log').write_text(r.stderr)
                    official = subprocess.check_output([str(vis), str(case), str(output)], cwd=out, text=True)
                    m = re.fullmatch(r'Score = (\d+)\s*', official)
                    if not m or int(m[1]) != own:
                        raise ValueError(f'Official score mismatch: {official!r}')
                    row = (case.stem, label, own, elapsed, 'ok')
                except (ValueError, RuntimeError, subprocess.SubprocessError) as e:
                    row = (case.stem, label, 0, time.perf_counter()-start, str(e))
                record.append(row)
                w.writerow(row); log.flush()
    (out/'summary.json').write_text(json.dumps({
        k: {'cases': len(rows), 'valid': sum(r[4]=='ok' for r in rows),
            'score_sum': sum(r[2] for r in rows), 'max_wall_s': max(r[3] for r in rows)}
        for k, _ in executables if (rows := [r for r in record if r[1] == k])}, indent=2))

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    for name in ('inputs','out','baseline','candidate','vis'):
        p.add_argument(name,type=Path)
    a = p.parse_args()
    bench(a.inputs.resolve(), a.out.resolve(),
          [('v10',a.baseline.resolve()),('short',a.candidate.resolve())], a.vis.resolve())
