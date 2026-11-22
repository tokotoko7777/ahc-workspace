#pragma once
#include <array>
#include <cstdint>

// Four-neighbor unweighted graph. Neighbor callback supplies -1 for a missing
// edge, and admissible(v) determines whether a vertex may be visited.
template<int N>
struct ShortestFirstMasks {
  std::array<int, N> distance, reachable;
  std::array<uint8_t, N> mask;
  int count = 0;
  template<class Neighbors, class Admissible>
  void run(int source, const Neighbors& neighbors, Admissible admissible) {
    distance.fill(-1);
    mask.fill(0);
    distance[source] = 0;
    reachable[0] = source;
    count = 1;
    for (int head = 0; head < count; ++head) {
      const int p = reachable[head], next_distance = distance[p] + 1;
      for (int d = 0; d < 4; ++d) {
        int q = neighbors[p][d];
        if (q < 0 || (distance[q] >= 0 && distance[q] != next_distance)) continue;
        if (distance[q] < 0) {
          if (!admissible(q)) continue;
          distance[q] = next_distance;
          reachable[count++] = q;
        }
        mask[q] |= p == source ? uint8_t(1u << d) : mask[p];
      }
    }
  }
};
