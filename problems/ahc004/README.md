# AHC004: Alien's Genome Assembly

[公式問題文](https://atcoder.jp/contests/ahc004/tasks/ahc004_a) /
[最終順位表](https://atcoder.jp/contests/ahc004/standings) /
[順位・点数のアーカイブ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc004.zip)。

本番1位は **7,321,305,762**（上記アーカイブの `ahc004//standings_scores.csv`。
アーカイブ自体のSHA-256は `2c43b467ddfabfa774306c350f397a272f1b4ae6023066a0c44ea93fe62a42c8`）。
公式の丸め済み絶対得点を100回合計して最大化します。各回3秒・1,024 MiBで、
ケースにAC以外があれば提出全体0点です。本番後のシステムテストはなく、配布の
ローカル用 seed 0–99 は本番と異なると明記されています。本番ケースを確認できず、
ローカルで1位相当の値を超えても **1位超えは未判定** とします。

### 候補

- 既存の [独立実装](../../examples/search/ahc004_genome_sa.cpp) を基準として維持。
  部分文字列を盤面に直接置いてから、焼きなましで改善する。
- `AHC_CONTIG_SEED` は、盤面に配置する前に高一致の末尾・先頭の重なりを兼容する順に
  連結し、周期付きの部分パターン候補を作る。各パターンを折り返しと文字の
  一致条件を守って配置し、残りの語は既存の処理で補う。通常の初期解候補も残し、
  初期構築150 ms・探索末尾2,780 msの制限は両者共通。
- 汎用化: 既存の `WeightedDsu` で相対位置は持てるが、合体前に文字の衝突を
  調べるための共通APIがなかったため、[周期付き部分パターンの併合](../../library/circular-pattern-components.hpp)
  を追加した。通常の整数オフセットは内部でそのまま使い、周期を法とする
  判定と文字データの更新をラッパーに分離した。
  [使用例](../../docs/LIBRARY_REFERENCE.md#周期付き部分パターンの併合)、
  [テスト](../../tests/circular_pattern_components_test.cpp)。

### 再現

公式問題文のリンクから入力の zip とツールを取得してください。ツール側で
`cargo build --release --locked` を実行し、下の `TOOLS_DIR` を指定します。
入力と Rust のビルド結果は Git には置きません。

```sh
g++ -std=c++23 -O3 examples/search/ahc004_genome_sa.cpp -o baseline
python3 scripts/bundle.py problems/ahc004/main.cpp -o contig.single.cpp
g++ -std=c++23 -O3 contig.single.cpp -o contig
g++ -std=c++23 -O2 tests/circular_pattern_components_test.cpp -o components_test
./components_test
python3 problems/ahc004/benchmark.py INPUTS_DIR dev --start 0 --end 10 --vis TOOLS_DIR/target/release/vis --entry baseline=baseline --entry contig=contig
python3 problems/ahc004/benchmark.py INPUTS_DIR holdout --start 60 --end 70 --vis TOOLS_DIR/target/release/vis --entry baseline=baseline --entry contig=contig
```

開発10件、候補設計に使わない60–69の未調整10件は実験前に固定しました。
それぞれ同一ケースで旧・新を交互に実行し、公式採点と独立スコア計算で照合します。

| 対象 | 既存の平均 | 合成候補の平均 | 差 | 候補勝ち |
|---|---:|---:|---:|---:|
| ローカル開発 seed 0–9 | 68,623,544.8 | 70,809,483.0 | +2,185,938.2 | 8/10 |
| ローカル未調整 seed 60–69 | 66,275,335.0 | 68,122,402.3 | +1,847,067.3 | 5/10 |

40出力すべて合法で公式採点と独立検査が一致、最大実測2.786604秒。
この作業ブランチでは平均の良かった候補を `main.cpp` の既定にしましたが、
試行数は少なく、未調整では5敗もあります。`-DAHC_BASELINE_SEED` または
元の例で既存方式に戻せます。本番の1位超えは未判定のままです。

次の大きい仮説は、各行を循環シフト・行順変更して横方向の出現を壊さず、
縦方向の語をまとめて増やす近傍です。列側も交互に行い、単語上書きのみの
探索で固定されやすい全行の位相を変えます。対向する全ラインの差分評価が必要な
ため、同一初期盤面・同じ時間で評価し、有効なケースとコストを分けて記録します。
