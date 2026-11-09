#include <algorithm>
#include <stdexcept>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/largest-empty-rectangle.hpp

// 整数格子の指定セル(x,y)を含む、障害物と重ならない最大面積の長方形。
// bounds内に限定する。境界が接するだけなら重なりではない。
// TODO: Rectへleft,bottom,right,topとarea()を書く。座標型は同じ整数型にする。
//   area()は面積がoverflowしない型を返すこと。
// TODO: obstaclesへ使用中の半開長方形を入れる。動かす対象自身は除く。
//   各障害物は正の面積を持つこと。障害物同士の重なりは許す。
// auto answer = largest_empty_rectangle(bounds, x, y, obstacles);
// 不正なbounds、範囲外/塞がれた指定セルはinvalid_argument。
// 計算量O(M^2 + M log M)、追加メモリO(M)。Mは障害物数。
// AHC001のような、点を含む広告の再配置で使える。
// 最大「面積」であり、任意の評価関数を最大化する関数ではない。
// 繰り返し使う場合だけ用意するscratch。問題のStateに入れずProblem側に1個置く。
// 同時実行では共有しない。obstaclesへのpointerは次の呼び出しで必ず破棄する。
template <class Rect>
struct LargestEmptyRectangleWorkspace {
  std::vector<decltype(Rect::left)> lefts;
  std::vector<const Rect*> rights;
};

template <class Rect>
Rect largest_empty_rectangle(const Rect& bounds, decltype(Rect::left) x,
                             decltype(Rect::bottom) y,
                             const std::vector<Rect>& obstacles,
                             LargestEmptyRectangleWorkspace<Rect>& workspace) {
  using Coordinate = decltype(Rect::left);
  if (!(bounds.left <= x && x < bounds.right &&
        bounds.bottom <= y && y < bounds.top)) {
    throw std::invalid_argument("anchor cell is outside bounds");
  }
  Coordinate low = bounds.left, high = bounds.right;
  // 指定セルと同じ高さを塞ぐ障害物で、左右の到達可能範囲を先に絞る。
  for (const auto& o : obstacles) {
    if (!(o.left < o.right && o.bottom < o.top)) {
      throw std::invalid_argument("invalid obstacle");
    }
    if (o.bottom <= y && y < o.top) {
      if (o.right <= x) low = std::max(low, o.right);
      else if (x < o.left) high = std::min(high, o.left);
      else throw std::invalid_argument("anchor cell is blocked");
    }
  }
  auto& lefts = workspace.lefts;
  auto& rights = workspace.rights;
  lefts.clear();
  rights.clear();
  lefts.push_back(low);
  lefts.reserve(obstacles.size() + 1);
  rights.reserve(obstacles.size());
  for (const auto& o : obstacles) {
    if (low < o.right && o.right <= x) lefts.push_back(o.right);
    if (x < o.left && o.left < high) rights.push_back(&o);
  }
  std::sort(lefts.begin(), lefts.end());
  lefts.erase(std::unique(lefts.begin(), lefts.end()), lefts.end());
  std::sort(rights.begin(), rights.end(), [](const Rect* a, const Rect* b) {
    return a->left < b->left;
  });
  Rect best{x, y, static_cast<Coordinate>(x + 1), static_cast<Coordinate>(y + 1)};
  for (Coordinate left : lefts) {
    // 障害物を無視した最大面積でも更新不能。以降のleftはさらに右なので打ち切れる。
    const Rect optimistic{left, bounds.bottom, high, bounds.top};
    if (optimistic.area() <= best.area()) break;
    Coordinate bottom = bounds.bottom, top = bounds.top;
    const auto restrict_y = [&](const Rect& o) {
      if (o.top <= y) bottom = std::max(bottom, o.top);
      else if (y < o.bottom) top = std::min(top, o.bottom);
    };
    for (const auto& o : obstacles) {
      if (left < o.right && o.left <= x) restrict_y(o);
    }
    const auto consider = [&](Coordinate right) {
      Rect candidate{left, bottom, right, top};
      if (best.area() < candidate.area()) best = candidate;
    };
    for (const Rect* o : rights) {
      consider(o->left); // 障害物へ接する所までは伸ばせる。
      restrict_y(*o);   // そこを越えるなら上下の幅を狭める。
    }
    consider(high);
  }
  return best;
}

// 一度だけ使う場合は従来の4引数でもよい。返す長方形と同点の選び方は同じ。
template <class Rect>
Rect largest_empty_rectangle(const Rect& bounds, decltype(Rect::left) x,
                             decltype(Rect::bottom) y,
                             const std::vector<Rect>& obstacles) {
  LargestEmptyRectangleWorkspace<Rect> workspace;
  return largest_empty_rectangle(bounds, x, y, obstacles, workspace);
}
