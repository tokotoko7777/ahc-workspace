# AHC012: AtCoder 10th Anniversary

- [公式問題文](https://atcoder.jp/contests/ahc012/tasks/ahc012_a) / [公式順位表](https://atcoder.jp/contests/ahc012/standings) / [公式ジェネレータ・採点器](https://img.atcoder.jp/ahc012/f756367b32.zip)
- 本番最終1位 **99,781,619**。[ALE-Bench の AtCoder 提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc012.zip) 内 `ahc012//standings_scores.csv`。SHA-256 `bf8c7de88433761a17cf37753cd730bae84355196fb8c25455168ad01eab368c` が公開 LFS ポインタと一致。順位と得点のみ参照。
- `round(10^6 * sum_d min(a[d], b[d]) / sum_d a[d])` を本番100件で合計し最大化する。理論最大100,000,000なので1位は満点ではない。カット100本以内、3秒、1024 MiB。コンテスト後のシステムテストはない。配布ローカルケースが本番100件と同一である証拠はなく、本番比較は **未判定**。

## 2方向と3方向の格子

既存解がなかったため、カットの位置を少しずつ動かすSAを実装した。対照は垂直・水平の2方向の平行線群。比較版は後半3回のリスタートに斜め方向の平行線群 `x+y=const.` を追加し、軸方向の本数を減らす。前半3回は同じ2方向で、両者とも壁時計2,710ms以内、最大6回のリスタート。3方向で三角形など異なる形を作れば、同じ格子に不足サイズを増やせるかを試す。

カットの投影値は間に整数点がないようにする。たとえば垂直成分の境界は、ケーキ外の整数点 `(t,-B)` と `(t+1,B)`（`B=1,000,000`）で表す。ケーキ内では `t < x < t+1` を通るので、整数点に対する分類が完全な垂直線と同じになり、イチゴの廃棄がない。斜め・水平も同様。更新時は各方向で点をソートしておき、移動した境界をまたぐ点だけ区画を変更する。

既存の要素被覆カウンタは「何個の点が入った区画がいくつあるか」の上限付き合計を直接持たない。区画サイズの変更を [CappedSizeHistogram](../../library/capped-size-histogram.hpp) に渡し、上限をまたいだときだけ目的値を更新した。[使用例](../../docs/LIBRARY_REFERENCE.md#サイズごとの上限付きヒストグラム)・[100,000回の変更／取消テスト](../../tests/capped_size_histogram_test.cpp)。[境界移動の差分テスト](../../tests/ahc012_grid_moves_test.cpp)では区画の全再分類も行う。

## 結果

| 分割 | 件数 | 2方向版の平均 | 斜め候補込みの平均 | 斜め候補版の勝 / 分 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 8 | 988,686.50 | 986,318.875 | 2 / 2 / 4 |
| holdout | 8 | 972,456.375 | 968,873.75 | 3 / 0 / 5 |

全32出力で公式vis・独立採点・内部の配布数が一致し、最大壁時計は2.730955秒だった。斜めありの出力が比較版で選ばれたのは16件のうち1件。両分割で平均が悪化したため、既定は2方向版。2方向版はdevの1件で理論上限1,000,000点に到達したが、本番1位との差は別の入力を使ったため厳密には判定していない。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc012/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC012_STATS -DAHC012_DIAGONAL=0 "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 -DAHC012_STATS -DAHC012_DIAGONAL=1 "$work/submission.cpp" -o "$work/portfolio"
python problems/ahc012/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/base" "$work/portfolio" "$work/results.csv"
for test in capped_size_histogram ahc012_grid_moves; do
  g++ -std=c++23 -O3 "tests/${test}_test.cpp" -o "$work/test"
  "$work/test"
done
```

devとholdoutの各8件は別々のSHA-256系列から公式ジェネレータで独立に生成し、両版の設定は計測前に固定。実行順を対ごとに交互とし、公式visと独立な整数外積／半平面符号列の集計が一致することを確認した。全ケースの詳細は [計測CSV](benchmark-results.csv)。

### 次の仮説

高得点の2方向格子から、余ったカット枠で欠損サイズをねらう直線を一本ずつ足す。現状の区画のうち過剰供給または10個を超える区画を選び、欠損サイズの点だけを分離できる方向と切断位置を候補化する。全盤面での副作用は整数符号列で厳密に集計し、一区画の改善をほかの区画の損失が上回らない場合だけ適用する。満点に届かないケースで、どのサイズの欠損をどの過剰区画から補えたかを記録する。

## 追試: 余った枠に一本だけ追加

`-DAHC012_TARGET_CUT_DIAGNOSTIC` では、2方向SAの2,710ms予算をそのまま使って解を凍結する。追加枠があり、未配布の参加者がいる場合にだけ、元の区画をまとめて分割する直線を探す。法線 `(a,b)` ごとに整数の射影 `a*x+b*y` をソートし、同じ射影値を持つ点をまとめて移動する。左右それぞれで区画のサイズ頻度を差分更新するので、目当ての区画だけでなく他区画への損失も含め、全オフセットを厳密に比較できる。

凍結解、水平・垂直の2方向だけで最善の追加、斜めも含めた24方向で最善の追加の3つを**同じ凍結解**から比較する。得点が厳密に上がらない場合は切らない。カットの端点は `a*x+b*y=c` と `a*x+b*y=c+1` 上で、接線方向にケーキより十分遠い整数点を使う。円内の符号は整数射影の閾値と一致する。探索は開始2,880msで打ち切る（法線を始める前に時計を確認）。

| 分割 | 件数 | 凍結平均 | 軸方向追加平均 | 全方向追加平均 | 全方向の勝 / 分 / 負（対凍結） |
| --- | ---: | ---: | ---: | ---: | ---: |
| dev | 8 | 974,401.000 | 974,664.625 | 974,920.250 | 2 / 6 / 0 |
| holdout | 8 | 978,711.750 | 978,711.750 | 978,711.750 | 0 / 8 / 0 |

[dev](target-dev-results.csv)・[holdout](target-holdout-results.csv)とも初回とは別の公式生成seed。devで2件だけ1人増え、うち1件は斜めでなければ改善しなかった。holdoutは全件・全24方向で改善なし。全48出力が合法で公式visと独立整数採点に一致し、最長2.727876秒。追加探索は任意フラグ `-DAHC012_TARGET_CUT` のまま、既定の解法は変更しない。

[共通閾値の上限付き分割スイープ](../../library/capped-partition-sweep.hpp)をライブラリに分けた。区画ごとに両側の個数を全再計算する[800ケースの差分テスト](../../tests/capped_partition_sweep_test.cpp)と、整数外積の符号を確認する[幾何テスト](../../tests/ahc012_extra_cut_geometry_test.cpp)を追加した。

上の再現手順で `work` と `tools_dir` を設定し、以下を実行する。

```sh
python scripts/bundle.py problems/ahc012/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC012_TARGET_CUT_DIAGNOSTIC "$work/submission.cpp" -o "$work/target"
for split in dev holdout; do
  python problems/ahc012/target-benchmark.py "$split" "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/target" "$work/target-$split.csv"
done
for test in capped_partition_sweep ahc012_extra_cut_geometry; do
  g++ -std=c++23 -O3 "tests/${test}_test.cpp" -o "$work/test"
  "$work/test"
done
```

次は既存の直線を1本外し、その直線の符号を除いた区画IDで同じ分割スイープを使う。追加だけでは小さい区画を統合できないが、置換なら両側の片を結合してから別角度で再分割できる。薄い斜めを候補に含め、不足サイズが複数同時に改善するかを確認する。
