# AHC020: Broadcasting

- [公式問題文](https://atcoder.jp/contests/ahc020/tasks/ahc020_a)・[公式順位表](https://atcoder.jp/contests/ahc020/standings)・[公式生成器・採点器](https://img.atcoder.jp/ahc020/db611066.zip)
- 本番最終1位 **504,832,157**。[AtCoder提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc020.zip)の `ahc020//standings_scores.csv`。SHA-256 `5cdc8b811a4c6062f059beda557c49df3458948c979d2ef65eb69ccd75600866` が公開LFSと一致。順位・得点のみ参照。
- 全員受信なら `round(10^6 × (1 + 10^8 / (S+10^7)))`、未達なら `round(10^6 × (受信人数+1)/K)`。絶対点を本番300件で合計し最大化。2秒、1024MiB。ローカル同梱seedは100件のみで本番との同一性は確認できず、終了後の再評価もない。1位超えは**未判定**。

## 構築

最寄りの局に各住民を割り当て、全員を覆う半径から開始。全局MSTで発信局と根だけを結ぶ枝を残す。半径を縮め、受信しなくなった住民を別局で補い、出力二乗費とMST枝費を合わせてSAする。局ごとに必要半径で住民を並べ、変更で横切る範囲だけ被覆カウントを更新する。

比較版は**同じ半径**を固定後、現在の配線のどの頂点からでも最短の未接続発信局までを追加する。全端点を開始点として試し、枝刈りMSTも候補に残して最小費用の配線を採る。既定は比較版、`-DAHC020_STEINER=0` は対照。

既存DSUはMSTでそのまま使用。評価毎の端点→根の巡回は [根付き端点部分木](../../library/rooted-terminal-tree.hpp) に分け、逆順一回で必要辺を判定する。汎用CoverageCounterの差分バッファと異なり、修復途中でも実カウントを参照しつつ半径の区間を増減するため、被覆には問題固有の更新を用いた。[木](../../tests/rooted_terminal_tree_test.cpp)・[半径](../../tests/ahc020_radius_updates_test.cpp)の差分テスト。

| 分割 | 件数 | MST枝刈り平均 | 再配線平均 | 再配線の勝 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 16 | 1,649,383.63 | 1,663,664.94 | 16 / 0 |
| holdout | 16 | 1,674,784.38 | 1,687,384.19 | 16 / 0 |

[計測CSV](benchmark-results.csv)の64出力すべて合法で全員受信し、公式visと独立整数採点が一致。最大壁時計0.026912秒は両版の生成込み。holdout平均は本番1位の300件平均より高いが、異なる入力なので厳密な順位は不明。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc020/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC020_DIAGNOSTIC "$work/submission.cpp" -o "$work/diagnostic"
python problems/ahc020/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/diagnostic" "$work/results.csv"
for test in rooted_terminal_tree ahc020_radius_updates; do
  g++ -std=c++23 -O3 "tests/${test}_test.cpp" -o "$work/test"
  "$work/test"
done
```

dev/holdoutは別のSHA-256系列から公式生成。診断版は半径1行と配線2種を出し、それぞれ採点する。次は半径のSA中にも発信局ビットマスクごとの再接続費をキャッシュして使い、同一初期解・同一試行数でMST枝費を使う版と比べる。発信局の違い・出力二乗費・配線費を別々に記録する。
