#include <algorithm>
#include <cassert>
#include <random>
#include "../library/rooted-terminal-tree.hpp"

int main() {
  std::mt19937 rng(200451);
  for(int trial=0;trial<550;++trial){
    const int n=1+rng()%101;
    std::vector<int> parent(n),order{0};
    for(int i=1;i<n;++i)parent[i]=rng()%i,order.push_back(i);
    std::vector<unsigned char> actual;
    for(int run=0;run<24;++run){
      std::vector<unsigned char> wanted(n),ref(n);
      for(int v=0;v<n;++v)wanted[v]=rng()%(trial%13+1)==0;
      rooted_terminal_tree(order,parent,[&](int v){return wanted[v];},actual);
      for(int v=0;v<n;++v)if(wanted[v]){
        for(int u=v;;u=parent[u]){
          ref[u]=1;
          if(u==0)break;
        }
      }
      assert(actual==ref);
    }
  }
}
