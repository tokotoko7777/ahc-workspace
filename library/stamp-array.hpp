#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/stamp-array.hpp

// clear() がほぼ O(1) の配列。BFS の訪問配列などを何度も使う時に便利。
// 使い方:
// StampArray<int> distance(n, -1);
// distance[0] = 0;
// distance.clear();  // 全要素が再び -1 に見える
template <class T>
struct StampArray {
  std::vector<T> values;
  std::vector<int> stamp;
  int generation = 1;
  T initial_value;

  StampArray(int size, T initial_value = T{})
      : values(size), stamp(size, 0), initial_value(std::move(initial_value)) {}

  int size() const { return static_cast<int>(values.size()); }

  T& operator[](int index) {
    assert(0 <= index && index < size());
    if (stamp[index] != generation) {
      stamp[index] = generation;
      values[index] = initial_value;
    }
    return values[index];
  }

  T get(int index) const {
    assert(0 <= index && index < size());
    return stamp[index] == generation ? values[index] : initial_value;
  }

  void clear() {
    if (generation == std::numeric_limits<int>::max()) {
      std::fill(stamp.begin(), stamp.end(), 0);
      generation = 1;
    } else {
      ++generation;
    }
  }
};
