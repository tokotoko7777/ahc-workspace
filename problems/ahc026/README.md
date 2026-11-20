# AHC026: Stack of Boxes

- [公式問題文](https://atcoder.jp/contests/ahc026/tasks/ahc026_a)・[公式順位表](https://atcoder.jp/contests/ahc026/standings)・[公式生成器・採点器](https://img.atcoder.jp/ahc026/lPQezTZx.zip)
- 本番最終1位 **1,423,183**。[AtCoder提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc026.zip)の `ahc026//standings_scores.csv`。SHA-256 `c9acb296ceff4948df2cb000459d8153af4ba8ddb60c987b565e19b628dd845a` は公開LFSと一致。順位と得点のみ参照。
- 移動は箱数+1の消費、正しい順の持ち出しは0。全箱を5,000操作以内で持ち出し `max(1, 10000−合計消費)` を本番150件で合計して最大化。2秒、1024MiB。公式ローカル同梱seedは100件で本番との同一性は未確認。コンテスト後の再評価もなく、1位超えは **未判定**。

## 比較

既存解 [決定的ロールアウト](../../examples/search/ahc026_deterministic_rollout.cpp) を変更せず比較した。毎回、次に必要な小さい箱を何番先まで個別に扱うか、最後まで仮実行して決める。

比較は、次の箱を持ち出すまでを一段とする幅50のビーム。取り出す箱の上をすべて一つに移す対照と、上部を二つの連続区間に分け、先に上の区間を移せる版を作った。2区間を同じ別の山に逆順で積むことも許す。現在消費と、下に小さい箱がある箱・そうした箱の連続区間への評価で候補を並べる。保管先のラベルだけが異なる同型状態は重複を省く。探索の締切は1,680ms、残りは合法な一括移動で完了する。

既存 [SimpleBeamSearch](../../library/simple-beam-search.hpp) のキー重複除外をそのまま使えた。候補で10本の動的配列を何度も複製するのを避けるため、箱列を連続配列にし、山の移動は区間回転で実装した。専用の状態表現なので汎用ヘッダには分けていない。[コンパクトな山配列を通常vectorと比較するテスト](../../tests/ahc026_stack_map_test.cpp)。

| 分割 | 件数 | 既存平均 | 一括ビーム平均 | 分割ビーム平均 | 分割の勝 / 負（対既存） |
| --- | ---: | ---: | ---: | ---: | ---: |
| dev | 16 | 9,335.13 | 9,129.75 | 9,283.94 | 2 / 14 |
| holdout | 16 | 9,344.63 | 9,143.81 | 9,273.06 | 2 / 14 |

分割は一括ビームに全32件で勝ち、未使用で平均+129.25点。ただし既存法には未使用で平均−71.56点なので、既定は既存法を変更せず使う。全96出力で公式採点とLIFO独立再生が一致。全ケースでビームは200段完了し、最大時間は既存0.372375秒、一括0.048316秒、分割0.139015秒。[計測CSV](benchmark-results.csv)。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc026/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 "$work/submission.cpp" -o "$work/old"
g++ -std=c++23 -O3 -DAHC026_BEAM=1 -DAHC026_STATS "$work/submission.cpp" -o "$work/bulk"
g++ -std=c++23 -O3 -DAHC026_BEAM=1 -DAHC026_STATS -DAHC026_SPLIT=1 "$work/submission.cpp" -o "$work/split"
python problems/ahc026/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/old" "$work/bulk" "$work/split" "$work/results.csv"
g++ -std=c++23 -O3 tests/ahc026_stack_map_test.cpp -o "$work/test"
"$work/test"
```

dev/holdoutは独立なSHA-256系列を公式生成器で変換し16件ずつ。次は既存解の途中状態に2区間移動を差し込み、その後を既存方策で最後まで計算して、局所評価に頼らず実消費が下がる場合だけ採る。保存した元解との比較で、どの段・どの形の分割が有効か確認する。
