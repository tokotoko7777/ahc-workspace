#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/bipartite-check.hpp

struct BipartiteCheckResult {
  bool bipartite;
  std::vector<int> color;  // 0または1。未訪問は-1。
  int component_count;
};

// 無向グラフを2色に塗れるか O(N+M) で判定する。非連結でもよい。
// auto result = bipartite_check(graph);
// if (result.bipartite) { int side = result.color[vertex]; }
inline BipartiteCheckResult bipartite_check(
    const std::vector<std::vector<int>>& graph) {
  const int n = static_cast<int>(graph.size());
  BipartiteCheckResult result{true, std::vector<int>(n, -1), 0};
  std::vector<int> queue;
  queue.reserve(n);
  for (int start = 0; start < n; ++start) {
    if (result.color[start] != -1) continue;
    ++result.component_count;
    result.color[start] = 0;
    queue.clear();
    queue.push_back(start);
    for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
      const int vertex = queue[head];
      for (int next : graph[vertex]) {
        assert(0 <= next && next < n);
        if (result.color[next] == -1) {
          result.color[next] = result.color[vertex] ^ 1;
          queue.push_back(next);
        } else if (result.color[next] == result.color[vertex]) {
          result.bipartite = false;
          return result;
        }
      }
    }
  }
  return result;
}
