#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/strongly-connected-components.hpp

struct StronglyConnectedComponents {
  std::vector<int> component_id;
  std::vector<std::vector<int>> groups;

  int component_count() const { return static_cast<int>(groups.size()); }

  bool same(int a, int b) const {
    assert(0 <= a && a < static_cast<int>(component_id.size()));
    assert(0 <= b && b < static_cast<int>(component_id.size()));
    return component_id[a] == component_id[b];
  }
};

// 有向グラフを、互いに行き来できる頂点のグループへ O(N + M) で分ける。
// groups は成分間の辺が前から後ろへ向く順番になる。
// 再帰を使わないため、長い一本道でも再帰スタックを消費しない。
inline StronglyConnectedComponents strongly_connected_components(
    const std::vector<std::vector<int>>& graph) {
  const int n = static_cast<int>(graph.size());
  std::vector<std::vector<int>> reverse_graph(n);
  std::vector<int> reverse_degree(n, 0);
  for (const auto& edges : graph) {
    for (int next : edges) {
      assert(0 <= next && next < n);
      ++reverse_degree[next];
    }
  }
  for (int vertex = 0; vertex < n; ++vertex) {
    reverse_graph[vertex].reserve(reverse_degree[vertex]);
  }
  for (int vertex = 0; vertex < n; ++vertex) {
    for (int next : graph[vertex]) reverse_graph[next].push_back(vertex);
  }

  std::vector<char> visited(n, false);
  std::vector<int> next_edge(n, 0);
  std::vector<int> finish_order;
  finish_order.reserve(n);
  std::vector<int> stack;
  stack.reserve(n);

  for (int start = 0; start < n; ++start) {
    if (visited[start]) continue;
    visited[start] = true;
    stack.push_back(start);
    while (!stack.empty()) {
      const int vertex = stack.back();
      if (next_edge[vertex] < static_cast<int>(graph[vertex].size())) {
        const int next = graph[vertex][next_edge[vertex]++];
        if (!visited[next]) {
          visited[next] = true;
          stack.push_back(next);
        }
      } else {
        finish_order.push_back(vertex);
        stack.pop_back();
      }
    }
  }

  StronglyConnectedComponents result{std::vector<int>(n, -1), {}};
  for (int i = n - 1; i >= 0; --i) {
    const int start = finish_order[i];
    if (result.component_id[start] != -1) continue;
    const int id = result.component_count();
    result.groups.push_back({});
    stack.push_back(start);
    result.component_id[start] = id;

    while (!stack.empty()) {
      const int vertex = stack.back();
      stack.pop_back();
      result.groups[id].push_back(vertex);
      for (int next : reverse_graph[vertex]) {
        if (result.component_id[next] != -1) continue;
        result.component_id[next] = id;
        stack.push_back(next);
      }
    }
  }
  return result;
}
