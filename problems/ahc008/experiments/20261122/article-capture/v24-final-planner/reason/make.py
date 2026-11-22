from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
rep(' lastcap::Planner finalCapture;',''' lastcap::Planner finalCapture;
 struct EndTrace{int t,left,size,oldBegin,oldRetry,tries,escapes,placed,closes,newBegin,tier,inOld,onOld;bool active;};
 vector<EndTrace> endTrace;
 bool endTraceOn=false;''')
a='''   if(auto q=finalCapture.plan(turn,wall,humans,alive,roads,retreat)){
    integratedMode[turn]=finalCapture.protectionTier==1?3:5;
    return *q;
   }'''
b='''   if(!endTraceOn){
    if(auto q=finalCapture.plan(turn,wall,humans,alive,roads,retreat)){
     integratedMode[turn]=finalCapture.protectionTier==1?3:5;
     return *q;
    }
   }else{
    int ob=finalCapture.began,orr=finalCapture.retryAt,ta=finalCapture.attempts,
        ee=finalCapture.escapes,pp=finalCapture.placed,cc=finalCapture.closes;
    int inOld=0,onOld=0;
    if(ob>=0){
     const auto& shape=finalCapture.active;
     for(auto a:pets)if(group[a.p]==principal){
      if(!shape.can[a.p])++inOld;
      if(find(shape.cut.begin(),shape.cut.end(),a.p)!=shape.cut.end())++onOld;
     }
    }
    auto q=finalCapture.plan(turn,wall,humans,alive,roads,retreat);
    endTrace.push_back({turn,rem,currentCt[principal],ob,orr,finalCapture.attempts-ta,
      finalCapture.escapes-ee,finalCapture.placed-pp,finalCapture.closes-cc,
      finalCapture.began,finalCapture.protectionTier,inOld,onOld,bool(q)});
    if(q){integratedMode[turn]=finalCapture.protectionTier==1?3:5;return *q;}
   }'''
rep(a,b)
rep(' g.setupObservedMotion();',' g.setupObservedMotion();\n g.endTraceOn=getenv("AHC008_END_REASON")!=nullptr;')
rep(' if(const char* p=getenv("AHC008_TERRY_LOG")){',''' if(const char* p=getenv("AHC008_END_REASON")){
  ofstream f(p);
  for(const auto& e:g.endTrace)
   f<<e.t<<','<<e.left<<','<<e.size<<','<<e.oldBegin<<','<<e.oldRetry<<','<<e.tries<<','<<e.escapes
    <<','<<e.placed<<','<<e.closes<<','<<e.newBegin<<','<<e.tier<<','<<e.inOld<<','<<e.onOld<<','<<e.active<<'\\n';
 }
 if(const char* p=getenv("AHC008_TERRY_LOG")){''')
Path(sys.argv[2]).write_text(s)
