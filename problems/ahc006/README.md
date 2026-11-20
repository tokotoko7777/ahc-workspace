# AHC006: Food Delivery

- [公式問題文](https://atcoder.jp/contests/ahc006/tasks/ahc006_a) / [公式順位表](https://atcoder.jp/contests/ahc006/standings) / [公式ジェネレータ・採点器](https://img.atcoder.jp/ahc006/c21daebb77aa4d38d65f4d7f7c7249.zip)
- 本番最終1位 **2,220,425**。[ALE-Bench の AtCoder 提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc006.zip) 内の `ahc006//standings_scores.csv`。アーカイブの SHA-256 `08f54f5ce375baf4a30c9730fdb71cd731619b89e88f63eec1894a850b59bf18` は公開 LFS ポインタと一致。順位と得点のみ参照。
- 100ケースの `round(100000000 / (1000 + 走行距離))` をケースごとに丸めて合計し、最大化。非ACが1ケースでもあれば0点。2秒、1024 MiB。公式ツールの配布100ケースは本番との同一性を確認できず、コンテスト後システムテストはない。下記は独立生成ケースの比較なので、1位超えは **未判定**。

## 比較した構築法

既存の [AHC006 SA](../../examples/search/ahc006_sa.cpp) をベースラインとした。対照は各注文を単独で配達した場合の距離順に50件を選ぶ。空間マルチスタート版は店・届け先の両方が近い128件を9つの中心ごとに抽出し、現在経路への最小挿入増分を主とする貪欲で50件を選ぶ。9候補と既存初期解の最短から同じSAを開始する。構築込みの壁時計予算はどちらも1,850 ms、温度・乱数初期化・SA近傍を同一とした。

[既存の ordered pair insertion](../../library/ordered-pair-insertion.hpp) を追加注文の最短挿入位置を探すためそのまま使用した。配送先を先に、その後に集荷点を挿入すると元経路の gap 添字がずれない。今回はこのAPIで足りたため、汎用ライブラリの増設はしていない。

## 結果

| 分割 | 件数 | 初期走行距離の平均（既存 → 空間） | 最終得点の平均（既存 → 空間） | 空間版の勝 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 10 | 6,490.4 → 4,407.6 | 21,827.3 → 21,535.6 | 2 / 8 |
| holdout | 10 | 6,357.8 → 4,484.4 | 21,463.0 → 21,422.9 | 3 / 7 |

40出力すべて合法で、公式採点器と独立検証の得点は一致した。最大壁時計時間は1.877630秒。初期解の距離は大幅に短くなったが、SA後の平均・中央値は両分割で悪化したため、デフォルトは既存法のまま。構築追加は平均約13〜15 msで、devでは空間版のSA反復数の平均はむしろ増えた。劣化を単純な反復数不足とは説明できない。

## 再現手順

リポジトリルートで実行。公式ツールを展開して Cargo でビルドする。ルート、50件の集荷・配達、距離、四捨五入は独立検証と公式 vis で突き合わせる。

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc006/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC006_STATS "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 -DAHC006_STATS -DAHC006_SPATIAL_SEED=1 "$work/submission.cpp" -o "$work/spatial"
python problems/ahc006/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/base" "$work/spatial" "$work/results.csv"
```

この計測は Linux の CPU8 に固定し、各組で実行順を交互にした。dev/holdout 各10件の seed は別々の SHA-256 系列で決定し、入力を見ずに両群のコードと設定を固定した。時刻ベースのSAのため反復回数は実行環境に依存する。詳細は [計測CSV](benchmark-results.csv)。

### 次に試す仮説

構築の改良ではなく、SAで複数の注文を同時に差し替える。経路の短い連続区間から高い抜去利得を持つ3〜5件を外し、（集荷前・配達後という順序を守った）複数注文の再挿入をビームで比較する。単独置換では悪化するが同じ区間への集荷・配達を共有して一括なら改善する候補を拾えるか、同一初期状態・同時間で検証する。まず正確な候補距離差分と先行制約を愚直な全再計算に突き合わせる。
