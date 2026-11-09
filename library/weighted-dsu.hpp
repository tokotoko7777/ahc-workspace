#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/weighted-dsu.hpp

// potential[b] - potential[a] = difference という制約を管理するUnion-Find。
template <class Weight>
struct WeightedDsu {
  std::vector<int> parent_or_size;
  std::vector<Weight> difference_to_parent;
  int components;

  explicit WeightedDsu(int n)
      : components(n) {
    assert(n >= 0);
    parent_or_size.assign(n, -1);
    difference_to_parent.assign(n, Weight{});
  }

  int leader(int vertex) {
    check_vertex(vertex);
    if (parent_or_size[vertex] < 0) return vertex;
    const int parent = parent_or_size[vertex];
    const int root = leader(parent);
    difference_to_parent[vertex] += difference_to_parent[parent];
    parent_or_size[vertex] = root;
    return root;
  }

  Weight potential(int vertex) {
    leader(vertex);
    return difference_to_parent[vertex];
  }

  bool same(int a, int b) { return leader(a) == leader(b); }

  // potential[b] - potential[a]。same(a,b)==trueの時だけ呼ぶ。
  Weight difference(int a, int b) {
    assert(same(a, b));
    return potential(b) - potential(a);
  }

  // 制約が追加可能ならtrue。既に連結済みでも整合すればtrue。
  // 現在の制約と矛盾する場合はfalseで、状態を変更しない。
  bool unite(int a, int b, Weight difference_b_minus_a) {
    check_vertex(a);
    check_vertex(b);
    difference_b_minus_a += potential(a);
    difference_b_minus_a -= potential(b);
    int root_a = leader(a);
    int root_b = leader(b);
    if (root_a == root_b) return difference_b_minus_a == Weight{};
    if (-parent_or_size[root_a] < -parent_or_size[root_b]) {
      std::swap(root_a, root_b);
      difference_b_minus_a = -difference_b_minus_a;
    }
    parent_or_size[root_a] += parent_or_size[root_b];
    parent_or_size[root_b] = root_a;
    difference_to_parent[root_b] = difference_b_minus_a;
    --components;
    return true;
  }

  int size(int vertex) { return -parent_or_size[leader(vertex)]; }
  int component_count() const { return components; }

 private:
  void check_vertex(int vertex) const {
    assert(0 <= vertex && vertex < static_cast<int>(parent_or_size.size()));
  }
};
