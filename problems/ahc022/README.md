# AHC022: Exploring Another Space

- [公式問題文](https://atcoder.jp/contests/ahc022/tasks/ahc022_a)・[公式順位表](https://atcoder.jp/contests/ahc022/standings)・[公式対話テスタ等](https://img.atcoder.jp/ahc022/u4u4grds7n.zip)
- 絶対点は `ceil(10^14 × 0.8^誤答数 / (配置費+計測費+10^5))`。ケース相対点は `round(10^9 × 自分の絶対点 / 順位対象中の最高絶対点)`、本番3000ケースを合計。4秒、1024MiB、計測は最大10,000回。
- 最終1位の相対点は**未確認**。公式順位表は未ログインで開けず、[ALE-Bench](https://huggingface.co/datasets/SakanaAI/ALE-Bench/tree/main)に `ahc022.zip` がない。[本番システムseed](https://img.atcoder.jp/ahc022/seeds.txt)のSHA-256は `2154215b63e063812cf2bb0ee2370c5193ae4682a3f7ce260e82a52a07663206`。本番seed・相対基準値を使っていないので **未判定**。

## 解法

対照は出口セルに異なる温度を割り当て、同じ位置を複数回観測する。比較版は温度2値の格子を作り、出口からマンハッタン距離3以内の複数オフセットを符号語として使う。候補配置から符号語間のハミング距離が小さすぎないものを選び、未観測セルは周囲から平滑化する。0と1000の境界値を推定時に特別扱いするのではなく、丸められ0〜1000にクリップされた正規観測の尤度を使う。

両方とも累積負対数尤度を [既存Hungarian](../../library/hungarian.hpp) に渡して一対一対応を求める。比較版は最初に濃度に応じた観測を一通り行い、その後は対応候補の尤度差が小さい入口だけを追加観測する。オフセットは、尤もらしい別候補との温度差・移動費・既観測回数で選ぶ。未来のノイズ列・真の対応は見ない。3,620msに達すれば追加観測を打ち切る。

通常の正規密度は0と1000に集約される裾確率を扱わない。このため [丸め・クリップ付き正規分布](../../library/rounded-clipped-normal.hpp) を追加し、[正規化・対称性・位置移動テスト](../../tests/rounded_clipped_normal_test.cpp)を作った。

## 結果

| 分割 | 件数 | 対照の平均誤答 | 符号版の平均誤答 | 符号版の勝 / 負 | 得点差の中央値 |
| --- | ---: | ---: | ---: | ---: | ---: |
| dev | 30 | 49.80 | 0 | 30 / 0 | +612,346 |
| holdout | 30 | 53.23 | 0 | 29 / 1 | +379,703 |

[計測CSV](benchmark-results.csv)の120出力すべてで合法性と公式テスタ・独立整数採点の一致を確認。最大0.092736秒、符号版最大0.059609秒・5,477計測。holdoutの絶対点単純平均は11,857,280→9,850,827で、S=1の一件の大きな悪化に影響された（349,064,508→100,910,822）。一方で全Sのうち29件で改善したため、問題ブランチの既定は符号版のまま。相対得点を実測したわけではない。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc022/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC022_STENCIL=0 "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 "$work/submission.cpp" -o "$work/stencil"
python problems/ahc022/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" "$work/base" "$work/stencil" "$work/results.csv"
g++ -std=c++23 -O3 tests/rounded_clipped_normal_test.cpp -o "$work/test"
"$work/test"
```

両分割は独立のSHA-256系列で、許されたSの30水準に1件ずつ公式生成。両版の設定を事前固定して測定した。真の対応とノイズはテスタへの入力にのみ含め、別処理で配置費・計測条件・対応・整数スコアを再計算した。

### 次の仮説

符号版は誤答0でも、特に中・高Sで配置費が測定費を大きく上回る。配置選択で符号距離だけでなく実テンション費を目的に入れ、固定するセルを減らして残りを平滑化する。低い配置費を追加観測費で補えるか、同じ配置とノイズ上で尤度差・誤答率・両費用を分けて計測する。
