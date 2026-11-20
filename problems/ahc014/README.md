# AHC014: RectJoin

- [公式問題文](https://atcoder.jp/contests/ahc014/tasks/ahc014_a) / [公式順位表](https://atcoder.jp/contests/ahc014/standings) / [公式ジェネレータ・採点器](https://img.atcoder.jp/ahc014/a3c240f5b1.zip)
- 本番最終1位 **3,122,399,998**。[ALE-Bench の AtCoder 提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc014.zip) 内の `ahc014//standings_scores.csv` で確認。SHA-256 `43624bb9c24795ef1e792bace66bfeb59787265b025750b8708bee692c2e5021` が公開 LFS ポインタと一致した。順位と得点のみ参照。
- 最終盤面の印の重み合計に、`10^6 × N² / (M × 全格子点の重み合計)` を掛けて丸め、2000システムケースを合計して最大化する。各奇数サイズ31〜61は125ケースずつ。制限5秒、1024MiB。
- [本番システムseed](https://img.atcoder.jp/ahc014/seeds.txt)（SHA-256 `907b41fcba240515612a21798a10b0df7dda744b1268b74b3bbd41b93a73095e`）は公開されているが、探索中は開かず、現段階では評価せず。本番超え **未判定**。

## 解法

既存解がなかったため細かい合法手の貪欲から始めた。追加する点の対角頂点を起点に、直角をなす2方向の最近傍の印を候補にする。最も手前の印以外は、途中に別の印があるため使えない。残り2辺に印がないこと、4辺すべてが既存の辺と正の長さで重ならないことを検証する。既存辺と点で交わるだけの候補は除外しない。

貪欲順は外側の点の重みを好み、長方形の辺を長く使うほど優先度を下げる。係数と乱数を変えて残る解の重みを比較した。対照版は毎回初期盤面から開始。再成長版は4回に1回の全リスタートを残し、その他は現時点のベスト手順の途中まで再生してからランダムな貪欲で作り直す。**盤面を巻き戻すだけでなく、枝選択を再探索する**ことで、良い初動を捨てずに末端の行き止まりを変えられるかを試した。時間予算は両方4,550ms。

[SquareLatticeLineBits](../../library/square-lattice-line-bits.hpp)を追加。格子内の最近傍と重複を毎回1マスずつ走査すると候補再列挙が重い。行・列・上下対角を64-bitマスクとして共有し、印の最近傍と辺の正長重複（端点のみは除外）を定数時間で求める。点と単位辺を別インスタンスに分けた。[使用例](../../docs/LIBRARY_REFERENCE.md#正方格子の8方向の点区間)・[差分テスト](../../tests/square_lattice_line_bits_test.cpp)。

## 結果

| 分割 | 件数 | 全再開の平均 | 再成長の平均 | 再成長の勝 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 6 | 788,064.17 | 810,083.17 | 5 / 1 |
| holdout | 6 | 862,623.67 | 880,722.83 | 4 / 2 |

全24出力が合法で公式採点と独立再生が一致した。最大壁時計4.564538秒。両分割で平均・中央値が増えたので、再成長を問題ブランチの既定とした。ただしholdoutには約9.3万点悪化したケースもあり、少数・別seedでの比較である。本番1位の平均156万点には大きな差がある。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc014/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC014_STATS -DAHC014_REWIND=0 "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 -DAHC014_STATS -DAHC014_REWIND=1 "$work/submission.cpp" -o "$work/regrowth"
python problems/ahc014/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/base" "$work/regrowth" "$work/results.csv"
g++ -std=c++23 -O3 tests/square_lattice_line_bits_test.cpp -o "$work/line-bits-test"
"$work/line-bits-test"
```

dev/holdoutは別系列のSHA-256による公式ジェネレータの各6件。両版の設定は先に固定し、実行順はペアごとに交互。[計測CSV](benchmark-results.csv)参照。公式visだけでなく、独立な単位辺集合で手順の全操作を再生し、重みと得点を一致確認した。

### 次に試す仮説

末尾全体ではなく、中盤の長方形を1〜3個選んで除外し、その新規頂点を前提とする後続操作だけ連鎖的に除外する。残りの操作は元の順番で再生し、成分的に独立した外周の成果を保存しつつ、取り除いた辺の周りを再成長させる。同じ凍結解から「約同じ個数を削るprefix」と比較し、除外後合法性・改善量・外側への増加を検証する。

別の案として、短い辺の貪欲は中心から外に向かう希少な伸び道を先に潰す。新しい印が可能にする長方形だけを抽出し、追加手の重みだけでなく次に作れる外側への点の重み・その方向でまだ使っていない辺を評価する。候補上位だけに一手先読みを適用し、同じ開始盤面と時間予算で、総点・外周4分の1に増えた印数・最終手数を比較する。
