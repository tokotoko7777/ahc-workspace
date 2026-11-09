#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/difference-array.hpp

// 区間 [left, right) への加算を O(1) で記録し、最後に O(N) で全要素を作る。
// 途中の区間和を答える用途ではなく、更新を全部先に処理できる時に使う。
template <class T>
struct DifferenceArray {
  int n;
  std::vector<T> difference;

  explicit DifferenceArray(int size)
      : n(size), difference(size + 1, T{}) {
    assert(size >= 0);
  }

  explicit DifferenceArray(const std::vector<T>& initial_values)
      : DifferenceArray(static_cast<int>(initial_values.size())) {
    for (int i = 0; i < n; ++i) {
      difference[i] += initial_values[i];
      difference[i + 1] -= initial_values[i];
    }
  }

  void add(int left, int right, const T& value) {
    assert(0 <= left && left <= right && right <= n);
    difference[left] += value;
    difference[right] -= value;
  }

  std::vector<T> build() const {
    std::vector<T> values(n);
    T current{};
    for (int i = 0; i < n; ++i) {
      current += difference[i];
      values[i] = current;
    }
    return values;
  }

  void clear() { std::fill(difference.begin(), difference.end(), T{}); }
};
