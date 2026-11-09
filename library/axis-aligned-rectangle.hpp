#include <algorithm>
#include <cassert>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/axis-aligned-rectangle.hpp

// 軸に平行な半開長方形 [left, right) × [bottom, top)。
// Coordinate は int、long long などを選べる。
// 辺が接するだけなら overlaps() は false。
//
// 使い方:
// AxisAlignedRectangle<int> rectangle{0, 0, 10, 20};
// long long area = rectangle.area();
// bool hit = rectangle.overlaps(other);
template <class Coordinate>
struct AxisAlignedRectangle {
  Coordinate left;
  Coordinate bottom;
  Coordinate right;
  Coordinate top;

  bool is_valid() const { return left < right && bottom < top; }

  Coordinate width() const {
    assert(is_valid());
    return right - left;
  }

  Coordinate height() const {
    assert(is_valid());
    return top - bottom;
  }

  long long area() const {
    return 1LL * width() * height();
  }

  bool contains(Coordinate x, Coordinate y) const {
    return left <= x && x < right && bottom <= y && y < top;
  }

  bool overlaps(const AxisAlignedRectangle& other) const {
    return std::max(left, other.left) < std::min(right, other.right) &&
           std::max(bottom, other.bottom) < std::min(top, other.top);
  }
};
