# AHC007：代替候補を待つリスクの符号を分ける

[旧版の正方向の補正](../adaptive_risk/README.md) は、未来の代替経路の
ボトルネックになる辺が標本ごとに変わるほど、現在の辺を採りやすくする。
しかし、代替候補が**並列**なら先の安値を見通せない分だけ採択閾値を
上げたい一方、候補が**直列**なら現在辺を採る側が未来の安値を
見通せないので、逆に閾値が下がることがある。

各標本のボトルネック辺を `f`、その長さを `L_f`、幾何距離を `d_f`
とすると、並列の最小値では選ばれた `L_f` は平均 `2*d_f` より
小さくなりがちで、直列の最大値では逆になりがち。そこで閾値を

```
mean + 0.24 * stddev * Pr(異なるボトルネック辺)
     * clamp(3 * (2 - sum(L_f) / sum(d_f)), -1.75, 2.2)
```

とする。これは当たりやすい側の符号を推定するヒューリスティックであり、
一般グラフでは最適性・上下界の保証はない。分散を少し縮めて計算する
近似と比べ、元の整数一様長さの標本をそのまま使える。

双方向ボトルネック探索、層化標本最大96本と12→24→48→96の逐次打ち切り、
処理時間ガード、連結性の保証、flushは旧版と共通。
生成器の評価seedはプログラムに渡さず、目の前の重みより先の真値は読まない。
汎用の経路閾値とボトルネック辺を返す機能は
[共通ヘッダとランダム検査](../adaptive_risk/)を利用する。

## 検証結果

| 150件 | 旧版 | 符号付き補正 | 差 | 勝ち / 同点 / 負け |
|---|---:|---:|---:|---:|
| 公式ツール同梱の公開入力 | 14,247,857,888 | **14,255,819,331** | +7,961,443 | 74 / 2 / 74 |
| 公式生成器の未調整入力 | 14,224,199,058 | **14,249,050,741** | +24,851,683 | 87 / 0 / 63 |

未調整範囲は 27,155,000–27,155,149。書いた実装を固定してから、
各入力に一辺ずつ真の長さを渡す入出力で実行した。比較両版・両集合の
全600回が合法で、独立した木・丸め検査と公式 `vis` のスコアが一致。
符号付き版の最大時間はロックステップ込みで公開0.316秒、未調整0.262秒、
solver CPU時間は0.306秒、0.249秒。時間ガードによる打ち切りは0件。

いずれも目標値14,241,733,941を上回るが、大会本番と同じ入力での
厳密な順位比較ではない。公開ケースで旧版より良くなったケースと悪く
なったケースが同数のため、総合改善を全ケースの改善とは扱わない。
ケースごとの時間・得点・採否ハッシュは
[`measurements.csv`](measurements.csv)。

## 再現

`gen`、`vis`、`public_in` は
[公式ZIP](https://img.atcoder.jp/ahc007/a855b6a456c9892b747e147001b0f89.zip)の
生成器、採点器、公開入力の絶対パス。旧版の再現は
[旧版README](../adaptive_risk/README.md)を参照。リポジトリルートから以下：

```sh
d=problems/ahc007/experiments/signed_witness_risk
out="$(pwd)/build/ahc007-signed-risk"
mkdir -p "$out/holdout"
python3 scripts/bundle.py "$d/main.cpp" -o "$out/submission.cpp"
cmp "$d/submission.cpp" "$out/submission.cpp"
g++ -std=c++23 "$out/submission.cpp" -o "$out/signed"
g++ -std=c++23 problems/ahc007/experiments/adaptive_risk/submission.cpp -o "$out/old"
python3 problems/ahc007/benchmark.py "$public_in" "$out/public" "$vis" "$out/old" "$out/signed"
seq 27155000 27155149 > "$out/holdout/seeds.txt"
(cd "$out/holdout" && "$gen" seeds.txt)
python3 problems/ahc007/benchmark.py "$out/holdout/in" "$out/private-check" \
 "$vis" "$out/old" "$out/signed"
```
