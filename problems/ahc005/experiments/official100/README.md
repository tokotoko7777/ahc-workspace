# AHC005 公式配布100件の基準

- 公式配布 seed 0–99、既存 path-aware 研究枝の通常ビルド（実験マクロなし）。
- **22,923,257点**。目標22,318,770点を604,487点上回った。配布入力は実際の本番入力とは異なり、順位比較ではない。
- 完全可視・帰着100件、公式採点と独立採点が全件一致、失敗0。直列最大2.709380秒、中央値2.638273秒。SA総試行445,565,440、ケース中央値4,448,000。
- [出力のケース別計測](baseline-results.csv)と[測定した単一ファイル](submission.single.cpp)。元コードの公開版は `a6b60ce8c6779c638c162ef63c65a4d484dc6dad`。
- 単一ソース SHA-256: `741db1ab81b9675833afd454bba0eeeab7183685e6f26998bbf5d10eb45a45c0`。
- CSV SHA-256: `ca1ef542a2b3850ac18c3c934077232e175bf896c0e9fe558776df9243b01115`。

[公式問題文](https://atcoder.jp/contests/ahc005/tasks/ahc005_a)から配布 [入力](https://img.atcoder.jp/ahc005/c746dac8cc11fd18c68063546997666e.zip) と [ツール](https://img.atcoder.jp/ahc005/dc9ed10f037e2dd4b48ca255dbd470d9.zip) を取得。公式ツールを展開して `cargo build --release --locked`。リポジトリのルートで `IN` に入力フォルダ、`VIS` に公式ツールの実行ファイルを設定して実行。壁時計で計算を切るため細かな得点差は再計測で変動する。

```sh
g++ -std=c++23 problems/ahc005/experiments/official100/submission.single.cpp -o solver
python3 problems/ahc005/benchmark.py "$IN" out --start 0 --end 100 --vis "$VIS" --entry "baseline=$PWD/solver"
```
