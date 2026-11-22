"""Assemble the first staged-cordon variant on the accepted v7."""
from pathlib import Path
import sys
p=Path(__file__).resolve().parent
s=Path(sys.argv[1]).read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a[:160]
 s=s.replace(a,b)
rep('const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& protect){\n  auto blocked=occupied',
    'const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& protect,int minCore=1,int maxCore=3){\n  auto blocked=occupied')
rep('for(int core:{1,2,3})for(int loss:{1,4})','for(int core=minCore;core<=maxCore;core++)for(int loss:{1,4})')
rep('using namespace std;\nnamespace {\nconstexpr int S=30, V=S*S;',
    (p/'cordon.hpp').read_text()+'\nusing namespace std;\nnamespace {\nconstexpr int S=30, V=S*S;')
rep(' lastcap::Planner finalCapture;', ' lastcap::Planner finalCapture;\n cordon::Front front;')
rep('  if(turn>=180&&rem>0&&rem<=2&&!overBudget()){','''  if(turn>=168&&rem>0&&rem<=3&&!overBudget()){
   vector<lastcap::Pet> relevant;
   for(auto a:pets)relevant.push_back({a.p,a.t});
   if(auto q=front.act(turn,wall,humans,relevant)){
    integratedMode[turn]=6;
    return *q;
   }
  }
  if(turn>=180&&rem>0&&rem<=2&&!overBudget()){''')
rep(' if(const char* p=getenv("AHC008_ARTICLE_LOG")){',''' if(const char* p=getenv("AHC008_CORDON_LOG")){
  ofstream out(p);
  const auto& q=g.front;
  out<<q.attempts<<" "<<q.shellWalls<<" "<<q.closes<<" "<<q.aborted<<" "
     <<q.heldOutside<<" "<<q.groundGain<<'\\n';
 }
 if(const char* p=getenv("AHC008_ARTICLE_LOG")){''')
(p/'solve.cpp').write_text(s)
