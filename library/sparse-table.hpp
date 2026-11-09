#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/sparse-table.hpp

// 変更されない配列の区間min・max・gcdなどを O(1) で求める。
// operation(x, x) == x を満たす演算専用。区間和には使えない。
// 使い方:
// auto minimum = make_sparse_table(
//     values, [](int a, int b) { return std::min(a, b); });
// int answer = minimum.query(left, right);  // [left, right), 空区間不可
template <class T, class Operation>
struct SparseTable {
  int n;
  Operation operation;
  std::vector<int> logarithm;
  std::vector<std::vector<T>> table;

  SparseTable(const std::vector<T>& values, Operation operation)
      : n(static_cast<int>(values.size())),
        operation(std::move(operation)),
        logarithm(n + 1, 0) {
    for (int i = 2; i <= n; ++i) logarithm[i] = logarithm[i / 2] + 1;
    if (n == 0) return;

    table.push_back(values);
    for (int level = 1; (1 << level) <= n; ++level) {
      const int half = 1 << (level - 1);
      const int length = 1 << level;
      const int count = n - length + 1;
      table.push_back(std::vector<T>());
      table.back().reserve(count);
      for (int left = 0; left < count; ++left) {
        table.back().push_back(
            this->operation(table[level - 1][left],
                            table[level - 1][left + half]));
      }
    }
  }

  int size() const { return n; }

  T query(int left, int right) const {
    assert(0 <= left && left < right && right <= n);
    const int level = logarithm[right - left];
    const int length = 1 << level;
    return operation(table[level][left], table[level][right - length]);
  }
};

template <class T, class Operation>
auto make_sparse_table(const std::vector<T>& values, Operation operation) {
  return SparseTable<T, std::decay_t<Operation>>(values,
                                                  std::move(operation));
}
