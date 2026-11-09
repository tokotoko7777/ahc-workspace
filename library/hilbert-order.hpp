#include <cassert>
#include <cstdint>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/hilbert-order.hpp

// 2次元の点を、近い点どうしが近い番号になりやすいHilbert順へ変換する。
// x, y は 0 <= x,y < 2^bits、bits は1以上31以下。
// 戻り値の大小で点をsortすれば、空間的にまとまった1次元順序を作れる。
//
// 使い方:
// sort(points.begin(), points.end(), [](auto a, auto b) {
//   return hilbert_order_2d(a.x, a.y, 10) <
//          hilbert_order_2d(b.x, b.y, 10);
// });
inline std::uint64_t hilbert_order_2d(std::uint32_t x, std::uint32_t y,
                                      int bits) {
  assert(1 <= bits && bits <= 31);
  const std::uint32_t size = std::uint32_t{1} << bits;
  assert(x < size && y < size);

  const std::uint32_t mask = size - 1;
  std::uint64_t order = 0;
  for (std::uint32_t side = size >> 1; side > 0; side >>= 1) {
    const std::uint32_t right = (x & side) != 0;
    const std::uint32_t top = (y & side) != 0;
    order += std::uint64_t{side} * side * ((3 * right) ^ top);

    if (top == 0) {
      if (right == 1) {
        x = mask - x;
        y = mask - y;
      }
      const std::uint32_t temporary = x;
      x = y;
      y = temporary;
    }
  }
  return order;
}
