#include "../library/max-flow.hpp"
#include <cassert>

int main() {
  MaxFlow<long long> cut(4);
  cut.add_edge(0,1,2);
  cut.add_edge(0,2,1);
  cut.add_edge(1,2,1);
  cut.add_edge(1,3,1);
  cut.add_edge(2,3,3);
  const auto [value, source_side] = cut.flow_and_min_cut(0,3);
  assert(value==3 && source_side[0] && !source_side[3]);
}
