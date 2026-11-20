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
