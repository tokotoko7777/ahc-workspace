"""Instrument only actually visited v7 latePlan candidate states."""
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def edit(old,new):
 global s
 assert s.count(old)==1,old
 s=s.replace(old,new,1)
edit(' bool latePlan(){','''
 struct WideClearDiag {int turn,kind;vector<int> cells;};
 vector<WideClearDiag> wideClear;
 bool latePlan(){''')
edit('  if(lineId<0){\n   double best=0.;','''
  if(lineId<0){
   wideClear.push_back({turn,0,{}});
   double best=0.;''')
edit('#endif\n      if(id<0||benefit<current*1.05||lcount[id]<areaReq)continue;',
'''#endif
      if(id<0||benefit<current*1.05||lcount[id]<areaReq)continue;
      if(lanimals[id]==0&&lcount[id]>=300&&lcount[id]*10>=currentCt[root]*7)
       wideClear.push_back({turn,1,w});''')
edit('  if(keep<0){lineId=-1;return false;}',
'''  if(keep<0){lineId=-1;return false;}
  wideClear.push_back({turn,2,toBuild});''')
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
edit(needle,''' if(const char* p=getenv("AHC008_WIDE_CLEAR_LOG")){
  ofstream f(p);
  for(const auto& r:g.wideClear){
   f<<r.turn<<" "<<r.kind;
   for(int q:r.cells)f<<" "<<q;
   f<<'\\n';
  }
 }
'''+needle)
Path(sys.argv[2]).write_text(s)
