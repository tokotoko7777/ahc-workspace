#include <algorithm>
#include <cassert>
#include <functional>
#include <queue>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/dijkstra.hpp

// 重み付きグラフの辺を追加する。graph[from] には {行き先, コスト} が入る。
template <class Cost>
void add_directed_edge(std::vector<std::vector<std::pair<int, Cost>>>& graph,
                       int from, int to, Cost cost) {
  assert(0 <= from && from < static_cast<int>(graph.size()));
  assert(0 <= to && to < static_cast<int>(graph.size()));
  assert(!(cost < Cost{}));
  graph[from].push_back({to, cost});
}

template <class Cost>
void add_undirected_edge(std::vector<std::vector<std::pair<int, Cost>>>& graph,
                         int a, int b, Cost cost) {
  add_directed_edge(graph, a, b, cost);
  add_directed_edge(graph, b, a, cost);
}

template <class Cost>
struct DijkstraResult {
  int start;
  std::vector<Cost> distance;
  std::vector<int> parent;

  bool reachable(int target) const {
    assert(0 <= target && target < static_cast<int>(parent.size()));
    return parent[target] != -1;
  }

  // start から target までの頂点列を返す。到達不能なら空の vector。
  std::vector<int> path_to(int target) const {
    assert(0 <= target && target < static_cast<int>(parent.size()));
    if (!reachable(target)) return {};

    std::vector<int> path;
    for (int vertex = target;; vertex = parent[vertex]) {
      path.push_back(vertex);
      if (vertex == start) break;
    }
    std::reverse(path.begin(), path.end());
    return path;
  }
};

// 0 以上の辺コストを持つグラフの、start からの最短距離を求める。
// 使い方:
// vector<vector<pair<int, long long>>> graph(n);
// add_undirected_edge(graph, a, b, cost);
// auto result = dijkstra(graph, start, (1LL << 60));
template <class Cost>
DijkstraResult<Cost> dijkstra(
    const std::vector<std::vector<std::pair<int, Cost>>>& graph, int start,
    Cost infinity) {
  const int n = static_cast<int>(graph.size());
  assert(0 <= start && start < n);

  DijkstraResult<Cost> result{
      start, std::vector<Cost>(n, infinity), std::vector<int>(n, -1)};
  using QueueEntry = std::pair<Cost, int>;
  std::priority_queue<QueueEntry, std::vector<QueueEntry>,
                      std::greater<QueueEntry>>
      queue;

  result.distance[start] = Cost{};
  result.parent[start] = start;
  queue.push({Cost{}, start});

  while (!queue.empty()) {
    const auto [current_distance, vertex] = queue.top();
    queue.pop();
    if (result.distance[vertex] < current_distance) continue;

    for (const auto& [next, cost] : graph[vertex]) {
      assert(0 <= next && next < n);
      assert(!(cost < Cost{}));
      const Cost next_distance = current_distance + cost;
      if (next_distance < result.distance[next]) {
        result.distance[next] = next_distance;
        result.parent[next] = vertex;
        queue.push({next_distance, next});
      }
    }
  }
  return result;
}
