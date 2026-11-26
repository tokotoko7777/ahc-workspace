# 最大流ワーク領域を頂点数ごとに再利用する

## 使いどころ

同じ頂点数で辺・容量だけが頻繁に変わるなら、`clear_edges()` を持つ版の `MaxFlow` は残余グラフを新規構築する代わりに領域を再利用できる。頂点数が少数種類なら、種類ごとに `std::optional<MaxFlow<int>>` を1個持つ。新規APIは不要。`clear_edges()` がない版のヘッダを使う場合はこの例をそのまま使わない。

```cpp
std::array<std::optional<MaxFlow<int>>, 8> work;
// 例: 頂点数が 30..37 の範囲で変わる系列を処理。
for (const auto& instance : cases) {
    const int n = instance.vertex_count();
    auto& slot = work.at(n - 30);
    if (slot) slot->clear_edges();
    else slot.emplace(n);
    for (const auto& [u, v, capacity] : instance.edges())
        slot->add_edge(u, v, capacity);
    const auto flow = slot->flow(instance.source(), instance.sink());
    consume(flow, slot->min_cut(instance.source()));
}
```

古い辺ID・流量・残余容量は失効する。次のグラフの辺を元と同じ順に追加すれば、同点順を変えずに使える。保持領域は各種類で到達した容量の合計になる。サイズが多い場合は1個だけ保持するか、上限を設ける。同じ領域を複数の並行した処理で共有しない。

## AHC008での確認

- 公式配布100件を終盤最大24組に揃え、全件で操作列、採点、仮想評価が一致。実際に終盤比較が走った16件のソルバCPU合計は約11.9%減、対ごとの中央値は約11.6%減。既存の `clear_edges()` を呼び、辺の生成順は変えていない。
- 通常の時間打ち切りでは探索した組数が1,058→1,245に増えたが、点数は6,201,888,887→6,200,777,776。全頭捕獲・全員共有・全頭4マス以下の件数は同じ。速度だけの結果と、同じ時間枠で方針が変わる結果を分ける。競技解答への採用や他の問題での同じ高速化率は保証しない。
