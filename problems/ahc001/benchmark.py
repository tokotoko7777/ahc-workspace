#!/usr/bin/env python3
import argparse,csv,os,pathlib,subprocess,time,math,sys
def check(inp, out):
    data=list(map(int,inp.split()))
    n=data[0]; points=list(zip(data[1::3],data[2::3],data[3::3]))
    rect=list(map(int,out.split()))
    if len(rect)!=4*n: raise ValueError(f'output size {len(rect)} expected {4*n}')
    boxes=list(zip(rect[::4],rect[1::4],rect[2::4],rect[3::4]))
    score=0.
    for i,((x,y,target),(l,b,r,t)) in enumerate(zip(points,boxes)):
        if not (0<=l<=x<r<=10000 and 0<=b<=y<t<=10000): raise ValueError(f'bad rectangle {i}')
        if any(l<q and p<r and b<s and k<t for p,k,q,s in boxes[:i]): raise ValueError(f'overlap {i}')
        area=(r-l)*(t-b)
        score+=1-(1-min(area,target)/max(area,target))**2
    return math.floor(1e9*score/n+0.5)
def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('inputs',type=pathlib.Path);parser.add_argument('out',type=pathlib.Path)
    parser.add_argument('--start',type=int,default=0);parser.add_argument('--end',type=int)
    parser.add_argument('--time',type=float,default=4.8)
    parser.add_argument('--entry',action='append',required=True)
    parser.add_argument('--vis',type=pathlib.Path)
    args=parser.parse_args()
    out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
    entries=[(x.split('=',1)[0],str(pathlib.Path(x.split('=',1)[1]).resolve())) for x in args.entry]
    vis=str(args.vis.resolve()) if args.vis else None
    fs=sorted(args.inputs.resolve().glob('*.txt'))[args.start:args.end]
    if not fs:raise SystemExit('no input files')
    with (out/'results.csv').open('w',newline='') as file:
        writer=csv.writer(file);writer.writerow(['file','solver','n','score','elapsed_s','official'])
        for k,f in enumerate(fs):
            data=f.read_bytes()
            for label,exe in entries[k%len(entries):]+entries[:k%len(entries)]:
                dest=out/label;dest.mkdir(exist_ok=True)
                start=time.monotonic()
                proc=subprocess.run([exe,str(args.time)],input=data,capture_output=True,timeout=5.0)
                elapsed=time.monotonic()-start
                if proc.returncode: raise RuntimeError(f'{label} {f}: '+proc.stderr.decode())
                score=check(data,proc.stdout)
                target=dest/f.name;target.write_bytes(proc.stdout)
                (dest/(f.stem+'.log')).write_bytes(proc.stderr)
                if vis:
                    rust=subprocess.run([vis,str(f),str(target)],cwd=dest,check=True,capture_output=True,text=True)
                    if int(rust.stdout.strip())!=score or rust.stderr:raise RuntimeError(f'official scorer mismatch {rust}')
                row=[f.name,label,data.split(maxsplit=1)[0].decode(),score,f'{elapsed:.6f}',int(bool(vis))]
                writer.writerow(row);file.flush()
                print(*row,flush=True)
if __name__=='__main__':main()
