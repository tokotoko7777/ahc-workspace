"""Read-only replay of exactly the held raw flips and useful-threshold flips."""
import argparse
import csv
import hashlib
import itertools
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
H = ROOT / 'ahc008_focus_solver/problems/ahc008'
sys.path.insert(0, str(H))
from benchmark import replay

MOVES = dict(zip('UDLR',((-1,0),(1,0),(0,-1),(0,1))))
BOARD = {(x,y) for x in range(1,31) for y in range(1,31)}
def step(q, d):
    a,b = MOVES[d.upper()]
    return q[0]+a, q[1]+b
ADJ = {q:tuple(step(q,d) for d in MOVES if step(q,d) in BOARD) for q in BOARD}


def load_ledger(path):
    out = {}
    for row in csv.DictReader(path.open()):
        out.setdefault(row['case'], {})[row['model']] = row
    assert len(out) == 100
    return out


def selected(ledger):
    for case, ar in sorted(ledger.items()):
        a,b = ar['baseline'], ar['article']
        assert a['input_sha256'] == b['input_sha256']
        raw = [int(r['all_clear']) for r in (a,b)]
        useful = [bool(c and float(r['mean_human_floor']) >= 300)
                  for c,r in zip(raw,(a,b))]
        if raw[0] != raw[1] or useful[0] != useful[1]:
            yield case, ar, raw, useful


def read_history(raw, trace):
    s = iter(raw.decode().splitlines())
    animals = [tuple(map(int,next(s).split())) for _ in range(int(next(s)))]
    p = [a[:2] for a in animals]
    h = [tuple(map(int,next(s).split())) for _ in range(int(next(s)))]
    w = set()
    hist = [(frozenset(w),tuple(p),tuple(h))]
    for actions, motions in zip(trace[::2],trace[1::2],strict=True):
        for i,a in enumerate(actions):
            if a.islower():
                w.add(step(h[i],a))
            elif a.isupper():
                h[i] = step(h[i],a)
        for i,replies in enumerate(motions.split()):
            for r in replies:
                if r!='.':
                    p[i] = step(p[i],r)
        hist.append((frozenset(w),tuple(p),tuple(h)))
    assert len(hist) == 301
    return hist


def distances(q, walls):
    assert q in BOARD and q not in walls
    result = {q:0}
    queue = [q]
    for u in queue:
        for v in ADJ[u]:
            if v not in walls and v not in result:
                result[v] = result[u]+1
                queue.append(v)
    return result


def stats(w,p,h, dists=False):
    if (set(p)|set(h)) & w:
        return None
    components = []
    assigned = {}
    for q in h:
        if q not in assigned:
            c = distances(q,w)
            for u in c:
                assigned[u] = len(components)
            components.append(c)
    area = [len(components[assigned[q]]) for q in h]
    counts = [sum(assigned.get(z)==assigned[q] for z in p) for q in h]
    score = 0.
    for a,k in zip(area,counts):
        score += (a/900.) * 2**-k
    from math import modf
    frac, whole = modf(1e8 * score / len(h))
    res = dict(people_components=len(components), human_union=len(assigned),
               people_area=area, mean_floor=sum(area)/len(area),
               residual=[i for i,q in enumerate(p) if q in assigned],
               snapshot_score=int(whole)+(frac>=.5))
    return (res,assigned) if dists else res


def match_eta(rows):
    # Each target gets a distinct real worker. Build execution costs one further turn.
    dp = {0:0}
    for row in rows:
        nxt = {}
        for mask, worst in dp.items():
            for i, eta in enumerate(row):
                if eta is not None and not mask>>i&1:
                    m = mask|1<<i
                    nxt[m] = min(nxt.get(m,901),max(worst,eta))
        dp = nxt
    return min(dp.values(),default=None)


def transfer(w,p,h,mandate,ids):
    prior = stats(w,p,h)
    if prior is None:
        return dict(valid_environment=False, people_on_old_wall=sorted(set(h)&w),
                    pets_on_old_wall=sorted(set(p)&w))
    pending = mandate-w
    on_h, on_p = pending&set(h), pending&set(p)
    near_p = sorted(g for g in pending if any(z in ADJ[g] for z in p))
    out = dict(valid_environment=True, pre=prior, pending=sorted(pending),
               people_on_new_wall=sorted(on_h), pets_on_new_wall=sorted(on_p),
               new_wall_adjacent_pet=near_p)
    if on_h or on_p:
        out.update(valid_projected_environment=False)
        return out
    final_walls = w|mandate
    after, exterior = stats(final_walls,p,h,dists=True)
    exact = sorted(set(ids)-set(after['residual']))
    newly = sorted(set(ids)&set(prior['residual'])-set(after['residual']))
    all_pet_legal = not near_p
    matrix = []
    walks = [distances(q,final_walls) for q in h] if pending else []
    for g in sorted(pending):
        stances = set(ADJ[g]) & exterior.keys()
        matrix.append([min((d[u] for u in stances if u in d),default=None) for d in walks])
    worst = match_eta(matrix)
    out.update(valid_projected_environment=True,post=after,detached_ids=exact,
               newly_detached_ids=newly, reproduces_ids=len(exact)==len(ids),
               causally_new_for_all_ids=len(newly)==len(ids),
               connected_reproduction=len(exact)==len(ids) and after['people_components']==1,
               useful_full=not after['residual'] and after['mean_floor']>=300,
               all_new_wall_cells_legal_now=all_pet_legal,
               distinct_stance_eta_matrix=matrix, distinct_max_walk_eta=worst,
               ready_legal_simultaneous=worst==0 and all_pet_legal and bool(pending))
    return out


def verified(case, inp, model, row, tapes):
    # Independent parser checks each real command and pet substep against walls,
    # and independently reproduces official Rust float-operation score.
    raw = (inp/case).read_bytes()
    assert hashlib.sha256(raw).hexdigest()==row['input_sha256']
    prefix = tapes/model/case
    actions = prefix.with_suffix('.actions').read_bytes()
    assert hashlib.sha256(actions).hexdigest()==row['actions_sha256']
    trace = prefix.with_suffix('.trace').read_text().splitlines()
    score,*_ = replay(raw.decode(),trace,actions.decode().splitlines(),detailed=True)
    assert score==int(row['score'])
    hist=read_history(raw,trace)
    final=stats(*hist[-1])
    assert final['snapshot_score']==score
    assert len(final['residual'])==int(row['final_reachable'])
    assert abs(final['mean_floor']-float(row['mean_human_floor']))<.00051
    return hist


def event(winner, loser, t, ids):
    win, lose = winner[t], loser[t]
    cut = winner[t+1][0]-win[0]
    assert cut
    variants = {}
    for mask in itertools.product('WL',repeat=3):
        state=tuple((win if m=='W' else lose)[i] for i,m in enumerate(mask))
        variants[''.join(mask)]=transfer(*state,cut,ids)
    assert variants['WWW']['reproduces_ids']
    return dict(action_turn=t,observed_turn=t+1,ids=ids,winning_cut=sorted(cut),
                input_wall_win_only=len(win[0]-lose[0]),input_wall_lose_only=len(lose[0]-win[0]),
                observed_pre={k:stats(*s) for k,s in [('winner',win),('loser',lose)]},
                observed_post={k:stats(*s) for k,s in [('winner',winner[t+1]),('loser',loser[t+1])]},
                variants_wall_pet_human=variants)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('ledger',type=Path)
    p.add_argument('--inputs',type=Path)
    p.add_argument('--tapes',type=Path)
    p.add_argument('--select',action='store_true',help='Print the case list; no replay needed.')
    a=p.parse_args()
    selection=list(selected(load_ledger(a.ledger)))
    assert sum(raw[0]!=raw[1] for _,_,raw,_ in selection)==20
    for case, ar, raw, useful in selection:
        if a.select:
            print(case,*raw,*map(int,useful))
            continue
        assert a.inputs and a.tapes
        win=int(raw[1]>raw[0]) if raw[0]!=raw[1] else int(useful[1]>useful[0])
        models=['baseline','article']
        if any(not (a.tapes/m/(Path(case).stem+'.trace')).exists() for m in models):
            print(json.dumps(dict(case=case,status='missing_exact_tape',raw=raw,useful=useful)))
            continue
        h=[verified(case,a.inputs,m,ar[m],a.tapes) for m in models]
        winner,loser=h[win],h[1-win]
        missing=stats(*loser[-1])['residual']
        events={}
        for part in ar[models[win]]['captures'].split(';'):
            if not part:continue
            t, pet, *_ = map(int,part.split(':'))
            if pet in missing:
                assert t>0
                events.setdefault(t-1,[]).append(pet)
        if not missing:
            # Common-clear 300-floor flips: transfer the winning final clearance
            # action only as context; area loss can have occurred much earlier.
            t=int(ar[models[win]]['first_all_clear'])
            if t>0:
                rem=stats(*winner[t-1])['residual']
                if rem:
                    events[t-1]=rem
        row=dict(case=case,winner=models[win],raw=raw,useful=useful,
                 final={m:{k:ar[m][k] for k in ('score','mean_human_floor','final_reachable')}
                       for m in models},
                 historical_final_missed=missing,
                 checkpoints={str(t):{m:stats(*s[t]) for m,s in zip(models,h)} for t in (160,205,225,260,275,300)},
                 early_first_wall_difference=next((t for t in range(160,301) if h[0][t][0]!=h[1][t][0]),None),
                 early_first_human_difference=next((t for t in range(160,301) if h[0][t][2]!=h[1][t][2]),None),
                 early_first_pet_difference=next((t for t in range(160,301) if h[0][t][1]!=h[1][t][1]),None),
                 events=[event(winner,loser,t,ids) for t,ids in sorted(events.items())])
        print(json.dumps(row))

if __name__=='__main__':
    main()
