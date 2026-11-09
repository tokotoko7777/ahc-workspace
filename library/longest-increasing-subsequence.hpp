#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/longest-increasing-subsequence.hpp

template <class T>
struct LongestIncreasingSubsequenceResult {
  std::vector<int> indices;
  std::vector<T> values;

  int length() const { return static_cast<int>(indices.size()); }
};

// 最長増加部分列を O(N log N) で1つ復元する。
// strict=true は狭義増加、false は広義増加。
template <class T>
LongestIncreasingSubsequenceResult<T> longest_increasing_subsequence(
    const std::vector<T>& values, bool strict = true) {
  const int n = static_cast<int>(values.size());
  std::vector<T> tail_value;
  std::vector<int> tail_index;
  std::vector<int> parent(n, -1);
  tail_value.reserve(n);
  tail_index.reserve(n);

  for (int i = 0; i < n; ++i) {
    const auto iterator =
        strict ? std::lower_bound(tail_value.begin(), tail_value.end(), values[i])
               : std::upper_bound(tail_value.begin(), tail_value.end(), values[i]);
    const int length = static_cast<int>(iterator - tail_value.begin());
    if (length > 0) parent[i] = tail_index[length - 1];
    if (iterator == tail_value.end()) {
      tail_value.push_back(values[i]);
      tail_index.push_back(i);
    } else {
      *iterator = values[i];
      tail_index[length] = i;
    }
  }

  LongestIncreasingSubsequenceResult<T> result;
  if (tail_index.empty()) return result;
  int current = tail_index.back();
  while (current != -1) {
    result.indices.push_back(current);
    current = parent[current];
  }
  std::reverse(result.indices.begin(), result.indices.end());
  result.values.reserve(result.indices.size());
  for (int index : result.indices) result.values.push_back(values[index]);
  return result;
}
