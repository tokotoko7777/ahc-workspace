"""Build research-only single-file AHC008 integrated experiment."""
from pathlib import Path
def local(name):return Path(name).read_text()
s=local('base.cpp')
s=s.replace('using namespace std;\nnamespace {',local('dog-target-filter.hpp').replace('#pragma once','')+'\n'+
            local('last_capture.hpp').replace('#pragma once','')+'\nusing namespace std;\nnamespace {',1)
s=s.replace(' int bankMode=0,bankRot=0,bankShift=4,bankStarted=0,bankRelease=225;',
            local('motion.inc')+'\n lastcap::Planner finalCapture;\n int bankMode=0,bankRot=0,bankShift=4,bankStarted=0,bankRelease=235;',1)
s=s.replace(' string bankPlan(){',local('hunt.inc').replace('#if __has_include("last_capture.hpp")','').replace('#endif','').replace('    vector<vector<Forecast>> placeholder; // no per-pet forecast retained\n','')+'\n string bankPlan(){',1)
needle='  string m;\n  for(auto &p:pets){\n   if(!(cin>>m))return false;'
replace='''  if(turn<235||!postReady)observedMotionGraph();
  string m;
  int pi=0;
  for(auto &p:pets){
   if(!(cin>>m))return false;
   if(turn<235)ingestMotion(pi,p.p,m);
   ++pi;'''
assert s.count(needle)==1
s=s.replace(needle,replace)
s=s.replace(' g.makeBanks();',' g.makeBanks();\n g.setupObservedMotion();',1)
s=s.replace('   g.act=g.terminal(g.tacticalGate(g.repair(g.bankMode?g.bankPlan():g.plan())));',
'''   {
    auto proposal=g.bankMode?g.integratedBankPlan():g.plan();
    proposal=g.repair(std::move(proposal));
    if(!g.bankMode||g.integratedMode[g.turn]==4)
     proposal=g.terminal(g.tacticalGate(std::move(proposal)));
    g.act=std::move(proposal);
   }''',1)
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
replace=''' if(const char* p=getenv("AHC008_ARTICLE_LOG")){
  ofstream f(p);
  for(int t=0;t<300;t++)if(g.integratedMode[t]||g.integratedReady[t]||g.integratedLocks[t])
   f<<t<<" "<<g.integratedMode[t]<<" "<<g.integratedReady[t]<<" "<<g.integratedLocks[t]<<'\\n';
 }
'''
s=s.replace(needle,replace+needle,1)
Path('integrated.cpp').write_text(s)
