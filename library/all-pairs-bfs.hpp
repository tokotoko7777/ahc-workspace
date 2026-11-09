#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/all-pairs-bfs.hpp

// 重みなしグラフの全頂点間最短距離を、各頂点からのBFSで求める。
// Distanceをuint16_tなどにすると、大きい距離表のメモリを減らせる。
// 到達不能は numeric_limits<Distance>::max()。
//
// 使い方:
// vector<vector<int>> graph(vertex_count);
// auto distance = all_pairs_bfs<unsigned short>(graph);
// cout << distance[from][to] << '\n';
//
// 頂点数をV、辺数をEとして、計算量は O(V(V+E))、メモリは O(V^2)。
template <class Distance = int>
std::vector<std::vector<Distance>> all_pairs_bfs(
    const std::vector<std::vector<int>>& graph) {
  static_assert(std::is_integral_v<Distance>);
  const int vertex_count = static_cast<int>(graph.size());
  const Distance unreachable = std::numeric_limits<Distance>::max();
  assert(static_cast<unsigned long long>(vertex_count)
         <= static_cast<unsigned long long>(unreachable));

  std::vector<std::vector<Distance>> distance(
      vertex_count,
      std::vector<Distance>(vertex_count, unreachable));
  std::vector<int> queue(vertex_count);

  for (int source = 0; source < vertex_count; ++source) {
    int head = 0;
    int tail = 0;
    queue[tail++] = source;
    distance[source][source] = Distance{0};

    while (head < tail) {
      const int vertex = queue[head++];
      for (int next : graph[vertex]) {
        assert(0 <= next && next < vertex_count);
        if (distance[source][next] != unreachable) continue;
        distance[source][next] =
            static_cast<Distance>(distance[source][vertex] + Distance{1});
        queue[tail++] = next;
      }
    }
  }
  return distance;
}
