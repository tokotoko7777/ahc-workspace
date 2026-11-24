#include "ordered-csr-max-flow.hpp"

#include <cassert>
#include <iostream>

int main() {
  ahc::OrderedCsrMaxFlow<long long> network(4);
  const int tracked = network.add_edge(0, 1, 4);
  network.add_edge(0, 2, 2);
  network.add_edge(1, 3, 3);
  network.add_edge(2, 3, 2);
  network.freeze();
  const auto initial = network.flow(0, 3, 2);
  const auto more = network.flow(0, 3);
  assert(initial == 2 && more == 3 && network.get_edge(tracked).flow == 3);
  const auto side = network.min_cut(0);
  assert(side[0] && side[1] && !side[2] && !side[3]);
  std::cout << initial + more << '\n';
}
