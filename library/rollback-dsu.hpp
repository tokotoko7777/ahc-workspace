#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/rollback-dsu.hpp

// 過去の状態へ戻せる Union-Find。leader では経路圧縮をしない。
// 使い方:
// RollbackDsu dsu(n);
// int snapshot = dsu.snapshot();
// dsu.unite(a, b);
// dsu.rollback(snapshot);
struct RollbackDsu {
  std::vector<int> parent_or_size;
  std::vector<std::pair<int, int>> history;

  explicit RollbackDsu(int n) : parent_or_size(n, -1) {}

  int leader(int vertex) const {
    assert(0 <= vertex && vertex < static_cast<int>(parent_or_size.size()));
    while (parent_or_size[vertex] >= 0) vertex = parent_or_size[vertex];
    return vertex;
  }

  bool unite(int a, int b) {
    a = leader(a);
    b = leader(b);
    if (a == b) return false;
    if (-parent_or_size[a] < -parent_or_size[b]) std::swap(a, b);
    history.emplace_back(a, parent_or_size[a]);
    history.emplace_back(b, parent_or_size[b]);
    parent_or_size[a] += parent_or_size[b];
    parent_or_size[b] = a;
    return true;
  }

  bool same(int a, int b) const { return leader(a) == leader(b); }

  int size(int vertex) const { return -parent_or_size[leader(vertex)]; }

  int snapshot() const { return static_cast<int>(history.size()); }

  void rollback(int snapshot) {
    assert(0 <= snapshot && snapshot <= static_cast<int>(history.size()));
    while (static_cast<int>(history.size()) > snapshot) {
      const auto [vertex, old_value] = history.back();
      history.pop_back();
      parent_or_size[vertex] = old_value;
    }
  }
};
