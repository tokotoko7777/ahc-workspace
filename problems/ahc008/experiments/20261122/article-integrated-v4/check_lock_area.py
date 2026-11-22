"""Cross-check planned tube locks against actual isolated replay components."""
import collections,csv,json,sys
for path in sys.argv[1:]:
    d={}
    with open(path) as f:
        for r in csv.DictReader(f):
            key=r['model']
            if key not in d:
                d[key]={'planned_animals':0,'planned_turns':0,'mismatches':[]}
            v=d[key];caps=collections.Counter()
            for x in r['captures'].split(';'):
                if x:
                    t,pet,typ,area,irr=map(int,x.split(':'))
                    caps[t-1,area==4]+=1
            for x in r['telemetry'].split(';'):
                if x:
                    t,mode,ready,n=map(int,x.split())
                    if n:
                        v['planned_animals']+=n
                        v['planned_turns']+=1
                        if caps[t,True]!=n or caps[t,False]:
                            v['mismatches'].append([r['case'],t,n,caps[t,True],caps[t,False]])
    print(json.dumps(dict(ledger=path,counts=d)))
