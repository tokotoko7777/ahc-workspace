#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/topological-sort.hpp

struct TopologicalSortResult {
  std::vector<int> order;
  bool has_cycle;

  bool is_dag() const { return !has_cycle; }
};

// 有向グラフを、すべての辺が前から後ろへ向く順番に並べる。
// 閉路がある場合、has_cycle が true になり、order は途中までになる。
inline TopologicalSortResult topological_sort(
    const std::vector<std::vector<int>>& graph) {
  const int n = static_cast<int>(graph.size());
  std::vector<int> indegree(n, 0);
  for (const auto& edges : graph) {
    for (int next : edges) {
      assert(0 <= next && next < n);
      ++indegree[next];
    }
  }

  std::vector<int> order;
  order.reserve(n);
  for (int vertex = 0; vertex < n; ++vertex) {
    if (indegree[vertex] == 0) order.push_back(vertex);
  }

  for (int head = 0; head < static_cast<int>(order.size()); ++head) {
    const int vertex = order[head];
    for (int next : graph[vertex]) {
      if (--indegree[next] == 0) order.push_back(next);
    }
  }
  return {order, static_cast<int>(order.size()) != n};
}
