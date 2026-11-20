# AHC016: Graphorean

- [公式問題文](https://atcoder.jp/contests/ahc016/tasks/ahc016_a) / [公式順位表](https://atcoder.jp/contests/ahc016/standings) / [公式生成器・対話テスタ](https://img.atcoder.jp/ahc016/d5f3c281.zip)
- 本番最終1位 **1,780,676,618,015（相対得点）**。[ALE-Bench の AtCoder 提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc016.zip) の `ahc016//standings_scores.csv`。SHA-256 `6be902402ee316cdfe59dbd0e6045507ff31d0dfd14bc68bc6f1258366ca54cb` は公開LFSと一致、順位と得点のみ参照。
- 100回の対話中の誤答数 `E` に対し、絶対点は `round(1e9 × 0.9^E / N)`。本番順位に使われるのは各ケースで `round(1e9 × 絶対点 / 順位対象者の最高絶対点)` を取り、2000件で合計した相対点。5秒、1024MiB。 [本番システムseed](https://img.atcoder.jp/ahc016/seeds.txt)（SHA-256 `4093b6cb740beea16eb0ecf55120ca6ca6fbef18015ea4a863e64d0bea3de91d`）は公開されているが未参照で、同じケース別基準値も取得していない。従って下記ローカル絶対点とは比べられず、本番1位超えは **未判定**。

## 解法

- **両版で共通**: ノイズ0なら4〜6頂点でグラフのラベル付け替えの軌道を列挙し、必要個数まで異なる軌道を選ぶ。正のノイズでは、等間隔の辺数を復号する場合の正規近似誤答率から期待絶対点を計算し、両版が同じ頂点数を使用する。
- **対照**: 0から全辺数まで、M個の辺数をほぼ等間隔に配置する。観測辺数は辺反転後の期待辺数に一番近い符号に復号する。
- **構造符号**: 最大3頂点群を使い、普遍頂点＋クリーク、2クリーク、完全二部と、それぞれの補グラフを候補とする。各頂点の次数について、元の辺が残る確率 `1-eps`、なかった辺が加わる確率 `eps` の独立な成功回数分布を計算。全頂点で期待次数ヒストグラムを合算して、平均と順位5区分の平均の差を特徴にする。特徴距離で最も近い既存候補から遠い候補を順に採用し、受信グラフは実測特徴で最近傍復号する。独立なのは**一頂点に接続する辺の反転**であり、全頂点の次数が独立とは仮定しない。

既存の均一確率の二項分布では、既存辺と非辺で確率が逆になる次数を直接表せない。[Bernoulli count distribution](../../library/bernoulli-count-distribution.hpp)を独立な異確率の成功回数分布に使用、[450ケースの全事象列挙](../../tests/bernoulli_count_distribution_test.cpp)・[短い使用例](../../docs/LIBRARY_REFERENCE.md#独立なベルヌーイ試行の成功回数分布)を追加した。

## 結果

| 分割 | 件数 | 対照の平均絶対点 | 構造符号の平均絶対点 | 構造符号の勝 / 分 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 40 | 31,956,874.08 | 36,791,740.40 | 30 / 8 / 2 |
| holdout | 40 | 31,882,017.48 | 38,032,050.10 | 32 / 7 / 1 |

未使用ケースで平均誤答数は29.650から12.775、ノイズ0を除くと33.886から14.600。ノイズ0.29以上の未使用10件は改善後も平均51.1誤答。両分割で上がったため問題ブランチ既定は構造符号とした。160対話すべて合法で、公式テスタの誤答数・絶対得点は整数の独立集計に一致。最大壁時計0.070234秒（テスタの実行を含む）。

### ノイズの再現

devとholdoutで別のSHA-256系列と、あらかじめ定めた `M, eps` の組を公式ジェネレータに渡す。同一ペアの2版には、公式テスタに同じ入力と頂点数を渡す。この組み合わせなら、公式実装は同じクエリごとに同じ置換・辺反転の乱数を消費する。プログラムには `M, eps` を渡し、符号表が確定してから1件ずつノイズ付きグラフを与える。真のラベル列や乱数seedはテスタ側にのみ保持する。各版のノイズ付きグラフは元符号が異なるため同じグラフとは限らない。[計測CSV](benchmark-results.csv)を参照。

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc016/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC016_MODEL_N=0 -DAHC016_DEGREES=0 "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 -DAHC016_MODEL_N=0 "$work/submission.cpp" -o "$work/degree"
python problems/ahc016/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" "$work/base" "$work/degree" "$work/results.csv"
g++ -std=c++23 -O3 tests/bernoulli_count_distribution_test.cpp -o "$work/dist-test"
"$work/dist-test"
```

### 次の仮説

高ノイズ・大Mでは辺数だけの誤答近似からNが小さく選ばれ、構造符号でも情報を十分に持たせられない。既定のNその他複数の候補Nで、入力の `M, eps` のみから人工ノイズを生成し、構造符号自身の誤答率とサイズ罰を推定して選ぶ。推定用と確認用の人工乱数を分け、5秒以内に収まる上限を設け、公式生成の新しいベンチマークで比較する。乱数は実テスタの割り当てやシードに依存させない。

## 追試: 人工ノイズで頂点サイズを選ぶ

符号と復号器の構造は同じ。既定の頂点数、その0.6・0.8・1.2倍、および32, 52, 72, 88, 100を候補にし、頂点数が重なる場合は一度だけ試す。各候補で各符号の辺を `eps` で反転して12回ずつ分類し、誤答率 `e` から `(1-0.1e)^100 / N` を推定する。上位二つと元のNを別の固定乱数で32回ずつ再評価し、推定点が最も高いNを採用。実対話の真値・ノイズseedは使わない。復号特徴は頂点順不変のため、人工試行では置換を省く。探索打ち切り1,470ms、測定の上限は公式5秒より厳しい2秒。途中で打ち切った候補の誤差推定は使わない。

| 分割 | 件数 | 元Nの平均絶対点 | モデルNの平均絶対点 | モデルNの勝 / 分 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 40 | 33,281,344.18 | 41,784,959.55 | 29 / 10 / 1 |
| holdout | 40 | 33,561,640.38 | 42,457,716.95 | 29 / 11 / 0 |

初回と別系列の[追試CSV](model-results.csv)。holdout平均誤答は12.825→6.075、ノイズ0.28以上の15件では34.133→15.733。160出力はすべて公式テスタと独立整数採点が一致し、最長0.368434秒だった。改善が両分割で再現したため、問題ブランチ既定をモデルNとした。ペアは同じ公式生成入力・ラベル列・seedで比較するが、**頂点数が違うと公式テスタの乱数消費が変わるため、同じノイズを受けているとは限らない**。絶対点ローカル値であり、本番相対点とは未比較。

```sh
# 上の再現手順で work と tools_dir を設定
python scripts/bundle.py problems/ahc016/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC016_MODEL_N=0 "$work/submission.cpp" -o "$work/degree"
g++ -std=c++23 -O3 "$work/submission.cpp" -o "$work/model"
python problems/ahc016/benchmark.py --tag model-v1 --limit 2 "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" "$work/degree" "$work/model" "$work/model.csv"
g++ -std=c++23 -O3 tests/ahc016_size_model_test.cpp -o "$work/model-test"
"$work/model-test"
```

次は高ノイズで混同される符号対を減らす。次数帯の平均だけでは同じ次数分布を持つ別のブロック構造を分けられないため、観測次数で頂点を帯に分け、帯間の残差辺密度を補助特徴にする。人工乱数の学習・確認を分けて同じN・同じ符号表で復号だけを比較し、その後に符号表の選抜へ反映する。
