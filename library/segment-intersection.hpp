#include <algorithm>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/segment-intersection.hpp

// x, yメンバを持つ整数座標Point用。pが閉線分[a,b]上にあるか。
template <class Point>
bool point_on_segment(const Point& a, const Point& b, const Point& p) {
  const auto cross_value =
      (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
  if (cross_value != 0) return false;
  return std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x) &&
         std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
}

// 閉線分[a,b]と[c,d]が共有点を持つか。端点接触・重なりもtrue。
template <class Point>
bool segments_intersect(const Point& a, const Point& b, const Point& c,
                        const Point& d) {
  const auto cross_value = [](const Point& origin, const Point& p,
                              const Point& q) {
    return (p.x - origin.x) * (q.y - origin.y) -
           (p.y - origin.y) * (q.x - origin.x);
  };
  const auto ab_c = cross_value(a, b, c);
  const auto ab_d = cross_value(a, b, d);
  const auto cd_a = cross_value(c, d, a);
  const auto cd_b = cross_value(c, d, b);
  if (ab_c == 0 && point_on_segment(a, b, c)) return true;
  if (ab_d == 0 && point_on_segment(a, b, d)) return true;
  if (cd_a == 0 && point_on_segment(c, d, a)) return true;
  if (cd_b == 0 && point_on_segment(c, d, b)) return true;
  return ((ab_c < 0) != (ab_d < 0)) && ((cd_a < 0) != (cd_b < 0));
}

// 端点や重なりを含まず、両線分の内部で交差するか。
template <class Point>
bool segments_properly_intersect(const Point& a, const Point& b,
                                 const Point& c, const Point& d) {
  const auto cross_value = [](const Point& origin, const Point& p,
                              const Point& q) {
    return (p.x - origin.x) * (q.y - origin.y) -
           (p.y - origin.y) * (q.x - origin.x);
  };
  const auto ab_c = cross_value(a, b, c);
  const auto ab_d = cross_value(a, b, d);
  const auto cd_a = cross_value(c, d, a);
  const auto cd_b = cross_value(c, d, b);
  return ab_c != 0 && ab_d != 0 && cd_a != 0 && cd_b != 0 &&
         ((ab_c < 0) != (ab_d < 0)) && ((cd_a < 0) != (cd_b < 0));
}
