# AHC010: Loop Lines

- [公式問題文](https://atcoder.jp/contests/ahc010/tasks/ahc010_a) / [公式順位表](https://atcoder.jp/contests/ahc010/standings) / [公式ジェネレータ・採点器](https://img.atcoder.jp/ahc010/d3a6c91ee76cb8.zip)
- 本番最終1位 **20,218,968**。[ALE-Bench の AtCoder 提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc010.zip) の `ahc010//standings_scores.csv`。SHA-256 `8f2e83fe0b13701d1df45c938b3397a2474518751aa646adf21f1eb0169b8eb6` が公開 LFS ポインタと一致。順位と得点のみ参照。
- 上位2閉路の長さの積を最大化し、100件の合計で評価する。閉路が2本未満なら0点。1件でも非ACなら提出全体は0点。2秒、1024 MiB。本番100件とローカル配布例が同一だと確認できず、コンテスト後のシステムテストはないため、本番1位超えは **未判定**。

## 既存解なしからの構築

30×30を左右の30×15に分け、半分ごとに少なくとも1本の大きな閉路を残す。各半面で3回焼き直し、SA中は長い閉路と「端点が近い長い開路」を主に評価し、最後は厳密に最長閉路を保存する。境界をまたぐ線路を見なくても、半面内で閉じた閉路の長さは全盤面でも変わらない。

対照は1枚回転の近傍。比較版では48%の提案で隣接2枚が共有辺を両側から使うように同時に向きを選ぶ。途中で接続を切る必要がある2枚変更を一手で試すのが狙い。オブジェクティブと計時予算（両半面合計1,810ms）は同一。近傍により乱数消費・受理・反復数が違うため軌跡は一致しない。

線路片の隣接を最低限の配列で更新し、成分サイズ・端点を [DegreeTwoComponents](../../library/degree-two-components.hpp) で列挙した。汎用BFSで近傍や成分ごとの頂点配列を毎回用意すると、SAの評価ごとに不要な確保が入る。既存のグラフBFSは始点からの距離が欲しい場合向け。ここでは接続判定を共有しつつ、活性頂点と2つの隣接先を渡して作業領域を再利用した。[使用例](../../docs/LIBRARY_REFERENCE.md#次数2以下の無向グラフを成分ごとに調べる)・[差分テスト](../../tests/degree_two_components_test.cpp)。

## 結果

| 分割 | 件数 | 1枚回転の平均 | 共有辺付きの平均 | 比較版の勝 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 10 | 46,492.4 | 53,175.2 | 6 / 4 |
| holdout | 10 | 33,180.0 | 56,482.0 | 7 / 3 |

全40出力で合法性・得点を確認し、最大壁時計時間は1.834601秒。[計測CSV](benchmark-results.csv)参照。
未使用ケースで第2閉路の平均長が137.4から207.0に伸び、両分割で平均・中央値が向上したため、比較版をブランチ既定とした。本番1位の平均約20.2万点とは、異なるケースとはいえまだ大きな隔たりがある。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc010/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC010_STATS -DAHC010_COORDINATED=0 "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 -DAHC010_STATS -DAHC010_COORDINATED=1 "$work/submission.cpp" -o "$work/edge"
python problems/ahc010/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/base" "$work/edge" "$work/results.csv"
g++ -std=c++23 -O3 tests/degree_two_components_test.cpp -o "$work/components-test"
"$work/components-test"
```

dev/holdoutは独立なSHA-256系列の公式生成seed。比較の前に両モードを固定し、同じ入力で実行順を交互にした。公式visと別実装の有向ポート追跡を突き合わせた。保存側で数えた左右の各最大閉路の積を、全盤面の上位2閉路の積が下回らないことも確認した。

### 次の仮説

共有辺を1本つなぐだけでは、同じ区間の別の出口を切ってしまう。2×2〜3×2のパッチ外を「どの境界ポートとどのポートが何本の線路片を経てつながるか」に縮約し、パッチ内全向きの境界つなぎ替えを全盤面走査なしで正確に評価する。パッチの頂点数が小さい場合は全回転組を列挙し、長い開路2本を結ぶ接続や、長い閉路を保存しながら差し替える接続を狙う。小盤面ランダムパッチを全走査スコアと比較する。
