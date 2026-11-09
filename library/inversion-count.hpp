#include <algorithm>
#include <cstddef>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/inversion-count.hpp

// i < j かつ values[i] > values[j] となる組の個数。O(N log N)。
// long long answer = inversion_count(values);
template <class Value>
long long inversion_count(const std::vector<Value>& values) {
  std::vector<Value> sorted = values;
  std::sort(sorted.begin(), sorted.end());
  sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
  std::vector<int> fenwick(sorted.size() + 1, 0);

  const auto prefix_sum = [&](int end) {
    int result = 0;
    for (int index = end; index > 0; index -= index & -index) {
      result += fenwick[index];
    }
    return result;
  };

  long long result = 0;
  int seen = 0;
  for (const Value& value : values) {
    const int rank = static_cast<int>(
        std::lower_bound(sorted.begin(), sorted.end(), value) - sorted.begin());
    result += seen - prefix_sum(rank + 1);
    for (int index = rank + 1; index < static_cast<int>(fenwick.size());
         index += index & -index) {
      ++fenwick[index];
    }
    ++seen;
  }
  return result;
}
