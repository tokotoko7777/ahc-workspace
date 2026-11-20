# AHC018: Excavation

- [公式問題文](https://atcoder.jp/contests/ahc018/tasks/ahc018_a) / [公式順位表](https://atcoder.jp/contests/ahc018/standings) / [公式ジェネレータ・対話テスタ](https://img.atcoder.jp/ahc018/6bada50282.zip)
- 各掘削の消費 `C+P` の合計を小さくする。相対得点はケースごとに `round(10^9 × 全参加者中の最小消費 / 自分の消費)` を取り、システム3000件で合計する。各Cは375件、5秒、1024MiB。
- 最終1位の相対点は**未確認**。順位表・JSONは未ログインで確認できず、[ALE-Bench公開ファイル一覧](https://huggingface.co/datasets/SakanaAI/ALE-Bench/tree/main)に `ahc018.zip` がない。[公開システムseed](https://img.atcoder.jp/ahc018/seeds.txt) のSHA-256は `3bbc58026730d18fcb0e03ead929ab2afb6f9f0c5eb7f0c10ac44df4e6b84a3f`。今回seedそのものや採点基準値は使っていないため、本番超えは **未判定**。

## 比較

掘削は両版共通。破壊に至った直前・直後の累積パワーの中点を硬さの推定値にし、隣の掘削済みセルを優先する。最初は予測値の一部を使い、破壊できなければCと残りの目安に応じた刻みで追加する。応答が2または-1なら直ちに停止する。

- **対照**: 水源と家を掘り、水が流れる全セルから未接続の最寄りの家へ、未掘削セル数をコストとする最短経路を追加する。
- **試掘版**: 端点周辺の境界箱内を間隔32で試掘し、その実測値から対数硬さを補間する。残りの家への経路は補間した硬さと固定費用の見積もりで選び、実際に掘った経路の測定値も更新に使う。補間値は既知の掘削済みセルの0コストとは区別する。

既存Dijkstraは明示の隣接配列と単一始点を要求する。今回、毎回変わるセル進入コストを辺配列に展開して仮想始点を作る必要があったため、[暗黙多始点Dijkstra](../../library/implicit-multi-source-dijkstra.hpp)を使用した。隣接をコールバックで生成して、条件を満たす最初の頂点で止まる。[使用例](../../docs/LIBRARY_REFERENCE.md#隣接配列を作らない複数始点-dijkstra)・[500回の差分テスト](../../tests/implicit_multi_source_dijkstra_test.cpp)。

## 結果

| 分割 | 件数 | 対照の平均消費 | 試掘の平均消費 | 試掘の勝 / 負 | 対照比の中央値 |
| --- | ---: | ---: | ---: | ---: | ---: |
| dev | 32 | 226,792.16 | 202,702.41 | 16 / 16 | 0.9683 |
| holdout | 32 | 220,242.63 | 209,857.16 | 15 / 17 | 1.0428 |

[計測CSV](benchmark-results.csv)の全128対話で合法、公式テスタと独立した岩盤・水の再生で消費が一致した。最長0.060524秒。大きな消費の一部を大幅に減らしたため単純平均は改善したが、holdoutのケース別比率の平均は1.1464で、最大2.963倍に悪化した。相対式の基準値が不明であり、絶対消費の単純平均だけで採用せず、既定は対照のままとした。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc018/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 -DAHC018_PROBES=1 "$work/submission.cpp" -o "$work/probes"
python problems/ahc018/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" "$work/base" "$work/probes" "$work/results.csv"
g++ -std=c++23 -O3 tests/implicit_multi_source_dijkstra_test.cpp -o "$work/dijkstra-test"
"$work/dijkstra-test"
```

Cの8水準を各4件ずつ、dev/holdout別のSHA-256系列から公式生成した。両版の設定は計測開始前に固定し実行順を交互にした。隠された硬さは公式テスタと独立再生にだけ渡し、解答プログラムは事前情報と1回ずつの応答のみを受け取る。

### 次の仮説

全面の試掘は、その地点を経路に使わないときも硬い岩盤の全費用を払う。予測経路とその近傍だけ、上限を設定した一回の軽い掘削で調べる。破壊しない場合は硬さの下限として記録し、予想価格が急上昇したとき未接続端点までを再計画する。途中までの掘削パワーを将来の進入コストから差し引き、不要な探査費用と良い迂回の利益を分けて計測する。

## 追試: 予定経路上だけを低出力で調べる

- `-DAHC018_ROUTE_PROBES=2` は最短経路の先頭から新規18セルだけを掘り、その途中の観測を補間に加えて再計画する。追加の試掘は行わない。
- `-DAHC018_ROUTE_PROBES=1` は同じ再計画に加え、未接続の家ごとに最初の予定経路の1/3・2/3地点だけ、各一回かつ70〜620のパワーで試す。短い経路は省く。破壊しない場合は硬さの下限が支払済みパワーを超えるため、それ以下の推定を棄却し、正の余裕を見込む。実際に掘るときは支払い済みパワーを二重計上せず、進入費用は残り推定値で比較する。
- 試掘の直接費用を `min(1550*K, 2*K*(620+C))` 以下に制限する。ログ上の試掘全件についてこの上限、各地点の重複なし、各回のパワー範囲を独立検証した。`unwatered_probe_cost` は最後まで水が流れなかった地点の試掘費。通水したすべての試掘が必要だったことまでは意味しない。

| 分割 | 対照 | 比較版 | 比較版の勝 / 負 | ケース比の平均 | ケース比の中央値 |
| --- | --- | --- | ---: | ---: | ---: |
| dev | 一様 | 再計画のみ | 17 / 15 | 0.9124 | 0.9663 |
| dev | 一様 | 経路試掘 | 20 / 12 | 0.8835 | 0.9617 |
| dev | 再計画のみ | 経路試掘 | 20 / 12 | 0.9836 | 0.9922 |
| holdout | 一様 | 再計画のみ | 24 / 8 | 0.9482 | 0.8852 |
| holdout | 一様 | 経路試掘 | 25 / 7 | 0.9202 | 0.8278 |
| holdout | 再計画のみ | 経路試掘 | 18 / 14 | 0.9674 | 0.9845 |

[別系列のdev/holdout各32件・計192結果](route-results.csv)。すべて公式テスタと独立再生が一致し、最大0.217050秒。holdoutの経路試掘消費は平均2,160、最終的に通水しなかった地点の試掘費は平均395.59。一様との改善は両分割で再現した一方、holdoutでは一様より最大2.2002倍の悪化があり、再計画そのものより試掘が増やす利益は小さかった。公式相対点とは比較せず、追加方式はいずれも任意フラグのまま残す。

```sh
# 上の再現手順で tools_dir と work を設定
python scripts/bundle.py problems/ahc018/main.cpp -o "$work/submission.cpp"
for mode in 0 1 2; do
  g++ -std=c++23 -O3 -DAHC018_ROUTE_PROBES=$mode "$work/submission.cpp" -o "$work/route-$mode"
done
python problems/ahc018/route-benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" "$work/route-0" "$work/route-2" "$work/route-1" "$work/route-results.csv"
g++ -std=c++23 -O3 tests/ahc018_partial_probe_test.cpp -o "$work/probe-test"
"$work/probe-test"
```

次は明確な迂回候補がある場所にだけ試掘を使う。硬さ予測の平均だけでなく、測定点からの距離と下限の幅をモデルに入れる。再計画前に「予定ルートと短い迂回の期待費用が接近していて、少量の観測で選択が変わるセル」を選ぶと、無意味な試掘を省きながら悪化例を減らせるかを、新しい公式生成ケースで比較する。
