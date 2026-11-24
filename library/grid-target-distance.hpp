#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace ahc {

// Distance to the nearest cell of each target group in an undirected
// four-neighbor grid. Rebuild when walls or targets change.
class GridTargetDistance {
 public:
  enum class BlockedOrigin { Unreachable, MayExit };

  GridTargetDistance(int height, int width,
                     std::span<const std::uint8_t> walls,
                     std::span<const std::vector<int>> targets)
      : width_(width) {
    const std::int64_t n = static_cast<std::int64_t>(height) * width;
    if (height <= 0 || width <= 0 || n > unreachable ||
        static_cast<std::size_t>(n) != walls.size())
      throw std::invalid_argument("invalid grid dimensions or wall mask");
    size_ = static_cast<int>(n);
    walls_.assign(walls.begin(), walls.end());
    rows_.reserve(targets.size());
    std::vector<int> queue(size_);
    for (const auto& group : targets) {
      rows_.emplace_back(size_, unreachable);
      auto& d = rows_.back();
      int head = 0, tail = 0;
      for (int x : group) {
        check(x);
        if (!walls_[x] && d[x] == unreachable) d[x] = 0, queue[tail++] = x;
      }
      while (head < tail) {
        const int x = queue[head++];
        for (int y : adjacent(x)) {
          if (y >= 0 && !walls_[y] && d[y] == unreachable) {
            d[y] = d[x] + 1;
            queue[tail++] = y;
          }
        }
      }
      // MayExit counts a blocked origin that already belongs to this target
      // set at distance zero; do not let it seed the expansion above.
      for (int x : group) if (walls_[x]) d[x] = 0;
    }
  }

  int distance(int cell, std::size_t group,
               BlockedOrigin mode = BlockedOrigin::Unreachable) const {
    check(cell);
    const auto& d = rows_.at(group);
    if (!walls_[cell]) return d[cell] == unreachable ? -1 : d[cell];
    if (mode == BlockedOrigin::Unreachable) return -1;
    if (d[cell] == 0) return 0;
    int best = unreachable;
    for (int y : adjacent(cell))
      if (y >= 0 && !walls_[y] && d[y] != unreachable)
        best = std::min(best, 1 + static_cast<int>(d[y]));
    return best == unreachable ? -1 : best;
  }

 private:
  static constexpr auto unreachable = std::numeric_limits<std::uint16_t>::max();
  int width_, size_;
  std::vector<std::uint8_t> walls_;
  std::vector<std::vector<std::uint16_t>> rows_;

  void check(int x) const {
    if (x < 0 || x >= size_) throw std::out_of_range("grid cell");
  }
  std::array<int, 4> adjacent(int x) const {
    return {x >= width_ ? x - width_ : -1,
            x % width_ + 1 < width_ ? x + 1 : -1,
            x + width_ < size_ ? x + width_ : -1,
            x % width_ ? x - 1 : -1};
  }
};

}  // namespace ahc
