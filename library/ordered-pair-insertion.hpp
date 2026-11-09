#include <cassert>
#include <type_traits>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/ordered-pair-insertion.hpp

template <class Cost> struct OrderedPairInsertion {
  Cost delta; // 挿入後の距離 - 挿入前の距離。符号付き数値型を使う。
  int first_gap, second_gap;
};

// 経路へfirst→secondの順で2点を入れる最良位置をO(n)、追加メモリO(1)で探す。
// gap g は「元のroute[g-1]とroute[g]の間」。両端の外には挿入しない。
// 同じgapならfirst,secondを連続で入れる。適用時はsecondを先に挿入すると添字がずれない。
// 同点は(first_gap,second_gap)の辞書順。非対称距離にも対応する。
// 先行制約以外（容量・時間窓など）は扱わない。必要なら問題側で別の探索を書く。
// Routeはsize()とoperator[]を持つ型。2点を除いた仮想ビューでもよく、全コピー不要。
// 使い方: auto p = best_ordered_pair_insertion(route, pickup, delivery, distance);
// vectorの場合の適用例（採用を決めた後だけ実行）:
//   route.insert(route.begin() + p.second_gap, delivery);
//   route.insert(route.begin() + p.first_gap, pickup);
//   cost += p.delta;
// TODO: 距離以外の制約がある問題では、この最良位置が合法か別途確認する。
template <class Route, class Point, class Distance>
auto best_ordered_pair_insertion(const Route& route, const Point& first,
                                 const Point& second, Distance distance) {
  using Cost = std::decay_t<decltype(distance(route[0], first))>;
  const int n = static_cast<int>(route.size());
  assert(n >= 2);
  const Cost between = distance(first, second);
  const auto together = [&](int g) -> Cost {
    return distance(route[g - 1], first) + between + distance(second, route[g]) -
           distance(route[g - 1], route[g]);
  };
  OrderedPairInsertion<Cost> best{together(1), 1, 1};
  Cost best_first{};
  int first_gap = -1;
  const auto consider = [&](Cost delta, int a, int b) {
    if (delta < best.delta || (delta == best.delta &&
        (a < best.first_gap || (a == best.first_gap && b < best.second_gap))))
      best = {delta, a, b};
  };
  for (int g = 1; g < n; ++g) {
    const Cost old_edge = distance(route[g - 1], route[g]);
    // 違う2本の辺へ挿入するなら、増分は独立。手前のfirstの最小増分だけ保持する。
    if (first_gap >= 0) {
      const Cost second_delta = distance(route[g - 1], second) +
                                distance(second, route[g]) - old_edge;
      consider(best_first + second_delta, first_gap, g);
    }
    consider(together(g), g, g); // 同じ辺へ入れる場合は共有辺を別計算する。
    const Cost first_delta = distance(route[g - 1], first) +
                             distance(first, route[g]) - old_edge;
    if (first_gap < 0 || first_delta < best_first) {
      best_first = first_delta;
      first_gap = g;
    }
  }
  return best;
}
