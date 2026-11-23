import csv,hashlib,itertools,json,math,statistics,subprocess,sys
from pathlib import Path
D=Path(__file__).resolve().parent;W=D.parent
def file(path,digest):
    p=W/path
    assert hashlib.sha256(p.read_bytes()).hexdigest()==digest,p
    return p
def run(path,inp,tape,mode,r):
    return subprocess.check_output([path,inp,tape,mode,str(r['cap']),str(r['normal']),str(r['chase'])],
        text=True,timeout=20)
def main():
    path=Path(sys.argv[1]).resolve()
    data=json.loads((D/'FIXTURES.json').read_text())
    for p,h in data['pins'].items():file(p,h)
    assert len(data['rows'])==22
    work=[]
    for r in data['rows']:
        inp=file(r['input'],r['input_sha'])
        action=file(r['action'],r['action_sha'])
        tape=file(r['trace'],r['trace_sha'])
        assert len(action.read_bytes().splitlines())==300
        assert tape.read_bytes().splitlines()[::2]==action.read_bytes().splitlines()
        assert run(path,inp,tape,'verify',r)=='verified\n'
        work.append((r,inp,tape))
    output=[]
    for i,(r,inp,tape) in enumerate(work):
        order=['SKI','SIK','KSI','KIS','ISK','IKS'][i%6]
        result={}
        for m,c,*chunk in csv.reader(run(path,inp,tape,order,r).splitlines()):
            x=list(map(int,chunk));assert len(x)==64
            values=[x[j:j+4] for j in range(0,64,4)]
            n,b,nf,bf=list(zip(*values))
            delta=[(y-z)/1e8 for z,y in zip(n,b)]
            result[m]=dict(cpu=float(c),variance=statistics.variance(delta),
                 covariance=statistics.covariance(n,b)/1e16,
                 mean_n=statistics.mean(n),mean_c=statistics.mean(b),
                 var_n=statistics.variance(n)/1e16,var_c=statistics.variance(b)/1e16,
                 full_n=sum(nf),full_c=sum(bf),samples=values)
        assert set(result)==set('SKI')
        a,b=result['S']['variance'],result['K']['variance']
        ratio=b/a if a else (math.inf if b else 1.)
        output.append(dict(panel=r['panel'],case=r['case'],mode=r['native_mode'],
            strata=r['strata'],ratio=ratio, arms=result))
    def spread(arm,key):return statistics.mean(r['arms'][arm][key] for r in output)
    ratio=statistics.median(r['ratio']for r in output)
    timing={m:sum(r['arms'][m]['cpu']for r in output)for m in 'SKI'}
    drift={key:spread('K',key)-spread('S',key)for key in ('mean_n','mean_c')}
    gate=ratio<=.9 and max(map(abs,drift.values()))<=2e6 and timing['K']<=1.1*timing['S']
    report=dict(n=len(output),verified_fixed_caps=len(work),sum_historical_pairs=sum(x[0]['cap']for x in work),
        fixed_pairs_each=16,median_ratio=ratio,serial_cpu=timing,mean_drift=drift,
        arm_stats={m:{k:spread(m,k)for k in ('variance','covariance','mean_n','mean_c','var_n','var_c','full_n','full_c')}for m in 'SKI'},
        median_pairwise_cpu_ratio=statistics.median(r['arms']['K']['cpu']/r['arms']['S']['cpu']for r in output),
        wins=sum(r['ratio']<1 for r in output),bias='Archived Map1 or actual Chase late-large capture; outcome-selected archival support',
        pass_gate=gate,cases=output)
    def strict(x):
        if isinstance(x,float) and not math.isfinite(x):return 'Infinity'
        if isinstance(x,dict):return {k:strict(v) for k,v in x.items()}
        if isinstance(x,list):return [strict(v) for v in x]
        return x
    print(json.dumps(strict(report),indent=2,allow_nan=False))
if __name__=='__main__':main()
