"""Compile-time audit only; no runtime policy/scores or stdout changes."""
import hashlib,sys
from pathlib import Path
src,out=map(Path,sys.argv[1:])
assert hashlib.sha256(src.read_bytes()).hexdigest()=='46f210cc5dd50b8a6088724861ae5c2b044d08edb109a891ee217352730fcb84'
s=src.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a[:150]
 s=s.replace(a,b)
rep(' bool latePlan(){',''' struct EarlyAuditTurn {
  int stage=0,line=-1,start=-1,root=-1,rootArea=-1,rootAnimals=-1,required=-1;
  int projected=-1,projectedPets=-1,eta=-1,walls=-1,mode=-1;
  string raw, repaired, gated, sent;
  vector<int> cells;
 };
 bool earlyAudit=false;
 array<EarlyAuditTurn,300> earlyTape{};
 bool latePlan(){''')
rep('  int bn=0;for(auto p:pets)bn+=group[p.p]==root;','''  int bn=0;for(auto p:pets)bn+=group[p.p]==root;
  if(earlyAudit&&humans.size()==5&&turn>=178&&(earlyShort||(lineId>=0&&lineStart<lineTurn))){
   auto& e=earlyTape[turn];e.stage=1;e.root=root;e.rootArea=currentCt[root];e.rootAnimals=bn;
   if(lineId>=0){e.line=lineId;e.start=lineStart;}
  }''')
rep('  if(earlyShort)areaReq=max(300,3*currentCt[root]/4);','''  if(earlyShort)areaReq=max(300,3*currentCt[root]/4);
  if(earlyAudit&&earlyTape[turn].stage)earlyTape[turn].required=areaReq;''')
target='      if(v>best){best=v;lineId=j;lineBase=i;line.at=e;lineH=root;lineStart=turn;toBuild=move(w);}'
rep(target,'''      if(v>best){best=v;lineId=j;lineBase=i;line.at=e;lineH=root;lineStart=turn;toBuild=move(w);
       if(earlyAudit&&earlyTape[turn].stage&&earlyShort){earlyTape[turn].eta=eta;}
      }''')
rep('  if(keep<0){lineId=-1;return false;}','''  if(keep<0){lineId=-1;return false;}
  if(earlyAudit&&humans.size()==5&&lineStart>=178&&lineStart<lineTurn){
   auto& e=earlyTape[turn];e.stage=2;e.line=lineId;e.start=lineStart;e.projected=lcount[keep];
   e.projectedPets=lanimals[keep];e.walls=toBuild.size();e.cells=toBuild;
  }''')
rep(' Game g;g.init(1,argv);',''' Game g;g.init(1,argv);
 const char* seamAuditPath=getenv("AHC008_EARLY_SEAM_AUDIT");
 g.earlyAudit=seamAuditPath&&g.humans.size()==5;''')
rep('''    proposal=g.repair(std::move(proposal));
    if(!g.bankMode||g.integratedMode[g.turn]==4)
     proposal=g.terminal(g.tacticalGate(std::move(proposal)));
    g.act=std::move(proposal);''','''    if(g.earlyAudit&&g.turn>=178)g.earlyTape[g.turn].raw=proposal;
    proposal=g.repair(std::move(proposal));
    if(g.earlyAudit&&g.turn>=178)g.earlyTape[g.turn].repaired=proposal;
    if(!g.bankMode||g.integratedMode[g.turn]==4){
     proposal=g.tacticalGate(std::move(proposal));
     if(g.earlyAudit&&g.turn>=178)g.earlyTape[g.turn].gated=proposal;
     proposal=g.terminal(std::move(proposal));
    }
    g.act=std::move(proposal);''')
rep('  changed=g.act.find_first_of("udlr")!=string::npos;','''  if(g.earlyAudit&&g.turn>=178){
   auto& e=g.earlyTape[g.turn];
   e.mode=g.integratedMode[g.turn];
   if(e.stage==0&&g.lineId>=0&&g.lineStart>=178&&g.lineStart<g.lineTurn){
    e.line=g.lineId;e.start=g.lineStart;
   }
   e.sent=g.act;
  }
  changed=g.act.find_first_of("udlr")!=string::npos;''')
rep(' if(const char* p=getenv("AHC008_LOCAL_STANDBY_LOG")){',''' if(g.earlyAudit){
  ofstream f(seamAuditPath);
  for(int t=178;t<300;t++){
   auto& e=g.earlyTape[t];
   if(e.sent.empty())continue;
   f<<t<<' '<<e.stage<<' '<<e.line<<' '<<e.start<<' '<<e.root<<' '<<e.rootArea<<' '
      <<e.rootAnimals<<' '<<e.required<<' '<<e.projected<<' '<<e.projectedPets<<' '
      <<e.eta<<' '<<e.walls<<' '<<e.mode<<' '
      <<(e.raw.empty()?"-":e.raw)<<' '<<(e.repaired.empty()?"-":e.repaired)<<' '
      <<(e.gated.empty()?"-":e.gated)<<' '<<e.sent;
   for(int p:e.cells)f<<' '<<p;
   f<<'\\n';
  }
 }
 if(const char* p=getenv("AHC008_LOCAL_STANDBY_LOG")){''')
out.write_text(s)
