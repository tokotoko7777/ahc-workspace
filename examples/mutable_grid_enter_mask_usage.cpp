#include "mutable-grid-enter-mask.hpp"

#include <array>
#include <cstdint>
#include <iostream>

int main() {
  ahc::MutableGridEnterMask grid(2, 3);
  const std::array<std::uint64_t, 1> walls{std::uint64_t{1} << 1};
  grid.update(walls);
  for (unsigned d = 0; d < 4; ++d) {
    if ((grid.out_mask(1) & (1U << d)) != 0) {
      std::cout << grid.neighbor(1, d) << '\n';
    }
  }
}
