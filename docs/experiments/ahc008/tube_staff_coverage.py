"""Static coverage model for repeated AHC008 tube-bank staff.

This exactly chooses nominal station rows in each aisle, with 1–3 workers
per aisle and a global worker budget. If an opportunity appears at a row,
the earliest feasible response on the static empty aisles is max(left
nearest, right nearest). This is a reachability diagnostic, not live
captures or a commitment to stay at covered rows.
"""
import numpy as np
from itertools import combinations

ROWS=np.arange(3,26,2)
configs=[c for k in (1,2,3) for c in combinations(ROWS,k)]
staffing=np.array([len(c) for c in configs])
D=np.array([np.min(np.abs(ROWS[:,None]-cfg),axis=1) for cfg in configs])
E=np.maximum(D[:,None,:],D[None,:,:])


def solve(banks, workers, weights=None, score='eta'):
    n=len(configs)
    if weights is None:
        weights=np.ones((banks,len(ROWS)))
    if score=='eta':
        K=E
    elif score=='discount':
        K=1.-np.exp(-E/2.)
    else:
        raise ValueError('unknown score')
    cost=np.einsum('ijr,br->bij',K,weights)
    dp=np.full((workers+1,n),np.inf)
    dp[staffing,np.arange(n)]=0.
    par=np.full((banks,workers+1,n),-1,dtype=int)
    for k in range(banks):
        nf=np.full_like(dp,np.inf)
        for used in range(1,workers):
            for extra in (1,2,3):
                if used+extra>workers: continue
                opts=np.flatnonzero(staffing==extra)
                q=dp[used,:,None]+cost[k][:,opts]
                ix=q.argmin(axis=0)
                dst=q[ix,np.arange(len(opts))]
                keep=dst<nf[used+extra,opts]
                nf[used+extra,opts[keep]]=dst[keep]
                par[k,used+extra,opts[keep]]=ix[keep]
        dp=nf
    last=dp[workers].argmin()
    value=dp[workers,last]
    used=workers; cfg=[last]
    for k in range(banks-1,-1,-1):
        j=par[k,used,last]
        assert j>=0
        used-=staffing[last]
        cfg.append(j)
        last=j
    cfg=cfg[::-1]
    e=np.array([E[i,j] for i,j in zip(cfg,cfg[1:])])
    assert sum(staffing[i] for i in cfg)==workers
    return value, (weights*e).sum()/weights.sum(), (weights*(e<=2)).sum()/weights.sum(), [configs[x] for x in cfg]


if __name__=='__main__':
    print('banks,workers,objective,mean_eta,within_2,stations')
    for b in (3,4):
        for m in range(b+1,11):
            for s in ('eta','discount'):
                value,eta,near,cfg=solve(b,m,score=s)
                conf='/'.join('+'.join(map(str,c)) for c in cfg)
                print(f'{b},{m},{s},{eta:.6f},{near:.6f},{conf}')
