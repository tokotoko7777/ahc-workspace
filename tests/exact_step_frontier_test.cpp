#include "../library/exact-step-frontier.hpp"
#include <array>
#include <random>

int main(){
 using Bits=std::bitset<9>;
 std::mt19937 rng(22691);
 for(int rep=0;rep<7000;++rep){
  constexpr int N=9;
  std::array<std::array<int,N>,N> adj{};
  for(int i=0;i<N;i++)for(int v=0;v<N;++v)adj[i][v]=(rng()%4==0)?v:-1;
  Bits walls,start;
  for(int i=0;i<N;++i)if(rng()%3==0)walls.set(i);else if(rng()%4==0)start.set(i);
  const auto lengths=(rep%9==0)?std::uint64_t(1)<<63:std::uint64_t(rng()%256);
  const bool stay=(rep%2)==0;
  Bits got;
  bool dp[64][N]{};
  for(int i=0;i<N;++i)dp[0][i]=start[i];
  for(int k=0;k<64;++k){
   for(int q=0;q<N;++q)if(dp[k][q]){
    if(lengths>>k&1)got.set(q);
    if(k==63)continue;
    bool allowed=false;
    for(int v=0;v<N;++v)if(adj[q][v]==v&&!walls[v]){
     dp[k+1][v]=true;allowed=true;
    }
    if(stay&&!allowed)dp[k+1][q]=true;
   }
  }
  assert(got==ahc::exact_step_frontier(adj,walls,start,lengths,stay));
 }
}
