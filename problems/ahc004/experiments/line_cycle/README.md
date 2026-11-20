# AHC004: 既存の横・縦の一致を保ったまま行/列を並べ替える

Base: `cf70c44eeb113528f102740a73fb5345c1a2dbb7`。
従来の語上書きだけでは行全体を動かしにくいため、一行の循環回転と、
二行（または二列）の交換を追加しました。回転したラインの出現語の多重集合は
不変で、交換した二本の多重集合は入れ替わるだけです。従って、平行側は
CoverageCounter を更新せず、直交側だけを再走査します。節約した走査とは別に、
後の上書き候補評価に使う符号化済みラインは平行側でも更新します。

この部品は [本体](../../main.cpp) の差分評価と
[ランダム差分テスト](../../../../tests/ahc004_line_cycle_test.cpp) に保存しました。
70盤面×100回で回転、交換、空白化を混ぜ、全再計算した多重集合、覆い数、
空白数、符号化と照合しました。一部では文字列を折り返し付きで直接照合しました。

## 追加105 ms、同じ開始盤面同士の比較

公開 seed 0–99 と重ならない、別途固定した8シードを公式 gen で生成。
先の実験の採用盤面を全ケースで一度だけ保存し、両群は各ケースで同じ盤面から開始。
短い追い焼きなましの温度・時間をそろえ、比較群は既存の語上書きのみ、
候補群は回転/交換と語上書きをほぼ半々にしました。個別ケースに対応する分岐はありません。

- 候補群は3勝5分、平均 **+140,634.375点**。改善3件は +155,279、+132,275、+837,521。
- 開始8盤面と両比較群の計24出力が合法、公式採点と独立検査一致。
- 初期計算と追探索を足した実測の最大は **2.894019秒**。
  実験は105 msの壁時計制限と190 msの外側タイムアウトを使用。
- 8件の小さい診断に限った改善。通常の `main.cpp` の探索方針は変えません。
  本番順位については判定しません。

次に固めるなら、改善が少数へ偏った理由を追うため、直交側で不足している語を
増やせる可能性がある行だけ回転量を全候補比較してから抽選する方式と、
修正を加えないこのランダム版を別の未調整ケースで比較します。

## 再現

`TOOLS_DIR` は公式配布の gen/vis を `cargo build --release --locked` でビルドした先です。
コマンドはリポジトリルートから逐次実行します。必要なら `taskset` を使ってCPUを分けます。

```sh
E=problems/ahc004/experiments/line_cycle
W=$(mktemp -d)
T=$(cd "${TOOLS_DIR:?}" && pwd)
S=$(cd "$E" && pwd)
mkdir -p "$W/input"
(cd "$W/input" && "$T/target/release/gen" "$S/seeds.txt")
g++ -std=c++23 -O3 problems/ahc004/main.cpp -o "$W/selected"
g++ -std=c++23 -O3 "$E/compare.cpp" -o "$W/compare"
g++ -std=c++23 -O2 tests/ahc004_line_cycle_test.cpp -o "$W/differential"
"$W/differential"
python3 problems/ahc004/benchmark.py "$W/input/in" "$W/starts" --vis "$T/target/release/vis" --entry "selected=$W/selected"
python3 "$E/run.py" "$W/input/in" "$W/starts" "$W/compare" "$T/target/release/vis" "$W/checked"
```

`$W/checked/results.csv` に結果が出ます。壁時計停止のため環境で値は変動します。
