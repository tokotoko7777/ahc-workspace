#include "last_capture.hpp"
#include <cassert>
#include <iostream>
using namespace lastcap;
constexpr int xy(int r,int c){return r*S+c;}
int main(){
 std::vector<int> pen{xy(10,10),xy(10,11),xy(10,12),xy(11,10),xy(12,10)};
 const int g1=xy(10,13),g2=xy(13,10);
 Board w{};
 for(int q:pen)for(int p:neighbors(q))
  if(p>=0&&p!=g1&&p!=g2&&std::find(pen.begin(),pen.end(),p)==pen.end())w[p]=1;
 std::vector<int> hs={xy(10,14),xy(14,10),xy(9,13),xy(3,3),xy(2,2),xy(8,8)};
 std::vector<Pet> a={{xy(10,10),1},{xy(0,0),1},{xy(22,19),2}};
 Planner original;original.cfg.start=160;original.teamFocus=0;original.teamMask=3;original.teamDeadline=205;
 original.began=160;original.deadline=205;original.active.cut={g1,g2};
 original.placed=7;original.total=9;
 for(auto setup:{std::pair{'D',false},std::pair{'d',false},std::pair{'R',true}}){
  Planner kept=original,trial=original;
  auto acts=trial.plan(170,w,hs,a);
  assert(acts&&(*acts)[0]=='l'&&(*acts)[1]=='u'&&(*acts)[2]=='.');
  assert(trial.placed==9&&trial.total==10);
  std::string normal="......";normal[2]=setup.first;
  auto merged=composeTeam(kept,std::move(trial),w,hs,a,normal,*acts);
  assert(bool(merged)==setup.second);
  if(!merged){
   assert(kept.placed==7&&kept.total==9&&kept.closes==0&&kept.active.cut==original.active.cut);
   continue;
  }
  assert((*merged)[2]==normal[2]&&kept.placed==9&&kept.total==10&&kept.teamArea==5);
  Board after=w;after[g1]=after[g2]=1;hs[2]+=1;
  BFS root;root.run(hs[0],Planner::walkable(after));
  assert(root.d[a[0].p]<0&&root.d[a[1].p]>=0);
  for(int h:hs)assert(root.d[h]>=0);
 }
}
