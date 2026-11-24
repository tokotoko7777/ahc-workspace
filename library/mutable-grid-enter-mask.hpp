#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace ahc {

class MutableGridEnterMask {
 public:
  enum Direction : unsigned { up, right, down, left };

  // Initially every cell is open. Cell IDs are row-major.
  MutableGridEnterMask(int height, int width)
      : height_(height), width_(width), size_(checked_size(height, width)),
        blocked_(words_for(size_)), out_(static_cast<std::size_t>(size_)) {
    for (int p = 0; p < size_; ++p) {
      for (unsigned d = 0; d < 4; ++d) {
        if (neighbor_unchecked(p, d) != -1) {
          auto& mask = out_[static_cast<std::size_t>(p)];
          mask = static_cast<std::uint8_t>(mask | (1U << d));
        }
      }
    }
  }

  [[nodiscard]] int height() const noexcept { return height_; }
  [[nodiscard]] int width() const noexcept { return width_; }
  [[nodiscard]] int size() const noexcept { return size_; }
  [[nodiscard]] std::size_t word_count() const noexcept { return blocked_.size(); }

  // Bit (p % 64) in word (p / 64) is 1 for a blocked destination.
  // Reject bad length or nonzero padding without changing the cache.
  void update(std::span<const std::uint64_t> snapshot) {
    if (snapshot.size() != word_count()) {
      throw std::invalid_argument("incorrect grid snapshot length");
    }
    const auto rem = static_cast<unsigned>(size_ % 64);
    if (rem != 0 && (snapshot.back() >> rem) != 0) {
      throw std::invalid_argument("nonzero grid snapshot padding");
    }
    for (std::size_t i = 0; i < snapshot.size(); ++i) {
      const auto next = snapshot[i];
      std::uint64_t changed = blocked_[i] ^ next;
      while (changed != 0) {
        const auto bit = static_cast<unsigned>(std::countr_zero(changed));
        const auto q = static_cast<int>(i * 64 + bit);
        for (unsigned d = 0; d < 4; ++d) {
          const int p = neighbor_unchecked(q, d);
          if (p < 0) continue;
          const unsigned inverse = (d + 2) % 4;
          // The old mask bit was exactly the old destination-open flag.
          auto& mask = out_[static_cast<std::size_t>(p)];
          mask = static_cast<std::uint8_t>(mask ^ (1U << inverse));
        }
        changed &= changed - 1;
      }
      blocked_[i] = next;
    }
  }

  // Geometry only: ignores blocked state, returns -1 beyond the board.
  [[nodiscard]] int neighbor(int p, unsigned direction) const {
    check_cell(p);
    if (direction >= 4) throw std::out_of_range("invalid grid direction");
    return neighbor_unchecked(p, direction);
  }

  // Only the destination is tested; a blocked p may have outgoing edges.
  [[nodiscard]] std::uint8_t out_mask(int p) const {
    check_cell(p);
    return out_[static_cast<std::size_t>(p)];
  }

  [[nodiscard]] bool is_blocked(int p) const {
    check_cell(p);
    const auto u = static_cast<std::size_t>(p);
    return ((blocked_[u / 64] >> (u % 64)) & 1U) != 0;
  }

 private:
  static int checked_size(int h, int w) {
    if (h <= 0 || w <= 0 || h > std::numeric_limits<int>::max() / w) {
      throw std::invalid_argument("invalid grid dimensions");
    }
    return h * w;
  }
  static std::size_t words_for(int n) noexcept {
    return (static_cast<std::size_t>(n) - 1) / 64 + 1;
  }
  void check_cell(int p) const {
    if (p < 0 || p >= size_) throw std::out_of_range("invalid grid cell");
  }
  int neighbor_unchecked(int p, unsigned d) const noexcept {
    switch (d) {
      case up: return p >= width_ ? p - width_ : -1;
      case right: return p % width_ != width_ - 1 ? p + 1 : -1;
      case down: return p < size_ - width_ ? p + width_ : -1;
      default: return p % width_ != 0 ? p - 1 : -1;
    }
  }
  int height_, width_, size_;
  std::vector<std::uint64_t> blocked_;
  std::vector<std::uint8_t> out_;
};

}  // namespace ahc
