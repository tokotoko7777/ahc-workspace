# 探索ライブラリを実問題に使う例

ヘッダと使用例を分けた、21本のC++コードです。
入力、状態、近傍または遷移、評価、最良解の保存、出力まで含みます。
`Problem`、`State`、`Move` / `Action` の書き方や、探索Runnerへの接続を読むために使えます。

汎用の探索処理は [`library/`](../../library/)、問題固有の処理は各 `.cpp` にあります。
探索手法の選び方は [SEARCH_GUIDE.md](../../SEARCH_GUIDE.md)、
部品ごとの短い使用例は [USAGE.md](../../USAGE.md) を参照してください。

## 使い方と1ファイルでの提出

1. 使いたい `.cpp` を開き、問題固有の状態・評価・更新の書き方を確認する。
2. 新しい問題へ使う場合は、必要な部分を自分の `main.cpp` へコピーして書き換える。
3. 提出時は `#include "../../library/○○.hpp"` などのローカルinclude行を、該当ヘッダの全文に置き換える。
4. 必要なヘッダをすべて貼った `main.cpp` だけをコンパイル・実行し、その1ファイルを提出する。

例えば `intro_heuristics_action_beam.cpp` なら、次の1行を
[`action-beam-search.hpp`](../../library/action-beam-search.hpp) の全文へ置き換えます。

```cpp
#include "../../library/action-beam-search.hpp"
```

`#include <iostream>` などの標準ライブラリのincludeはそのまま残します。
複数のヘッダを使う例は元のinclude順に貼り、同じヘッダを二重に貼らないでください。
ヘッダ内の参考リンクと、著作権・許諾表示も保持します。

リポジトリ上で元の使用例を動かす場合は、以下のようにコンパイルできます。
相対includeは各 `.cpp` から `library/` を参照します。コマンドはリポジトリのルートで実行します。

```sh
g++ -O2 -std=c++17 examples/search/intro_heuristics_action_beam.cpp -o /tmp/search_example
/tmp/search_example < input.txt > output.txt
```

提出用にヘッダを貼り終えた後は、そのファイルだけで確認します。

```sh
g++ -O2 -std=c++17 main.cpp -o /tmp/submit_check
/tmp/submit_check < input.txt > output.txt
```

`input.txt` には、その例が対象とする問題の入力を用意してください。
AHC003の例は対話形式なので、対応するローカルテスタで実行します。

## ビームサーチ

| ファイル | 使う探索 | 題材 | 読むポイント |
|---|---|---|---|
| [`intro_heuristics_simple_beam.cpp`](intro_heuristics_simple_beam.cpp) | `SimpleBeamSearch` | Introduction to Heuristics Contest A | 日程を先頭から構築。子状態の生成、評価、履歴の保存 |
| [`intro_heuristics_action_beam.cpp`](intro_heuristics_action_beam.cpp) | `ActionBeamRunner` | Introduction to Heuristics Contest A | `State`・`Action`・`Score`と3つの処理を1つの`Problem`へまとめる |
| [`ahc011_tree_beam.cpp`](ahc011_tree_beam.cpp) | `TreeBeamRunner` | AHC011 | 盤面をapply/revertし、差分hashと同一局面除去を使う。候補から手順を復元 |
| [`ahc021_tree_beam.cpp`](ahc021_tree_beam.cpp) | `TreeBeamRunner` + `RadixHeap` | AHC021 | 最短路で球を順に確定。候補生成と完成解の評価 |
| [`ahc032_action_beam.cpp`](ahc032_action_beam.cpp) | `ActionBeamRunner` | AHC032 | 軽いActionで候補を絞り、採用候補だけ盤面をコピー |
| [`ahc032_chokudai.cpp`](ahc032_chokudai.cpp) | `ChokudaiSearch` | AHC032 | 層別キューを巡回。容量・時間制限と、あらかじめ用意した完成解 |
| [`variable_cost_beam.cpp`](variable_cost_beam.cpp) | `CostTreeBeamRunner` | 締切付き宝集め | 1・2・3世代進む行動と再訪。自己テスト用の厳密DPも収録 |
| [`ahc038_variable_cost_beam.cpp`](ahc038_variable_cost_beam.cpp) | `CostTreeBeamRunner` | AHC038 | 次の把持・解放までを可変長の1行動にまとめる。候補・更新・評価・進行量・keyの実装 |
| [`ahc071_action_beam.cpp`](ahc071_action_beam.cpp) | `ActionBeamRunner` | AHC071 | 行ごとの構築と区間再構築を同じProblemで扱い、次段の条件をkeyでまとめる |

## 局所探索（焼きなまし・山登り・部分再構築）

| ファイル | 使う探索 | 題材 | 読むポイント |
|---|---|---|---|
| [`ahc001_region_sa.cpp`](ahc001_region_sa.cpp) | `TimeBasedAnnealingRunner` | AHC001 | 領域の移動・再構築、閾値による評価の打ち切り、仮変更buffer |
| [`ahc002_destroy_repair_sa.cpp`](ahc002_destroy_repair_sa.cpp) | `TimeBasedAnnealingRunner` | AHC002 | 経路の末尾再構築と区間DFS修復。採用時の更新と最良解からの再開 |
| [`ahc002_destroy_repair_lns.cpp`](ahc002_destroy_repair_lns.cpp) | `LargeNeighborhoodSearch` | AHC002 | 経路の変更をdestroy/repairへ分け、採用前のcache再構築を省く |
| [`ahc003_online_fit.cpp`](ahc003_online_fit.cpp) | `TimeBasedAnnealingRunner`（山登り） | AHC003 | 観測履歴から未知の辺コストを推定。関係する観測の予測値を差分更新 |
| [`ahc004_genome_sa.cpp`](ahc004_genome_sa.cpp) | `TimeBasedAnnealingRunner` + `AhoCorasick` | AHC004 | 変更した行・列だけ文字列の出現を再計算し、採用時にcacheを反映 |
| [`ahc005_patrol_sa.cpp`](ahc005_patrol_sa.cpp) | `TimeBasedAnnealingRunner` | AHC005 | 道路の代表点と巡回順を探索。移動料金の扱いと境界辺の差分評価 |
| [`ahc006_sa.cpp`](ahc006_sa.cpp) | `TimeBasedAnnealingRunner` | AHC006 | 経路の辺差分、逆引き位置cache、2点挿入、距離の前計算 |
| [`ahc058_prefix_sa.cpp`](ahc058_prefix_sa.cpp) | `TimeBasedAnnealingRunner` + `PrefixReplay` | AHC058 | 先読みで作った購入順序を改善。共通する前半の再利用と待機の一括更新 |
| [`ahc059_ils.cpp`](ahc059_ils.cpp) | `IteratedLocalSearch` | AHC059 | 大きい摂動と小さい局所改善を分離。探索の打ち切りと受理方式 |
| [`ahc059_lns.cpp`](ahc059_lns.cpp) | `LargeNeighborhoodSearch` + `AdaptiveOperatorSelector` | AHC059 | ペアの削除・再挿入。焼きなまし・RRT・山登りの受理方式と近傍選択 |

## 決定的な先読み

| ファイル | 使う部品 | 題材 | 読むポイント |
|---|---|---|---|
| [`ahc026_deterministic_rollout.cpp`](ahc026_deterministic_rollout.cpp) | `DeterministicRolloutRunner` | AHC026 | 各候補の先を貪欲で仮実行し、消費energyが最小の手を選ぶ |
| [`ahc058_deterministic_rollout.cpp`](ahc058_deterministic_rollout.cpp) | `DeterministicRolloutRunner` | AHC058 | 合法手・投資・生産をProblemにまとめ、先読みして最初の1手だけを適用 |

## 読む順番

初めてビームサーチを組むなら、`intro_heuristics_simple_beam.cpp` から始め、
次に `intro_heuristics_action_beam.cpp` で `Problem` に処理をまとめる形を読むと追いやすいです。
焼きなましなら `ahc006_sa.cpp` で近傍・差分評価・採用後の更新の対応を確認できます。
時間制限、ビーム幅、温度などは各コードの定数・マクロで指定しています。

[リポジトリのREADMEへ戻る](../../README.md)
