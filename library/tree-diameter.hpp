#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/tree-diameter.hpp

template <class Cost>
struct TreeDiameterResult {
  int endpoint_a;
  int endpoint_b;
  Cost length;
  std::vector<int> path;
};

// 非負重みの木の直径を O(N) で求める。graph[a]には {b, cost} を入れる。
// 使い方:
// vector<vector<pair<int, long long>>> tree(n);
// tree[a].push_back({b, cost}); tree[b].push_back({a, cost});
// auto result = tree_diameter(tree);
template <class Cost>
TreeDiameterResult<Cost> tree_diameter(
    const std::vector<std::vector<std::pair<int, Cost>>>& graph) {
  const int n = static_cast<int>(graph.size());
  if (n == 0) return {-1, -1, Cost{}, {}};

  const auto farthest_from = [&](int start, std::vector<int>* saved_parent) {
    std::vector<int> parent(n, -1);
    std::vector<Cost> distance(n, Cost{});
    std::vector<int> stack;
    stack.reserve(n);
    parent[start] = start;
    stack.push_back(start);
    int farthest = start;
    while (!stack.empty()) {
      const int vertex = stack.back();
      stack.pop_back();
      if (distance[farthest] < distance[vertex]) farthest = vertex;
      for (const auto& [next, cost] : graph[vertex]) {
        assert(0 <= next && next < n);
        assert(!(cost < Cost{}));
        if (parent[next] != -1) continue;
        parent[next] = vertex;
        distance[next] = distance[vertex] + cost;
        stack.push_back(next);
      }
    }
    for (int vertex = 0; vertex < n; ++vertex) assert(parent[vertex] != -1);
    if (saved_parent != nullptr) *saved_parent = std::move(parent);
    return std::make_pair(farthest, distance[farthest]);
  };

  const int endpoint_a = farthest_from(0, nullptr).first;
  std::vector<int> parent;
  const auto [endpoint_b, length] = farthest_from(endpoint_a, &parent);
  std::vector<int> path;
  for (int vertex = endpoint_b;; vertex = parent[vertex]) {
    path.push_back(vertex);
    if (vertex == endpoint_a) break;
  }
  std::reverse(path.begin(), path.end());
  return {endpoint_a, endpoint_b, length, std::move(path)};
}
