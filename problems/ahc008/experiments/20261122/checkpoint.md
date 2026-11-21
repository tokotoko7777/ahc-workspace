# AHC008 100ケース固定計測チェックポイント

- 既存の同一配布100: 259,408,028点。
- 研究中の標準: 最小カット v2 高速版、配布100 **1,338,602,186**点、全100件合法。
  提出用単体ファイル: [mincut-speed.single.cpp](mincut-speed.single.cpp)。
  SHA-256: `2779f84451377b80ff255d8f5783efc438b5ca604092610c3ea3c46f090bab2f`。
- 比較対象: ユーザー提示 6,263,333,336点（[採点結果](https://atcoder.jp/contests/ahc008/submissions/29654603)）。
  配布ケースとリンク先の採点入力が同じとは未確認。歴史的本番2,000件は未使用・未判定。マージなし。
- 配布での最高値は面積優先 Flex の1,459,735,698点。独立生成100件の一組では
  v2の1,483,627,404に対して1,453,107,690だったため、別案として保存し、標準置換しない。
- 直線仕切りの行ビット化、最大流領域再利用、安全な楽観的上界を導入した際、
  配布100と独立100の行動ハッシュが全件一致。
  配布100の中央値 .383→.225秒、最大1.104→.408秒。保存した単一ファイルを
  再ビルドしても配布100は全件合法、1,338,602,186点、最大.429秒。

## 代表的な不採用案

- 作業者割当を重み付き最小費用で選択: 開発100 1.684B→1.500B。
- 既存の直線の局所変形: 開発100 1.989B→1.836B。
- 一枚の囲いで複数のペット近傍を強制: 開発100 1.756B→1.538B。
  連結通路を追加した対照は別の100で+2.34%、独立100で−5.90%、不採用。
- 仕切り完成までに動物が境界を越える確率を入れた評価: 開発100 1.756B→1.679B。
- 猫の行動推定は方向の予測精度を上げたが、短い時刻予測で新しい囲いを作ると、
  既に離れているため、完成した18個で対象の猫を一匹も捕らえていなかった。

## 再現

```sh
# tools に公式 v3 の展開済みディレクトリを入れる
(cd "$tools" && cargo build --locked --release)
work=$(mktemp -d)
archive="$PWD/problems/ahc008/experiments/20261122"
g++ -std=c++23 "$archive/mincut-speed.single.cpp" -o "$work/solver"
python problems/ahc008/benchmark_cases.py "$tools/target/release/tester" "$tools/in" "$work/practice.csv" speed="$work/solver"
(cd "$work" && "$tools/target/release/gen" "$archive/independent-seeds.txt")
python problems/ahc008/benchmark_cases.py "$tools/target/release/tester" "$work/in" "$work/independent.csv" speed="$work/solver"
```
