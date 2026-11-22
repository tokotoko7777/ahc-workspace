#define main fixture_main
#include "integrated.cpp"
#undef main

int main(){
 for(int p=0;p<V;p++){
  int r=p/30,c=p%30;
  nb[p]={r?p-30:-1,r<29?p+30:-1,c?p-1:-1,c<29?p+1:-1};
 }
 Game g;g.bankMode=5;g.narrowRescue=true;
 for(int i=0;i<8;i++)g.humans.push_back(33+i*47);
 g.makeBanks(); assert(g.banks.size()==52);
 auto &a=g.banks[0],&b=g.banks[1],&c=g.banks[2];
 assert(a.b==b.b&&b.b==c.b&&b.r==a.r+2&&c.r==b.r+2);
 set<int> shared;for(int p:a.side)if(find(b.side.begin(),b.side.end(),p)!=b.side.end())shared.insert(p);
 assert(shared.size()==4);
 int gateShoulder=*shared.begin();
 bitset<V> wall;for(int p:a.side)if(p!=gateShoulder)wall[p]=true;
 for(int p:b.side)if(p!=gateShoulder)wall[p]=true;
 auto missing=[&](const auto &x){return count_if(x.side.begin(),x.side.end(),[&](int p){return !wall[p];});};
 assert(missing(a)==1&&missing(b)==1&&missing(c)==4);
 assert(Game::bankBraceCredit(missing(a))+Game::bankBraceCredit(missing(b))==24);
 wall[gateShoulder]=true;
 assert(missing(a)==0&&missing(b)==0&&Game::bankBraceCredit(missing(b))==0);
 assert(Game::bankPartnerTie(2,4)==1&&Game::bankPartnerTie(1,0)==5);
 assert(Game::bankPartnerTie(3,0)==0&&Game::bankPartnerTie(1,5)==0);
 bitset<V> shoulders;array<int,V> body;body.fill(-1);
 for(int p:g.bankWalls)shoulders[p]=true;
 for(int i=0;i<int(g.banks.size());i++)for(int p:g.banks[i].inner)body[p]=i;
 Game::BankPath path;
 path.run(b.h,shoulders,body);
 assert(path.d[a.h]==2); // distinct partner on opposite aisle, axial row change
 assert(path.d[a.inner[1]]<0); // no route through a planned cage body
 assert(path.d[a.l]>=7); // must take real exterior way, not cross the cage
}
