#ifndef AHC_MASKED_BFS_WORKSPACE_HPP
#define AHC_MASKED_BFS_WORKSPACE_HPP
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

// Reuse storage for repeated unweighted searches on integer vertex IDs.
// graph[v] may be any iterable adjacency range, allow(v) tests the current mask.
// A run does no allocation or O(node_count) clear, except stamp wraparound.
class MaskedBfsWorkspace {
  std::vector<uint32_t> stamp;
  std::vector<int> dist, bfs_parent, queue;
  uint32_t now = 0;

public:
  explicit MaskedBfsWorkspace(int n)
      : stamp(n), dist(n), bfs_parent(n), queue(n) { assert(n >= 0); }

  template<class Graph,class Allow>
  void run(const Graph& graph, int start, Allow allow) {
    assert(0 <= start && start < int(queue.size()));
    if (++now == 0) {
      std::fill(stamp.begin(), stamp.end(), 0);
      now = 1;
    }
    if (!allow(start)) return;
    int end = 0;
    queue[end++] = start;stamp[start] = now;dist[start] = 0;bfs_parent[start] = start;
    for (int head = 0; head < end; ++head) {
      int v = queue[head];
      for (int u : graph[v])
        if (stamp[u] != now && allow(u)) {
          queue[end++] = u;stamp[u] = now;dist[u] = dist[v] + 1;bfs_parent[u] = v;
        }
    }
  }

  int distance(int v) const { return now && stamp[v] == now ? dist[v] : -1; }
  int parent(int v) const { return now && stamp[v] == now ? bfs_parent[v] : -1; }
};
#endif
