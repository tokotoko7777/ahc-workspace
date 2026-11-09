#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/max-flow.hpp

// Dinic法による最大流。整数容量で使う。
// 使い方:
// MaxFlow<long long> flow(n);
// flow.add_edge(from, to, capacity);
// long long answer = flow.flow(source, sink);
template <class Capacity>
struct MaxFlow {
  struct Edge {
    int to;
    int reverse_index;
    Capacity capacity;
  };

  struct EdgeInfo {
    int from;
    int to;
    Capacity capacity;
    Capacity flow;
  };

  int n;
  std::vector<std::vector<Edge>> graph;
  std::vector<std::pair<int, int>> edge_positions;
  std::vector<int> level;
  std::vector<int> next_edge;

  explicit MaxFlow(int vertex_count)
      : n(vertex_count), graph(vertex_count), level(vertex_count),
        next_edge(vertex_count) {
    assert(vertex_count >= 0);
  }

  int add_edge(int from, int to, Capacity capacity) {
    assert(0 <= from && from < n && 0 <= to && to < n);
    assert(capacity >= Capacity{});
    const int index = static_cast<int>(edge_positions.size());
    edge_positions.push_back({from, static_cast<int>(graph[from].size())});
    const int from_index = static_cast<int>(graph[from].size());
    int to_index = static_cast<int>(graph[to].size());
    if (from == to) ++to_index;
    graph[from].push_back({to, to_index, capacity});
    graph[to].push_back({from, from_index, Capacity{}});
    return index;
  }

  EdgeInfo get_edge(int index) const {
    assert(0 <= index && index < static_cast<int>(edge_positions.size()));
    const auto [from, position] = edge_positions[index];
    const Edge& edge = graph[from][position];
    const Edge& reverse = graph[edge.to][edge.reverse_index];
    return {from, edge.to, edge.capacity + reverse.capacity,
            reverse.capacity};
  }

  std::vector<EdgeInfo> edges() const {
    std::vector<EdgeInfo> result;
    result.reserve(edge_positions.size());
    for (int i = 0; i < static_cast<int>(edge_positions.size()); ++i) {
      result.push_back(get_edge(i));
    }
    return result;
  }

  Capacity flow(int source, int sink,
                Capacity limit = std::numeric_limits<Capacity>::max()) {
    assert(0 <= source && source < n && 0 <= sink && sink < n);
    assert(source != sink && limit >= Capacity{});
    Capacity result{};
    while (result < limit && build_level_graph(source, sink)) {
      std::fill(next_edge.begin(), next_edge.end(), 0);
      while (result < limit) {
        const Capacity pushed =
            send_flow(source, sink, limit - result);
        if (pushed == Capacity{}) break;
        result += pushed;
      }
    }
    return result;
  }

  // 現在の残余グラフでsourceから到達できる頂点。最大流後は最小カットになる。
  std::vector<char> min_cut(int source) const {
    assert(0 <= source && source < n);
    std::vector<char> reachable(n, false);
    std::vector<int> queue;
    queue.reserve(n);
    reachable[source] = true;
    queue.push_back(source);
    for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
      const int vertex = queue[head];
      for (const Edge& edge : graph[vertex]) {
        if (edge.capacity == Capacity{} || reachable[edge.to]) continue;
        reachable[edge.to] = true;
        queue.push_back(edge.to);
      }
    }
    return reachable;
  }

 private:
  bool build_level_graph(int source, int sink) {
    std::fill(level.begin(), level.end(), -1);
    std::vector<int> queue;
    queue.reserve(n);
    level[source] = 0;
    queue.push_back(source);
    for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
      const int vertex = queue[head];
      for (const Edge& edge : graph[vertex]) {
        if (edge.capacity == Capacity{} || level[edge.to] != -1) continue;
        level[edge.to] = level[vertex] + 1;
        queue.push_back(edge.to);
      }
    }
    return level[sink] != -1;
  }

  Capacity send_flow(int vertex, int sink, Capacity upper_bound) {
    if (vertex == sink) return upper_bound;
    for (int& index = next_edge[vertex];
         index < static_cast<int>(graph[vertex].size()); ++index) {
      Edge& edge = graph[vertex][index];
      if (edge.capacity == Capacity{} ||
          level[edge.to] != level[vertex] + 1) {
        continue;
      }
      const Capacity pushed =
          send_flow(edge.to, sink, std::min(upper_bound, edge.capacity));
      if (pushed == Capacity{}) continue;
      edge.capacity -= pushed;
      graph[edge.to][edge.reverse_index].capacity += pushed;
      return pushed;
    }
    return Capacity{};
  }
};
