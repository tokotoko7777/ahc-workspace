#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/segment-tree.hpp

// 1点を変更し、区間の和・最小値・最大値などを求める Segment Tree。
// operation は実際のラムダ式の型で保持するため、std::function の間接呼び出しがない。
// 使い方:
// auto minimum = make_segment_tree(
//     values, 1000000000, [](int a, int b) { return std::min(a, b); });
// minimum.set(index, new_value);
// int answer = minimum.query(left, right);  // [left, right)
template <class T, class Operation>
struct SegmentTree {
  int n;
  int leaf_count;
  T identity;
  Operation operation;
  std::vector<T> data;

  SegmentTree(int size, T identity_element, Operation combine)
      : n(size),
        leaf_count(1),
        identity(std::move(identity_element)),
        operation(std::move(combine)) {
    assert(size >= 0);
    while (leaf_count < n) leaf_count *= 2;
    data.assign(leaf_count * 2, identity);
  }

  SegmentTree(const std::vector<T>& values, T identity_element,
              Operation combine)
      : SegmentTree(static_cast<int>(values.size()),
                    std::move(identity_element), std::move(combine)) {
    for (int i = 0; i < n; ++i) data[leaf_count + i] = values[i];
    for (int i = leaf_count - 1; i >= 1; --i) {
      data[i] = operation(data[i * 2], data[i * 2 + 1]);
    }
  }

  int size() const { return n; }

  void set(int index, const T& value) {
    assert(0 <= index && index < n);
    int node = leaf_count + index;
    data[node] = value;
    while (node > 1) {
      node /= 2;
      data[node] = operation(data[node * 2], data[node * 2 + 1]);
    }
  }

  const T& get(int index) const {
    assert(0 <= index && index < n);
    return data[leaf_count + index];
  }

  // values[left] から values[right - 1] を順番どおりにまとめる。
  T query(int left, int right) const {
    assert(0 <= left && left <= right && right <= n);
    T left_result = identity;
    T right_result = identity;
    left += leaf_count;
    right += leaf_count;

    while (left < right) {
      if (left & 1) left_result = operation(left_result, data[left++]);
      if (right & 1) right_result = operation(data[--right], right_result);
      left /= 2;
      right /= 2;
    }
    return operation(left_result, right_result);
  }

  T all() const { return data[1]; }
};

// C++17 の型推論を使い、長いラムダ式の型名を書かずに高速版を作る。
template <class T, class Operation>
auto make_segment_tree(int size, T identity, Operation operation) {
  return SegmentTree<T, std::decay_t<Operation>>(
      size, std::move(identity), std::move(operation));
}

template <class T, class Operation>
auto make_segment_tree(const std::vector<T>& values, T identity,
                       Operation operation) {
  return SegmentTree<T, std::decay_t<Operation>>(
      values, std::move(identity), std::move(operation));
}
