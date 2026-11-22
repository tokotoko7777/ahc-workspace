#define main fixture_main
#include "integrated.cpp"
#undef main
int main(){
 for(int x=0;x<V;x++){
  int r=x/30,c=x%30;
  nb[x]={r?x-30:-1,r<29?x+30:-1,c?x-1:-1,c<29?x+1:-1};
 }
 for(int m=5;m<=7;m++){
  Game g;g.bankMode=5;g.narrowRescue=true;
  for(int i=0;i<m;i++)g.humans.push_back(33+47*i);
  g.makeBanks();g.initBraceRoutes();
  if(m>6){assert(g.braceNominal<0);continue;}
  assert(g.braceNominal>15&&g.braceNominal<120);
  bitset<V> expected,seen;
  for(int p:g.bankWalls)expected[p]=true;
  for(int i=0;i<m;i++)for(auto x:g.braceRoute[i]){
   assert(!expected[x.stance]&&g.braceEgress[i]>=0&&!expected[g.braceEgress[i]]);
   for(int p:x.work){
    assert(expected[p]&&!seen[p]&&dir(x.stance,p)>=0);
    seen[p]=true;
   }
  }
  assert(seen==expected&&seen.count()==112);
 }
}
