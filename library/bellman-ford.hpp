#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/bellman-ford.hpp

template <class Cost>
struct BellmanFordEdge {
  int from;
  int to;
  Cost cost;
};

template <class Cost>
struct BellmanFordResult {
  int start;
  Cost infinity;
  std::vector<Cost> distance;
  std::vector<int> parent;
  std::vector<char> affected_by_negative_cycle;

  bool reachable(int target) const {
    assert(0 <= target && target < static_cast<int>(distance.size()));
    return distance[target] != infinity;
  }

  bool shortest_path_exists(int target) const {
    return reachable(target) && !affected_by_negative_cycle[target];
  }

  // 到達不能または負閉路の影響下なら空のvectorを返す。
  std::vector<int> path_to(int target) const {
    assert(0 <= target && target < static_cast<int>(distance.size()));
    if (!shortest_path_exists(target)) return {};
    std::vector<int> path;
    for (int vertex = target;; vertex = parent[vertex]) {
      path.push_back(vertex);
      if (vertex == start) break;
    }
    std::reverse(path.begin(), path.end());
    return path;
  }
};

// 負の辺を含む有向グラフの最短路。O(NM)。
// 使い方:
// vector<BellmanFordEdge<long long>> edges = {{0, 1, -3}, {1, 2, 5}};
// auto result = bellman_ford(n, edges, start, (1LL << 60));
template <class Cost>
BellmanFordResult<Cost> bellman_ford(
    int vertex_count, const std::vector<BellmanFordEdge<Cost>>& edges,
    int start, Cost infinity) {
  assert(vertex_count > 0);
  assert(0 <= start && start < vertex_count);
  for (const auto& edge : edges) {
    assert(0 <= edge.from && edge.from < vertex_count);
    assert(0 <= edge.to && edge.to < vertex_count);
  }

  BellmanFordResult<Cost> result{
      start, infinity, std::vector<Cost>(vertex_count, infinity),
      std::vector<int>(vertex_count, -1),
      std::vector<char>(vertex_count, false)};
  result.distance[start] = Cost{};
  result.parent[start] = start;

  for (int iteration = 0; iteration + 1 < vertex_count; ++iteration) {
    bool changed = false;
    for (const auto& edge : edges) {
      if (result.distance[edge.from] == infinity) continue;
      const Cost candidate = result.distance[edge.from] + edge.cost;
      if (result.distance[edge.to] <= candidate) continue;
      result.distance[edge.to] = candidate;
      result.parent[edge.to] = edge.from;
      changed = true;
    }
    if (!changed) break;
  }

  std::vector<std::vector<int>> graph(vertex_count);
  for (const auto& edge : edges) graph[edge.from].push_back(edge.to);
  std::vector<int> queue;
  queue.reserve(vertex_count);
  for (const auto& edge : edges) {
    if (result.distance[edge.from] == infinity) continue;
    if (result.distance[edge.to] <= result.distance[edge.from] + edge.cost) {
      continue;
    }
    if (!result.affected_by_negative_cycle[edge.to]) {
      result.affected_by_negative_cycle[edge.to] = true;
      queue.push_back(edge.to);
    }
  }
  for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
    const int vertex = queue[head];
    for (int next : graph[vertex]) {
      if (result.affected_by_negative_cycle[next]) continue;
      result.affected_by_negative_cycle[next] = true;
      queue.push_back(next);
    }
  }
  return result;
}
