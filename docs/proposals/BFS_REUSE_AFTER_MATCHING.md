# 距離を使う割当から経路復元まで、1 回の BFS で済ませる

AHC008 終盤の追い込みでは、全員から候補の壁までの距離で担当を決めたあと、採用された行き先への経路を同じ盤面でもう一度 BFS していた。割当用の BFS で親も記録しておけば、割当後の復元は経路長に比例し、盤面全体を再探索しなくてよい。

## 使い方

距離だけでなく親も返す既存部品 [`MaskedBfsWorkspace`](https://github.com/tokotoko7777/ahc-workspace/blob/cc2c7b4e86abc712f7ce27bf8a992175adb45ae1/library/masked-bfs-workspace.hpp) を再利用できる。格子で隣接辺を動的に列挙せず、盤面に依存しない近傍順を一度作り、壁だけ述語で切り替える。等距離時の挙動も再現したいなら、従来の探索と同じ辺順（AHC008 では上・右・下・左）にする。無効な `-1` は近傍列から除く。

```cpp
// neighbors[v] returns only valid IDs, in the same order as the old BFS.
std::vector<MaskedBfsWorkspace> tree;
tree.reserve(starts.size());
for (int start : starts) {
    tree.emplace_back(vertex_count);
    tree.back().run(neighbors, start, [&](int v) { return !wall[v]; });
}
// Fill the matching costs using tree[i].distance(candidate_cell).
// After matching, goal[i] is either a reachable target or -1.
for (int i = 0; i < static_cast<int>(tree.size()); ++i) {
    const int target = goal[i];
    if (target < 0 || tree[i].distance(target) < 0) continue;
    for (int v = target; v != starts[i]; v = tree[i].parent(v))
        path[i].push_back(v);
    std::reverse(path[i].begin(), path[i].end());  // excludes starts[i]
}
```

担当決定前に同じワークスペースを上書きすると親を失うので、始点ごとに保持する。盤面または辺順が変わったら当該探索をやり直す。先頭の移動だけが必要な場合は [`flat-grid-first-step-bfs.hpp`](../../library/flat-grid-first-step-bfs.hpp) で足りる。

## 検証と提案

- 格子アダプターで通行マスクを変える 3,000 回の BFS を独立なキュー版と比較。距離 663,000 点、親を辿る各辺、到達不能、同距離での順序を照合した。
- AHC008 の実験では座標ごとの親を既存距離計算と同時に作り、割当の直後に使った。固定探索 24 組において、親の再利用と [最大流領域の再利用](MAX_FLOW_WORKSPACE_REUSE.md) を合わせた版は公式配布 100 件の行動と得点が一致し、終盤探索が動いた 16 件の最終出力までの合計は 19.12 秒から 16.84 秒へ短縮。**合わせた変更の値であり、この汎用アダプター単体の測定ではない。**
- 同じ用途が増えたら、近傍を返すコールバックを直接受ける入口を既存の `MaskedBfsWorkspace` に足すと、盤面ごとのアダプターを手書きしなくてよい。到達不能と元の同距離順は変えない。新しい BFS クラスを重複して追加する必要はない。
