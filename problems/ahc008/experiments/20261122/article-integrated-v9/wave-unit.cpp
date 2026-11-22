#define main integrated_test_main
#include "integrated.cpp"
#undef main
int main(){
 auto xy=[](int r,int c){return r*30+c;};
 for(int p=0;p<V;p++){
  int r=p/30,c=p%30;
  nb[p]={r?p-30:-1,r<29?p+30:-1,c?p-1:-1,c<29?p+1:-1};
 }
 auto run=[&](bool obstruction){
  Game g;
  g.humans={xy(2,2),xy(5,9),xy(7,16),xy(12,24),xy(14,28),xy(28,9),xy(24,24)};
  g.bankMode=5;g.narrowRescue=true;g.makeBanks();
  g.chooseWaves();
  assert(g.waveSelected.size()==3&&g.waveNominal>=30&&g.waveDeadline<=75);
  bitset<V> installed;
  set<int> who;
  for(auto &z:g.waveSelected)for(auto &s:z.side){
   assert(who.insert(s.actor).second&&s.work.size()==12);
   for(auto j:s.work){
    assert(dir(j.stance,j.wall)>=0&&g.waveFoot[j.wall]&&!g.waveFoot[j.stance]);
    assert(!installed[j.wall]);installed[j.wall]=1;
   }
  }
  assert(installed.count()==72);
  const int dead=g.waveSelected[0].side[0].work[0].wall;
  if(obstruction)g.pets.push_back({dead,1});
  for(g.turn=1;g.turn<=g.waveDeadline+1;g.turn++){
   bitset<V> u,b,m,r;
   g.act.assign(g.humans.size(),'.');g.block=g.wall;
   for(int p:g.humans)g.block[p]=1;
   for(auto p:g.pets){g.block[p.p]=1;for(int x:nb[p.p])if(x>=0)g.block[x]=1;}
   // One mock prior exact-capture use at t1 consumes the owner only
   // for this fixture; do not apply it to the test board.
   int guarded=g.waveSelected[0].side[0].actor;
   if(g.turn==1){u[guarded]=1;g.act[guarded]='.';}
   g.executeWaves(u,b,m,r);
   if(g.turn==1)assert(g.act[guarded]=='.');
   auto after=g.humans;
   bitset<V> put;
   for(int i=0;i<(int)g.humans.size();i++){
    char c=g.act[i];if(c=='.')continue;
    int h=g.humans[i],d=int(islower(c)?find(BUILD,BUILD+4,c)-BUILD:find(MOVE,MOVE+4,c)-MOVE);
    assert(d<4);
    int q=nb[h][d];assert(q>=0&&!g.wall[q]);
    if(islower(c)){assert(!g.block[q]&&!put[q]&&g.waveFoot[q]);put[q]=1;}
    else after[i]=q;
   }
   for(int x:after)assert(!put[x]);
   g.humans=after;
   for(int p=0;p<V;p++)if(put[p])g.wall[p]=1;
  }
  int got=0,abort=0;
  for(auto row:g.waveLog){got+=row[1]==1;abort+=row[1]==2;}
  assert(got+abort==3&&none_of(g.waveSelected.begin(),g.waveSelected.end(),[](auto& x){return x.active;}));
  if(!obstruction){assert(got==3&&abort==0);}
  else assert(abort>=1&&!g.wall[dead]);
 };
 run(false);run(true);
}
