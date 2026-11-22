#define main test_interactive_main
#include "integrated.cpp"
#undef main
int main(){
 auto xy=[](int r,int c){return r*30+c;};
 for(int p=0;p<900;p++){
  int r=p/30,c=p%30;
  nb[p]={r?p-30:-1,r<29?p+30:-1,c?p-1:-1,c<29?p+1:-1};
 }
 auto fresh=[&](){
  Game g;g.turn=152;g.bankRot=0;
  for(int r:{13,15}){
   Game::TubeBank x{0,r,xy(r,4),xy(r,5),xy(r,10),xy(r,11),{},{}};
   for(int j=0;j<4;j++)x.inner[j]=xy(r,6+j);
   g.banks.push_back(x);
  }
  g.humans={xy(13,4),xy(10,4),xy(2,2),xy(6,2),xy(8,2)};
  g.pets={{xy(25,25),1}};
  g.act=".....";return g;
 };
 for(int bad=0;bad<7;bad++){
  auto g=fresh();bitset<V> used,build,moved,paired;
  vector<int> ready={1,1};
  if(bad==0)used[0]=1;
  if(bad==1)g.humans[2]=xy(7,11);
  if(bad==2)g.pets[0].p=xy(13,7);
  if(bad==3)build[xy(13,5)]=1;
  if(bad==4)paired[0]=1;
  if(bad==5)g.turn=153;
  if(bad==6)ready[1]=0;
  auto before=g.act;
  g.maybeStartBridge(ready,used,build,moved,paired);
  assert(g.crossing.actor<0&&g.act==before&&g.bridgeReport.empty());
 }
 auto run=[&](bool blocked){
  auto g=fresh();bitset<V> used,build,moved,paired;
  g.maybeStartBridge({1,1},used,build,moved,paired);
  assert(g.crossing.actor==0&&used[0]&&g.act=="R....");
  assert(g.crossing.reserves(0)&&!g.crossing.reserves(1));
  for(int i=0;i<7;i++){
   if(g.act[0]=='R')g.humans[0]++;
   else if(g.act[0]=='L')g.humans[0]--;
   ++g.turn;g.act.assign(5,'.');used.reset();moved.reset();
   if(blocked&&i==2)g.wall[xy(13,10)]=1;
   g.continueBridge(used,moved);
   if(blocked&&g.crossing.actor<0)break;
  }
  assert(!g.crossing.reserves(0)&&g.act==".....");
  assert(g.humans[0]==xy(13,blocked?4:11));
  assert(g.turn<160);
 };
 run(false);run(true);
}
