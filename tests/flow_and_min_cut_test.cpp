#include "../library/max-flow.hpp"
#include <cassert>
#include <random>
#include <tuple>
int main(){
  using Flow=MaxFlow<long long>;
  std::mt19937 random(17419);
  for(int j=0;j<10000;++j){
    const int n=2+random()%7, m=random()%36;
    Flow one(n), two(n);
    std::vector<std::tuple<int,int,long long>> edges;
    for(int k=0;k<m;++k){
      int a=random()%n,b=random()%n;
      const long long c=random()%21;
      one.add_edge(a,b,c);two.add_edge(a,b,c);
      edges.emplace_back(a,b,c);
    }
    const auto value=one.flow(0,n-1);
    const auto reachable=one.min_cut(0);
    long long least=1e18;unsigned intersection=(1u<<n)-1;
    for(unsigned mask=1;mask<(1u<<(n-1));mask+=2){
      long long cost=0;
      for(auto [a,b,c]:edges)if((mask>>a&1)&&!(mask>>b&1))cost+=c;
      if(cost<least){least=cost;intersection=mask;}
      else if(cost==least)intersection&=mask;
    }
    auto [amount,side]=two.flow_and_min_cut(0,n-1);
    assert(amount==value&&value==least&&side==reachable);
    for(int v=0;v<n;++v)assert(side[v]==int(intersection>>v&1));
    // If previous limited calls leave residuals, the combined operation
    // returns only the additional flow, while the canonical cut is unchanged.
    two=Flow(n);
    for(auto [a,b,c]:edges)two.add_edge(a,b,c);
    const auto prefix=two.flow(0,n-1,random()%28);
    auto [rest,last]=two.flow_and_min_cut(0,n-1,j%2==0);
    assert(prefix+rest==least&&last==side);
  }
  MaxFlow<int> a(2);
  a.add_edge(0,1,std::numeric_limits<int>::max());
  auto [x,side]=a.flow_and_min_cut(0,1);
  assert(x==std::numeric_limits<int>::max()&&side[0]&&!side[1]);
  a=MaxFlow<int>(2);
  a.add_edge(0,1,std::numeric_limits<int>::max());a.add_edge(0,1,1);
  auto throws=false;
  try{a.flow_and_min_cut(0,1);}catch(const std::overflow_error&){throws=true;}
  assert(throws);
}
