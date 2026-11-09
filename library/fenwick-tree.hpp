#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/fenwick-tree.hpp

// 1点に足し算し、区間和を求める Fenwick Tree (Binary Indexed Tree)。
// 使い方:
// FenwickTree<long long> sum(values);
// sum.add(index, difference);
// long long answer = sum.query(left, right);  // [left, right)
template <class T>
struct FenwickTree {
  int n;
  std::vector<T> data;

  explicit FenwickTree(int size) : n(size), data(size + 1, T{}) {
    assert(size >= 0);
  }

  explicit FenwickTree(const std::vector<T>& values)
      : FenwickTree(static_cast<int>(values.size())) {
    for (int i = 0; i < n; ++i) add(i, values[i]);
  }

  int size() const { return n; }

  // values[index] に delta を足す。
  void add(int index, const T& delta) {
    assert(0 <= index && index < n);
    for (int i = index + 1; i <= n; i += i & -i) data[i] += delta;
  }

  // values[0] + ... + values[right - 1] を返す。
  T prefix_sum(int right) const {
    assert(0 <= right && right <= n);
    T result{};
    for (int i = right; i > 0; i -= i & -i) result += data[i];
    return result;
  }

  // values[left] + ... + values[right - 1] を返す。
  T query(int left, int right) const {
    assert(0 <= left && left <= right && right <= n);
    return prefix_sum(right) - prefix_sum(left);
  }

  // 先頭からの和が target 以上になる最初の要素番号を返す。
  // 全体の和が target 未満なら n を返す。
  // この関数を使う時は、各要素が 0 以上である必要がある。
  int lower_bound(const T& target) const {
    if (!(T{} < target)) return 0;

    int index = 0;
    T accumulated{};
    int step = 1;
    while (step <= n / 2) step *= 2;

    for (; step > 0; step /= 2) {
      const int next = index + step;
      if (next <= n && accumulated + data[next] < target) {
        index = next;
        accumulated += data[next];
      }
    }
    return index;
  }
};
