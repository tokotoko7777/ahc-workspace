#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/range-add-range-minimum.hpp

// 区間加算と区間最小値をどちらも O(log N) で行う。
// 使い方:
// RangeAddRangeMinimum<long long> values(initial_values, (1LL << 60));
// values.add(left, right, amount);       // [left, right)
// long long minimum = values.query(left, right);
template <class Value>
struct RangeAddRangeMinimum {
  int n;
  int size;
  Value infinity;
  std::vector<Value> minimum;
  std::vector<Value> lazy;

  RangeAddRangeMinimum(int n, Value infinity)
      : n(n), size(1), infinity(infinity) {
    assert(n >= 0);
    while (size < n) size *= 2;
    minimum.assign(size * 2, infinity);
    lazy.assign(size * 2, Value{});
  }

  RangeAddRangeMinimum(const std::vector<Value>& values, Value infinity)
      : RangeAddRangeMinimum(static_cast<int>(values.size()), infinity) {
    for (int i = 0; i < n; ++i) minimum[size + i] = values[i];
    for (int i = size - 1; i >= 1; --i) {
      minimum[i] = std::min(minimum[i * 2], minimum[i * 2 + 1]);
    }
  }

  void add(int left, int right, Value amount) {
    assert(0 <= left && left <= right && right <= n);
    add_impl(left, right, amount, 1, 0, size);
  }

  Value query(int left, int right) {
    assert(0 <= left && left <= right && right <= n);
    if (left == right) return infinity;
    return query_impl(left, right, 1, 0, size);
  }

  Value get(int index) {
    assert(0 <= index && index < n);
    return query(index, index + 1);
  }

  Value all_minimum() const { return minimum[1]; }

 private:
  void apply(int node, Value amount) {
    minimum[node] += amount;
    lazy[node] += amount;
  }

  void push(int node) {
    if (lazy[node] == Value{}) return;
    apply(node * 2, lazy[node]);
    apply(node * 2 + 1, lazy[node]);
    lazy[node] = Value{};
  }

  void add_impl(int query_left, int query_right, Value amount, int node,
                int node_left, int node_right) {
    if (query_right <= node_left || node_right <= query_left) return;
    if (query_left <= node_left && node_right <= query_right) {
      apply(node, amount);
      return;
    }
    push(node);
    const int middle = (node_left + node_right) / 2;
    add_impl(query_left, query_right, amount, node * 2, node_left, middle);
    add_impl(query_left, query_right, amount, node * 2 + 1, middle,
             node_right);
    minimum[node] = std::min(minimum[node * 2], minimum[node * 2 + 1]);
  }

  Value query_impl(int query_left, int query_right, int node, int node_left,
                   int node_right) {
    if (query_right <= node_left || node_right <= query_left) return infinity;
    if (query_left <= node_left && node_right <= query_right) {
      return minimum[node];
    }
    push(node);
    const int middle = (node_left + node_right) / 2;
    return std::min(
        query_impl(query_left, query_right, node * 2, node_left, middle),
        query_impl(query_left, query_right, node * 2 + 1, middle,
                   node_right));
  }
};
