#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/cumulative-sum.hpp

// 1次元累積和。int、long long、double などを選べる。
// 使い方:
// CumulativeSum<long long> sum(values);
// long long total = sum.query(left, right);  // [left, right)
template <class T>
struct CumulativeSum {
  std::vector<T> sum;

  explicit CumulativeSum(const std::vector<T>& values)
      : sum(values.size() + 1, T{}) {
    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
      sum[i + 1] = sum[i] + values[i];
    }
  }

  int size() const { return static_cast<int>(sum.size()) - 1; }

  T query(int left, int right) const {
    assert(0 <= left && left <= right && right <= size());
    return sum[right] - sum[left];
  }
};
