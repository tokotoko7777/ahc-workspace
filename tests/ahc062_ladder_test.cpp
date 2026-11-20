#include <cassert>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>
#include "../problems/ahc062/ladder.hpp"

int main() {
  std::mt19937 rng(62007);
  for(int n:{6,8,10,12,30,200})
    for(int seed=0;seed<8;++seed){
      std::vector<int> w(n*n);
      for(auto& v:w)v=rng()%10000;
      for(int flags=0;flags<16;++flags){
        auto route=two_lane_king_path(n,w,flags);
        std::vector<char> seen(n*n);
        for(int i=0;i<int(route.size());++i){
          int u=route[i];assert(0<=u&&u<n*n&&!seen[u]);seen[u]=true;
          if(i){
            int v=route[i-1];
            assert(std::max(std::abs(u/n-v/n),std::abs(u%n-v%n))==1);
          }
        }
        assert(std::all_of(seen.begin(),seen.end(),[](bool v){return v;}));
      }
    }
}
