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


## 経路が不要なら、距離制限付きの複数始点 BFS

「どれかの始点から D 歩以内か」だけ必要なら、始点ごとの距離表は作らない。すべての始点を深さ 0 に入れ、重複を除き、D 層まで展開する。得られるのは個別の BFS の到達集合の和で、始点別の距離や経路は復元できない。1 点から距離 2 だけ必要なら、通行可能な隣接マスとその隣接マスだけを列挙し、重複を除けばよい。後続のグラフに辺を足す順序まで維持する場合は、結果を従来順に列挙する。

- ランダムな格子 12,800 枚で、深さ制限付きの複数始点探索と距離 2 専用探索を、それぞれ個別の全距離探索と比較。計 25,600 件の到達集合が完全一致。
- AHC008 では二つを合わせた版を比較。配布 100 件・固定 24 組の探索で操作、得点、探索の評価がすべて一致。終盤探索が動いた 16 件では最終出力までの時間が対ごとの中央値で 6.97% 短縮。
- 同じ 2.4 秒の内部打ち切りで、完了した探索が配布 100 件で 1,188→1,273 組、新規生成 100 件で 1,333→1,440 組。各入力の操作と得点は変わっていない。ローカル比較であり、大会の計測や得点更新ではない。


## 同じ通行条件で複数始点を探索するなら、通れる隣接先を共有

前節の「どれか一つから届く」用途とは異なり、各始点の距離表が必要な場合。盤面の通行条件を変えずに移動シミュレータと BFS を繰り返すなら、移動側が既に持つ順序付き隣接表を BFS にも渡せる。盤面上の全点を毎回探索する必要はないので、始点ごとの距離表は従来どおり必要になった時点で作る。壁が変わったときは隣接表を更新し、その世代の古い距離表は使わない。従来の隣接順と無効始点の扱いは変えない。

```cpp
// passable_neighbors[v]: ordered valid edges for this board version.
// The shared view is read-only while producing or using any distance row.
for (int start : requested_starts) {
    workspace.run(passable_neighbors, start, [](int) { return true; });
    for (int v : probes)
        read_distance(start, v, workspace.distance(v));
}
```

現在の AHC008 では、キャッシュ済みの固定次数テーブルを専用の小さなキューにそのまま渡した。既存の BFS に同じ近傍ビューを渡せる用途でも、ワークスペースの初期化や距離格納コストは別なので同じ高速化率とは限らない。通行可能な辺をテーブルにする費用を他処理が払っていない場合や、探索がほとんど呼ばれない場合は、マスクをその場で見る版も比較する。

- 壁なし・壁ありの格子で独立のキュー BFS と比較し、12,800 始点のすべての距離が一致。距離表を照合するだけでは乱数の変化を検出できないため、公式採点器と独立再生で全 300 ターンの操作も確認。
- 固定 24 組の配布 100 件、別の公式生成 100 件ともに全操作・全得点・探索回数が一致。最終出力までの合計はそれぞれ 29.218→28.347 秒、36.406→35.417 秒。単独・交互実行のローカル値で、新しい得点記録ではない。
