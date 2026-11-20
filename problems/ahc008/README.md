# AHC008: Territory

- [公式問題文](https://atcoder.jp/contests/ahc008/tasks/ahc008_a) / [公式順位表](https://atcoder.jp/contests/ahc008/standings) / [公式テスタ・ジェネレータ v3](https://img.atcoder.jp/ahc008/tools_v3.zip)
- 本番最終1位 **121,980,050,607**。[ALE-Bench の AtCoder 提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc008.zip) の `ahc008//standings_scores.csv` で確認。アーカイブの SHA-256 は公開 LFS ポインタ `7dc040eb07f4543a16ef550a198859519a98af899771237b2bf1517c8a419300` と一致。順位・得点のみ参照。
- 各人の `到達可能面積 / 900 / 2^到達可能ペット数` を平均し、`1e8` を掛けてケースごとに丸め、システムテスト2000件の合計を最大化する。300ターンの対話形式、3秒、1024 MiB。不正出力や時間超過のシステムケースは0点。
- 公式が [システムseed](https://img.atcoder.jp/ahc008/seeds.txt)（SHA-256 `33973e4ded08e3a607fc2e841e14751ff110ae10154b286e7fd5f766ff86d706`）を公開しているが、現段階では実行・参照していない。本番比較は **未判定**。

## 構築と実験

同一の第1フェンス（1始まり10行目、中央1マスに通路）を作る。壁候補は全員・全ペットのターン開始時占有を参照し、同ターンの建設予定と移動先を相互に除外する。全員が保持側へ移動済みで、現在のペット配置なら得点が厳密に上がる時だけ通路を閉める。

対照は1本で終了。適応版は、保持領域にまだ動物がいれば、分離できそうな頭数・保持面積・壁の長さを見て、次の長方形分割を選ぶ。累積的な行動制約と塀の占有は両方同じ。既存の解答はなかった。

| 分割 | ペア | 1本版の平均 | 適応版の平均 | 適応版の勝 |
| --- | ---: | ---: | ---: | ---: |
| dev | 24 | 252,872.04 | 3,298,934.42 | 24 |
| holdout | 24 | 68,770.42 | 2,235,677.13 | 24 |

[計測CSV](benchmark-results.csv)の全96実行で、公式テスタと中継ログの独立再生が一致。独立再生では300ターンすべての合法性、移動・建設の同時性、最終到達領域とペット数、丸めを確認した。テスタ直呼び最長0.049386秒（中継付き最長0.090556秒）。入力seedは分割ごとのSHA-256から独立生成し、両群の実装を先に固定した。ブランチ既定は適応版（対照は `-DAHC008_ADAPTIVE=0`）。保持面積はholdout平均135.29/900、同領域に残ったペットは平均3.375匹で、勝った対照も弱く、1位の本番平均約6099万点に近いとは言えない。

### ライブラリ

候補マスへの距離と最初の1歩だけが必要だった。既存 `grid_bfs` では再探索ごとの行別ベクタ確保と、移動時の経路復元が不便だったため、平坦添字で作業メモリを使い回す [FlatGridFirstStepBfs](../../library/flat-grid-first-step-bfs.hpp) を追加した。[短い例](../../docs/LIBRARY_REFERENCE.md#平坦グリッドで最初の一歩を引くbfs)・[4,000回の差分テスト](../../tests/flat_grid_first_step_bfs_test.cpp)。

### 次の仮説

順次分割は、ペットを数匹減らすたびに保持領域を大きく失う。端から複数の小さな部屋を並行して作り、内部に動物が入り、入口とその周囲から離れた瞬間に扉を閉める方式なら、隔離した頭数に対する面積損失を抑えられる。保持面積、捕獲匹数、無ペットケース率を別々に記録する。

## 再現

```sh
tools_dir=/path/to/extracted/tools
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc008/main.cpp -o "$work/submission.cpp"
g++ -std=c++23 -O3 -DAHC008_ADAPTIVE=0 "$work/submission.cpp" -o "$work/base"
g++ -std=c++23 -O3 "$work/submission.cpp" -o "$work/adaptive"
python problems/ahc008/benchmark.py "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" "$work/base" "$work/adaptive" "$work/results.csv"
g++ -std=c++23 -O3 tests/flat_grid_first_step_bfs_test.cpp -o "$work/bfs-test"
"$work/bfs-test"
```
