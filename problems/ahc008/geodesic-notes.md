# 別案: geodesic enclosure

本番システム seed・参加者解説・提出は参照していない。既定ソルバーは変更していない。

15-300 マス程度の長方形候補から、現在の壁を再利用して、工事当初のペット配置から予測した捕獲しやすい区画を選ぶ。もう閉鎖済みの領域は再対象にしない。ペットごとに現状の壁を考慮する BFS を行い、距離減衰で予測質量を配置する。犬は誘導なしの閉じ込めが難しいので重みを落とした。予測質量・まだ開いている面積・境界建設費の各累積和を使い、数万形状を比較する。人間には近傍の工事・待機位置を順次割り当て、区画内の人を外へ逃がした上で扉を閉鎖する。建設が停滞したらそれまでの壁は残し、形状を再選定する。

| 開発30件 | 合計 | 平均 |
| --- | ---: | ---: |
| 既存の適応分割 | 59,250,211 | 1,975,007 |
| 内側2帯の固定小部屋 | 10,492,133 | 349,738 |
| ユークリッド近傍予測の動的長方形 | 98,520,753 | 3,284,025 |
| 壁込み距離予測の動的長方形・停滞時の再選定なし | 127,734,058 | 4,257,802 |
| 壁込み距離予測の動的長方形・14ターン停滞で再選定 | 144,737,195 | 4,824,573 |
| 壁込み予測＋単マス関節点の即時閉鎖（別設定） | 116,715,659 | 3,890,522 |
| 一扉の境界犬誘導→動的長方形 | 81,897,981 | 2,729,933 |
| 二扉の中央犬誘導→動的長方形 | 74,646,866 | 2,488,229 |

上の開発条件から基本版を固定し、未使用100件での公式テスタ合計は 513,452,317（対照 271,612,416、70勝30敗）。50,000,000以上のケースは両版0。対照からは改善したが、6,263,333,336 の100件目標とは大差があり、既定に採用しない。テスタ込み実時間の最大観測値は 0.256 秒（並行作業もある環境で測定）。

再現:
```sh
tools_dir=/path/to/extracted/tools
work=$(mktemp -d)
python scripts/bundle.py problems/ahc008/main.cpp -o "$work/base.cpp"
g++ -std=c++23 -O3 "$work/base.cpp" -o "$work/base"
g++ -std=c++23 problems/ahc008/geodesic-enclosure.cpp -o "$work/geodesic"
python problems/ahc008/benchmark.py --tag geodesic-v1 --candidate-label geodesic "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" "$work/base" "$work/geodesic" "$work/geodesic-replayed.csv"
```

独立100件計測の seed は各ケース番号 `i=0..99` に対して `int.from_bytes(sha256(f"ahc008/habitat/1/partition_verify/{i}".encode()).digest()[:8], 'little')`。入力生成に公式ジェネレータ、採点に公式テスタを使用。独立再生の48ペアは別系列。詳細は [同時比較のCSV](geodesic-fresh100.csv) と [独立再生のCSV](geodesic-replayed.csv)。
