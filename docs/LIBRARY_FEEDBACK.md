# AHC ライブラリ不便メモ

過去問で実際に使ったときの不便を記録する。同じ課題が別の問題でも出た場合は、行を増やさず問題番号を追記する。推測だけの課題はここには載せない。

| 問題番号 | 使った部品 | やりたかったこと・困った点 | 実際の回避策 | 改善案・対応状況 |
|---|---|---|---|---|
| AHC001 | `axis-aligned-rectangle.hpp` | 既存の状態は軸・方向で引く `q[4]`。名前付き座標の型へ置換するか、近傍探索中に繰り返し変換が必要だった | 座標アクセス関数を渡し、既存状態をコピー・変換せず接続 | 既存部品は変更せず、汎用の座標取得インターフェースにした |
| AHC001 | 未提供 → [`coordinate-face-slide.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/fe7273b40cc8cc8671283ace5d30b6aaf6ceec1a/library/coordinate-face-slide.hpp) | 接触する複数の辺を一緒に移動させる成分作成・合法な座標区間を既存部品だけでは表せなかった | 非重複を壊さない区間を先に計算し、二乗の成分構築は終盤の一括処理に限定 | 問題別ブランチに利用例と愚直比較 800 レイアウトのテストを追加。共有境界の近傍自体は未調整集合で伸びず、既定解には未採用 |
| AHC001 | [`max-flow.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/54f9862b1def3e9129385a3acf84a6693d409a12/library/max-flow.hpp) | 二値の面移動にかかる単項・二項の期待損失を、最小カットの容量と定数項に自力で変換する必要があった | [`binary-submodular-energy.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/54f9862b1def3e9129385a3acf84a6693d409a12/library/binary-submodular-energy.hpp) を追加。直接 include と新部品経由を併用できるよう `max-flow.hpp` に `#pragma once` | 1,400 件の全割当との突き合わせを追加。AHC001 では実験対象。 |
| AHC002 | グリッド用の訪問管理 | 同一タイルの ID が複数マスに現れ、頂点を 1 回だけ訪問する制約では足りなかった | タイル ID をコールバックで取得し、経路をコピーせず使用済み色を更新する [bounded-colored-path.hpp](https://github.com/tokotoko7777/ahc-workspace/blob/c09a5d30611f6991d5cb3761b5a5038f5eb0316d/library/bounded-colored-path.hpp) を追加 | [使用例](https://github.com/tokotoko7777/ahc-workspace/blob/c09a5d30611f6991d5cb3761b5a5038f5eb0316d/examples/bounded_colored_path_usage.cpp) と、ランダム 1,500 グラフの愚直全列挙比較。問題の既定解は未変更。 |
| AHC003 | ガウス回帰の観測評価（既存の汎用部品なし） | 候補経路内の複数辺が同じ推定変数を使い、観測の効果を経路ごとに素朴に作ると密行列のコピーや処理の重複が発生する | 経路の変数 ID を集約し、既存の行優先共分散を span で渡す [gaussian-linear-information.hpp](https://github.com/tokotoko7777/ahc-workspace/blob/0d42d1839a9ff4a1f35b2830ea4b2e2eeba682c4/library/gaussian-linear-information.hpp) を追加 | [使用例](https://github.com/tokotoko7777/ahc-workspace/blob/0d42d1839a9ff4a1f35b2830ea4b2e2eeba682c4/examples/gaussian_linear_information_usage.cpp)、乱択 1,000 件の密行列版比較。問題の既定解は変更しない。 |
| AHC004 | WeightedDsu | 相対オフセットは持てても、周期上で文字が衝突する結合を併合前に拒否する API がなかった | [circular-pattern-components.hpp](https://github.com/tokotoko7777/ahc-workspace/blob/74c0189dc1bea65370a25ee45a0b74e5b8ddd591/library/circular-pattern-components.hpp) で周期・文字の衝突判定と DSU を分離。既存 WeightedDsu に多重 include のガード | [例](https://github.com/tokotoko7777/ahc-workspace/blob/74c0189dc1bea65370a25ee45a0b74e5b8ddd591/docs/LIBRARY_REFERENCE.md#周期付き部分パターンの併合) と [ランダム 700 件比較](https://github.com/tokotoko7777/ahc-workspace/blob/74c0189dc1bea65370a25ee45a0b74e5b8ddd591/tests/circular_pattern_components_test.cpp) を追加。AHC004 問題別ブランチに保存。 |

速度差で自前実装に戻した場合は条件と計測値を、変換が必要だった場合は入力・出力の型や契約を記録する。汎用化した項目には使用例とテストへのリンクを添える。
