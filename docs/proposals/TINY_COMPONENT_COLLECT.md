# 小さい連結成分の列挙

`bounded_component_probe.hpp` は「指定した頂点集合に当たるか」を
早く判定する用途を想定している。見つかった時点で探索を止めるので、
「成分全体が4頂点以下で、Aの点は含むがBの点は含まない」
という条件にはそのまま使えない。

`library/tiny-component-collect.hpp` は `Limit` を超えたか、それ以下で
列挙できたかを区別する。小さな配列だけで重複を調べ、頂点数に比例する
訪問済み領域は確保・初期化しない。探索コストは
`O(Limit² × 最大次数)`、作業領域は `O(Limit)`。大きな上限なら通常のBFSが適する。

```cpp
auto group = ahc::collect_tiny_component<900, 4>(
    seed, [&](int v) -> const auto& { return neighbors[v]; },
    [&](int v) { return !walls[v] && v != proposed_wall; });
if (group.status == ahc::TinyComponentStatus::complete) {
    bool has_animal = false, has_person = false;
    for (auto v : group.complete_vertices()) {
        has_animal |= animal[v];
        has_person |= person[v];
    }
    // 小ささと人物・動物の条件を確認した後、盤面全体を別途検証。
}
```

AHC008の適用先は、既存の割り当て後に「合法な柵1枚で4マス以下に
新しい捕獲ができるか」を調べる試作。全探索を呼ぶ前に、柵の隣だけを
この部品で調べる。人の連結や共有地の面積、他の動物の閉じ込めは
従来どおり盤面全体で確認する。この部品自体は採点や合法性を保証しない。

`tests/tiny_component_collect_test.cpp` は有向ランダムグラフと普通のBFSを
比較し、閉じた始点、無効な隣接先、重複、上限ちょうどと超過を確認する。

```sh
g++ -std=gnu++23 tests/tiny_component_collect_test.cpp -o /tmp/tiny_component_test
/tmp/tiny_component_test
```

試作の置き換え比較: 公式配布と別生成を各100ケース、同じ実行ファイルで計測。
手書き版と候補数・成立数・全操作・得点が全件一致した。最終出力までの
ケース別時間差の中央値は配布 +3.46 ms、別生成 +0.61 ms。速くなったとはいえず、
競技用の既定版には入れていない。上記の愚直比較は672,000件。
