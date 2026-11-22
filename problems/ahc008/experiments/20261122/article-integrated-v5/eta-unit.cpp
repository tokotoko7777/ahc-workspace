#include "last_capture.hpp"
#include <cassert>
#include <random>
#include <algorithm>
#include <iostream>
using namespace lastcap;
int brute(const CrewCosts& d,int n,int m){
 std::array<int,10> p;for(int i=0;i<m;i++)p[i]=i;
 int ans=INF;
 do{int mx=0;for(int i=0;i<n;i++)mx=std::max(mx,d[i][p[i]]);ans=std::min(ans,mx);}
 while(std::next_permutation(p.begin(),p.begin()+m));
 return ans;
}
int main(){
 std::mt19937 g(0xabcddcab);
 for(int k=0;k<3000;k++){
  int m=1+g()%7,n=1+g()%m;
  CrewCosts d;for(auto& a:d)for(int &x:a)x=g()%4?int(g()%50):INF;
  assert(crewBottleneck(d,n,m)==brute(d,n,m));
 }
 // Workers are physically close to a pending gate from across the center
 // corridor, but the certified cut excludes both a near stance and route.
 // The final exterior remains connected through the lower board.
 Board open{},ext{};open.fill(1);ext=open;
 int q1=5*S+8,q2=6*S+8;
 for(int r=0;r<16;r++)ext[r*S+8]=0;
 std::vector<int> h={4*S+7,7*S+7};
 // Only the right-side stance is allowed for each of these two jobs.
 ext[5*S+7]=0;ext[6*S+7]=0;
 int fast=exteriorBuildEta(open,{q1,q2},h);
 int safe=exteriorBuildEta(ext,{q1,q2},h);
 assert(fast==1&&safe>15&&safe<INF);
 // Distinct constraint binds even when both gates individually reach
 // the same worker without any travel.
 CrewCosts special;for(auto& row:special)row.fill(INF);
 special[0][0]=special[1][0]=0;special[0][1]=19;special[1][1]=22;
 assert(crewBottleneck(special,2,2)==19);
 std::cout<<fast<<" vs "<<safe<<"\n";
}
