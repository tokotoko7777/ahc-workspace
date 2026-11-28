#pragma once
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace ahc {
// Four-neighbor flood fill on a compile-time rectangular grid, using one
// unsigned word per row. Bit c denotes column c. In 'open', 1 means traversable.
// Example: const auto seen = row_flood<30,30>(open, 12, 7);
template <std::size_t Height, unsigned Width, class UInt = std::uint32_t>
std::array<UInt, Height> row_flood(const std::array<UInt, Height>& open,
                                  std::size_t row, unsigned col) {
  static_assert(Height > 0 && Width > 0 && Width <= std::numeric_limits<UInt>::digits);
  static_assert(std::is_unsigned_v<UInt> && !std::is_same_v<UInt, bool>);
  std::array<UInt, Height> reached{};
  if (row >= Height || col >= Width || !(open[row] & (UInt{1} << col)))
    return reached;

  constexpr unsigned bits = std::numeric_limits<UInt>::digits;
  constexpr UInt mask = [] {
    if constexpr (Width == bits) return std::numeric_limits<UInt>::max();
    else return static_cast<UInt>((UInt{1} << Width) - 1);
  }();
  std::array<std::size_t, Height> queue{};
  std::array<bool, Height> queued{};
  std::size_t head = 0, tail = 0, pending = 0;
  auto push = [&](std::size_t r, UInt seed) {
    const UInt next = static_cast<UInt>(seed & open[r] & mask & ~reached[r]);
    if (!next) return;
    reached[r] |= next;
    if (queued[r]) return;
    queued[r] = true;
    queue[tail] = r;
    if (++tail == Height) tail = 0;
    ++pending;
  };
  push(row, static_cast<UInt>(UInt{1} << col));
  while (pending) {
    const auto r = queue[head];
    if (++head == Height) head = 0;
    --pending;
    queued[r] = false;
    UInt east = reached[r], west = reached[r], pe = open[r] & mask, pw = pe;
    for (unsigned d = 1; d < Width; d <<= 1) {
      east |= static_cast<UInt>(pe & static_cast<UInt>(east << d));
      west |= static_cast<UInt>(pw & (west >> d));
      pe &= static_cast<UInt>(pe << d);
      pw &= static_cast<UInt>(pw >> d);
    }
    reached[r] |= east | west;
    if (r) push(r - 1, reached[r]);
    if (r + 1 < Height) push(r + 1, reached[r]);
  }
  return reached;
}
}
