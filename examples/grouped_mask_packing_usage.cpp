#include "grouped-mask-packing.hpp"
#include <iostream>

int main() {
  // Three available workers. Alternatives for the same group are exclusive.
  using ahc::MaskPackingOption;
  const std::vector<std::vector<MaskPackingOption>> jobs{
      {{0b101,10}}, {{0b011,9}}, {{0b100,8}}};
  const auto answer=ahc::grouped_mask_packing(3,jobs);
  std::cout << answer.value << '\n';  // 17: second and third jobs
}
