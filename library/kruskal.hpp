#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/kruskal.hpp

template <class Cost>
struct KruskalEdge {
  int from;
  int to;
  Cost cost;
};

template <class Cost>
struct KruskalResult {
  Cost total_cost;
  std::vector<KruskalEdge<Cost>> edges;
  int component_count;

  bool connected() const { return component_count <= 1; }
};

// 無向グラフの最小全域木を O(M log M) で求める。
// 連結でない場合は、各連結成分の最小全域木を合わせた森を返す。
template <class Cost>
KruskalResult<Cost> kruskal(int vertex_count,
                            std::vector<KruskalEdge<Cost>> edges) {
  assert(vertex_count >= 0);
  for (const auto& edge : edges) {
    assert(0 <= edge.from && edge.from < vertex_count);
    assert(0 <= edge.to && edge.to < vertex_count);
  }
  std::sort(edges.begin(), edges.end(), [](const auto& a, const auto& b) {
    return a.cost < b.cost;
  });

  std::vector<int> parent(vertex_count);
  std::vector<int> size(vertex_count, 1);
  std::iota(parent.begin(), parent.end(), 0);
  const auto leader = [&](int vertex) {
    int root = vertex;
    while (parent[root] != root) root = parent[root];
    while (parent[vertex] != vertex) {
      const int next = parent[vertex];
      parent[vertex] = root;
      vertex = next;
    }
    return root;
  };

  KruskalResult<Cost> result{Cost{}, {}, vertex_count};
  if (vertex_count > 0) result.edges.reserve(vertex_count - 1);
  for (KruskalEdge<Cost>& edge : edges) {
    int a = leader(edge.from);
    int b = leader(edge.to);
    if (a == b) continue;
    if (size[a] < size[b]) std::swap(a, b);
    parent[b] = a;
    size[a] += size[b];
    result.total_cost += edge.cost;
    result.edges.push_back(std::move(edge));
    --result.component_count;
    if (result.component_count == 1) break;
  }
  return result;
}
