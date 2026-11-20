# AHC001

[問題文](https://atcoder.jp/contests/ahc001/tasks/ahc001_a)

指定点を含む長方形を、移動・伸縮・周囲の配置の修復を組み合わせた焼きなましで改善します。
`main.cpp` は独立に作成した単独のC++17ファイルです。
既定値は探索時間4.9秒、乱数seed 0、実装v64です。

```sh
g++ -O3 -std=c++17 main.cpp -o solver
./solver < input.txt > output.txt
```

公式公開シードの先頭30件で平均 **993,851,080.03点**、最大実測時間4.911595秒。
全出力で、独立した検証と公式採点器の得点が一致しました。
小規模比較の結果であり、全本番ケースでの順位を保証するものではありません。
時間を基準に探索を終了するため、実行環境によって結果は変わります。

## 比較実験: 接触成分の共有境界移動

通常ビルドは独立作成の既存 v64 のままです。
`-DAHC_FACE_SLIDE` では、焼きなまし後に辺を共有する面を連結成分ごとに
一緒に動かします。非重複と指定点の条件から移動可能な区間を求め、得点を
増やす候補だけ採用します。既存の単一長方形、2長方形の仕上げとは別の近傍です。

- [公式問題文](https://atcoder.jp/contests/ahc001/tasks/ahc001_a): 各ケースで丸めた絶対得点の総和を最大化。本番1,000件、各回5秒・1,024 MiB。
- [本番順位表](https://atcoder.jp/contests/ahc001/standings) / [本番順位・点数を含むアーカイブ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc001.zip): `ahc001//standings_scores.csv` の1位は **993,823,071,683**。公開本番シードの全1,000件を公式ジェネレータ・公式採点器で再現してのみ比較します。
- [汎用部品](../../library/coordinate-face-slide.hpp)、[使用例](../../docs/LIBRARY_REFERENCE.md#長方形の共有境界移動)、[テスト](../../tests/coordinate_face_slide_test.cpp)。
- 既存 `AxisAlignedRectangle` は名前付き座標ですがソルバは配列 `q[4]`。高頻度の探索で型変換・コピーしないよう、汎用部品は座標を取得する関数を受け取ります。成分構築は二乗走査のため、終盤にまとめます。

```sh
g++ -std=c++23 -O3 problems/ahc001/main.cpp -o baseline
g++ -std=c++23 -O3 -DAHC_FACE_SLIDE problems/ahc001/main.cpp -o face_slide
python3 scripts/bundle.py problems/ahc001/main.cpp -o submission.cpp
g++ -std=c++23 -O3 -DAHC_FACE_SLIDE submission.cpp -o submission
g++ -std=c++23 -O2 tests/coordinate_face_slide_test.cpp -o face_test
./face_test
```

公式ツールは問題文のリンクから取得し、`cargo build --release --locked` で gen と vis を
ビルドします。同梱の本番とは異なる入力 0000–0007 を開発用、別途固定した10シードを
未調整確認用としました。枠・交差・全指定点の含有を独立検査し、公式採点器と照合します。

```sh
python3 problems/ahc001/benchmark.py TOOLS_DIR/in bench-dev --start 0 --end 8 --time 4.8 --entry baseline=baseline --entry face_slide=face_slide --vis TOOLS_DIR/target/release/vis
mkdir -p holdout-cases
(cd holdout-cases && ../TOOLS_DIR/target/release/gen < ../problems/ahc001/holdout-seeds.txt)
python3 problems/ahc001/benchmark.py holdout-cases/in bench-holdout --time 4.8 --entry baseline=baseline --entry face_slide=face_slide --vis TOOLS_DIR/target/release/vis
```

| 条件 | 既存 v64 の平均 | 共有境界版の平均 | 最大実測時間 (旧/新) | 新版勝ち |
|---|---:|---:|---:|---:|
| 開発8件、同一入力、探索上限4.8秒 | 993,052,040.625 | 993,961,106.250 | 4.815867 / 4.718097 秒 | 6/8 |

この比較で本番順位は確定しません。経過時間で反復回数が変わるため、
上の差すべてを共有境界の効果とはみなせません。

```sh
python3 scripts/bundle.py problems/ahc001/main.cpp -o frozen.cpp
g++ -std=c++23 -O3 -DAHC_DEFAULT_SECONDS=4.8 frozen.cpp -o FROZEN_BINARY
# 本番シード: 問題文の seeds.zip から展開した seeds.txt
mkdir -p final-cases
(cd final-cases && ../TOOLS_DIR/target/release/gen < ../seeds.txt)
python3 problems/ahc001/final_suite.py final-cases/in FROZEN_BINARY TOOLS_DIR/target/release/vis seeds.txt final-run --jobs 1 --seconds 4.8
# 同じコマンドで再開。manifest.json と一致した既済ケースは再測定しない。
```

final_suite.py は公開シード・全入力・提出バイナリ・採点器をハッシュで固定し、
ケース単位で記録します。全件が揃ったときだけ summary.json を作成します。

未調整の10件では、既存 v64 が **995,156,057.5**、共有境界版が
**994,949,731.1** (差 −206,326.4、4勝6敗)。最大実測時間は旧4.815320秒・新4.686314秒。
開発・未調整の合計36出力すべてで、独立検査と公式採点が一致しました。
共有境界版は既定に採用せず、未調整側で優った既存 v64 を本番比較用の凍結候補とします。
次は、片方の軸の面だけを動かすのではなく、細長い領域内で複数の境界座標を
順序制約付きで同時に再配分する近傍を新しい開発セットで比較します。

## 比較実験2: min-cutによる一括境界移動

`-DAHC_CUT_SHIFT` のときの実験版です。各長方形の片方の軸にある全ての辺を
対象に、各辺を同じ整数変位だけ動かすかどうかを一括で選びます。直交方向に
重なる長方形対の位置関係を保ち、幅・枠・指定点の条件を守ります。
移動に伴う面積スコアを0/1のペアコストとして表現し、最小カットで候補を選びます。
サブモジュラでないペアを保守的に近似し、元の評価関数で正に改善する場合だけ
採用するため、任意の近似が改善を保証するとは仮定していません。
[汎用min-cutラッパー](../../library/binary-submodular-energy.hpp)は既存の
[max-flow.hpp](../../library/max-flow.hpp)を使っています。

```sh
g++ -std=c++23 -O3 problems/ahc001/main.cpp -o baseline
g++ -std=c++23 -O3 -DAHC_CUT_SHIFT problems/ahc001/main.cpp -o cut_shift
python3 scripts/bundle.py problems/ahc001/main.cpp -o submission.cpp
g++ -std=c++23 -O3 -DAHC_CUT_SHIFT -DAHC_DEFAULT_SECONDS=4.8 submission.cpp -o submission
g++ -std=c++23 -O2 tests/binary_submodular_energy_test.cpp -o energy_test
./energy_test
for set in dev holdout; do
  mkdir -p "mincut-$set"
  (cd "mincut-$set" && ../TOOLS_DIR/target/release/gen < "../problems/ahc001/mincut-$set-seeds.txt")
  python3 problems/ahc001/benchmark.py "mincut-$set/in" "bench-mincut-$set" --time 4.8 --entry baseline=baseline --entry cut_shift=cut_shift --vis TOOLS_DIR/target/release/vis
done
```

開発・未調整確認ではそれぞれ新規12件を、公式公開本番1,000件、公式ツール
同梱50件および旧実験の未調整10件から分けて使用します。公開本番ケース別の
採点結果はこの版の調整に使用しません。凍結v64の本番1,000件の再計測は
**993,411,282,631**（全件有効、最大4.83806秒）で、比較基準
**993,823,071,683** より **411,789,052** 点低く、1位超えは未達です。

別実験の `-DAHC_CUT_SHIFT_HEADROOM` では、焼きなましの予算を削らず、
実行可能な余裕の中だけで min-cut 仕上げを実行します。長方形の更新前と後を
同一の実行内で比較し、乱数・実測時間による別軌道との混同を避けます。
4.8秒で探索を終える条件では min-cut の新規呼び出しを開始時刻+4.87秒で打ち切り、
外側でもプロセスを5秒で打ち切ります。min-cut 一回の実行時間は変動するため、
内側の締切だけで全入力の5秒未満を保証するものではありません。
このモードに限り `pre_cut_score` を標準エラーから読み、公式得点との差を記録します。

```sh
g++ -std=c++23 -O3 -DAHC_CUT_SHIFT_HEADROOM -DAHC_DEFAULT_SECONDS=4.8 submission.cpp -o headroom
mkdir -p mincut-headroom
(cd mincut-headroom && ../TOOLS_DIR/target/release/gen < ../problems/ahc001/mincut-headroom-seeds.txt)
python3 problems/ahc001/benchmark.py mincut-headroom/in bench-mincut-headroom --time 4.8 --entry headroom=headroom --vis TOOLS_DIR/target/release/vis
```

| min-cut v1（4.8秒の中で仕上げ時間を確保） | v64平均 | min-cut平均 | 新版勝ち | 最大実測 旧 / 新 |
|---|---:|---:|---:|---:|
| 新規開発12件 | 992,146,747.17 | 992,021,790.58 | 4/12 | 4.817154 / 4.698933秒 |
| 新規未調整12件 | 993,659,948.42 | 993,503,229.58 | 6/12 | 4.816588 / 4.704133秒 |

合計48出力すべてで公式採点と独立検査が一致。仕上げ自体は全24ケースで
改善（開発で平均56,573.42点、未調整で平均68,239.92点）しましたが、前段を短く
した版全体の平均はそれぞれ124,956.58点、156,718.83点悪化しました。
別軌道の焼きなましとの差を仕上げ自体に帰属させず、既定版には採用しません。

4.8秒の前段を維持した版では、別の8件全てで合法・公式採点一致、改善は8/8件。
同一実行内の仕上げ前後差は平均 **61,350.38** 点
（9,159～171,399点）、提出形式の1ファイルで最大 **4.815982秒**。
1,000件の本番平均差にすると約411,789点/件が必要であり、この試験だけで
1位超えとはしません。両モードを任意指定のまま保存します。

次に試す構造変更は「詰まっている長方形の直交側を一時的に縮め、
真横・上下の間の順序制約を切り替え、両軸のmin-cutをかけた後で再拡張」です。
既存の順序を固定したままの移動や探索時間の微調整とは別の候補で、
試行全体を公式スコアで採否判定します。

単一ブロッカー縮小の試行は、[独立の比較診断](experiments/topology_v1/README.md)
に再現コードと結果を保存しました。開発8件で平均+511.25点に留まり、不採用です。
