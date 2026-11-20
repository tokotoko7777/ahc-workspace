#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <random>
#include <utility>
#include <vector>
#include "../library/sorted-bottleneck-assignment.hpp"

int main() {
  std::mt19937 g(91853);
  for(int scenario=0;scenario<1800;++scenario){
    int n=g()%6,m=n+g()%4;
    std::vector<long long> t(n),v(m);
    for(auto& x:t)x=int(g()%29)-14;
    for(auto& x:v)x=int(g()%29)-14;
    std::sort(t.begin(),t.end());
    std::sort(v.begin(),v.end());
    auto ans=sorted_bottleneck_assignment(t,v);
    std::pair<long long,long long> best{LLONG_MAX,LLONG_MAX};
    auto visit=[&](auto&& self,int i,int mask,long long maximum,long long sum)->void{
      if(i==n){best=std::min(best,std::pair{maximum,sum});return;}
      for(int j=0;j<m;++j)if(!(mask>>j&1)){
        auto d=std::abs(t[i]-v[j]);
        self(self,i+1,mask|(1<<j),std::max(maximum,d),sum+d);
      }
    };
    visit(visit,0,0,0,0);
    assert(std::is_sorted(ans.begin(),ans.end()));
    assert(std::adjacent_find(ans.begin(),ans.end())==ans.end());
    long long maximum=0,sum=0;
    for(int i=0;i<n;++i){auto d=std::abs(t[i]-v[ans[i]]);maximum=std::max(maximum,d);sum+=d;}
    assert((std::pair{maximum,sum}==best));
  }
}
