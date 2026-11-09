#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/rolling-hash.hpp

// 64bit整数の自然なオーバーフローを使う Rolling Hash。
// 部分列 [left, right) のhashを O(1)、LCPを O(log N) で求める。
// hash衝突の可能性は0ではないため、完全な一致保証が必要な判定には使わない。
struct RollingHash {
  std::uint64_t base;
  std::vector<std::uint64_t> power;
  std::vector<std::uint64_t> prefix;

  explicit RollingHash(const std::string& text,
                       std::uint64_t base = 1000000007ULL)
      : base(base), power(text.size() + 1), prefix(text.size() + 1) {
    assert(base >= 3 && (base & 1));
    power[0] = 1;
    for (int i = 0; i < static_cast<int>(text.size()); ++i) {
      power[i + 1] = power[i] * base;
      const auto symbol = static_cast<unsigned char>(text[i]);
      prefix[i + 1] = prefix[i] * base + symbol + 1;
    }
  }

  template <class T>
  explicit RollingHash(const std::vector<T>& values,
                       std::uint64_t base = 1000000007ULL)
      : base(base), power(values.size() + 1), prefix(values.size() + 1) {
    assert(base >= 3 && (base & 1));
    power[0] = 1;
    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
      power[i + 1] = power[i] * base;
      prefix[i + 1] =
          prefix[i] * base + static_cast<std::uint64_t>(values[i]) + 1;
    }
  }

  int size() const { return static_cast<int>(prefix.size()) - 1; }

  std::uint64_t hash(int left, int right) const {
    assert(0 <= left && left <= right && right <= size());
    return prefix[right] - prefix[left] * power[right - left];
  }

  // left_hash の後ろに、長さ right_length の right_hash を連結する。
  std::uint64_t concatenate(std::uint64_t left_hash,
                            std::uint64_t right_hash,
                            int right_length) const {
    assert(0 <= right_length && right_length <= size());
    return left_hash * power[right_length] + right_hash;
  }

  int longest_common_prefix(int first_left, int first_right,
                            const RollingHash& other, int second_left,
                            int second_right) const {
    assert(base == other.base);
    assert(0 <= first_left && first_left <= first_right &&
           first_right <= size());
    assert(0 <= second_left && second_left <= second_right &&
           second_right <= other.size());
    int ok = 0;
    int ng = std::min(first_right - first_left,
                      second_right - second_left) +
             1;
    while (ng - ok > 1) {
      const int middle = (ok + ng) / 2;
      if (hash(first_left, first_left + middle) ==
          other.hash(second_left, second_left + middle)) {
        ok = middle;
      } else {
        ng = middle;
      }
    }
    return ok;
  }
};
