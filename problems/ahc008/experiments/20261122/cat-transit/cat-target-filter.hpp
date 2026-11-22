#pragma once
#include <array>
#include <algorithm>
#include <bit>
#include "shortest-first-masks.hpp"

// Posterior state is over target cells and the 'choose a new target' sentinel.
// Decisions depend only on the judge's public movement history, not its RNG.
template<int N>
struct CatTargetFilter {
  std::array<float,N> target{};
  float reset = 1.0f;
  static constexpr std::array<float,5> inv{0,1,0.5f,1.f/3,0.25f};

  template<class Neighbors, class Admissible>
  void observe(int origin, int first_direction, int mid, int finish,
               const Neighbors& neighbors, Admissible admissible,
               ShortestFirstMasks<N>& bfs) {
    if (first_direction < 0 || first_direction >= 4) {
      target.fill(0); reset = 1; return;
    }
    bfs.run(origin, neighbors, admissible);
    if (bfs.count < 2) { target.fill(0); reset = 1; return; }
    float renew = reset + target[origin];
    for (int q = 0; q < N; ++q)
      if (bfs.distance[q] < 0) renew += target[q], target[q] = 0;
    const float uniform = renew / (bfs.count-1);
    const uint8_t bit = 1u << first_direction;
    float total = 0;
    target[origin] = 0;
    for (int i = 1; i < bfs.count; ++i) {
      int q = bfs.reachable[i];
      const unsigned m = bfs.mask[q];
      target[q] = m & bit ? (target[q] + uniform) * inv[std::popcount(m)] : 0;
      total += target[q];
    }
    if (total < 1.e-30f) {
      target.fill(0); reset = 1; return;
    }
    reset = (target[mid] + (finish == mid ? 0 : target[finish])) / total;
    target[mid] = target[finish] = 0;
    const float factor = 1.0f / total;
    for (int i = 1; i < bfs.count; ++i) target[bfs.reachable[i]] *= factor;
  }

  template<class Neighbors, class Admissible>
  std::array<float,4> next(int origin, const Neighbors& neighbors,
                           Admissible admissible, ShortestFirstMasks<N>& bfs) const {
    bfs.run(origin, neighbors, admissible);
    std::array<float,4> ans{};
    if (bfs.count < 2) return ans;
    float renew = reset + target[origin];
    for (int q = 0; q < N; ++q)
      if (bfs.distance[q] < 0) renew += target[q];
    const float uniform = renew / (bfs.count-1);
    for (int i = 1; i < bfs.count; ++i) {
      const int q = bfs.reachable[i];
      unsigned m = bfs.mask[q];
      const float p = (target[q] + uniform) * inv[std::popcount(m)];
      while (m) {
        ans[std::countr_zero(m)] += p;
        m &= m - 1;
      }
    }
    return ans;
  }
};
