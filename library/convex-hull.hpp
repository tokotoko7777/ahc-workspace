#include <algorithm>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/convex-hull.hpp

// x, yメンバを持つ整数座標Pointの凸包。O(N log N)。
// 返り値は辞書順最小点から反時計回り。先頭点の重複はない。
// auto hull = convex_hull(points);  // Point2D<long long>や自作structに使える
template <class Point>
std::vector<Point> convex_hull(std::vector<Point> points,
                               bool include_collinear = false) {
  std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
    return a.x != b.x ? a.x < b.x : a.y < b.y;
  });
  points.erase(std::unique(points.begin(), points.end(),
                           [](const Point& a, const Point& b) {
                             return a.x == b.x && a.y == b.y;
                           }),
               points.end());
  if (points.size() <= 1) return points;

  const auto turn = [](const Point& a, const Point& b, const Point& c) {
    return (b.x - a.x) * (c.y - a.y) -
           (b.y - a.y) * (c.x - a.x);
  };
  bool all_collinear = true;
  for (int i = 2; i < static_cast<int>(points.size()); ++i) {
    if (turn(points[0], points[1], points[i]) != 0) all_collinear = false;
  }
  if (include_collinear && all_collinear) return points;

  std::vector<Point> lower;
  std::vector<Point> upper;
  lower.reserve(points.size());
  upper.reserve(points.size());
  for (const Point& point : points) {
    while (lower.size() >= 2) {
      const auto value = turn(lower[lower.size() - 2], lower.back(), point);
      if (include_collinear ? value >= 0 : value > 0) break;
      lower.pop_back();
    }
    lower.push_back(point);
  }
  for (auto iterator = points.rbegin(); iterator != points.rend(); ++iterator) {
    const Point& point = *iterator;
    while (upper.size() >= 2) {
      const auto value = turn(upper[upper.size() - 2], upper.back(), point);
      if (include_collinear ? value >= 0 : value > 0) break;
      upper.pop_back();
    }
    upper.push_back(point);
  }
  lower.pop_back();
  upper.pop_back();
  lower.insert(lower.end(), upper.begin(), upper.end());
  return lower;
}
