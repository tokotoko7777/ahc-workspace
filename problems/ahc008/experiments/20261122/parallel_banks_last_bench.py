"""Paired rescue/last-animal dev comparison: official tester and exact replay."""
import argparse, csv, hashlib, re, sys, tempfile
from pathlib import Path

from parallel_banks_rescue_bench import count
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL, B, idx, flood


def one(state):
    ps, hs, ws = state
    open_ = ALL ^ sum(B[idx(w)] for w in ws)
    h = flood(B[idx(hs[0])], open_)
    yes = all(B[idx(p)] & h for p in hs)
    reachable = [i for i, p in enumerate(ps) if B[idx(p)] & h]
    return reachable[0] if yes and len(reachable) == 1 else None


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for n in ('harness', 'tester', 'baseline', 'variant', 'cases', 'output'):
        p.add_argument(n, type=Path)
    p.add_argument('--case-count', type=int, default=30)
    a = p.parse_args()
    sys.path.insert(0, str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    out = []
    for f in sorted(a.cases.glob('*.txt'))[:a.case_count]:
        case = f.read_bytes()
        originals = []
        for model, cmd in [
            ('rescue', [a.baseline.resolve()]),
            ('last', [a.variant.resolve(), '5', '1', '1']),
        ]:
            with tempfile.TemporaryDirectory() as t:
                temp = Path(t)
                meta, rescue, event = (temp / s for s in ['meta', 'rescue', 'event'])
                cmd = [a.tester.resolve(), 'env', f'AHC008_BANK_LOG={meta}',
                       f'AHC008_RESCUE_LOG={rescue}', f'AHC008_LAST_LOG={event}', *cmd]
                act, err, rc, sec = run(cmd, case, 3.)
                score_match = re.fullmatch(rb'Score = (\d+)\s*', err)
                assert rc == 0 and score_match, (f.name, model, err)
                target, trace = temp / 'act', temp / 'trace'
                target.write_bytes(act)
                ack, err2, rc2, _ = run([a.tester.resolve(), sys.executable, a.harness.resolve() /
                                         'benchmark_cases.py', '--playback', target, trace], case, 10.)
                assert (ack, err2, rc2) == (act, err, 0)
                actions = act.decode().splitlines()
                lines = trace.read_text().splitlines()
                score, *_ = replay(case.decode(), lines, actions, detailed=True)
                assert score == int(score_match[1])
                kinds, states, _ = observed(case, lines)
                n160, release = map(int, rescue.read_text().split())
                rotshift, *geo = meta.read_text().splitlines()
                rot, shift, _ = map(int, rotshift.split())
                summary = count(states,kinds, '\n'.join([f'{rot} {shift} {release}', *geo]))
                first = next(((j, i) for j in range(max(185, release),299)
                              if (i := one(states[j])) is not None), (-1, -1))
                row = {k: summary[k] for k in ['final_safe_union', 'final_mean_human_area',
                       'final_residual_pets','bank_capture4'] + [f'final4_{i}' for i in range(1,6)]}
                row.update(case=f.name, model=model, score=score, time=sec, release=release, residual160=n160,
                           actions_sha256=hashlib.sha256(act).hexdigest(), input_sha256=hashlib.sha256(case).hexdigest(),
                           first_one=first[0], first_species=kinds[first[1]] if first[1]>=0 else 0,
                           all_clear=int(summary['final_residual_pets']==0), events='', scans=0, candidates=0,
                           closed_size=0, closed_species=0, first_changed=-1, final_pen_of_plan=0)
                if model == 'last':
                    original, before = originals
                    assert (n160, release, first, actions[:max(185,release)]) == (
                        before['residual160'], before['release'], before['first'], original[:max(185, release)])
                    ev = event.read_text().splitlines()
                    row['events'] = ';'.join(ev[:-1])
                    _, scans, candidates = ev[-1].split()
                    row.update(scans=int(scans), candidates=int(candidates))
                    first_changed = next((i for i, (x,y) in enumerate(zip(original,actions)) if x!=y), -1)
                    row['first_changed'] = first_changed
                    plan = [l.split() for l in ev if l.startswith('plan ')]
                    assert len(plan)<=1
                    if plan:
                        target_id=int(plan[0][2])
                        assert one(states[int(plan[0][1])]) == target_id
                        assert first_changed==-1 or int(plan[0][1]) <= first_changed
                        ps,hs,ws=states[-1]
                        grid=ALL ^ sum(B[idx(w)] for w in ws)
                        pen=flood(B[idx(ps[target_id])],grid)
                        if not any(B[idx(h)] & pen for h in hs):
                            row['final_pen_of_plan']=pen.bit_count()
                    else:
                        assert first_changed == -1
                    for l in ev:
                        label, *vs = l.split()
                        if label != 'close':
                            continue
                        ti,pi,sp,_,*walljobs=map(int,vs)
                        assert kinds[pi]==sp and one(states[ti])==pi
                        ps,hs,ws=states[ti+1]
                        grid=ALL ^ sum(B[idx(w)] for w in ws)
                        pen=flood(B[idx(ps[pi])],grid)
                        outside=flood(B[idx(hs[0])],grid)
                        assert 4<=pen.bit_count()<=48 and not (pen&outside)
                        assert all(B[idx(h)]&outside for h in hs)
                        assert all(not B[k]&grid for k in walljobs)
                        row.update(closed_size=pen.bit_count(), closed_species=sp)
                else:
                    originals=[actions,{**row,'first':first}]
                out.append(row)
        with a.output.open('w') as z:
            w=csv.DictWriter(z,list(out[0]),lineterminator='\n')
            w.writeheader()
            w.writerows(out)
        print(f.name,flush=True)

if __name__=='__main__':
    main()
