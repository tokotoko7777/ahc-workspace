# AHC024: Topological Map

- [公式問題文](https://atcoder.jp/contests/ahc024/tasks/ahc024_a)・[公式順位表](https://atcoder.jp/contests/ahc024/standings)・[公式生成器・採点器](https://img.atcoder.jp/ahc024/AU5KcDyn.zip)
- 1位の本番最終得点 **335,417**。[AtCoder提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc024.zip) の `ahc024//standings_scores.csv`。アーカイブのSHA-256 `b4f02d3cbcea3200d3e7d29dcf27b1e0bcce1fb566ed249541683114f7986585` は公開LFSと一致し、順位と得点だけ使った。
- 外部を含む各色の連結性と色ペアの隣接関係を保つ。盤内0の個数+1の絶対値を本番150件で合計して最大化。2秒、1024MiB。配布入力は100件で本番150件と同一条件とは確認できず、終了後のシステム再評価もない。1位超えは**未判定**。

## 構築

対照は、まっすぐな行・列を削り、その後、外部につながる0の隣のセルを1マスずつ消す。どの操作も、外側に0を一周付けた盤面で全色の連結性と隣接ペア集合が完全に一致した場合だけ確定する。

比較版は、対照と同じまっすぐな削除の最終盤を複製し、行ごとに異なる列を選ぶ。前の行から左右に最大1だけずれられ、新しく禁止ペアが生じる列や遷移はDPから除外する。削除で希少な隣接を失う可能性を加点し、乱数を変えて合法な列を探す。候補の採否は全体再検査で決める。横方向は盤を転置して探す。最後に同じ1マス削除を適用し、対照より悪ければ対照を返す。

各行の可否と上下の縫い合わせ可否を連携したいが、普通のグラフ探索へ展開すると制約や経路復元の記述が重複するため、[遷移幅付き層DP](../../library/bounded-layer-shortest-path.hpp) に抽出した。[短い例](../../docs/LIBRARY_REFERENCE.md#層ごとの状態遷移で最短経路)・[1,200件の全列挙差分テスト](../../tests/bounded_layer_shortest_path_test.cpp)。

| 分割 | 件数 | 直線のみ平均 | 波状込み平均 | 波状込みの勝 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 24 | 1,588.25 | 2,004.71 | 24 / 0 |
| holdout | 24 | 1,621.42 | 2,025.63 | 24 / 0 |

全96出力で公式採点と独立の色グラフ・連結検証が一致、最大壁時計0.037359秒。holdoutで波状削除を平均17.25回採用した。問題ブランチの既定に選択。[個別CSV](benchmark-results.csv)。本番1位の150件平均約2,236点と比べるには公式の同じケースが必要。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc024/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC024_DIAGNOSTIC -DAHC024_RESTARTS=0 "$work/submission.cpp" -o "$work/diagnostic"
python problems/ahc024/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/diagnostic" "$work/results.csv"
g++ -std=c++23 -O3 tests/bounded_layer_shortest_path_test.cpp -o "$work/test"
"$work/test"
```

各24件は独立のSHA-256系列から公式生成。診断版は凍結盤面からの対照をstderrに、比較解をstdoutに出し、個々に採点する。

### 次の仮説

失敗する切断の大半が希少な色ペアの唯一の接触や、色の細い連結箇所を切るなら、DP前に関節点や接触の証拠辺を保護することで、同じ時間で有望な経路を増やせる。さらに向き・初期縮小順の違う再スタートを複数比較する。合法候補率と1手で抜けた面積を、同一開始盤で基準と比較する。

## 追試: 波状削除の開始盤を変える

両版は同じ既定の1回目を保存してから、乱数列を変えた波状削除を追加で10回まで試す。対照 `-DAHC024_RESTARTS=1` は毎回同じ直線削除後の盤面から始める。比較版 `-DAHC024_RESTARTS=2` は初期盤に戻り、直線削除の候補順をシャッフルしてから同じ波状削除を行う。波状部分には各開始ごとに共通の乱数初期値を与える。どちらも1,810ms締切、10回上限。同数の試行で、盤のサイズや途中状態の違いにより実時間は異なる。

| 分割 | 件数 | 同じ開始盤の平均 | 多様な開始盤の平均 | 多様化の勝 / 分 / 負 |
| --- | ---: | ---: | ---: | ---: |
| dev | 24 | 2,052.25 | 2,061.17 | 14 / 2 / 8 |
| holdout | 24 | 2,048.33 | 2,065.79 | 18 / 1 / 5 |

全ペアで1回目の得点が同一で、どちらも10追加試行を完了した。holdoutで開始盤の内容は固定版1種類、多様版平均9.96種類、得点差中央値+18.5点。[追試CSV](restart-results.csv)の全96出力で公式採点と独立検証が一致し、最長0.327639秒。問題ブランチの既定は多様版とする。`-DAHC024_RESTARTS=0` で初回の単発版に戻せる。

```sh
# 上の再現手順で tools_dir と work を設定
g++ -std=c++23 -O3 -DAHC024_RESTART_STATS -DAHC024_RESTARTS=1 "$work/submission.cpp" -o "$work/same"
g++ -std=c++23 -O3 -DAHC024_RESTART_STATS -DAHC024_RESTARTS=2 "$work/submission.cpp" -o "$work/diverse"
python problems/ahc024/restart-benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/vis" "$work/same" "$work/diverse" "$work/restarts.csv"
g++ -std=c++23 -O3 tests/ahc024_restart_test.cpp -o "$work/test"
"$work/test"
```

本番1位の平均約2,236点との差に対し、再スタートで得られた追加改善は小さい。次は外部を含む希少な色ペアの接触辺を候補ごとの「証拠」とし、DPで切断を避ける。個々の色の関節点・接触を把握し、全検証で拒否された理由（接触消失・新規接触・非連結）を先に分解して、最も多い原因を保護する。
