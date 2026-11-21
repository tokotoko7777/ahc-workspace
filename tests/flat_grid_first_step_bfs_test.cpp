#include <cassert>
#include <random>
#include "../library/flat-grid-first-step-bfs.hpp"
#include "../library/grid-bfs.hpp"

int main() {
  std::mt19937 rng(0x804d);
  for (int t = 0; t < 400; ++t) {
    const int h = 1 + rng() % 23, w = 1 + rng() % 23;
    FlatGridFirstStepBfs bfs(h, w);
    for (int q = 0; q < 10; ++q) {
      std::vector<int> blocked(h * w);
      for (int& b : blocked) b = rng() % 7 < 2;
      const int s = rng() % (h * w);
      blocked[s] = 0;
      bfs.run(s, [&](int p) { return !blocked[p]; });
      auto ref = grid_bfs(h, w, {s/w, s%w}, [&](int r, int c) {
        return !blocked[r*w+c];
      });
      for (int p = 0; p < h*w; ++p) {
        assert(ref.distance[p/w][p%w] == bfs.distance(p));
        const auto path = ref.path_to({p/w, p%w});
        if (path.empty()) assert(bfs.first_step(p) == -1);
        else {
          const auto [r,c] = path[path.size() > 1];
          assert(bfs.first_step(p) == r*w+c);
        }
      }
    }
  }
}
