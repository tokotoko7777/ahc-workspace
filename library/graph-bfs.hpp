#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/graph-bfs.hpp

struct GraphBfsResult {
  int start;
  std::vector<int> distance;
  std::vector<int> parent;

  bool reachable(int target) const {
    assert(0 <= target && target < static_cast<int>(parent.size()));
    return distance[target] != -1;
  }

  std::vector<int> path_to(int target) const {
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

// 重みなしグラフの最短距離を O(頂点数 + 辺数) で求める。
// graph[vertex] には、vertex から1辺で行ける頂点を入れる。
// 使い方:
// auto result = graph_bfs(graph, start);
// int distance = result.distance[goal];
// vector<int> path = result.path_to(goal);
inline GraphBfsResult graph_bfs(const std::vector<std::vector<int>>& graph,
                                int start) {
  const int n = static_cast<int>(graph.size());
  assert(0 <= start && start < n);

  GraphBfsResult result{start, std::vector<int>(n, -1),
                        std::vector<int>(n, -1)};
  std::vector<int> queue;
  queue.reserve(n);
  result.distance[start] = 0;
  result.parent[start] = start;
  queue.push_back(start);

  for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
    const int vertex = queue[head];
    for (int next : graph[vertex]) {
      assert(0 <= next && next < n);
      if (result.distance[next] != -1) continue;
      result.distance[next] = result.distance[vertex] + 1;
      result.parent[next] = vertex;
      queue.push_back(next);
    }
  }
  return result;
}
