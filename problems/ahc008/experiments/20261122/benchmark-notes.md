# AHC008: 公式配布100、採点器と頂点除去の追試

公式 v3 読み戻し全700実行（ペア200 + ペア200 + 3案300）が合法。直呼びで得た
行動・スコアを公式 v3 テスタ中継と別実装の再生でも照合した。

| ケース群 | 既存・適応分割 | 外周小部屋 | 終盤で置換 | 隣接閉鎖を優先 |
| --- | ---: | ---: | ---: | ---: |
| 公式配布100 | 259,408,028 | 75,383,487 | 未実施 | 未実施 |
| 新規開発100 | 265,848,086 | 未実施 | 262,214,623 | 未実施 |
| 別の新規100 | 266,733,939 | 未実施 | 264,234,372 | 258,225,474 |

配布100で適応分割が全員をペットのいない領域に置けたのは7ケース。該当する人間の
平均到達面積は114.92/900。外周小部屋の人間平均到達面積は593.26/900だが、
平均8.26匹が残り、ペットなしは0ケース。二つとも目標6,263,333,336より大幅に低く、
配布100がリンク先の提出の採点ケースと同じとは未確認。

関節点閉鎖は分離後の正確な得点が上がる候補を列挙したものの、先の有望な仕切りを止め、
期待より小さい領域へ固定する副作用があった。両方不採用。汎用の
`VertexRemovalComponents` は4,000個の乱択グラフで全塗り直しと一致。別の候補生成に
転用可能。

公式 v3 は `( 1e8 * sum(area / 900.0 * 2^-pets) / humans ).round()` を binary64 で
求める。配布ケース0018の小部屋版は数学上の有理数丸め460,938に対し公式は460,937。
独立採点は座標・連結成分を個別に再生したあと、加算順と二進丸めを公式に合わせる。

## 再現
```sh
python scripts/bundle.py problems/ahc008/main.cpp -o /tmp/ahc008.cpp
g++ -std=c++23 /tmp/ahc008.cpp -o /tmp/ahc008-adaptive
g++ -std=c++23 -DAHC008_PARALLEL_PENS /tmp/ahc008.cpp -o /tmp/ahc008-pens
g++ -std=c++23 -DAHC008_EXACT_ENDGAME -DAHC008_EXACT_REPLACE /tmp/ahc008.cpp -o /tmp/ahc008-replace
g++ -std=c++23 -DAHC008_EXACT_ENDGAME /tmp/ahc008.cpp -o /tmp/ahc008-hybrid
python problems/ahc008/benchmark_cases.py "$tools/target/release/tester" "$tools/in" \
  /tmp/ahc008-official100.csv adaptive=/tmp/ahc008-adaptive pens=/tmp/ahc008-pens
g++ -std=c++23 -O3 tests/vertex_removal_components_test.cpp -o /tmp/ahc008-cut-test
/tmp/ahc008-cut-test
```

出力の最大実測は0.128秒未満（公式テスタ含む同一環境の逐次計測、他プロセスの負荷は残る）。
時間の厳密な本番比較は未実施。

### 連結成分ルーチンの速度確認

30×30、250盤面、平均約52候補/盤面。全候補について除去後の各成分のサイズと
重みの集約チェックサムが一致。3回の反復で前処理込みの中央値0.03788秒、
候補ごとのBFSは0.54360秒で、この条件では約14.4倍。ソルバー全体の高速化ではない。

```sh
g++ -std=c++23 problems/ahc008/experiments/20261122/vertex-removal-microbench.cpp -o /tmp/ahc008-cut-bench
/tmp/ahc008-cut-bench
```
