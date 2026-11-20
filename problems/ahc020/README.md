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

## 追試: 半径のSAにも再配線費を入れる

`-DAHC020_JOINT=1` は近傍の評価時に、仮の発信局を端点とした再配線を実際に作り、選んだ辺の総額を使う。これは**生成した配線の正確な費用**であり、全Steiner木の厳密最適値ではない。端点集合を2つの64bitに保存し、活動局が変わらなければ同じ費用を再利用する。対照と候補は、同一の最寄り局初期解・乱数初期値・12,000試行上限・SA締切1,770ms、終了後に同じ再配線を使う。実際に両群すべて12,000反復を完了し、異なる受理により訪れる局集合は変わった。

| 分割 | 件数 | 後処理だけの平均 | 共同評価の平均 | 共同評価の勝 / 負 | 改善の中央値 |
| --- | ---: | ---: | ---: | ---: | ---: |
| dev | 16 | 1,662,880.00 | 1,669,044.13 | 7 / 9 | -706 |
| holdout | 16 | 1,654,711.81 | 1,665,150.44 | 12 / 4 | +10,253 |

[追試CSV](joint-results.csv)は初回と異なるdev/holdoutの公式生成seed。全64出力が合法で全員受信し、公式採点と独立再生が一致した。最長0.340581秒。holdoutで有効近傍平均3,859回のうち、端点集合の未キャッシュは平均1,010回。活動局はholdout全16件で変わり、配線費は平均35,633,895→35,043,396、出力費は平均108,478,224→106,613,160だった。一方、devでは半数以上が悪化したため、通常は後処理だけ、共同評価は任意フラグのままにした。

```sh
# 上の再現手順で tools_dir と work を設定
python scripts/bundle.py problems/ahc020/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC020_STATS "$work/submission.cpp" -o "$work/after"
g++ -std=c++23 -O3 -DAHC020_STATS -DAHC020_JOINT=1 "$work/submission.cpp" -o "$work/joint"
python problems/ahc020/joint-benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/after" "$work/joint" "$work/joint.csv"
g++ -std=c++23 -O3 tests/ahc020_joint_network_test.cpp -o "$work/network-test"
"$work/network-test"
```

[差分テスト](../../tests/ahc020_joint_network_test.cpp)では80ランダムグラフ×20端点集合で、キャッシュ費用と実配線の総額・活動局の連結性を比較し、半径だけ変えたときにキャッシュが利用されることを確認した。次は時間余裕を使って両探索を走らせ、実際により安い解だけを残す。追加の探索を同じ長さの再スタートだけに使う場合とも、等時間で比較する。
