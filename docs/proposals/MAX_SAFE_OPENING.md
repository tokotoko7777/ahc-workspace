# 静的な壁の安全な間引き
`library/max-safe-opening.hpp` の `ahc::max_safe_opening` は、無向グラフで開けてよい壁の部分集合を選び、危険点に到達せず始点から届く領域を最大化する。壁を開ける数に上限やコストがなく、危険点・始点・隣接関係は固定の場合に限る。

```cpp
auto result = ahc::max_safe_opening(root, walls, removable, hazards, neighbors);
if (result.feasible) {
    walls &= ~result.opened;
    auto safe_cells = result.reachable.count();
}
```

元の盤面で危険点から届く成分を列挙し、その境界の壁を保持する。境界の壁が始点から到達可能なら、元の成分を通って危険点にも到達できる。他の候補壁を全部開けることはこの境界を壊さないので、安全な任意の部分集合が達成する到達領域を含む。返値の `opened` は到達領域内に限るため、到達結果に影響しない壁は除かない。時間は O(V+E)、追加メモリは O(V)。

AHC008 では最終配置における静的な余地の診断に対応する。序盤に同じ柵を置かなければ動物の移動経路や以降の合法手が変わるので、実際の改善や実行可能な戦略を保証しない。

`g++ -std=gnu++23 tests/max-safe-opening-test.cpp -o /tmp/test && /tmp/test`。ランダム盤面で全ての候補部分集合を試し、到達領域が一致することを確認する。
