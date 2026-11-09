#include <cassert>
#include <type_traits>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/route-utils.hpp

// 経路の長さと、挿入・削除・移動・交換・区間反転の差分を計算する。
// Route は vector<int>、vector<pair<int, int>> など自由に選べる。
// distance(a, b) は2点間の距離を返す関数にする。
// 差分には負の値もあるため、距離の戻り値には符号付き整数や浮動小数点型を使う。
//
// 使い方:
// auto distance = [](Point a, Point b) { ... };
// long long cost = route_length(route, distance);
// long long delta = route_insertion_delta(route, position, point, distance);

template <class Route, class Distance>
auto route_length(const Route& route, Distance distance) {
  using Cost = std::decay_t<decltype(distance(route[0], route[0]))>;
  Cost total{};
  for (int i = 1; i < static_cast<int>(route.size()); ++i) {
    total += distance(route[i - 1], route[i]);
  }
  return total;
}

// position の直前へ point を挿入した時の「新しい距離 - 古い距離」。
template <class Route, class Point, class Distance>
auto route_insertion_delta(
    const Route& route,
    int position,
    const Point& point,
    Distance distance) {
  assert(0 < position && position < static_cast<int>(route.size()));
  return distance(route[position - 1], point) +
         distance(point, route[position]) -
         distance(route[position - 1], route[position]);
}

// position の点を削除した時の「新しい距離 - 古い距離」。
template <class Route, class Distance>
auto route_removal_delta(
    const Route& route,
    int position,
    Distance distance) {
  assert(0 < position && position + 1 < static_cast<int>(route.size()));
  return distance(route[position - 1], route[position + 1]) -
         distance(route[position - 1], route[position]) -
         distance(route[position], route[position + 1]);
}

// [left, right] をreverseした時の「新しい距離 - 古い距離」。
// Manhattan距離やEuclid距離のように distance(a,b)==distance(b,a) の時だけ使える。
template <class Route, class Distance>
auto route_reverse_delta(
    const Route& route,
    int left,
    int right,
    Distance distance) {
  assert(0 < left && left <= right);
  assert(right + 1 < static_cast<int>(route.size()));
  return distance(route[left - 1], route[right]) +
         distance(route[left], route[right + 1]) -
         distance(route[left - 1], route[left]) -
         distance(route[right], route[right + 1]);
}

// route[from]を抜き、最終的に添字toへ置く差分。端点は固定する。
// from/toは変更前/変更後の添字。非対称距離にも対応、経路を変更せずO(1)。
template <class Route, class Distance>
auto route_relocate_delta(const Route& route, int from, int to, Distance distance) {
  using Cost = std::decay_t<decltype(distance(route[0], route[0]))>;
  assert(0 < from && from + 1 < static_cast<int>(route.size()));
  assert(0 < to && to + 1 < static_cast<int>(route.size()));
  if (from == to) return Cost{};
  const int left = to < from ? to - 1 : to;
  const int right = left + 1;
  return static_cast<Cost>(route_removal_delta(route, from, distance) +
         distance(route[left], route[from]) + distance(route[from], route[right]) -
         distance(route[left], route[right]));
}

// 2点交換の差分。隣接時に同じ辺を二重計上しない。非対称距離にも対応、O(1)。
template <class Route, class Distance>
auto route_swap_delta(const Route& route, int first, int second, Distance distance) {
  using Cost = std::decay_t<decltype(distance(route[0], route[0]))>;
  assert(0 < first && first + 1 < static_cast<int>(route.size()));
  assert(0 < second && second + 1 < static_cast<int>(route.size()));
  const int edges[] = {first - 1, first, second - 1, second};
  const auto after = [&](int i) -> decltype(auto) {
    return route[i == first ? second : i == second ? first : i];
  };
  Cost delta{};
  for (int k = 0; k < 4; ++k) {
    bool duplicate = false;
    for (int j = 0; j < k; ++j) duplicate |= edges[j] == edges[k];
    if (duplicate) continue;
    const int i = edges[k];
    delta += distance(after(i), after(i + 1)) - distance(route[i], route[i + 1]);
  }
  return delta;
}
