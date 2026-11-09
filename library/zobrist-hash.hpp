#include <cassert>
#include <cstdint>
#include <random>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/zobrist-hash.hpp

// 配列状態を差分更新できる64bit hashにする。
// 各場所の値は 0 以上 value_kinds 未満の整数で表す。
//
// 使い方:
// ZobristHash zobrist(positions, value_kinds, 123);
// uint64_t hash = zobrist.build(values);
// zobrist.change(hash, position, old_value, new_value);
struct ZobristHash {
  int positions;
  int value_kinds;
  std::vector<std::vector<std::uint64_t>> table;

  ZobristHash(
      int positions,
      int value_kinds,
      std::uint64_t seed = 0)
      : positions(positions),
        value_kinds(value_kinds),
        table(positions < 0 ? 0 : positions,
              std::vector<std::uint64_t>(
                  value_kinds <= 0 ? 0 : value_kinds)) {
    assert(positions >= 0);
    assert(value_kinds > 0);
    std::mt19937_64 engine(seed);
    for (auto& row : table) {
      for (std::uint64_t& value : row) value = engine();
    }
  }

  std::uint64_t build(const std::vector<int>& values) const {
    assert(static_cast<int>(values.size()) == positions);
    std::uint64_t hash = 0;
    for (int position = 0; position < positions; ++position) {
      assert(0 <= values[position] && values[position] < value_kinds);
      hash ^= table[position][values[position]];
    }
    return hash;
  }

  void change(
      std::uint64_t& hash,
      int position,
      int old_value,
      int new_value) const {
    assert(0 <= position && position < positions);
    assert(0 <= old_value && old_value < value_kinds);
    assert(0 <= new_value && new_value < value_kinds);
    hash ^= table[position][old_value];
    hash ^= table[position][new_value];
  }
};
