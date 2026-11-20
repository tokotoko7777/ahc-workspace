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
| 高速化＋正確な配置再選択 | **7,012,486,156** | **+62,257,878** | 40／48／12 | 2.825020秒 |

300出力すべて公式採点と独立検査が一致。計測CSVは
[再選択・観測閉路](variants-results.csv)と[高速化](fast-results.csv)。
高速化は探索反復数の中央値を336,976回から855,504回へ（2.54倍）。
短・中のケースで計48,287,001点伸びたが、長さパラメータ8〜10では40件すべて同点。
[併用100件](combined-results.csv)も全件採点一致。高速化単独に対し
+13,970,877点（29勝49分22敗）、現時点最高で目標まで308,819,606点。
長さ8〜10には−405,831点で、短い断片での改善が主。

観測閉路は長い40ケースで延べ1,204本を列挙できたが、全100件で閉路から
作った盤面はSAの開始盤面に一度も選ばれなかった。得点が微増したことは
閉路の有効性を示さない。次は通常と閉路の盤面を同じ時間だけ試し焼きし、
初期点で早く捨てていたのか、交差の置き方自体が弱いのかを分ける。
同じ公式入力100件で、列挙だけを20,000訪問上限・壁時計打ち切りなしで調べると、
パラメータ9では平均32.9本、10では平均35.1本あり、掲載回数で重みづけした
観測語の81.82%／88.49%を候補閉路の和集合が含んだ。同時に配置できる
保証はなく、盤面の得点ではない。この差から、少数の閉路を貪欲に置く
だけでなく、複数の交点で行・列の相対位置をまとめて絞る案を検討する。

### 閉路の初期点で選別しすぎていたか

[対照付きの試し焼き](https://github.com/tokotoko7777/ahc-workspace/tree/5b4fa0c304fb1a13f92f56fb2665bf68d7fe2059)。
通常＋通常の次点と、通常＋観測閉路の候補をそれぞれ120msずつ育ててから、
同じ終了時刻まで通常の探索を続けた。[公式100件の明細](scout-results.csv)。
閉路側6,942,173,585点、通常対照6,953,227,258点。差は−11,053,673点
（21勝50分29敗）、長い40件では+214,698点。全200出力は採点一致。
閉路候補が実際に並んだ78件では、試し焼き後に閉路側が選ばれたのは3件。
78件全体の収録語数は閉路23,185→28,632、通常31,621→31,728。
初期点だけで捨てるよりは良いが、同じ探査時間で通常盤面を追い越せない。
従って既定にはせず、少数の閉路を先に置く方式より大きな交差条件を使う。

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
g++ -std=c++23 -O3 -DAHC004_FAST_PLACEMENT -DAHC004_EXACT_PLACEMENT problems/ahc004/main.cpp -o "$W/fast_rerank"
g++ -std=c++23 -DAHC004_FAST_PLACEMENT -DAHC004_EXACT_PLACEMENT tests/ahc004_line_cycle_test.cpp -o "$W/differential"
"$W/differential"
python3 scripts/bundle.py problems/ahc004/main.cpp -o "$W/submission.cpp"
g++ -std=c++23 "$W/submission.cpp" -o "$W/submission"
taskset -c "${CPU:-6}" python3 problems/ahc004/benchmark.py "$IN" "$W/base" --vis "$VIS" --entry "original=$W/original" --entry "incumbent=$W/incumbent"
taskset -c "${CPU:-6}" python3 problems/ahc004/benchmark.py "$IN" "$W/candidates" --vis "$VIS" --entry "rerank=$W/rerank" --entry "fast=$W/fast" --entry "fast_rerank=$W/fast_rerank"
python3 "$E/summarize.py" "$IN" "$W/base/results.csv" "$W/candidates/results.csv"
```

候補閉路の診断だけを再現する場合、`CYCLE` は観測閉路実験のチェックアウト先。
CSV列はファイル名、長さパラメータ、入力語数、候補数、全候補・上位20本・
上位30本の語被覆数、訪問上限到達の順。

```sh
g++ -std=c++23 -O3 -I "$CYCLE/problems/ahc004" "$E/cycle-bank-diagnostic.cpp" -o "$W/bank"
"$W/bank" "$IN"/*.txt > "$W/cycle-bank-diagnostic.csv"
```
