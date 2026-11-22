#define main ahc008_entry
#include "solve.cpp"
#undef main
int main(){
 for(int p=0;p<V;p++){
  nb[p]={p>=S?p-S:-1,p+S<V?p+S:-1,p%S?p-1:-1,p%S+1<S?p+1:-1};
 }
 for(int m:{5,6,7,10}){
  Game g;g.bankMode=1;g.narrowRescue=true;g.humans.resize(m);
  for(int i=0;i<m;i++)g.humans[i]=pos(i+6,i+6);
  g.makeBanks();
  assert(g.bankN==(m<=6?2:3));
  assert(g.banks.size()==(m<=6?24:36));
  assert(g.bankWalls.size()==(m<=6?112:156));
  bitset<V> walls;
  array<int,V> body;body.fill(-1);
  for(int q:g.bankWalls)walls[q]=true;
  for(int k=0;k<int(g.banks.size());k++){
   const auto& z=g.banks[k];
   walls[z.g]=walls[z.k]=true;
   for(int q:z.inner)body[q]=k;
  }
  if(m<=6)for(int r:{15})for(int c=0;c<g.bankN;c++){
   const int p=g.bp(r,g.bankShift+7*c),q=g.bp(r,g.bankShift+7*(c+1));
   assert(!walls[p]&&!walls[q]);Game::BankPath nav;
   nav.run(p,walls,body);assert(nav.d[q]==7);
  }
  Game::BankPath nav;nav.run(g.banks[0].l,walls,body);
  for(const auto& z:g.banks){
   assert(nav.d[z.l]>=0&&nav.d[z.h]>=0);
   lastcap::Board w{};
   for(int i=0;i<V;i++)w[i]=!walls[i];
   lastcap::BFS b;
   assert(b.run(z.inner.front(),w)==4);
  }
 }
}
