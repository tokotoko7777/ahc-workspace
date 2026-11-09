#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/range-add-range-sum.hpp

// 区間加算と区間和を、どちらも O(log N) で処理する専用Lazy Segment Tree。
// 使い方:
// RangeAddRangeSum<long long> sum(values);
// sum.add(left, right, value);       // [left, right) に加算
// long long answer = sum.query(left, right);
template <class T>
struct RangeAddRangeSum {
  int n;
  int leaf_count = 1;
  std::vector<T> tree;
  std::vector<T> lazy;

  explicit RangeAddRangeSum(int size) : n(size) {
    assert(size >= 0);
    while (leaf_count < n) leaf_count *= 2;
    tree.assign(leaf_count * 2, T{});
    lazy.assign(leaf_count * 2, T{});
  }

  explicit RangeAddRangeSum(const std::vector<T>& values)
      : RangeAddRangeSum(static_cast<int>(values.size())) {
    for (int i = 0; i < n; ++i) tree[leaf_count + i] = values[i];
    for (int node = leaf_count - 1; node >= 1; --node) {
      tree[node] = tree[node * 2] + tree[node * 2 + 1];
    }
  }

  int size() const { return n; }

  void add(int left, int right, const T& value) {
    assert(0 <= left && left <= right && right <= n);
    add_impl(left, right, value, 1, 0, leaf_count);
  }

  T query(int left, int right) {
    assert(0 <= left && left <= right && right <= n);
    return query_impl(left, right, 1, 0, leaf_count);
  }

  T all() const { return tree[1]; }

 private:
  void apply(int node, int segment_length, const T& value) {
    tree[node] += value * segment_length;
    lazy[node] += value;
  }

  void push(int node, int segment_length) {
    if (segment_length == 1 || lazy[node] == T{}) return;
    const int child_length = segment_length / 2;
    apply(node * 2, child_length, lazy[node]);
    apply(node * 2 + 1, child_length, lazy[node]);
    lazy[node] = T{};
  }

  void add_impl(int query_left, int query_right, const T& value, int node,
                int left, int right) {
    if (right <= query_left || query_right <= left) return;
    if (query_left <= left && right <= query_right) {
      apply(node, right - left, value);
      return;
    }
    push(node, right - left);
    const int middle = (left + right) / 2;
    add_impl(query_left, query_right, value, node * 2, left, middle);
    add_impl(query_left, query_right, value, node * 2 + 1, middle, right);
    tree[node] = tree[node * 2] + tree[node * 2 + 1];
  }

  T query_impl(int query_left, int query_right, int node, int left,
               int right) {
    if (right <= query_left || query_right <= left) return T{};
    if (query_left <= left && right <= query_right) return tree[node];
    push(node, right - left);
    const int middle = (left + right) / 2;
    return query_impl(query_left, query_right, node * 2, left, middle) +
           query_impl(query_left, query_right, node * 2 + 1, middle, right);
  }
};
