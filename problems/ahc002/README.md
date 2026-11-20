# AHC002

[問題文](https://atcoder.jp/contests/ahc002/tasks/ahc002_a)

同じタイルを重複して踏まない経路を、末尾の作り直し・端点の回転・途中区間の付け替えで改善します。
`main.cpp` は独立に作成した単独のC++17ファイルです。
既定値は探索時間1.9秒、乱数seed 0、実装v10です。

```sh
g++ -O3 -std=c++17 main.cpp -o solver
./solver < input.txt > output.txt
```

開発に使っていない追加20件（公式生成器seed 30〜49）で平均 **62,676.55点**、最大実測時間1.902172秒。
全出力で、独立した検証と公式採点器の得点が一致しました。
本番ケースと同一の入力集合ではなく、公式順位を保証する結果ではありません。
時間を基準に探索を終了するため、実行環境によって結果は変わります。

## 短区間の全探索を追加した比較実験（未採用）

V10 の乱択区間再構築とは別に、最良経路を終盤に全走査します。各位置で
1～4 辺の区間を選び、端点を固定して元の辺数 + 2 以下の手数で通れる
区間を全探索し、得点が厳密に増える差し替えだけを採用します。外側で
使用中のタイルは通れません。座標が違ってもタイル ID が同じなら
重複として扱います。候補版には 1.90 秒の共通予算から最大 25 ms を
確保しました。既定のビルドは V10 のままです。

| 公開ジェネレータによるローカル比較 | V10 合計 | 全探索版 合計 | 差 | 合法（両版） |
|---|---:|---:|---:|---:|
| 新規開発 12 件 | 744,958 | 746,876 | +1,918 | 24/24 |
| 事前凍結した未調整 20 件 | 1,254,587 | 1,254,478 | −109 | 40/40 |

全出力で、公式 Rust 採点器と独自検証の得点が一致しました。最長実測
は未調整集合で 1.9183 秒／1.8903 秒未満でした。開発 12 件の V10 出力に
終盤処理だけをかけた参考比較では、3 件で改善し平均増分は 15.75 点、
最長約 8.1 ms でした。開発集合の通し実行差はこの増分だけでは説明できず、
時間依存探索の差も含みます。未調整集合の合計では改善しなかったため、
全探索版は既定にしません。

### 本番比較は未判定

[公式問題](https://atcoder.jp/contests/ahc002/tasks/ahc002_a)では、本番
100 件の絶対得点の合計を最大化し、1 件でも非 AC なら提出全体が 0 点です。
事後システムテストはありません。公式ローカル seed 0～99 は本番と
別物と明記されています。AtCoder 提供の
[ALE-Bench の順位アーカイブ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc002.zip)
に含まれる `standings_scores.csv` の 1 位は **6,255,977**。
公開ファイルの SHA-256
`c39b6db91b1956d3f816e63755103622e0ec090d84c6cf81dba1abb5c631ebd9`
と照合し、得点集計ファイルの先頭順位だけを確認しました。

[ALE-Bench の論文 §A.2.2](https://arxiv.org/html/2506.09050v2)
によれば、フル版の最終評価は元コンテストと同じケースを使用しますが、
採点実行環境の完全一致までは保証されません。
[公式フレームワーク README](https://github.com/SakanaAI/ALE-Bench#setup)
の `session.private_eval` を利用するには、実行環境に Docker と
対象言語の実行イメージが必要です。今回の作業環境には Docker CLI／
ソケットがないため未実行です。隠し入力やシードは読み込んでいません。
上のローカル点から最終順位の超過を判定せず、main へのマージもしません。

### 使った部品・再現

既存の回復処理では頂点とタイル ID が別です。頂点を使った通常の
グリッド探索だけでは、同じタイルの別頂点を誤って再使用し得るため、
色で重複を禁止するコールバック式の
[`bounded-colored-path.hpp`](../../library/bounded-colored-path.hpp)を
追加しました。既存の隣接配列と経路上のタイル位置はコピーせず参照し、
DFS の作業配列を各区間で使い回します。
[ライブラリ仕様](../../docs/LIBRARY_REFERENCE.md#bounded-colored-pathhpp)、
[最小使用例](../../examples/bounded_colored_path_usage.cpp)、
[小グラフ 1,500 件の愚直比較](../../tests/bounded_colored_path_test.cpp)。

リポジトリ直下で実行します。公式ジェネレータ・ビジュアライザは
問題文のリンクから取得し、`cargo build --release --locked` で
ビルドします。`gen` と `vis` はそのバイナリの絶対パスです。

```bash
bash problems/ahc002/build_frozen.sh build/ahc002
for suite in dev holdout; do
  mkdir -p "build/ahc002/$suite"
  (cd "build/ahc002/$suite" && "$gen" "../../../problems/ahc002/$suite-seeds.txt")
  python3 problems/ahc002/benchmark.py "build/ahc002/$suite/in" \
    "build/ahc002/$suite-results" build/ahc002/v10 build/ahc002/short-repair "$vis"
done
```

次の仮説は、独立の短区間を磨くのではなく、離れた二つの切断箇所を
同時に付け替えて「片方だけでは開かない領域」に長い経路を引き込むものです。
既存未調整 20 件とは別のケースで確認します。
