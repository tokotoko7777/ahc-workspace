#include <cassert>
#include <cmath>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/point-2d.hpp

// 2次元座標の基本演算。整数座標は Point2D<long long> にすると溢れにくい。
template <class Coordinate>
struct Point2D {
  Coordinate x{};
  Coordinate y{};

  Point2D operator+(const Point2D& other) const {
    return {x + other.x, y + other.y};
  }

  Point2D operator-(const Point2D& other) const {
    return {x - other.x, y - other.y};
  }

  Point2D operator*(Coordinate scale) const { return {x * scale, y * scale}; }

  Point2D& operator+=(const Point2D& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  Point2D& operator-=(const Point2D& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  bool operator==(const Point2D& other) const {
    return x == other.x && y == other.y;
  }

  bool operator!=(const Point2D& other) const { return !(*this == other); }

  bool operator<(const Point2D& other) const {
    return x != other.x ? x < other.x : y < other.y;
  }
};

template <class Coordinate>
Coordinate dot(const Point2D<Coordinate>& a, const Point2D<Coordinate>& b) {
  return a.x * b.x + a.y * b.y;
}

template <class Coordinate>
Coordinate cross(const Point2D<Coordinate>& a, const Point2D<Coordinate>& b) {
  return a.x * b.y - a.y * b.x;
}

template <class Coordinate>
Coordinate cross(const Point2D<Coordinate>& origin,
                 const Point2D<Coordinate>& a,
                 const Point2D<Coordinate>& b) {
  return cross(a - origin, b - origin);
}

template <class Coordinate>
Coordinate squared_distance(const Point2D<Coordinate>& a,
                            const Point2D<Coordinate>& b) {
  const Point2D<Coordinate> difference = a - b;
  return dot(difference, difference);
}

template <class Coordinate>
Coordinate manhattan_distance(const Point2D<Coordinate>& a,
                              const Point2D<Coordinate>& b) {
  using std::abs;
  return abs(a.x - b.x) + abs(a.y - b.y);
}

template <class Coordinate>
long double euclidean_distance(const Point2D<Coordinate>& a,
                               const Point2D<Coordinate>& b) {
  return std::sqrt(static_cast<long double>(squared_distance(a, b)));
}

// a->bから見てcが左なら1、一直線なら0、右なら-1。
template <class Coordinate>
int orientation(const Point2D<Coordinate>& a, const Point2D<Coordinate>& b,
                const Point2D<Coordinate>& c) {
  const Coordinate value = cross(a, b, c);
  return (Coordinate{} < value) - (value < Coordinate{});
}
