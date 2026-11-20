# AHC004 公式配布100ケースでの比較

[公式問題文](https://atcoder.jp/contests/ahc004/tasks/ahc004_a)で配布される
ローカル用の **seed 0〜99 全100ケース**を使う。本番ケースとは別集合。
合計の目標は **7,321,305,762点**。50ケースからの外挿はしない。

- [公式入力ZIP](https://img.atcoder.jp/ahc004/a45fa3f18ab177158bf5961b12872f93.zip) SHA-256: `b0d1cd27284d96af3f409ee58304b661ddd2711400fbfbb37ffa60ad29d38cf9`
- [公式ツールZIP](https://img.atcoder.jp/ahc004/222362f13a30b1342bf79d0041bd4d39.zip) SHA-256: `b17da9bf7e05231fb25cbc1733e2ef07063bb4bdb9cea13dd24e2e2227c9c4ed`
- 公式 Rust `vis` と独立 Python 採点を全出力で照合。時間はソルバだけ（起動を含む）。
  同じ一論理CPUで直列実行し、ペアは先に走らせる版をケースごとに交替させる。
  壁時計で探索を打ち切るため、別マシンや負荷条件で値は変動する。
  計測は全版で同じGNU C++23・`-O3`ビルドに揃え、提出用ファイルは
  ソース冒頭のpragmaのみでも別途コンパイルする。

## 基準の実測

| 暫定評価版 | 公式100ケース合計 | 最長（ソルバ） |
|---|---:|---:|
| 元の単独例 | 6,727,041,455 | 2.791325秒 |
| 重なり連結の既存版 | **6,950,228,278** | 2.808593秒 |

200出力すべて公式採点と独立検査が一致。既存版は78勝2分20敗で
223,186,823点増えたが、目標までは371,077,484点。増加のうち
188,231,188点が長さパラメータ8〜10の40ケースに集中した。
既存版のその40ケースの平均は約6,412万点で、短い断片（パラメータ4〜5、
30ケース）の約8,260万点より伸びしろがある。

[200件の明細](baseline-results.csv)のSHA-256:
`918278bfb9e5c72448eeacd826ce77054747de1a6a1e86d8c396c4ad206ec136`。
GNU g++ 14.2.0、単独CPU 6、元の単独例と既存版を開始順交互で実行。

## 実験

基準は単独例 `examples/search/ahc004_genome_sa.cpp` と、
[重なり連結の既存版](https://github.com/tokotoko7777/ahc-workspace/tree/88dfd64092fbefe29c5d2e8770d08813ee522452)。
候補は基準を残して専用マクロで切り替える。

- `AHC004_EXACT_PLACEMENT`: 通常の一致文字数を使って候補を絞り、約1/4の提案で
  最良一致の場所と別ライン上の最大3候補を、正確な収録語の増減で比較する。
  狙いは、一致度が同じでも既存の語を壊さない候補を選ぶこと。
- `AHC004_FAST_PLACEMENT`: ラインの循環窓を、ラインが変わったときだけキャッシュ。
  提案中の除算、可変シフト、マスク再計算を減らす。
  GNU x86ではホストのPOPCNT対応をGCCが実行時に判定し、非対応環境では汎用版を使う。
  幾何的な評価式・走査順・採用条件は基準と同一。
- 周期20の閉路を配置する候補は別の実験作業ツリー。基準と同じ探索終了時刻で比較する。

### 公式100ケースでの一次結果

| 実験 | 合計 | 基準差 | 勝／同点／敗 | 最長 |
|---|---:|---:|---:|---:|
| 正確な配置再選択 | 6,984,932,092 | +34,703,814 | 33／54／13 | 2.788073秒 |
| [観測閉路の初期化](https://github.com/tokotoko7777/ahc-workspace/tree/6cc6477214c0a339a546486e44c99742e3132ba6) | 6,952,490,997 | +2,262,719 | 42／17／41 | 2.788058秒 |
| 循環窓キャッシュ／POPCNT | **6,998,515,279** | **+48,287,001** | 35／56／9 | 2.800836秒 |

300出力すべて公式採点と独立検査が一致。計測CSVは
[再選択・観測閉路](variants-results.csv)と[高速化](fast-results.csv)。
高速化は探索反復数の中央値を336,976回から855,504回へ（2.54倍）。
短・中のケースで計48,287,001点伸びたが、長さパラメータ8〜10では40件すべて同点。
現時点の最高は高速化単独版で目標まで322,790,483点。再選択との併用は別に計測する。

観測閉路は長い40ケースで延べ1,204本を列挙できたが、全100件で閉路から
作った盤面はSAの開始盤面に一度も選ばれなかった。得点が微増したことは
閉路の有効性を示さない。次は通常と閉路の盤面を同じ時間だけ試し焼きし、
初期点で早く捨てていたのか、交差の置き方自体が弱いのかを分ける。

## 再現

公式ツール側で `cargo build --release --locked`。環境変数 `IN` を入力ZIPの
`in/`、`VIS` を公式ビジュアライザの実行ファイルに設定し、リポジトリルートで実行。
`taskset` がない環境では、CPUにほかの重い処理がない状態でコマンドを直列に実行する。

```sh
W=$(mktemp -d)
E=problems/ahc004/experiments/local100
g++ -std=c++23 -O3 examples/search/ahc004_genome_sa.cpp -o "$W/original"
g++ -std=c++23 -O3 problems/ahc004/main.cpp -o "$W/incumbent"
g++ -std=c++23 -O3 -DAHC004_EXACT_PLACEMENT problems/ahc004/main.cpp -o "$W/rerank"
g++ -std=c++23 -O3 -DAHC004_FAST_PLACEMENT problems/ahc004/main.cpp -o "$W/fast"
g++ -std=c++23 -DAHC004_FAST_PLACEMENT -DAHC004_EXACT_PLACEMENT tests/ahc004_line_cycle_test.cpp -o "$W/differential"
"$W/differential"
python3 scripts/bundle.py problems/ahc004/main.cpp -o "$W/submission.cpp"
g++ -std=c++23 "$W/submission.cpp" -o "$W/submission"
taskset -c "${CPU:-6}" python3 problems/ahc004/benchmark.py "$IN" "$W/base" --vis "$VIS" --entry "original=$W/original" --entry "incumbent=$W/incumbent"
taskset -c "${CPU:-6}" python3 problems/ahc004/benchmark.py "$IN" "$W/candidates" --vis "$VIS" --entry "rerank=$W/rerank" --entry "fast=$W/fast"
python3 "$E/summarize.py" "$IN" "$W/base/results.csv" "$W/candidates/results.csv"
```
