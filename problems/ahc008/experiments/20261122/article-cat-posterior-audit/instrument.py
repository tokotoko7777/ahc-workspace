from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def edit(old,new):
 global s
 assert s.count(old)==1,old
 s=s.replace(old,new,1)
header=(Path('shortest-first-masks.hpp').read_text().replace('#pragma once\n','')+
        '\n'+Path('cat-target-filter.hpp').read_text().replace('#pragma once\n','').replace('#include "../../library/shortest-first-masks.hpp"\n',''))
edit('using namespace std;\nnamespace {',header+'\nusing namespace std;\nnamespace {')
edit(' vector<int> catHead;',Path('catdiag.inc').read_text()+'\n vector<int> catHead;')
edit('  catHead.assign(pets.size(),-1);','  catHead.assign(pets.size(),-1);\n  catPosterior.resize(pets.size());')
edit('   if(maxEta&&!overBudget()){','''
   vector<array<int,3>> catJobs(n,{99,-1,-1});
   for(int k=0;k<n;k++)for(auto p:pairInfo[k])
    if(p.d<catJobs[k][0]&&p.d<=3)catJobs[k]={p.d,p.a,p.b};
   if(maxEta)auditCatForecast(catJobs,service);
   if(maxEta&&!overBudget()){''')
edit('   if(turn<235)ingestMotion(pi,p.p,m);','''
   auditCatObserved(pi,p.p,m);
   if(turn<235)ingestMotion(pi,p.p,m);''')
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
edit(needle,''' if(const char* p=getenv("AHC008_CAT_DIAG")){
  ofstream f(p);f<<setprecision(12);
  f<<"T "<<double(g.catObserveTicks)/CLOCKS_PER_SEC<<" "<<double(g.catForecastTicks)/CLOCKS_PER_SEC<<"\\n";
  for(auto r:g.catNll)f<<"N "<<r.turn<<" "<<r.id<<" "<<r.dir<<" "<<r.old<<" "<<r.next<<"\\n";
  for(auto r:g.catProbe)f<<"P "<<r.turn<<" "<<r.tube<<" "<<r.eta<<" "<<r.id<<" "<<r.a<<" "<<r.b
   <<" "<<r.oldSafe<<" "<<r.newSafe<<" "<<r.others<<"\\n";
 }
'''+needle)
Path(sys.argv[2]).write_text(s)
