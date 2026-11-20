# AHC005: Patrolling

[公式問題文](https://atcoder.jp/contests/ahc005/tasks/ahc005_a) /
[最終順位表](https://atcoder.jp/contests/ahc005/standings) /
[順位・得点アーカイブ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc005.zip)。

本番1位は **22,318,770**（アーカイブ内 `ahc005//standings_scores.csv`）。
アーカイブのSHA-256は `2d223bee36c1499c98f1e9e927766bbf135f5d8a7f4e05890b4df044d790b484`。
3秒・1,024 MiB、完全可視のときの得点は round(10000 + 10^7 × N / 移動時間)、
100件の丸め済み絶対得点を合計して最大化します。配布ローカル seed 0–99 は
本番と異なり、後採点もないと明記。本番100件を確認できないので1位超えは未判定です。

## 通過経路の部分置換（不採用）

既存の [独自実装](../../examples/search/ahc005_patrol_sa.cpp) は代表点の順序を
探索し、最後に不要な代表点を取り除きます。新実験は展開済みの道路巡回に対し、
任意の歩行位置を境界にループを削除したり、その間を短い経路で置換します。
取った区間から得ていた視界を差し引き、挿入経路で補った後、道路の完全被覆と
移動時間の減少を満たした場合だけ採用します。道路1マスを水平・垂直方向の
連続区間同士の辺とみなし、両端の少なくとも一方の活動数が正なら可視です。

汎用の [二部グラフの辺被覆カウンタ](../../library/bipartite-edge-cover-counter.hpp)
を追加。既存の CoverageCounter に各視点が見る全マスを渡すと、同じ区間を
繰り返し通るたびにマス列を扱う必要があるため、区間の通過数と境界越えを
別に管理しました。[使用例](../../docs/LIBRARY_REFERENCE.md#二部グラフの辺被覆カウンタ) /
[差分と巻き戻しテスト](../../tests/bipartite_edge_cover_counter_test.cpp)。

| ローカル | 既存平均 | 候補平均 | 差 | 候補勝ち |
|---|---:|---:|---:|---:|
| 開発 seed 0–7 | 235,610.875 | 236,128.500 | +517.625 | 2/8 |
| 未調整 seed 60–67 | 220,145.000 | 218,851.500 | −1,293.500 | 2/8 |

32出力すべて完全可視・合法で、公式採点と独立照合が一致。最大実測2.638180秒。
比較する2本は時間で探索するため前段の巡回が異なり、それによって平均点に差が出ています。
同一候補実行内で置換前後の移動時間を比べると、**16ケース全件で変化なし**。
この置換は通常ビルドで有効にしません（試す場合は `-DAHC005_SPLICE`）。

## 視界の最小辺被覆診断

水平・垂直区間を頂点、道路マスを接続辺とします。長さ1の直交区間を持つマスは、
その直交区間を訪ねても長い側の区間まで同時に訪れるため、長い側を必ず訪ねます。
開始点で両区間も訪問済みです。[診断コード](segment_cover_diagnostic.py) は、
強制区間の辺を除き、残る二部グラフの最大マッチングから追加必要数を求めます。
両評価集合の16/16件で、全長区間訪問に比べて削れる区間数は0でした。
この範囲では、区間を選び直すだけで得られる改善の根拠はありません。

次の候補は区間の義務を消すのではなく、**探索中に代表点の有無を変え、
区間をつなぐ最短路上の視界まで差分に入れる**方式です。既存のように順序を
固定してから削るだけでなく、順序変更と代表点削除を同じ状態にします。

## 再現

Input と tools は公式問題文から取得し、 tools は `cargo build --release --locked`。
配布物・生成物はコミットしません。

```sh
g++ -std=c++23 -O3 examples/search/ahc005_patrol_sa.cpp -o baseline
python3 scripts/bundle.py problems/ahc005/main.cpp -o submission.cpp
g++ -std=c++23 -O3 -DAHC005_SPLICE submission.cpp -o splice
g++ -std=c++23 -O2 tests/bipartite_edge_cover_counter_test.cpp -o cover_test
./cover_test
python3 problems/ahc005/benchmark.py INPUTS_DIR dev --start 0 --end 8 --vis TOOLS_DIR/target/release/vis --entry baseline=baseline --entry splice=splice
python3 problems/ahc005/benchmark.py INPUTS_DIR holdout --start 60 --end 68 --vis TOOLS_DIR/target/release/vis --entry baseline=baseline --entry splice=splice
python3 problems/ahc005/segment_cover_diagnostic.py INPUTS_DIR 0 1 2 3 4 5 6 7 60 61 62 63 64 65 66 67
```

## 通過中の可視性を含む可変長の代表点探索（小規模診断）

`-DAHC005_PATH_AWARE_DIAGNOSTIC` は先の方式を変更せずに通常の探索を終え、
同じ代表点順序を2群へ複製します。追加の1点削除だけを繰り返す群と、
連続2点の同時削除や代表点の移動を含める群を比較します。
新旧の有向最短路で通過する区間の集合を差分として費用と可視性に反映し、
不完全なら変更を全て巻き戻します。同距離の最短路は向きが逆だと内容が
変わり得るため、逆辺の可視性は流用しません。

本番や他のローカル集合と重ならない新規8シードを公式 gen で生成しました。
各群最大12,000近傍試行・66 ms（全体締切あり）。**8件のいずれも両群で移動時間は変化せず**、
最長実測は **2.652082秒**。全出力が完全可視・公式採点と独立検査一致。
これは、既に順序を調整済みの局面から費用減少だけを認める小規模な仕上げでは
改善が見えなかったという結果です。通常ビルドへ採用しません。
通過区間のキャッシュ更新は[差分テスト](../../tests/ahc005_path_coverage_test.cpp)で
最大100×30の状態遷移を全再計算と比較しています。

次に試すなら、終盤の仕上げでなく途中から代表点を追加・削除可能にし、
制約を満たした別の経路へ動くため一時的な費用増を許す温度付き探索と、
独立の短い純粋なランダム比較を設計します。この時も出力へ採用する解には
完全可視・帰着・公式計算を要求します。

```sh
T=$(cd "${TOOLS_DIR:?}" && pwd)
S=$(pwd)/problems/ahc005/path-aware-seeds.txt
W=$(mktemp -d)
(cd "$W" && "$T/target/release/gen" "$S")
python3 scripts/bundle.py problems/ahc005/main.cpp -o "$W/main.cpp"
g++ -std=c++23 -O3 -DAHC005_PATH_AWARE_DIAGNOSTIC "$W/main.cpp" -o "$W/variant"
g++ -std=c++23 -O2 tests/ahc005_path_coverage_test.cpp -o "$W/test"
"$W/test" < "$W/in/0000.txt"
python3 problems/ahc005/benchmark.py "$W/in" "$W/scored" --vis "$T/target/release/vis" --entry "diagnostic=$W/variant"
```
