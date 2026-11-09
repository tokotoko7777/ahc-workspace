#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/range-add-range-maximum.hpp

// 区間加算と区間最大値をどちらも O(log N) で行う。
// RangeAddRangeMaximum<long long> values(initial_values, -(1LL << 60));
// values.add(left, right, amount);
// long long maximum = values.query(left, right);
template <class Value>
struct RangeAddRangeMaximum {
  int n;
  int size = 1;
  Value negative_infinity;
  std::vector<Value> maximum;
  std::vector<Value> lazy;

  RangeAddRangeMaximum(int n, Value negative_infinity)
      : n(n), negative_infinity(negative_infinity) {
    assert(n >= 0);
    while (size < n) size *= 2;
    maximum.assign(size * 2, negative_infinity);
    lazy.assign(size * 2, Value{});
  }

  RangeAddRangeMaximum(const std::vector<Value>& values,
                       Value negative_infinity)
      : RangeAddRangeMaximum(static_cast<int>(values.size()),
                             negative_infinity) {
    for (int i = 0; i < n; ++i) maximum[size + i] = values[i];
    for (int node = size - 1; node >= 1; --node) {
      maximum[node] = std::max(maximum[node * 2], maximum[node * 2 + 1]);
    }
  }

  void add(int left, int right, Value amount) {
    assert(0 <= left && left <= right && right <= n);
    add_impl(left, right, amount, 1, 0, size);
  }

  Value query(int left, int right) {
    assert(0 <= left && left <= right && right <= n);
    if (left == right) return negative_infinity;
    return query_impl(left, right, 1, 0, size);
  }

  Value get(int index) {
    assert(0 <= index && index < n);
    return query(index, index + 1);
  }

  Value all_maximum() const { return maximum[1]; }

 private:
  void apply(int node, Value amount) {
    maximum[node] += amount;
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
    maximum[node] = std::max(maximum[node * 2], maximum[node * 2 + 1]);
  }

  Value query_impl(int query_left, int query_right, int node, int node_left,
                   int node_right) {
    if (query_right <= node_left || node_right <= query_left) {
      return negative_infinity;
    }
    if (query_left <= node_left && node_right <= query_right) {
      return maximum[node];
    }
    push(node);
    const int middle = (node_left + node_right) / 2;
    return std::max(
        query_impl(query_left, query_right, node * 2, node_left, middle),
        query_impl(query_left, query_right, node * 2 + 1, middle,
                   node_right));
  }
};
