from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def edit(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b,1)
edit(' Shape active;',' Shape active;\n std::vector<Shape> routeGeoms;')
edit('||(teamMode()&&int(z.cut.size())>H)','||(teamMode()&&int(z.cut.size())>2*H)')
edit('   int required=std::min(cfg.minArea,size*7/10);','   if(teamMode())routeGeoms.push_back(z);\n   int required=std::min(cfg.minArea,size*7/10);')
schedule=Path(sys.argv[2]).read_text().replace('#pragma once\n','').replace('#include "last_capture.hpp"\n','')
edit('using namespace std;\nnamespace {',schedule+'\nusing namespace std;\nnamespace {')
edit(' vector<EarlyTrace> earlyTrace;',''' vector<EarlyTrace> earlyTrace;
 vector<vector<int>> routeAudit;
 void recordRoutes(int id,unsigned mask,const vector<lastcap::Pet>& animals,const lastcap::Planner& trial){
  const auto blocked=lastcap::Planner::occupied(humans,animals);
  vector<int> crew,crewID;
  for(int i=0;i<(int)humans.size();i++)if(mask>>i&1){
   crew.push_back(humans[i]);crewID.push_back(i);
  }
  routeAudit.push_back({0,turn,id,(int)mask});
  set<vector<int>> seen;
  for(auto geo:trial.routeGeoms){
   sort(geo.cut.begin(),geo.cut.end());
   if(!seen.insert(geo.cut).second)continue;
   auto z=tworoutes::solve(geo.can,geo.cut,crew);
   vector<int> row{1,turn,id,(int)mask,(int)geo.cut.size(),z.time,0};
   for(int p:geo.cut)row[6]+=blocked[p];
   for(int p:geo.cut)row.push_back(p);
   for(int j=0;j<(int)crewID.size();j++){
    row.push_back(crewID[j]);
    auto q=z.time>=lastcap::INF?tworoutes::Path{}:z.work[j];
    for(int k:{q.a,q.b,q.sa,q.sb,q.t,q.firstTravel,q.secondTravel})row.push_back(k);
   }
   routeAudit.push_back(std::move(row));
  }
 }
''')
start=s.index('  auto proposal=trial.plan(turn,wall,humans,animals);')
end=s.index('\n }',s.index('earlyTrace.push_back({turn,2,',start))
s=s[:start]+'''  trial.plan(turn,wall,humans,animals);
  recordRoutes(id,mask,animals,trial);
  return ordinary;'''+s[end:]
needle=' if(const char* p=getenv("AHC008_GLOBAL_CLEAR_LOG")){'
edit(needle,''' if(const char* p=getenv("AHC008_TWO_ROUTE_LOG")){
  ofstream f(p);
  for(const auto& row:g.routeAudit){
   for(int q:row)f<<q<<" ";
   f<<'\\n';
  }
 }
'''+needle)
Path(sys.argv[3]).write_text(s)
