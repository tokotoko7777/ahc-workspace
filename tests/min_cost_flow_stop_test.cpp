#include "../library/min-cost-flow.hpp"
#include <cassert>
#include <utility>

MinCostFlow<int,long long> make_flow(){
  MinCostFlow<int,long long> f(4);
  f.add_edge(0,1,1,0);
  f.add_edge(1,3,1,7);
  f.add_edge(0,2,2,3);
  f.add_edge(2,3,2,8);
  return f;
}

int main(){
  auto halted=make_flow();
  assert((halted.flow_until(0,3,3,[]{return true;})==std::pair<int,long long>{0,0}));
  auto partial=make_flow();
  int checks=0;
  assert((partial.flow_until(0,3,3,[&]{return checks++>0;})==std::pair<int,long long>{1,7}));
  assert(partial.get_edge(0).flow==1 && partial.get_edge(2).flow==0);
  auto full=make_flow();
  assert((full.flow(0,3,3)==std::pair<int,long long>{3,29}));
  auto unsatisfied=make_flow();
  assert((unsatisfied.flow_until(0,3,10,[]{return false;})==std::pair<int,long long>{3,29}));
}
