#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/floyd-warshall.hpp

// distance[i][j] を、全頂点間の最短距離へ更新する。
// 負の辺も使えるが、到達可能な負閉路がある場合は通常の最短距離にならない。
// infinity + 有限距離がオーバーフローしない値を infinity に渡す。
template <class Cost>
void floyd_warshall(std::vector<std::vector<Cost>>& distance,
                    const Cost& infinity) {
  const int n = static_cast<int>(distance.size());
  for (const auto& row : distance) {
    assert(static_cast<int>(row.size()) == n);
  }

  for (int middle = 0; middle < n; ++middle) {
    for (int from = 0; from < n; ++from) {
      if (distance[from][middle] == infinity) continue;
      for (int to = 0; to < n; ++to) {
        if (distance[middle][to] == infinity) continue;
        const Cost candidate =
            distance[from][middle] + distance[middle][to];
        if (candidate < distance[from][to]) distance[from][to] = candidate;
      }
    }
  }
}

template <class Cost>
bool has_negative_cycle(
    const std::vector<std::vector<Cost>>& shortest_distance) {
  const int n = static_cast<int>(shortest_distance.size());
  for (int vertex = 0; vertex < n; ++vertex) {
    assert(static_cast<int>(shortest_distance[vertex].size()) == n);
    if (shortest_distance[vertex][vertex] < Cost{}) return true;
  }
  return false;
}
