from pathlib import Path
import sys
base,out=map(Path,sys.argv[1:])
s=base.read_text()
root=Path(__file__).resolve().parent
insert=' string integratedBankPlan(){'
assert s.count(insert)==1
s=s.replace(insert,(root/'route.inc').read_text()+'\n'+insert)
old='  // Untasked workers build local brace frontiers without disrupting a paired hold.\n  if(turn<bankStaffTurn+32){'
assert s.count(old)==1
s=s.replace(old,old+'\n   if(humans.size()<=6&&narrowRescue)runBraceRoutes(usedPeople,build,moveTo);')
tag=' if(const char* p=getenv("AHC008_ARTICLE_LOG")){'
assert s.count(tag)==1
s=s.replace(tag,''' if(const char* p=getenv("AHC008_CONTRACT_LOG")){
  ofstream f(p);
  if(g.braceNominal>=0){
   f<<g.braceNominal<<' '<<g.humans.size()<<'\\n';
   for(int i=0;i<int(g.humans.size());i++){
    f<<g.braceInitialJobs[i];
    for(int k:g.braceReport[i])f<<' '<<k;
    f<<' '<<g.braceNext[i]<<'\\n';
   }
  }
 }
'''+tag)
out.write_text(s)
