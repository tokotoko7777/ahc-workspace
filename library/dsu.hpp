#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/dsu.hpp

// Union-Find。
// 使い方:
// Dsu dsu(n);
// dsu.unite(a, b);
// if (dsu.same(a, b)) { ... }
struct Dsu {
  std::vector<int> parent_or_size;
  int components;

  explicit Dsu(int n) : parent_or_size(n, -1), components(n) {
    assert(n >= 0);
  }

  int leader(int vertex) {
    assert(0 <= vertex && vertex < static_cast<int>(parent_or_size.size()));
    if (parent_or_size[vertex] < 0) return vertex;
    return parent_or_size[vertex] = leader(parent_or_size[vertex]);
  }

  bool unite(int a, int b) {
    a = leader(a);
    b = leader(b);
    if (a == b) return false;
    if (-parent_or_size[a] < -parent_or_size[b]) std::swap(a, b);
    parent_or_size[a] += parent_or_size[b];
    parent_or_size[b] = a;
    --components;
    return true;
  }

  bool same(int a, int b) { return leader(a) == leader(b); }

  int size(int vertex) { return -parent_or_size[leader(vertex)]; }

  int component_count() const { return components; }

  std::vector<std::vector<int>> groups() {
    const int n = static_cast<int>(parent_or_size.size());
    std::vector<int> group_index(n, -1);
    std::vector<std::vector<int>> result;
    for (int vertex = 0; vertex < n; ++vertex) {
      const int root = leader(vertex);
      if (group_index[root] == -1) {
        group_index[root] = static_cast<int>(result.size());
        result.push_back({});
      }
      result[group_index[root]].push_back(vertex);
    }
    return result;
  }
};
