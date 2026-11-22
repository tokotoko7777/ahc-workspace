from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a[:120]
 s=s.replace(a,b)
a='''  if(turn<lineTurn||overBudget())return false;
  int hm=humans.size(),root=-1;'''
rep(a,'''  // An early short seam is separate from the late desperate floor.
  // Only try when the live rail phase already released to the old rescue.
  const bool earlyShort=humans.size()==5&&turn>=178&&turn<lineTurn;
  if((turn<lineTurn&&!earlyShort)||overBudget())return false;
  int hm=humans.size(),root=-1;''')
a='''  if(turn>=281)areaReq=max(0,areaReq-(turn-281)*10);
  vector<int> toBuild;'''
rep(a,'''  if(turn>=281)areaReq=max(0,areaReq-(turn-281)*10);
  if(earlyShort)areaReq=max(300,3*currentCt[root]/4);
  vector<int> toBuild;''')
a='''    if(id<0||benefit<current*(turn<280?1.06:1.02)||lcount[id]<areaReq){lineId=-1;}'''
rep(a,'''    if(id<0||benefit<current*(turn<280?1.06:1.02)||lcount[id]<areaReq||
       (earlyShort&&bn-lanimals[id]<2)){lineId=-1;}''')
a='''   int limit=min(lineMax,(299-turn)*max(1,int(hm)-1));'''
rep(a,a+'\n   if(earlyShort)limit=min(limit,10);')
a='''      if(turn+eta>=300)continue;
      auto [benefit,id]=testLine(w,root,false,eta,areaReq,(j*S+i)*(S+1)+e);'''
rep(a,'''      if(turn+eta>=300||(earlyShort&&eta>9))continue;
      auto [benefit,id]=testLine(w,root,false,eta,areaReq,(j*S+i)*(S+1)+e);
      if(earlyShort&&(id<0||bn-lanimals[id]<2))continue;''')
a='''   if(lcount[k]<6)continue;
#endif
   double v=ldexp(lcount[k],-lanimals[k]);'''
rep(a,'''   if(lcount[k]<(humans.size()==5&&turn>=178&&turn<lineTurn?minArea:6))continue;
#endif
   double v=ldexp(lcount[k],-lanimals[k]);''')
Path(sys.argv[2]).write_text(s)
