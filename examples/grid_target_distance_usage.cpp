#include "grid-target-distance.hpp"
#include <iostream>

int main() {
  // .#.    Each group contains one pair of target cells.
  // ...    Distances use row-major cell IDs.
  const std::vector<std::uint8_t> wall{0,1,0,0,0,0};
  const std::vector<std::vector<int>> goals{{0,2},{3,5}};
  const ahc::GridTargetDistance d(2,3,wall,goals);
  std::cout << d.distance(4,0) << ' ' << d.distance(4,1) << '\n'; // 2 1
}
