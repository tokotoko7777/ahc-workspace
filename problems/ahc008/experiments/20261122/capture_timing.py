"""Compare durable all-clear times (at telemetry's sampling resolution)."""
import argparse
import csv
import json
from collections import defaultdict
from statistics import mean,median

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("telemetry")
    p.add_argument("baseline")
    p.add_argument("candidate")
    p.add_argument("--csv")
    a=p.parse_args()
    observed=defaultdict(list)
    for line in open(a.telemetry):
        r=json.loads(line)
        clear=not any(sum(kinds) for humans,_,kinds in r["groups"] if humans)
        observed[r["case"],r["model"]].append((r["turn"],clear))
    turns={}
    for key,trace in observed.items():
        trace.sort()
        assert trace[-1][0]==300
        turns[key]=min((t for j,(t,_) in enumerate(trace) if all(c for _,c in trace[j:])),default=-1)
    pairs=[(name,t,turns[name,a.candidate]) for (name,arm),t in sorted(turns.items()) if arm==a.baseline]
    print("reliability wins", sum(x<0<=y for _,x,y in pairs), "losses",sum(y<0<=x for _,x,y in pairs),
          "paired clear",sum(min(x,y)>=0 for _,x,y in pairs))
    both=[(x,y) for _,x,y in pairs if min(x,y)>=0]
    if both:
        for f in (mean,median):
            print(f.__name__,*[round(f(z),3) for z in zip(*both)])
    if a.csv:
        with open(a.csv,"w") as file:
            w=csv.writer(file,lineterminator="\n")
            w.writerow(["case",a.baseline,a.candidate])
            w.writerows(pairs)
if __name__=="__main__":main()
