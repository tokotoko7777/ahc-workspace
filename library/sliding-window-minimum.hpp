#include <cassert>
#include <functional>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/sliding-window-minimum.hpp

// 各連続区間の最小値または最大値の位置を、全体 O(N) で求める。
template <class T, class Better>
std::vector<int> sliding_window_best_indices(const std::vector<T>& values,
                                             int width, Better better) {
  const int n = static_cast<int>(values.size());
  assert(1 <= width && width <= n);
  std::vector<int> deque(n);
  int head = 0;
  int tail = 0;
  std::vector<int> result;
  result.reserve(n - width + 1);

  for (int i = 0; i < n; ++i) {
    while (head < tail && !better(values[deque[tail - 1]], values[i])) {
      --tail;
    }
    deque[tail++] = i;
    const int left = i - width + 1;
    if (head < tail && deque[head] < left) ++head;
    if (left >= 0) result.push_back(deque[head]);
  }
  return result;
}

template <class T>
std::vector<T> sliding_window_minimum(const std::vector<T>& values,
                                      int width) {
  const auto indices =
      sliding_window_best_indices(values, width, std::less<T>());
  std::vector<T> result;
  result.reserve(indices.size());
  for (int index : indices) result.push_back(values[index]);
  return result;
}

template <class T>
std::vector<T> sliding_window_maximum(const std::vector<T>& values,
                                      int width) {
  const auto indices =
      sliding_window_best_indices(values, width, std::greater<T>());
  std::vector<T> result;
  result.reserve(indices.size());
  for (int index : indices) result.push_back(values[index]);
  return result;
}
