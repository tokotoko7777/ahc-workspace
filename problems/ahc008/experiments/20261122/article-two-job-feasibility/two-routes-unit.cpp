#include "two_routes.hpp"
#include <random>
#include <iostream>
using namespace tworoutes;
int oracle(const std::vector<std::vector<Path>>& a,int k){
 int ans=INF;
 auto rec=[&](auto&& self,int w,int use,int time)->void{
  if(w==a.size()){
   if(use==(1<<k)-1)ans=std::min(ans,time);
   return;
  }
  for(auto p:a[w]){
   int m=(p.a<0?0:1<<p.a)|(p.b<0?0:1<<p.b);
   if(!(use&m))self(self,w+1,use|m,std::max(time,p.t));
  }
 };
 rec(rec,0,0,0);
 return ans;
}
int main(){
 std::mt19937 rng(509);
 for(int t=0;t<2000;t++){
  int m=1+rng()%4,k=1+rng()%(m*2);
  std::vector<std::vector<Path>> a(m);
  for(auto &w:a){
   w.emplace_back();
   for(int i=0;i<k;i++)if(rng()%3)w.push_back({i,-1,0,0,int(1+rng()%12)});
   for(int i=0;i<k;i++)for(int j=i+1;j<k;j++)if(rng()%4==0)w.push_back({i,j,0,0,int(2+rng()%12)});
  }
  assert(choose(a,k).time==oracle(a,k));
 }
 Board ext;ext.fill(1);std::vector<int> cuts={15*30+11,15*30+12,15*30+14};
 for(int x:cuts)ext[x]=0;
 std::vector<int> humans={15*30+10,15*30+15};
 auto r=solve(ext,cuts,humans);
 assert(r.time==5);
 int m=0;
 for(int i=0;i<(int)r.work.size();i++){
  auto p=r.work[i];
  if(p.a<0)continue;
  assert(!(m>>p.a&1));m|=1<<p.a;
  BFS d;d.run(humans[i],ext);
  const auto n1=neighbors(cuts[p.a]);
  assert(std::find(n1.begin(),n1.end(),p.sa)!=n1.end());
  assert(d.d[p.sa]==p.firstTravel);
  assert(p.t<=r.time);
  if(p.b>=0){
   assert(!(m>>p.b&1));m|=1<<p.b;
   d.run(p.sa,ext);assert(d.d[p.sb]==p.secondTravel);
   const auto n2=neighbors(cuts[p.b]);
   assert(std::find(n2.begin(),n2.end(),p.sb)!=n2.end());
   assert(d.d[p.sb]+p.firstTravel+2==p.t);
  }else assert(p.firstTravel+1==p.t);
 }
 assert(m==7);
 std::cout<<"2,000 exact mask oracles and real 3-wall/2-worker fixture passed\n";
}
