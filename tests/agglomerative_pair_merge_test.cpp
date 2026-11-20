#include "../library/agglomerative-pair-merge.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <vector>
using namespace std;
int main(){
  mt19937 gen(37037);
  for(int trial=0;trial<1300;++trial){
    const int count=gen()%27;
    vector<int> v(count);
    for(auto& x:v)x=int(gen()%101)-50;
    auto rank=[](int a,int b){return std::pair{-abs(a-b),max(a,b)};};
    auto merge=[](int a,int b){return min(a,b);};
    auto tree=agglomerative_pair_merge(v,rank,merge);
    vector<pair<int,int>> children(count,{-1,-1});
    vector<bool> active(count,true);
    for(int i=count;i<2*count-1;++i){
      tuple<pair<int,int>,int,int> chosen{{-200,-200},-1,-1};
      for(int a=0;a<i;++a)for(int b=a+1;b<i;++b)if(active[a]&&active[b])
        chosen=max(chosen,tuple{rank(v[a],v[b]),a,b});
      auto [value,a,b]=chosen;
      active[a]=active[b]=false;active.push_back(true);
      v.push_back(merge(v[a],v[b]));
      children.emplace_back(a,b);
    }
    assert(tree.values==v&&tree.children==children);
  }
}
