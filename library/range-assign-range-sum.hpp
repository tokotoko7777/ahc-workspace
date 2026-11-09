#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/range-assign-range-sum.hpp

// 区間代入と区間和をどちらも O(log N) で行う遅延セグメント木。
// RangeAssignRangeSum<long long> sum(initial_values);
// sum.assign(left, right, value);  // [left, right)をvalueにする
// long long answer = sum.query(left, right);
template <class Value>
struct RangeAssignRangeSum {
  int n;
  int size = 1;
  std::vector<Value> sum;
  std::vector<Value> lazy_value;
  std::vector<char> has_lazy;

  explicit RangeAssignRangeSum(int n) : n(n) {
    assert(n >= 0);
    while (size < n) size *= 2;
    sum.assign(size * 2, Value{});
    lazy_value.assign(size * 2, Value{});
    has_lazy.assign(size * 2, false);
  }

  explicit RangeAssignRangeSum(const std::vector<Value>& values)
      : RangeAssignRangeSum(static_cast<int>(values.size())) {
    for (int i = 0; i < n; ++i) sum[size + i] = values[i];
    for (int node = size - 1; node >= 1; --node) {
      sum[node] = sum[node * 2] + sum[node * 2 + 1];
    }
  }

  void assign(int left, int right, Value value) {
    assert(0 <= left && left <= right && right <= n);
    assign_impl(left, right, value, 1, 0, size);
  }

  Value query(int left, int right) {
    assert(0 <= left && left <= right && right <= n);
    return query_impl(left, right, 1, 0, size);
  }

  Value get(int index) {
    assert(0 <= index && index < n);
    return query(index, index + 1);
  }

  Value all_sum() const { return sum[1]; }

 private:
  void apply(int node, int length, Value value) {
    sum[node] = value * static_cast<Value>(length);
    lazy_value[node] = value;
    has_lazy[node] = true;
  }

  void push(int node, int length) {
    if (!has_lazy[node] || length == 1) return;
    apply(node * 2, length / 2, lazy_value[node]);
    apply(node * 2 + 1, length / 2, lazy_value[node]);
    has_lazy[node] = false;
  }

  void assign_impl(int query_left, int query_right, Value value, int node,
                   int node_left, int node_right) {
    if (query_right <= node_left || node_right <= query_left) return;
    if (query_left <= node_left && node_right <= query_right) {
      apply(node, node_right - node_left, value);
      return;
    }
    push(node, node_right - node_left);
    const int middle = (node_left + node_right) / 2;
    assign_impl(query_left, query_right, value, node * 2, node_left, middle);
    assign_impl(query_left, query_right, value, node * 2 + 1, middle,
                node_right);
    sum[node] = sum[node * 2] + sum[node * 2 + 1];
  }

  Value query_impl(int query_left, int query_right, int node, int node_left,
                   int node_right) {
    if (query_right <= node_left || node_right <= query_left) return Value{};
    if (query_left <= node_left && node_right <= query_right) return sum[node];
    push(node, node_right - node_left);
    const int middle = (node_left + node_right) / 2;
    return query_impl(query_left, query_right, node * 2, node_left, middle) +
           query_impl(query_left, query_right, node * 2 + 1, middle,
                      node_right);
  }
};
