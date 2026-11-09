#include <cassert>
#include <type_traits>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/farthest-point-sampling.hpp

// 互いに離れた代表をgreedyに選ぶ。distance(a, b)は整数でも小数でもよい。
// 計算量は O(item_count * sample_count)、追加メモリは O(item_count)。
//
// 使い方:
// auto sample = farthest_point_sampling(
//     points.size(), 10,
//     [&](int a, int b) { return squared_distance(points[a], points[b]); });
template <class Distance>
std::vector<int> farthest_point_sampling(
    int item_count,
    int sample_count,
    Distance distance,
    int first = 0) {
  assert(item_count > 0);
  assert(0 < sample_count && sample_count <= item_count);
  assert(0 <= first && first < item_count);

  using DistanceType =
      std::decay_t<decltype(distance(first, first))>;
  std::vector<DistanceType> nearest_distance(item_count);
  std::vector<char> selected(item_count, false);
  std::vector<int> result;
  result.reserve(sample_count);

  selected[first] = true;
  result.push_back(first);
  for (int item = 0; item < item_count; ++item) {
    nearest_distance[item] = distance(first, item);
  }

  while (static_cast<int>(result.size()) < sample_count) {
    int farthest = -1;
    for (int item = 0; item < item_count; ++item) {
      if (selected[item]) continue;
      if (farthest == -1 ||
          nearest_distance[farthest] < nearest_distance[item]) {
        farthest = item;
      }
    }

    selected[farthest] = true;
    result.push_back(farthest);
    for (int item = 0; item < item_count; ++item) {
      const DistanceType candidate = distance(farthest, item);
      if (candidate < nearest_distance[item]) {
        nearest_distance[item] = candidate;
      }
    }
  }
  return result;
}
