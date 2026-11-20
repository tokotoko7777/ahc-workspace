#!/usr/bin/env python3
"""Paired three-arm official tester and independent route verification."""
import argparse
import hashlib
import json
from pathlib import Path
from benchmark import run_one, sha256

labels=("incumbent","local","future")


def main():
    p=argparse.ArgumentParser()
    for field in ("inputs","output","tester",*labels):
        p.add_argument(field,type=lambda x:Path(x).resolve())
    a=p.parse_args()
    files=sorted(a.inputs.glob("*.txt"))
    assert files
    binaries={k:getattr(a,k) for k in labels}
    a.output.mkdir(parents=True,exist_ok=True)
    manifest={"tester":sha256(a.tester),"tools":[sha256(Path(__file__)),sha256(Path(__file__).with_name("benchmark.py"))],
              "solvers":{k:sha256(v) for k,v in binaries.items()},
              "inputs":{f.name:sha256(f) for f in files}}
    meta=a.output/"manifest.json"
    if meta.exists() and json.loads(meta.read_text())!=manifest:
        raise ValueError("checkpoint manifest mismatch")
    meta.write_text(json.dumps(manifest,indent=2)+"\n")
    rows={k:[] for k in labels}
    signals={"local":[],"future":[]}
    for i,case in enumerate(files):
        for label in labels[i%3:]+labels[:i%3]:
            r=run_one(a.tester,binaries[label],case,a.output,label)
            if r["status"]!="ok":
                raise ValueError((label,r))
            rows[label].append(r)
            if label in signals:
                lines=(a.output/f"{case.stem}-{label}.stderr").read_text().splitlines()
                signal=next(json.loads(s) for s in lines if s.startswith("{"))
                signals[label].append([signal[n] for n in ("gap_shots","future_targets","changed_queries")])
    summary={k:{"valid":len(r),"score":sum(x["score"] for x in r),"max_wall":max(x["wall_s"] for x in r),
                "max_cpu":max(x["cpu_s"] for x in r)} for k,r in rows.items()}
    for label,pairs in signals.items():
        summary[label]["shot_range"]=[min(x[0] for x in pairs),max(x[0] for x in pairs)]
        summary[label]["target_total"]=sum(x[1] for x in pairs)
        summary[label]["changed"]=sum(x[2] for x in pairs)
    for control in ("incumbent","local"):
        delta=[x["score"]-y["score"] for x,y in zip(rows["future"],rows[control])]
        summary[f"against_{control}"]={"delta":sum(delta),"wins":sum(x>0 for x in delta),
                                       "losses":sum(x<0 for x in delta),"ties":sum(x==0 for x in delta)}
    (a.output/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
