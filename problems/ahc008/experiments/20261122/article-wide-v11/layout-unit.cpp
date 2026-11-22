#define main fixture_main
#include "integrated.cpp"
#undef main
int main(){
 for(int p=0;p<V;p++){
  int r=p/30,c=p%30;
  nb[p]={r?p-30:-1,r<29?p+30:-1,c?p-1:-1,c<29?p+1:-1};
 }
 for(int m=5;m<=10;m++)for(int pick=0;pick<3;pick++){
  Game g;g.bankMode=5;g.narrowRescue=true;
  for(int i=0;i<m;i++)g.humans.push_back(((i*3+pick*7)%30)*30+(i*11+pick*5)%30);
  for(int j=0;j<13;j++)g.pets.push_back({((j*2+pick*5)%30)*30+(j*13+pick)%30,1+(j%5)});
  g.makeBanks();
  const int n=m<=6?36:52;
  assert((int)g.banks.size()==n);
  assert(g.bankWalls.size()==(m<=6?156:224));
  bitset<V> put,expected;
  for(int p:g.bankWalls){assert(0<=p&&p<V);put[p]=true;}
  for(auto& x:g.banks){
   assert(!put[x.g]&&!put[x.k]&&!put[x.l]&&!put[x.h]);
   put[x.g]=put[x.k]=true;
   for(int p:x.inner){assert(!expected[p]&&!put[p]);expected[p]=true;}
   assert(dir(x.l,x.g)>=0&&dir(x.h,x.k)>=0);
  }
  const auto flood=[&](int src){
   bitset<V> seen;vector<int> q{src};assert(!put[src]);seen[src]=true;
   for(int i=0;i<(int)q.size();i++)for(int p:nb[q[i]])
    if(p>=0&&!put[p]&&!seen[p])seen[p]=true,q.push_back(p);
   return seen;
  };
  bitset<V> main=flood(0);
  assert(main.count()==900-put.count()-4*n);
  for(auto& x:g.banks){
   assert(main[x.l]&&main[x.h]);
   const auto here=flood(x.inner[0]);
   assert(here.count()==4&&(here&expected)==here);
  }
 }
}
