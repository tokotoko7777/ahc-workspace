"""One-sided bypass crossbar extensions for prepared straight AHC008 banks.

The board is frozen and assumed prebuilt. Animals and staffing are modelled,
but no human paths, wall-work cost, mutual blocking or pet target observation
are being forecast. All-open and all-closed actual topology is checked.
Pass the directory containing bank_capped_hybrid_model.py as the optional argument.
"""
from pathlib import Path
import sys
sys.path.insert(0,sys.argv[1] if len(sys.argv)>1 else str(Path(__file__).resolve().parents[1]/'ahc008_focus_solver/problems/ahc008/experiments/20261122'))
from bank_capped_hybrid_model import Motion, propagate, BOARD, NB, D


def data(low=False):
    k,banks,shift=(13,2,8) if low else (12,3,4)
    w=set()
    tubes=[]
    lanes=[shift+7*j for j in range(banks+1)]
    for a,b in zip(lanes,lanes[1:]):
        for y in range(2,3+2*k,2):
            w.update(y*D+x for x in range(a+2,b-1))
        for y in range(3,2+2*k,2):
            body={y*D+x for x in range(a+2,b-1)}
            gates={y*D+a+1,y*D+b-1}
            safe={y*D+a+3,y*D+a+4}
            tubes.append((body,gates,safe))
    home=tuple(r*D+c for r in (7,19) for c in lanes)
    return w,tubes,home,lanes,2+2*k


def reachable(start,wall):
    seen={start}
    todo=[start]
    for u in todo:
        for v in NB[u]:
            if v not in seen and v not in wall:
                seen.add(v)
                todo.append(v)
    return seen


def run():
    print('banks,mode,site,added,duty,epoch,time,cow,pig,rabbit,dog,all_closed_average_anchor_distance')
    for low in (False,True):
        base,tube,home,lanes,bottom=data(low)
        safe=set().union(*(x[2] for x in tube))
        inner=set().union(*(x[0] for x in tube))
        gates=set().union(*(x[1] for x in tube))
        for site in (3,):
            for mode in ('none','alternating','opposite','all_top','all_bottom'):
                w=set(base)
                for j,a in enumerate(lanes[:-1]):
                    if mode=='none':continue
                    top=(mode=='all_top' or (mode in ('alternating','opposite') and (j%2==0)==(mode=='alternating')))
                    y=range(2) if top else range(bottom+1,D)
                    w.update(r*D+a+site for r in y)
                assert set(home)<=BOARD-w and not (w&(inner|gates))
                assert reachable(home[0],w|gates)==BOARD-w-gates-inner
                assert all(reachable(next(iter(b)),w|gates)==b for b,_,_ in tube)
                m=Motion(w,home)
                kg,gn=m.dog()
                # Common start set excludes all hypothetical extension squares.
                excluded={r*D+a+site for a in lanes[:-1] for r in [0,1,*range(bottom+1,D)]}
                start=BOARD-base-excluded
                closed=Motion(w|gates|inner,home)
                dists=[closed.dist(h) for h in home]
                dist=sum(dists[i][home[j]] for i in range(len(home))
                         for j in range(i))/max(1,len(home)*(len(home)-1)/2)
                for p in (.07,.11,.18):
                    for ep in (1,3):
                        terms=(60,120,160)
                        vals=[propagate(m.uniform,D*D,s,start,safe,[],terms,p,0.,ep) for s in (1,2,3)]
                        vals.append(propagate(kg,gn,1,start,safe,[],terms,p,0.,ep))
                        for t in terms:
                            print(len(lanes)-1,mode,site,len(w-base),p,ep,t,
                                  *(f'{sum(v[t]):.6f}' for v in vals),f'{dist:.2f}',sep=',')

if __name__=='__main__':run()
