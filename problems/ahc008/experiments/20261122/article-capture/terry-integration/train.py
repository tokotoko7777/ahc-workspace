"""Offline deterministic numpy fit; keep stock runtime untrained by default."""
import argparse
import csv
import json
import os
import pathlib
os.environ.setdefault("OPENBLAS_NUM_THREADS","1")
os.environ.setdefault("OMP_NUM_THREADS","1")
import numpy as np
from collect import digest, features, seed

DIMS=(7,32,32,32,32,5)
NORM=np.array([10,20,10,10,10,10,10],np.float32)
COLUMNS=("M","N","cow","pig","rabbit","dog","cat")

def load(p,split):
    m=json.loads((p/"manifest.json").read_text())
    f=p/"labels.csv"
    if (m.get("schema")!="ahc008-terry-five-map-v1" or m["split"]!=split or
        m["score_scale"]!=100_000_000 or m["forced_maps"]!=list(range(5)) or
        digest(f)!=m["labels_sha256"]):
        raise ValueError(f"Unverified or incompatible labels: {p}")
    rows=list(csv.DictReader(f.open(newline="")))
    if len(rows)!=m["count"] or m["count"]!=m["cases_collected"]:
        raise ValueError(f"Incomplete labels: {p}")
    x=[]; y=[]; seeds=set(); boards=set()
    for j,r in enumerate(rows):
        i=m["start"]+j
        source=p/"in"/f"{j:04d}.txt"
        a=[int(r[k]) for k in COLUMNS]
        if (int(r["case_id"])!=i or int(r["seed"])!=seed(i) or
            r["input_sha256"]!=digest(source) or a!=features(source.read_bytes())):
            raise ValueError(f"Provenance/hash mismatch: {p}, case {i}")
        seeds.add(int(r["seed"]))
        boards.add(r["input_sha256"])
        b=[int(r[f"score_{k}"]) for k in range(5)]
        if any(not(0<=w<=m["score_scale"]) for w in b):
            raise ValueError(f"Out of range official score: {p}, case {i}")
        x.append(a);y.append(b)
    if len(seeds)!=len(rows) or len(boards)!=len(rows):
        raise ValueError(f"Duplicated case identity: {p}")
    return (np.array(x,np.float32)/NORM,np.array(y,np.float32)/m["score_scale"]),m,seeds,boards

def initial(rng):
    return [(rng.normal(0,np.sqrt(2./a),(a,b)).astype(np.float32),np.zeros(b,np.float32))
            for a,b in zip(DIMS,DIMS[1:])]

def forward(x,net):
    act=[x]
    for j,(w,b) in enumerate(net):
        z=act[-1]@w+b
        act.append(np.maximum(0.,z) if j<len(net)-1 else 1/(1+np.exp(-np.clip(z,-60,60))))
    return act

def losses(p,y,static):
    picked=p.argmax(axis=1)
    selected=y[np.arange(len(y)),picked]
    delta=selected-y[:,static]
    return dict(mse=float(np.mean((p-y)**2)), policy=float(selected.mean()),
        fixed_train_best=int(static), fixed_train_best_score=float(y[:,static].mean()),
        fixed_lean4_score=float(y[:,4].mean()), oracle=float(y.max(axis=1).mean()),
        improvement=float(delta.mean()), different=int((picked!=static).sum()),
        delta_stderr=float(delta.std(ddof=1)/len(y)**.5),
        selected_histogram=np.bincount(picked,minlength=5).tolist())

def fit(x,y,va,vb,random_seed,epochs):
    rng=np.random.default_rng(random_seed)
    net=initial(rng)
    # Adam; deterministic shuffle, no variation by instance seed at inference.
    vectors=[z for w,b in net for z in (w,b)]
    m=[np.zeros_like(a) for a in vectors];v=[a.copy() for a in m]
    step=0; best=float("inf");chosen=None;when=0
    static=int(y.mean(axis=0).argmax())
    for ep in range(1,epochs+1):
        order=rng.permutation(len(x))
        for st in range(0,len(x),32):
            ids=order[st:st+32]
            a=forward(x[ids],net)
            g=2./a[-1].size*(a[-1]-y[ids])*a[-1]*(1-a[-1])
            grad=[]
            for j in range(len(net)-1,-1,-1):
                grad[:0]=(a[j].T@g+1e-5*net[j][0],g.sum(axis=0))
                if j:g=(g@net[j][0].T)*(a[j]>0)
            step+=1
            for z,g,u,h in zip(vectors,grad,m,v):
                u[:]=.9*u+.1*g;h[:]=.999*h+.001*g*g
                z-=.0015*(u/(1-.9**step))/(np.sqrt(h/(1-.999**step))+1e-8)
        err=float(((forward(va,net)[-1]-vb)**2).mean())
        if err < best:
            best=err;when=ep;chosen=[(w.copy(),b.copy()) for w,b in net]
    report=losses(forward(va,chosen)[-1],vb,static)
    report["chosen_epoch"]=when
    return chosen,report

def emit(p,net,comment):
    arr=[v for w,b in net for j in range(len(b)) for v in [b[j],*w[:,j]]]
    assert len(arr)==3589 and np.isfinite(arr).all()
    def fmt(x):
        z=format(float(x),".9g")
        return (z if any(s in z for s in ".e") else z+".0")+"f"
    lines=["#pragma once","#include <array>","namespace terry_maps::weights {",
           "// "+comment,"inline constexpr bool trained = true;",
           "inline constexpr std::array<float, 3589> values{"]
    lines.extend("    "+", ".join(map(fmt,arr[i:i+8]))+"," for i in range(0,len(arr),8))
    lines.extend(["};","}",""])
    tmp=p.with_suffix(".tmp")
    tmp.write_text("\n".join(lines))
    tmp.replace(p)

def main():
    p=argparse.ArgumentParser()
    p.add_argument("train",type=pathlib.Path);p.add_argument("validation",type=pathlib.Path)
    p.add_argument("--weights",type=pathlib.Path,required=True)
    p.add_argument("--report",type=pathlib.Path,required=True)
    p.add_argument("--seed",type=int,default=574008)
    p.add_argument("--epochs",type=int,default=160)
    p.add_argument("--allow-pilot-export",action="store_true",
        help="Emit experimental weights even without strong independent evidence; not automatic adoption.")
    a=p.parse_args()
    if a.epochs<1:p.error("epochs must be positive")
    (x,y),tm,ts,tb=load(a.train,"train")
    (vx,vy),vm,vs,vb=load(a.validation,"validation")
    if ts&vs or tb&vb or len(vx)<2:
        raise SystemExit("invalid independent validation partitions")
    if any(vm[k]!=tm[k] for k in ("gen_sha256","tester_sha256","solver_sha256")):
        raise SystemExit("trainer requires fixed generator, tester and solver across the partitions")
    net,r=fit(x,y,vx,vy,a.seed,a.epochs)
    r.update(training=tm,validation=vm,trainer_seed=a.seed,
             architecture=list(DIMS),epochs=a.epochs)
    sufficient=(len(x)>=200 and len(vx)>=60 and
               r["improvement"]>max(1e-6,1.96*r["delta_stderr"]) and
               r["policy"]>r["fixed_lean4_score"])
    r["publication_gate_passed"]=bool(sufficient)
    r["exploratory_export"]=bool(a.allow_pilot_export and not sufficient)
    r["weights_written"]=bool(sufficient or a.allow_pilot_export)
    if r["weights_written"]:
        label="Exploratory only" if r["exploratory_export"] else "Independently favorable fit"
        emit(a.weights,net,f"{label}; official five-map labels (train {len(x)}, validation {len(vx)}).")
        r["weights_sha256"]=digest(a.weights)
    a.report.write_text(json.dumps(r,indent=2)+"\n")
    print(json.dumps({k:v for k,v in r.items() if k not in ("training","validation")},indent=2))

if __name__=="__main__":
    main()
