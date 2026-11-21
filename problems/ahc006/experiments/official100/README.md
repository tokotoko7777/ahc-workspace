# AHC006: 公式配布100ケースでの比較

[公式問題文](https://atcoder.jp/contests/ahc006/tasks/ahc006_a) / [公式ツール・配布100件](https://img.atcoder.jp/ahc006/c21daebb77aa4d38d65f4d7f7c7249.zip)。
公式配布 seed 0–99 の100件の各整数得点を合計。2,220,425点を超えることが目標。
本番と別入力なので、本番1位超えの判定ではない。

通常設定の既存版（公開ソース `0e7380c86bf2f895cc47b0f44cd976f7bb01e9cd`）は **2,078,408点**、
目標を厳密に超えるにはあと142,018点。[ケース別結果](baseline-results.csv)、[測定時の単一ファイル](baseline.single.cpp)。
全100件合法・公式と独立採点一致。最長実測1.857923秒、中央値1.853609秒。
総走行距離381,574、ケース中央値3,815。
ソース SHA-256 `7ad27e14ea732bc4eb644e0a27acd75b2f4431fe2217447a8da21d76ae50ab28`、
CSV SHA-256 `92382ca3416a27203cbad7ba9b969ae5870881cac7edb8c93b8c2bab88a247a6`。

## 再現手順

公式ZIPを展開したディレクトリで `cargo build --release --locked`。リポジトリのルートで
`TOOLS` に公式ツールのディレクトリを設定して実行する。GNU C++23、追加最適化フラグは不要。出力は直列に検証する。

```sh
E=problems/ahc006/experiments/official100
g++ -std=c++23 "$E/baseline.single.cpp" -o solver
python3 "$E/run100.py" . "$TOOLS/in" "$TOOLS/target/release/vis" out --entry "baseline=$PWD/solver"
```

時計で探索を終えるため、マシンや負荷の違いで探索回数・得点は変わり得る。

## 注文入替の挿入位置の高速化

対称距離表で新規集荷・配達点への距離を行単位で参照し、前の位置で読んだ距離を再利用。
合成2,048候補で距離差分と2つの挿入位置が従来版と一致。
固定作業を交互6回計測し、挿入走査だけで約4.7倍。近傍の定義や同点規則は変更していない。

公式配布100件では [**2,128,693点**](pair-scan-results.csv)（+50,285、74勝0分26敗）、
総距離370,198、全件合法・両採点一致、最長1.881348秒。
[単一ファイル](pair-scan.single.cpp)は GNU C++23 以外の追加最適化フラグなし。
ソース SHA-256 `7963739a08141a1407c776d781a04d0023e7a2e921fd815c5eba96c35d8653a2`。
