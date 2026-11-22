from pathlib import Path
import sys,hashlib
p=Path(sys.argv[1]); assert hashlib.sha256(p.read_bytes()).hexdigest()=='46f210cc5dd50b8a6088724861ae5c2b044d08edb109a891ee217352730fcb84'
s=p.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a[:120]
 s=s.replace(a,b)
rep(' int lineId=-1, lineStart=0, lineH=0, lineBase=0;',
    ' int lineId=-1, lineStart=0, lineH=0, lineBase=0,lineInitialRemain=-1;')
a='''    if(id<0||benefit<current*(turn<280?1.06:1.02)||lcount[id]<areaReq||
       (earlyShort&&bn-lanimals[id]<2)){lineId=-1;}'''
rep(a,'''    const bool hasConstruction=earlyShort&&int(toBuild.size())<lineInitialRemain;
    const int need=hasConstruction?1:2;
    if(id<0||benefit<current*(turn<280?1.06:1.02)||lcount[id]<areaReq||
       (earlyShort&&bn-lanimals[id]<need)){lineId=-1;}''')
a='''      if(v>best){best=v;lineId=j;lineBase=i;line.at=e;lineH=root;lineStart=turn;toBuild=move(w);}'''
rep(a,'''      if(v>best){best=v;lineId=j;lineBase=i;line.at=e;lineH=root;lineStart=turn;
       lineInitialRemain=w.size();toBuild=move(w);}''')
Path(sys.argv[2]).write_text(s)
