#include <algorithm>
#include <cassert>
#include <deque>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/zero-one-bfs.hpp

struct ZeroOneBfsResult {
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

// 辺コストが0か1だけのグラフの最短距離を O(頂点数 + 辺数) で求める。
// graph[vertex] には {行き先, 0または1} を入れる。
inline ZeroOneBfsResult zero_one_bfs(
    const std::vector<std::vector<std::pair<int, int>>>& graph, int start) {
  const int n = static_cast<int>(graph.size());
  assert(0 <= start && start < n);

  ZeroOneBfsResult result{start, std::vector<int>(n, -1),
                          std::vector<int>(n, -1)};
  std::deque<int> queue;
  result.distance[start] = 0;
  result.parent[start] = start;
  queue.push_back(start);

  while (!queue.empty()) {
    const int vertex = queue.front();
    queue.pop_front();
    for (const auto& [next, cost] : graph[vertex]) {
      assert(0 <= next && next < n);
      assert(cost == 0 || cost == 1);
      const int next_distance = result.distance[vertex] + cost;
      if (result.distance[next] != -1 &&
          result.distance[next] <= next_distance) {
        continue;
      }
      result.distance[next] = next_distance;
      result.parent[next] = vertex;
      if (cost == 0) {
        queue.push_front(next);
      } else {
        queue.push_back(next);
      }
    }
  }
  return result;
}
