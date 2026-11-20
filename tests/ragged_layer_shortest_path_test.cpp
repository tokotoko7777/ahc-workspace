#include "../library/ragged-layer-shortest-path.hpp"
#include <array>
#include <cassert>
#include <functional>
#include <limits>
#include <random>
using namespace std;
int main(){
  auto empty=ragged_layer_shortest_path<int>({},[](int){return -1;},[](int,int,int){return -1;});
  assert(empty.first==0&&empty.second.empty());
  mt19937 gen(28281);
  for(int t=0;t<1000;++t){
    vector<int> widths(1+gen()%6);
    for(int& w:widths)w=1+gen()%4;
    array<int,4> start;
    for(int& a:start)a=int(gen()%101)-50;
    array<array<array<int,4>,4>,6> cost;
    for(auto& layer:cost)for(auto& a:layer)for(int& x:a)x=int(gen()%101)-50;
    auto [answer,path]=ragged_layer_shortest_path<int>(widths,[&](int c){return start[c];},
      [&](int layer,int a,int b){return cost[layer][a][b];});
    int best=numeric_limits<int>::max();
    function<void(int,int,int)> brute=[&](int layer,int previous,int total){
      if(layer==int(widths.size())){best=min(best,total);return;}
      for(int q=0;q<widths[layer];++q)
        brute(layer+1,q,total+(layer?cost[layer][previous][q]:start[q]));
    };
    brute(0,0,0);
    assert(path.size()==widths.size());
    int replay=0;
    for(int i=0;i<int(path.size());++i){
      assert(path[i]>=0&&path[i]<widths[i]);
      replay+=i?cost[i][path[i-1]][path[i]]:start[path[i]];
    }
    assert(answer==best&&answer==replay);
  }
}
