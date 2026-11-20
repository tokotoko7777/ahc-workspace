#pragma once
#include <algorithm>
#include <cassert>
#include <vector>

// 四近傍の平坦グリッド。多数の候補への最初の一歩を、経路復元なしで引く。
// 例: bfs.run(start, [&](int cell) { return !wall[cell]; });
//     int next = bfs.first_step(goal); // 到達不能なら -1、goal==start なら start
// 行優先 index = row * width + col。run() ごとに内部領域を再利用する。
class FlatGridFirstStepBfs {
 public:
  FlatGridFirstStepBfs(int height, int width)
      : width_(width), dist_(height * width), first_(height * width),
        queue_(height * width) {
    assert(height > 0 && width > 0);
  }

  template <class CanEnter>
  void run(int start, CanEnter can_enter) {
    const int n = static_cast<int>(dist_.size());
    assert(0 <= start && start < n && can_enter(start));
    std::fill(dist_.begin(), dist_.end(), -1);
    dist_[start] = 0;
    first_[start] = start;
    int top = 0, end = 1;
    queue_[0] = start;
    while (top < end) {
      const int v = queue_[top++];
      const auto push = [&](int u) {
        if (dist_[u] >= 0 || !can_enter(u)) return;
        dist_[u] = dist_[v] + 1;
        first_[u] = v == start ? u : first_[v];
        queue_[end++] = u;
      };
      if (v >= width_) push(v - width_);
      if (v + width_ < n) push(v + width_);
      if (v % width_ != 0) push(v - 1);
      if (v % width_ != width_ - 1) push(v + 1);
    }
  }

  int distance(int cell) const { return dist_[cell]; }
  int first_step(int cell) const { return dist_[cell] < 0 ? -1 : first_[cell]; }

 private:
  int width_;
  std::vector<int> dist_, first_, queue_;
};
