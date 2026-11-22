#include "last_capture.hpp"
#include <cassert>
#include <iostream>
using namespace lastcap;
constexpr int xy(int r,int c){return r*S+c;}
int main(){
 std::vector<int> chamber{xy(10,10),xy(10,11),xy(10,12),xy(11,10),xy(12,10)};
 const int g1=xy(10,13),g2=xy(13,10);
 Board initial{};
 for(int p:chamber)for(int q:neighbors(p))
  if(q>=0&&q!=g1&&q!=g2&&std::find(chamber.begin(),chamber.end(),q)==chamber.end())initial[q]=1;
 for(bool blocked:{false,true}){
  std::vector<int> hs={xy(10,14),blocked?xy(14,10):xy(14,11),xy(12,16),xy(17,8),xy(5,6)};
  std::vector<Pet> pets{{blocked?xy(12,10):xy(10,10),1}};
  Planner p;p.cfg.start=0;p.began=200;p.deadline=225;p.active.cut={g1,g2};
  auto w=initial;
  auto play=[&](int t){
   auto r=p.plan(t,w,hs,pets);
   assert(r.has_value());
   Board forbid=Planner::occupied(hs,pets),build{};
   auto before=hs;
   for(int i=0;i<int(hs.size());i++){
    char c=(*r)[i];if(c=='.')continue;
    auto ops=c>='a'?BUILD:MOVE;int d=std::find(ops,ops+4,c)-ops;assert(d<4);
    int q=neighbors(before[i])[d];assert(q>=0&&!w[q]);
    if(c>='a'){assert(!forbid[q]&&!build[q]);build[q]=1;}
    else hs[i]=q;
   }
   for(int q=0;q<V;q++)if(build[q])w[q]=1;
   BFS f;f.run(hs[0],Planner::walkable(w));
   for(int h:hs)assert(f.d[h]>=0&&!w[h]);
   return r.value();
  };
  auto a=play(200);
  assert(a[0]=='l'&&w[g1]&&!w[g2]&&p.finalSince==-1);
  if(!blocked)assert(a[1]=='L');else assert(a[1]=='.'),pets[0].p=xy(11,10);
  auto b=play(201);
  assert(b[1]=='u'&&w[g2]);
  BFS f;f.run(hs[0],Planner::walkable(w));assert(f.d[pets[0].p]<0);
 }
}
