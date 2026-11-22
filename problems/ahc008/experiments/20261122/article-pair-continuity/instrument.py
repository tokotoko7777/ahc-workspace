from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def edit(old,new):
 global s
 assert s.count(old)==1,old
 s=s.replace(old,new,1)
edit(' array<int,300> integratedMode{},integratedReady{},integratedLocks{};',
''' array<int,300> integratedMode{},integratedReady{},integratedLocks{};
 vector<array<int,9>> pairedAudit;
 vector<array<int,4>> immediateAudit;''')
edit('    integratedLocks[turn]+=count[j.tube];',
'''    integratedLocks[turn]+=count[j.tube];
    immediateAudit.push_back({turn,j.tube,j.a,j.b});''')
edit('     pairedTubes[j.tube]=true;',
'''     pairedTubes[j.tube]=true;
     const auto& sides=banks[j.tube];
     pairedAudit.push_back({turn,j.tube,j.a,j.b,j.eta,paths[j.a].d[sides.l],
      paths[j.b].d[sides.h],paths[j.a].step[sides.l],paths[j.b].step[sides.h]});''')
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
edit(needle,''' if(const char* p=getenv("AHC008_PAIR_COMMIT_LOG")){
  ofstream f(p);
  for(const auto& r:g.pairedAudit){f<<"P";for(int q:r)f<<" "<<q;f<<'\\n';}
  for(const auto& r:g.immediateAudit){f<<"I";for(int q:r)f<<" "<<q;f<<'\\n';}
 }
'''+needle)
Path(sys.argv[2]).write_text(s)
