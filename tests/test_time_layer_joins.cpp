#pragma GCC optimize("O3")
#include "time-layer-joins.hpp"
#include <cassert>
#include <iostream>
#include <random>
#include <array>
#include <functional>
int main(){
 using I=std::int64_t;
 constexpr I inf=ahc::TimeLayerJoin::infinity;
 std::mt19937 rng(28401);
 size_t checks=0;
 for(int j=0;j<4200;++j){
  int n=1+rng()%7,T=rng()%7,B=rng()%(T+1),M=B+rng()%(T-B+1),S=rng()%n,G=rng()%n;
  I cost[6][7][7]{};
  bool edge[7][7]{};
  for(int v=0;v<n;++v)for(int u=0;u<n;++u){
   edge[v][u]=rng()%4<2;
   for(int s=0;s<T;++s)cost[s][v][u]=(rng()%4==0?inf:I(rng()%23));
  }
  auto go=[&](int u,auto use){for(int v=0;v<n;++v)if(edge[u][v])use(v);};
  auto weight=[&](int t,int u,int v){return cost[t][u][v];};
  auto exact=ahc::time_layer_joins(n,B,M,T,S,G,go,weight);
  std::array<I,7> expected{},pre{},post{};
  expected.fill(inf);pre.fill(inf);post.fill(inf);
  auto enumerate=[&](auto&& self,int t,int u,int reached,I subtotal)->void{
   if(t==M){reached=u;pre[u]=std::min(pre[u],subtotal);}
   if(t==T){if(u==G)expected[reached]=std::min(expected[reached],subtotal);return;}
   go(u,[&](int v){if(cost[t][u][v]<inf)self(self,t+1,v,reached,subtotal+cost[t][u][v]);});
  };
  enumerate(enumerate,B,S,0,0);
  for(int z=0;z<n;++z){
   auto reverse_truth=[&](auto&& self,int t,int u,I subtotal)->void{
    if(t==T){if(u==G)post[z]=std::min(post[z],subtotal);return;}
    go(u,[&](int v){if(cost[t][u][v]<inf)self(self,t+1,v,subtotal+cost[t][u][v]);});
   };
   reverse_truth(reverse_truth,M,z,0);
   assert(expected[z]==exact.via(z));
   assert(pre[z]==exact.arrive[z]&&post[z]==exact.onward[z]);
   ++checks;
  }
 }
 auto x=ahc::time_layer_joins(2,0,1,2,0,0,
       [](int,auto v){v(0);v(1);},[&](int,int,int){return inf-1;});
 assert(x.via(0)==inf);
 std::cout<<checks<<" routes match exhaustive\n";
}
