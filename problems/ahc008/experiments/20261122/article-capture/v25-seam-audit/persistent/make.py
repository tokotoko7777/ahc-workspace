from pathlib import Path
import sys,hashlib
p=Path(sys.argv[1])
assert hashlib.sha256(p.read_bytes()).hexdigest()=='46f210cc5dd50b8a6088724861ae5c2b044d08edb109a891ee217352730fcb84'
s=p.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a[:150]
 s=s.replace(a,b)
rep('int minArea = 6, int memoKey=-1){','int minArea = 6, int memoKey=-1,bool floorOwned=false){')
rep('if(lcount[k]<(humans.size()==5&&turn>=178&&turn<lineTurn?minArea:6))continue;',
    'if(lcount[k]<((humans.size()==5&&turn>=178&&turn<lineTurn)||floorOwned?minArea:6))continue;')
a='''   else {
    auto [benefit,id]=testLine(toBuild,root,false,0,areaReq);
    if(id<0||benefit<current*(turn<280?1.06:1.02)||lcount[id]<areaReq||
       (earlyShort&&bn-lanimals[id]<2)){lineId=-1;}
   }'''
rep(a,'''   else {
    const bool ongoing=humans.size()==5&&lineStart>=178&&lineStart<lineTurn&&turn>=lineTurn;
    const int validateArea=ongoing?max({areaReq,300,3*currentCt[root]/4}):areaReq;
    auto [benefit,id]=testLine(toBuild,root,false,0,validateArea,-1,ongoing);
    if(id<0||benefit<current*(turn<280?1.06:1.02)||lcount[id]<validateArea||
       (earlyShort&&bn-lanimals[id]<2)){lineId=-1;}
   }''')
a='''  auto [gain,keep]=testLine(toBuild,root,true,0,areaReq);'''
rep(a,'''  const bool ongoing=humans.size()==5&&lineStart>=178&&lineStart<lineTurn&&turn>=lineTurn;
  const int validateArea=ongoing?max({areaReq,300,3*currentCt[root]/4}):areaReq;
  auto [gain,keep]=testLine(toBuild,root,true,0,validateArea,-1,ongoing);''')
Path(sys.argv[2]).write_text(s)
