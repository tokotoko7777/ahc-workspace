# 盤面対称変換のライブラリ案

| 問題 | 使用箇所 | 困ったこと | 現状と提案 |
| --- | --- | --- | --- |
| AHC008 | 設営方向の選択と、観測・操作の座標変換 | 人と動物の初期座標、毎ターンの動物の移動、人間の指示を別々に反転すると逆変換を間違えやすい。 | 座標・差分・逆写像を同じ型にまとめた。長方形でも全8方向と往復をテストする。 |

`GridIsometry{H, W, flags}` はコピーなしで元の盤面から新しい向きへの位置・差分を写す。出力を元の向きに戻すときは `inverse()` を使う。問題固有の操作文字への変換はソルバーの責務。

```bash
c++ -std=gnu++23 -Wall -Wextra -pedantic-errors grid_isometry_test.cpp -o /tmp/grid_isometry_test
/tmp/grid_isometry_test
```

## 試作ヘッダー: grid-isometry.hpp

```cpp
#ifndef AHC_GRID_ISOMETRY_HPP
#define AHC_GRID_ISOMETRY_HPP
#include <cassert>
#include <utility>

// Orient a rectangular grid without copying it. The transpose, if any,
// is applied before the row and column reflections in the destination.
// Example: GridIsometry t{10, 20, GridIsometry::transpose};
//          auto [r, c] = t.position(3, 8); // (8, 3), in a 20x10 grid
struct GridIsometry {
  static constexpr unsigned reflect_rows = 1;
  static constexpr unsigned reflect_cols = 2;
  static constexpr unsigned transpose = 4;

  int height;
  int width;
  unsigned flags;

  constexpr int dst_height() const { return flags & transpose ? width : height; }
  constexpr int dst_width() const { return flags & transpose ? height : width; }

  constexpr std::pair<int, int> position(int r, int c) const {
    assert(0 <= r && r < height && 0 <= c && c < width && flags < 8);
    if (flags & transpose) std::swap(r, c);
    if (flags & reflect_rows) r = dst_height() - 1 - r;
    if (flags & reflect_cols) c = dst_width() - 1 - c;
    return {r, c};
  }
  constexpr int index(int flat) const {
    assert(width > 0 && flat >= 0);
    auto [r, c] = position(flat / width, flat % width);
    return r * dst_width() + c;
  }
  constexpr std::pair<int, int> delta(int dr, int dc) const {
    if (flags & transpose) std::swap(dr, dc);
    if (flags & reflect_rows) dr = -dr;
    if (flags & reflect_cols) dc = -dc;
    return {dr, dc};
  }
  constexpr GridIsometry inverse() const {
    return {dst_height(), dst_width(), (flags & transpose)
        ? transpose | ((flags & reflect_rows) << 1) | ((flags & reflect_cols) >> 1)
        : flags};
  }
};
#endif
```

## テスト: grid_isometry_test.cpp

```cpp
#include "grid-isometry.hpp"
#include <array>
constexpr bool exhaustive(int h, int w) {
  for (unsigned o = 0; o < 8; ++o) {
    GridIsometry x{h, w, o};
    const auto y = x.inverse();
    for (int r = 0; r < h; ++r) for (int c = 0; c < w; ++c) {
      auto [r2, c2] = x.position(r, c);
      if (r2 < 0 || r2 >= x.dst_height() || c2 < 0 || c2 >= x.dst_width()) return false;
      if (y.position(r2, c2) != std::pair{r,c}) return false;
      if (y.index(x.index(r*w+c)) != r*w+c) return false;
      for (auto [dr,dc] : std::array<std::pair<int,int>,4>{{{-1,0}, {1,0}, {0,-1}, {0,1}}}) {
        if (r+dr < 0 || r+dr >= h || c+dc < 0 || c+dc >= w) continue;
        auto [u, v] = x.position(r+dr,c+dc);
        if (x.delta(dr,dc) != std::pair{u-r2,v-c2}) return false;
      }
    }
  }
  return true;
}
static_assert(exhaustive(2,3) && exhaustive(1,7) && exhaustive(6,4));
int main() {}
```
