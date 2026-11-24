# 提案: 同じ盤面で繰り返す目的地グループへの距離

[GridTargetDistance](../library/grid-target-distance.hpp) は、無向四近傍
グリッドで目的地グループごとに逆向きの多始点 BFS を一度実行します。
例えば、各人から「列のどこか」「いずれかの出口」への最短距離を何度も
問い合わせるとき、各人から BFS し直す必要がありません。
既存の [FlatGridFirstStepBfs](../library/flat-grid-first-step-bfs.hpp) は
経路の一歩目も返します。この部品は距離だけを返し、既存 API は変更しません。

```cpp
const ahc::GridTargetDistance table(height, width, walls, target_groups);
const int steps = table.distance(worker_cell, group_id);  // 到達不能なら -1
```

盤面は行優先、壁マスクの 0 は通行可、0 以外は壁です。目的地は行優先
マス番号の配列の配列で、空グループや重複も使えます。グループ数を G、
マス数を N として構築は O(GN + 目的地指定の総数)、領域は O(GN + N)、
距離照会は O(1)。マス数は最大 65,535。スナップショットは不変なので、
壁や目的地が変わったら、新しい表を構築してください。1 度きりの問い合わせでは、
通常の BFS の方が少ない作業で済むことがあります。

通常は壁上の始点が到達不能です。建設完了を仮定した盤面などで、現在地だけが
仮想的に壁の場合は `BlockedOrigin::MayExit` を明示すると、そこから空きマスへ
一歩出られます。始点自身が目的地なら距離 0 です。どちらのモードも、他の
壁を経由したり、壁の目的地へ入ったりすることはありません。

AHC008 での用途は、柵の設営が終わった人を捕獲担当の通路列へ割り当てる際の
距離の計算です。既存の探索、割当の同点処理、合法性判定とは切り離しています。
このヘッダは汎用提案で、同じ考え方を試した AHC008 専用実装とは別物です。

[短い使用例](../examples/grid_target_distance_usage.cpp) と
[差分テスト](../tests/grid_target_distance_test.cpp) が付属します。
各始点からの愚直な BFS と比較し、壁からの退出、空の目的地、重複、65,535
マスの上限を検証します。

```sh
g++ -std=gnu++23 -Wall -Wextra -Wpedantic -Werror -I library \
    tests/grid_target_distance_test.cpp -o /tmp/grid-target-distance-test
/tmp/grid-target-distance-test
```

AHC008 の固定作業量比較では、公式配布100ケースで専用試作の行動と得点が
全件一致しました。ソルバー CPU 時間は全体で 2.2% 短く、終盤の比較探索が
走らなかった87件では 6.1% 短くなりました。探索が走った13件では 0.3% の差に
とどまり、通常時計の採用版には入れていません。この数値は汎用ヘッダ自体の
ベンチマーク値ではありません。汎用ヘッダは UBSan を有効にして 380,880 件の
距離照会が独立 BFS と一致し、使用例は `2 1` を出力しました。
