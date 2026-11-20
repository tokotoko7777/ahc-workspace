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

## 追試: 2×2パッチの境界縮約

`-DAHC010_PATCH_DIAGNOSTIC` は、共有辺版SAの1,810ms予算を変えずに、左右それぞれの凍結盤面に単発の2×2パッチを試す。パッチ外へ出た入口を片側ずつ追い、ほかの入口に戻る場合はその間の線路片数を重みとして記録する。外側だけで閉じる環は構築時に一度記録する。全回転組では最大8頂点の内部線路片と縮約辺だけをたどれば、半面の上位2閉路長が厳密に分かる。外側に起因する開路は閉路の加点対象外。時間厳守のため左右それぞれの壁時計締切を1,860ms・1,910msとする。

各半面で開始状態から独立に、(a) 全パッチの候補のうち1枚だけ変更できる場合と、(b) 最大4枚同時変更できる場合の最良一手を比較する。各試行の改善を次の試行に引き継がない。最長閉路で半面ごとの一手を選んだ後、3出力（凍結、単枚、複数枚）すべてを公式採点器と独立な有向ポート追跡で再採点する。境界にまたがる偶発的な閉路があるため、半面で厳密でも全体の順位は再採点が必要。

| 分割 | 件数 | 凍結時平均 | 単枚修正平均 | 複数枚修正平均 | 単枚に対する勝 / 分 / 負 |
| --- | ---: | ---: | ---: | ---: | ---: |
| dev | 8 | 52,258.5 | 52,492.5 | 54,069.5 | 7 / 1 / 0 |
| holdout | 8 | 50,833.5 | 50,833.5 | 52,521.5 | 8 / 0 / 0 |

[dev](patch-dev-results.csv)と[holdout](patch-holdout-results.csv)は初回とは別の公式生成seedで、両試験を始める前に設定を固定。全48出力が公式採点器と独立検証で一致した。全入力で左右各406パッチを試せ、最長1.842705秒。未使用ケースの平均改善は1,688点（約3.3%）で、単枚では改善が出なかったため一括変更自体には有効性が見られた。本番条件は引き続き未判定で、診断は任意フラグのまま。

ランダム30盤面でパッチを選び、採用回転を更新しながら8,400通りについて、縮約された上位2閉路長を全半面の有向ポート追跡と比較した。タイルとポートの対応が問題固有なので、縮約コードは `problems/ahc010/` に置いた。

上の再現手順で `tools_dir` と `work` を設定し、次で実行する。

```sh
python scripts/bundle.py problems/ahc010/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC010_PATCH_DIAGNOSTIC "$work/submission.cpp" -o "$work/patch"
for split in dev holdout; do
  python problems/ahc010/patch-benchmark.py "$split" "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/patch" "$work/patch-$split.csv"
done
g++ -std=c++23 -O3 tests/ahc010_contracted_patch_test.cpp -o "$work/patch-test"
"$work/patch-test"
```

次は、長い開路または閉路を含む近傍だけを対象に、受理した2×2／3×2改善を別パッチへ連鎖させる。ポート間の重みだけでなく、外に開いた線路の長さと端点も縮約側に保存すれば、既存SAの「端点が近い長い開路」も再計算でき、単発の後処理では越えられない谷を温度付きで探索できる。まずその差分を元の評価関数に突き合わせる。
