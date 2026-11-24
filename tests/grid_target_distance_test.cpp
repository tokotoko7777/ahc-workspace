#include "grid-target-distance.hpp"
#include <cassert>
#include <iostream>
#include <queue>
#include <random>
#include <utility>

int reference(int h,int w,const std::vector<std::uint8_t>& b,
              const std::vector<int>& goals,int start,bool exit) {
  if(b[start]&&!exit)return -1;
  std::vector<int> d(h*w,-1);
  d[start]=0;std::queue<int> q;q.push(start);
  while(!q.empty()){
    const int t=q.front();q.pop();
    if(std::find(goals.begin(),goals.end(),t)!=goals.end())return d[t];
    for(auto [dr,dc]:std::array<std::pair<int,int>,4>{{{0,1},{1,0},{-1,0},{0,-1}}}){
      const int r=t/w+dr,c=t%w+dc,u=r*w+c;
      if(r>=0&&r<h&&c>=0&&c<w&&!b[u]&&d[u]<0){
        d[u]=d[t]+1;q.push(u);
      }
    }
  }
  return -1;
}
int main(){
  using D=ahc::GridTargetDistance;
  using M=D::BlockedOrigin;
  std::mt19937 rng(0x008a15eu);
  int queries=0;
  for(int test=0;test<1800;++test){
    const int h=1+rng()%10,w=1+rng()%12,n=h*w,k=1+rng()%5;
    std::vector<std::uint8_t> wall(n);
    for(auto& b:wall)b=static_cast<int>(rng()%5)<test%6;
    std::vector<std::vector<int>> goals(k);
    for(auto& g:goals)
      for(int i=0,left=rng()%std::min(10,n+1);i<left;++i)g.push_back(rng()%n);
    const D d(h,w,wall,goals);
    for(int p=0;p<n;++p)for(int j=0;j<k;++j)for(bool exit:{false,true}){
      assert(d.distance(p,j,exit?M::MayExit:M::Unreachable)==reference(h,w,wall,goals[j],p,exit));
      ++queries;
    }
  }
  std::vector<std::uint8_t> wall(65535);
  const std::vector<std::vector<int>> goals{{65534},{0},{}};
  const D long_line(1,65535,wall,goals);
  assert(long_line.distance(0,0)==65534);
  assert(long_line.distance(65534,1)==65534);
  assert(long_line.distance(32000,2)==-1);
  try{D invalid(1,65536,wall,goals);assert(false);}catch(const std::invalid_argument&){}
  wall[0]=1;
  const D start_blocked(1,65535,wall,goals);
  assert(start_blocked.distance(0,0,M::MayExit)==65534);
  assert(start_blocked.distance(0,1,M::MayExit)==0);
  assert(start_blocked.distance(1,1,M::MayExit)==-1);
  assert(start_blocked.distance(0,0)==-1);
  std::cout << queries << " differential queries\n";
}
