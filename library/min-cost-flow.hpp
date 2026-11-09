#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <queue>
#include <type_traits>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/min-cost-flow.hpp

// 非負コスト辺用の、potential + Dijkstraによる最小費用流。
// add_edgeには cost >= 0 の辺だけを渡す。この制約により初期Bellman-Fordを省く。
// 使い方:
// MinCostFlow<int, long long> flow(n);
// flow.add_edge(from, to, capacity, cost);
// auto [sent, minimum_cost] = flow.flow(source, sink, wanted_flow);
template <class Capacity, class Cost>
struct MinCostFlow {
  static_assert(std::is_signed<Cost>::value,
                "Cost must be signed because reverse edges have negative cost");

  struct Edge {
    int to;
    int reverse_index;
    Capacity capacity;
    Cost cost;
  };

  struct EdgeInfo {
    int from;
    int to;
    Capacity capacity;
    Capacity flow;
    Cost cost;
  };

  int n;
  std::vector<std::vector<Edge>> graph;
  std::vector<std::pair<int, int>> edge_positions;
  bool already_run = false;

  explicit MinCostFlow(int vertex_count)
      : n(vertex_count), graph(vertex_count) {
    assert(vertex_count >= 0);
  }

  int add_edge(int from, int to, Capacity capacity, Cost cost) {
    assert(!already_run);
    assert(0 <= from && from < n && 0 <= to && to < n);
    assert(capacity >= Capacity{} && cost >= Cost{});
    const int index = static_cast<int>(edge_positions.size());
    edge_positions.push_back({from, static_cast<int>(graph[from].size())});
    const int from_index = static_cast<int>(graph[from].size());
    int to_index = static_cast<int>(graph[to].size());
    if (from == to) ++to_index;
    graph[from].push_back({to, to_index, capacity, cost});
    graph[to].push_back({from, from_index, Capacity{}, -cost});
    return index;
  }

  EdgeInfo get_edge(int index) const {
    assert(0 <= index && index < static_cast<int>(edge_positions.size()));
    const auto [from, position] = edge_positions[index];
    const Edge& edge = graph[from][position];
    const Edge& reverse = graph[edge.to][edge.reverse_index];
    return {from, edge.to, edge.capacity + reverse.capacity,
            reverse.capacity, edge.cost};
  }

  // 最大 flow_limit だけ流し、{実際に流せた量, 最小費用} を返す。
  // 安全な十分大きい値を infinity に渡す。1つのインスタンスにつき1回だけ呼べる。
  std::pair<Capacity, Cost> flow(
      int source, int sink, Capacity flow_limit,
      Cost infinity = std::numeric_limits<Cost>::max() / 4) {
    assert(!already_run);
    already_run = true;
    assert(0 <= source && source < n && 0 <= sink && sink < n);
    assert(source != sink && flow_limit >= Capacity{});

    Capacity total_flow{};
    Cost total_cost{};
    std::vector<Cost> potential(n, Cost{});
    std::vector<Cost> distance(n);
    std::vector<int> previous_vertex(n);
    std::vector<int> previous_edge(n);
    using QueueEntry = std::pair<Cost, int>;

    while (total_flow < flow_limit) {
      std::fill(distance.begin(), distance.end(), infinity);
      distance[source] = Cost{};
      std::priority_queue<QueueEntry, std::vector<QueueEntry>,
                          std::greater<QueueEntry>>
          queue;
      queue.push({Cost{}, source});

      while (!queue.empty()) {
        const auto [current_distance, vertex] = queue.top();
        queue.pop();
        if (distance[vertex] < current_distance) continue;
        for (int index = 0; index < static_cast<int>(graph[vertex].size());
             ++index) {
          const Edge& edge = graph[vertex][index];
          if (edge.capacity == Capacity{}) continue;
          const Cost next_distance = current_distance + edge.cost +
                                     potential[vertex] - potential[edge.to];
          if (distance[edge.to] <= next_distance) continue;
          distance[edge.to] = next_distance;
          previous_vertex[edge.to] = vertex;
          previous_edge[edge.to] = index;
          queue.push({next_distance, edge.to});
        }
      }
      if (distance[sink] == infinity) break;

      for (int vertex = 0; vertex < n; ++vertex) {
        if (distance[vertex] != infinity) potential[vertex] += distance[vertex];
      }
      Capacity pushed = flow_limit - total_flow;
      for (int vertex = sink; vertex != source;
           vertex = previous_vertex[vertex]) {
        pushed = std::min(
            pushed,
            graph[previous_vertex[vertex]][previous_edge[vertex]].capacity);
      }
      for (int vertex = sink; vertex != source;
           vertex = previous_vertex[vertex]) {
        Edge& edge = graph[previous_vertex[vertex]][previous_edge[vertex]];
        edge.capacity -= pushed;
        graph[edge.to][edge.reverse_index].capacity += pushed;
      }
      total_flow += pushed;
      total_cost += static_cast<Cost>(pushed) * potential[sink];
    }
    return {total_flow, total_cost};
  }
};
