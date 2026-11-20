# AHC015: Halloween Candy

- [公式問題文](https://atcoder.jp/contests/ahc015/tasks/ahc015_a) / [公式順位表](https://atcoder.jp/contests/ahc015/standings) / [公式ジェネレータ・ビジュアライザ](https://img.atcoder.jp/ahc015/b639c75d.zip)
- 本番最終1位 **161,011,437**。[ALE-Bench の AtCoder 提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc015.zip) の `ahc015//standings_scores.csv`。SHA-256 `11293d19419acd7e8118126ffc1cfe4a0137629769108935c10a0bd3b554ce02` が公開LFSポインタと一致。順位と得点のみ参照。
- `round(10^6 × 同色4近傍連結成分サイズの二乗和 / 色別総数の二乗和)` を本番200ケースで合計して最大化。2秒、1024MiB、100手の対話形式。コンテスト後のシステムテストはなく、ローカル配布例と本番の200ケースが同一という根拠はない。本番1位超えは **未判定**。

## 探索

既存解なし。盤面の4方向の圧縮を行・列の走査で実装する。基本の評価は公式の分子と、途中だけ同色の隣接ペアに加点する項。最後の3手分の未知空き位置は、各手番の傾け方と全挿入位置を完全に列挙し、公式分子の期待値を最大化する。現在ターンまでに受け取った番号のみ使い、未来の空きマス番号は見ない。色順だけは問題仕様に従い先に受け取る。

- **対照**: 次の色1個について、全ての空き順を試し、その後の1手も貪欲に最適化する。4方向のなかから平均が大きい方向を選ぶ。
- **ロールアウト**: 6手先まで最大26列の空き順を独立にサンプルし、4方向すべてに同じ乱数列を適用する。仮想途中手は現在の評価に対する貪欲、候補選択は末端評価を主に途中平均も加える。終盤は残り手数まで。
- 1,790msに達したら計算を続けず1手の評価だけで応答し、各手のサンプルは15msを上限に打ち切る。各手は1行でflushする。

評価のたびに通常のBFSでキューと距離配列を作り、同色マスを個別にたどる必要があった。新しい [SmallGridBitFlood](../../library/small-grid-bit-flood.hpp) では成分探索をビット並列化し、配列の確保も不要。128bit型を持つGCC/Clang向け。利用例は[ライブラリ参考](../../docs/LIBRARY_REFERENCE.md#小さい格子のビット並列連結成分)。[4サイズ・4,800盤面の独立BFS比較](../../tests/small_grid_bit_flood_test.cpp)と[2,000盤面×4方向の逐次移動比較](../../tests/ahc015_tilt_test.cpp)を追加した。

## 結果

| 分割 | 件数 | 1手先の平均 | 6手先の平均 | 6手先の勝 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 24 | 568,462.63 | 663,282.21 | 18 / 6 |
| holdout | 24 | 600,825.08 | 663,091.75 | 17 / 7 |

全96出力が合法で公式採点と独立再生が一致。最大壁時計は対照0.061769秒、6手先0.204520秒。未使用ケースで平均+62,266.67点のため、問題ブランチの既定を6手先とした。一方、未使用ケースで最大26万点も悪化した例がある。本番1位の平均約805,057点に対して、この別seedの平均はまだ低い。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc015/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC015_ROLLOUT=0 "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 -DAHC015_ROLLOUT=1 "$work/submission.cpp" -o "$work/rollout"
python problems/ahc015/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/base" "$work/rollout" "$work/results.csv"
for test in small_grid_bit_flood ahc015_tilt; do
  g++ -std=c++23 -O3 "tests/${test}_test.cpp" -o "$work/test"
  "$work/test"
done
```

dev・holdout各24件の公式生成seedを別々のSHA-256系列から用意し、設定を測定前に固定。公式ツールに対話用testerはなく、1手応答が来るまで次の番号を渡さないハーネスを用いた。出力は公式visと独立Python再生・整数採点で比較し、2秒に対してはプロセス生成と対話を含む壁時計を記録した。[計測CSV](benchmark-results.csv)参照。

### 次の仮説

仮想途中手が目先の成分二乗に寄り過ぎるため、深さを伸ばしても同色の離れた塊を結ぶ道を見落とす可能性がある。ロールアウト内方策に「同色成分同士の接続までの距離」や、色ごとの近接域を保つ項を導入し、同じ最初の状態・同じ空き順列・同じ試行数で現行方策と比較する。時間余裕を使う版では、終盤まで一律に試行を増やすより中盤にサンプルと探索深さを配分する。

## 追試: 同じ仮想配置数で探索を深くする

`-DAHC015_LONG_ROLLOUT=1` は既定の「最大6手先・26経路」を「最大12手先・13経路」にする。他の方策・評価・時間ガードは固定し、各方向あたりの仮想配置上限を156回とした。終盤で残り手数が異なる場合は、既定版の仮想配置数を超えないよう経路数を切り下げる。各場面の4方向に共通の乱数列を適用する点は同じだが、深さが変わるので試行全体で消費する乱数と実際の軌跡は一致しない。

| 分割 | 件数 | 6手先の平均 | 12手先の平均 | 12手先の勝 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 24 | 640,541.50 | 607,777.38 | 9 / 15 |
| holdout | 24 | 617,989.38 | 594,117.13 | 10 / 14 |

初回とは別のseed系列で48ペアを計測。[追試CSV](long-results.csv)の全96出力で公式採点と独立対話再生が一致した。最長0.246614秒（6手版は0.206682秒）。この固定配置数では、深さを増やして経路数を減らす交換は両分割で悪化したため、既定は6手のまま。深さと経路数を同時に変えた比較であり、経路数を保った深い探索まで否定はできない。

上の再現手順でバンドルした後、以下を実行する。（このCSVの `base` は6手先版。）

```sh
g++ -std=c++23 -O3 "$work/submission.cpp" -o "$work/short"
g++ -std=c++23 -O3 -DAHC015_LONG_ROLLOUT=1 "$work/submission.cpp" -o "$work/long"
python problems/ahc015/benchmark.py --tag long-v1 --candidate-label long "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/short" "$work/long" "$work/long.csv"
```

次は単に深くするのではなく、途中盤面で同色が隔てられていても短距離で結べる塊を評価し、深いロールアウトの仮想方策を変える。既定方策と近接域を評価する方策を、同じ目の前の盤面・同じ乱数列・同じ総配置数で比較し、プレイ全体での得点と最終の色別最大成分を測る。
