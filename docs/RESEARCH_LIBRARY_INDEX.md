# AHC研究ライブラリ: 用途別索引

「何をしたいか」からC++部品を探す索引です。[進捗](AHC_PROGRESS.md)と[利用時の不便メモ](LIBRARY_FEEDBACK.md)で言及された公開部品を、ヘッダと検証先を版固定で掲載しています。AHC番号は用途の由来で、本番採用・順位・別問題での性能を示すものではありません。

## 取得と出典の残し方

- まずヘッダと例・テストのリンクを開き、必要な依存・C++の版を確認してください。この表の多くは問題別ブランチのコミットを指します。進捗ブランチや `main` に同じヘッダが揃っているわけではありません。
- 提出用の展開に使う既存の `scripts/bundle.py --source-ref` は **単一のコミット**を参照し、展開する各ヘッダがその版に存在し、手元とバイト一致することを検査します。索引の異なるSHAのURLを混在させたまま、進捗ブランチのSHA一つで出典を付けることはできません。
- 版が分かれているときは、[既存ガイドの「手動で結合する場合」](https://github.com/tokotoko7777/ahc-workspace/blob/d52289db3c63865dc09f2f979634f54267e20b54/docs/SOURCE_PROVENANCE.md#手動で結合する場合)に従い、使う各部品と依存を同じ公開元の版で確認し、参照URLと元の著作権・許諾表示を残してください。各コミットの公開時期・第三者からの閲覧可否と、参加する大会・部門の適用条件は別途確認してください。索引のSHAやテスト結果は出典確認の助けであり、規則への適合を保証しません。

状態注記のない項目も、部品の検証先を確認したもので、問題別ソルバーへの採用を意味しません。出典確認待ちの部品、ソルバー固有の実装、未公開試作は対象外です。一般ヘッダ・雛形は[既存ライブラリリファレンス](https://github.com/tokotoko7777/ahc-workspace/blob/d52289db3c63865dc09f2f979634f54267e20b54/docs/LIBRARY_REFERENCE.md)を参照してください。

## グラフ・木の探索

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 時間で壁が変わる中で、指定地点に寄って元の作業へ戻れるか調べる | [`time-layer-joins.hpp`](../library/time-layer-joins.hpp)（AHC008） | [使い方と実盤面の照合](proposals/TIME_LAYER_JOINS.md) / [小例](../examples/time_layer_joins_usage.cpp) / [全列挙テスト](../tests/test_time_layer_joins.cpp) |
| 頂点ごとに削除した場合の始点側の広さと人・動物を知る | [`rooted-vertex-cut.hpp`](../library/rooted-vertex-cut.hpp)（AHC008） | [使い方と実問題](proposals/ROOTED_VERTEX_CUT.md) / [例](../examples/rooted_vertex_cut_usage.cpp) / [愚直BFS比較](../tests/test_rooted_vertex_cut.cpp) |
| 局所的に柵を変えても外側の最短距離が同じか確認する | [`boundary-transit.hpp`](../library/boundary-transit.hpp)（AHC008） | [証明・実問題での使用](proposals/BOUNDARY_TRANSIT.md) / [小さな例](../examples/boundary_transit_usage.cpp) / [BFS照合](../tests/test_boundary_transit.cpp) |
| 小さい連結領域の中に目的地がないことを確認する | [`bounded-component-probe.hpp`](../library/bounded-component-probe.hpp)（AHC008） | [使い方・実問題での計測](proposals/BOUNDED_COMPONENT_PROBE.md) / [全探索との照合](../tests/test_bounded_component_probe.cpp) |
| 一部の目的地までの短い距離だけを求める | [`target-limited-bfs.hpp`](../library/target-limited-bfs.hpp)（AHC008） | [使い方・実問題での計測](proposals/TARGET_LIMITED_BFS.md) / [全探索との照合](../tests/target_limited_bfs_test.cpp) |
| 壁が途中で閉じる日程で、干渉を避けて最終位置に着く確率を求める | [`fixed-walk-schedule.hpp`](../library/fixed-walk-schedule.hpp)（AHC008） | [使い方と中盤診断](proposals/FIXED_WALK_SCHEDULE.md) / [全列挙](../tests/fixed_walk_schedule_test.cpp) |
| 壁を足したとき、残る領域の経路が最大で何歩伸びるか調べる | [`retained-route-stretch.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/be6418377a07761bc3cd351998ef84772d0cf868/library/retained-route-stretch.hpp)（AHC008） | [使い方と証明](https://github.com/tokotoko7777/ahc-workspace/blob/be6418377a07761bc3cd351998ef84772d0cf868/docs/proposals/RETAINED_ROUTE_STRETCH.md) / [全点対との比較](https://github.com/tokotoko7777/ahc-workspace/blob/be6418377a07761bc3cd351998ef84772d0cf868/tests/retained_route_stretch_test.cpp)。溝の閉鎖を診断 |
| 監視時間内にランダムに動く対象が指定集合へ一度でも入る確率を求める | [`capture-window.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/7dee4477fc69323b33e50b8488192e6a1eaf07ef/library/capture-window.hpp)（AHC008） | [使い方](https://github.com/tokotoko7777/ahc-workspace/blob/7dee4477fc69323b33e50b8488192e6a1eaf07ef/docs/proposals/CAPTURE_WINDOW.md) / [全軌道列挙によるテスト](https://github.com/tokotoko7777/ahc-workspace/blob/7dee4477fc69323b33e50b8488192e6a1eaf07ef/tests/capture_window_test.cpp)。固定グラフ上の予測、ソルバー未採用 |
| 危険点に到達せず、外してよい壁から始点領域を最大にする | [`max-safe-opening.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/ee27536f3fa145988bb9bd6cce3a7d9d9c92a651/library/max-safe-opening.hpp)（AHC008） | [使い方・証明・テスト](https://github.com/tokotoko7777/ahc-workspace/blob/ee27536f3fa145988bb9bd6cce3a7d9d9c92a651/docs/proposals/MAX_SAFE_OPENING.md)。静的な終局診断 |
| 既採用辺で縮約した後、辺フィルタごとに最小ボトルネックを求める | [`kruskal-bottleneck.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/2793a9a782184c4b77376fab17482d24b353d6bd/library/kruskal-bottleneck.hpp) · AHC007 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/2793a9a782184c4b77376fab17482d24b353d6bd/examples/kruskal_bottleneck_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/2793a9a782184c4b77376fab17482d24b353d6bd/tests/kruskal_bottleneck_test.cpp) |
| 同じ格子で距離と最初の一歩だけを何度も求める | [`flat-grid-first-step-bfs.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/af41b4e70a89fef3c0389c689aa09dc633fffa16/library/flat-grid-first-step-bfs.hpp) · AHC008 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/af41b4e70a89fef3c0389c689aa09dc633fffa16/tests/flat_grid_first_step_bfs_test.cpp) |
| 最大次数2のグラフの成分を作業領域を再利用して数える | [`degree-two-components.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/8e99892f7dc12317ba365750a15a2a7c072b3731/library/degree-two-components.hpp) · AHC010 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/8e99892f7dc12317ba365750a15a2a7c072b3731/tests/degree_two_components_test.cpp) |
| 明示的な辺リストなしで、複数始点・可変コストの最短路を求める | [`implicit-multi-source-dijkstra.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/ff72f9a9577cff6a5ce466081ef959cdde836522/library/implicit-multi-source-dijkstra.hpp) · AHC018 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/ff72f9a9577cff6a5ce466081ef959cdde836522/tests/implicit_multi_source_dijkstra_test.cpp) |
| 根から選んだ端点までに必要な枝と費用を抜き出す | [`rooted-terminal-tree.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/96a97144e57022f452ce216eddf9c2d76adc3b32/library/rooted-terminal-tree.hpp) · AHC020 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/96a97144e57022f452ce216eddf9c2d76adc3b32/tests/rooted_terminal_tree_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/96a97144e57022f452ce216eddf9c2d76adc3b32/docs/LIBRARY_REFERENCE.md) |
| DAGを両端から確定し、次に確定できる頂点を追う | [`two-sided-dag-frontier.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/9306c3d973e9b622e3a35431363f5f7f6ab64225/library/two-sided-dag-frontier.hpp) · AHC021 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/9306c3d973e9b622e3a35431363f5f7f6ab64225/examples/two_sided_dag_frontier_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/9306c3d973e9b622e3a35431363f5f7f6ab64225/tests/two_sided_dag_frontier_test.cpp) |
| 層間で位置が一定範囲だけ動ける経路を最適化する | [`bounded-layer-shortest-path.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/c5b3b9f3d88b74ff5f9fa67446d16555dd509ef1/library/bounded-layer-shortest-path.hpp) · AHC024 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/c5b3b9f3d88b74ff5f9fa67446d16555dd509ef1/tests/bounded_layer_shortest_path_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/c5b3b9f3d88b74ff5f9fa67446d16555dd509ef1/docs/LIBRARY_REFERENCE.md) |
| 層ごとに候補数が異なる最短路を復元する | [`ragged-layer-shortest-path.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/84d5508c4284fa8e75116f1a038a8124946246ac/library/ragged-layer-shortest-path.hpp) · AHC028 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/84d5508c4284fa8e75116f1a038a8124946246ac/tests/ragged_layer_shortest_path_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/84d5508c4284fa8e75116f1a038a8124946246ac/docs/LIBRARY_REFERENCE.md) |
| 辺を重み順に試し、次数2以下で閉路のないパス群を作る | [`greedy-path-forest.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/fb26d23b729e0e30e19da4d6181bd0421588196e/library/greedy-path-forest.hpp)（依存: [`dsu.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/fb26d23b729e0e30e19da4d6181bd0421588196e/library/dsu.hpp)） · AHC036 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/fb26d23b729e0e30e19da4d6181bd0421588196e/examples/greedy_path_forest_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/fb26d23b729e0e30e19da4d6181bd0421588196e/tests/greedy_path_forest_test.cpp) |
| 深さ上限付きで部分木を付け替え、差分評価する | [`bounded-depth-forest.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/3e6e4f9ea3ea3d9e65c0bd7e7a07e1e2fdccf908/library/bounded-depth-forest.hpp) · AHC041 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/3e6e4f9ea3ea3d9e65c0bd7e7a07e1e2fdccf908/examples/bounded_depth_forest_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/3e6e4f9ea3ea3d9e65c0bd7e7a07e1e2fdccf908/tests/bounded_depth_forest_test.cpp) |
| 割当で距離を使い、確定した行き先への経路を再探索せずに復元する | [既存の MaskedBfsWorkspace を再利用](proposals/BFS_REUSE_AFTER_MATCHING.md) · AHC008 終盤 | 格子の同距離順を保持。乱択 3,000 回の照合 |
| 通行マスクを差し替えながら作業領域を再利用してBFSする | [`masked-bfs-workspace.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/cc2c7b4e86abc712f7ce27bf8a992175adb45ae1/library/masked-bfs-workspace.hpp) · AHC054 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/cc2c7b4e86abc712f7ce27bf8a992175adb45ae1/tests/masked_bfs_workspace_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/cc2c7b4e86abc712f7ce27bf8a992175adb45ae1/docs/LIBRARY_REFERENCE.md) |
| 直前の頂点へ戻れないグラフの最短経路を求める | [`nonbacktracking-path.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/204933bff927c6bac159926766888181897cc748/library/nonbacktracking-path.hpp) · AHC060 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/204933bff927c6bac159926766888181897cc748/tests/nonbacktracking_path_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/204933bff927c6bac159926766888181897cc748/docs/LIBRARY_REFERENCE.md) |
| 操作回数を最小化し、同回数なら副費用も最小にする | [`unit-lexicographic-bfs.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/b69f28ac27f1bdfc0e4cf829de16762ea408ae35/library/unit-lexicographic-bfs.hpp) · AHC066 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/b69f28ac27f1bdfc0e4cf829de16762ea408ae35/tests/unit_lexicographic_bfs_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/b69f28ac27f1bdfc0e4cf829de16762ea408ae35/docs/LIBRARY_REFERENCE.md) |
| 無向グラフで除くと成分が分かれる辺を、並行辺込みで求める | [`undirected-bridges.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/5799ad6db455dd0d973d5fcdea14cb56a32f98d2/library/undirected-bridges.hpp) · AHC067 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/5799ad6db455dd0d973d5fcdea14cb56a32f98d2/tests/undirected_bridges_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/5799ad6db455dd0d973d5fcdea14cb56a32f98d2/docs/LIBRARY_REFERENCE.md) |

## 格子・幾何・対称変換

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 接触する矩形の辺をまとめ、非重複を保つ移動範囲を求める（近傍は未採用） | [`coordinate-face-slide.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/fe7273b40cc8cc8671283ace5d30b6aaf6ceec1a/library/coordinate-face-slide.hpp) · AHC001 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/fe7273b40cc8cc8671283ace5d30b6aaf6ceec1a/tests/coordinate_face_slide_test.cpp) |
| 行・列のビットから四方向の最近占有セルを引く | [`grid-line-occupancy64.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/7526fecad74457ce05f786e5127c4e45c0357203/library/grid-line-occupancy64.hpp) · AHC013 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/7526fecad74457ce05f786e5127c4e45c0357203/examples/grid_line_occupancy64_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/7526fecad74457ce05f786e5127c4e45c0357203/tests/grid_line_occupancy64_test.cpp) |
| 格子の縦・横・斜め8方向のマークや辺をビットで調べる | [`square-lattice-line-bits.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/25c2dea8a3d5f9b07432ee1a2b239cf4bd820cff/library/square-lattice-line-bits.hpp) · AHC014 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/25c2dea8a3d5f9b07432ee1a2b239cf4bd820cff/tests/square_lattice_line_bits_test.cpp) |
| 小盤面の同色連結成分をビット演算で求める | [`small-grid-bit-flood.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/a2b1c24d2d58f6e42db8475e62eb7ae00ef9373e/library/small-grid-bit-flood.hpp) · AHC015 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/a2b1c24d2d58f6e42db8475e62eb7ae00ef9373e/tests/small_grid_bit_flood_test.cpp) |
| ボクセルの24通りの正回転を列挙し、鏡映を除く | [`cube-rotations.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/7252e85e2d8a5d75038c0680f4ba43b4a43b3f05/library/cube-rotations.hpp) · AHC019 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/7252e85e2d8a5d75038c0680f4ba43b4a43b3f05/examples/cube_rotations_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/7252e85e2d8a5d75038c0680f4ba43b4a43b3f05/tests/cube_rotations_test.cpp) |
| 正方形盤面・部分パッチ・アンカーを回転・反転する | [`square-dihedral.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/890cee4f0a8b60a076d3c98d6d98d6bffed43099/library/square-dihedral.hpp) · AHC032 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/890cee4f0a8b60a076d3c98d6d98d6bffed43099/tests/square_dihedral_test.cpp) |
| ビット盤面から行・列を抽出してずらし、端から出た値を返す | [`packed-line-shift.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/e8796e286c2e197c2f1f5324755e4fb58da8f3f5/library/packed-line-shift.hpp) · AHC042 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/e8796e286c2e197c2f1f5324755e4fb58da8f3f5/tests/ahc042_shift_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/e8796e286c2e197c2f1f5324755e4fb58da8f3f5/docs/LIBRARY_REFERENCE.md) |
| 固定の壁配置で4方向の滑走停止点を前計算する | [`static-slide-grid.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/00edc1ce4028926dcc2cfb8316e64994f3fe89bb/library/static-slide-grid.hpp) · AHC046 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/00edc1ce4028926dcc2cfb8316e64994f3fe89bb/tests/static_slide_grid_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/00edc1ce4028926dcc2cfb8316e64994f3fe89bb/docs/LIBRARY_REFERENCE.md) |
| 壁の追加・削除後に4方向の滑走停止点を更新する | [`orthogonal-slide-grid.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/c1084c6814a6b1a98fa2b6de854a1a2941f09098/library/orthogonal-slide-grid.hpp) · AHC050 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/c1084c6814a6b1a98fa2b6de854a1a2941f09098/examples/orthogonal_slide_grid_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/c1084c6814a6b1a98fa2b6de854a1a2941f09098/tests/orthogonal_slide_grid_test.cpp) |
| 周期境界上の等速運動で、整数時刻の最近接を求める | [`periodic-closest-approach.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/597f72f03de27400f38dd4cff9f5ee779055c0be/library/periodic-closest-approach.hpp) · AHC057 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/597f72f03de27400f38dd4cff9f5ee779055c0be/tests/periodic_closest_approach_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/597f72f03de27400f38dd4cff9f5ee779055c0be/docs/LIBRARY_REFERENCE.md) |
| 格子上の始点を追加するたびに周期境界なしのマンハッタン距離を更新する | [`incremental-manhattan-distance.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/3737c5016654eb51756f431f8fc90cd54ab58314/library/incremental-manhattan-distance.hpp) · AHC070 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/3737c5016654eb51756f431f8fc90cd54ab58314/tests/incremental_manhattan_distance_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/3737c5016654eb51756f431f8fc90cd54ab58314/docs/LIBRARY_REFERENCE.md) |
| 反射を使う軸上の最短移動と、その値を取るすべての開始位置の整数区間を求める | [`reflection-launch-range.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/614cf90a321ba056b75894fdb9d00da56428a94c/library/reflection-launch-range.hpp) · AHC073 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/614cf90a321ba056b75894fdb9d00da56428a94c/tests/reflection_launch_range_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/614cf90a321ba056b75894fdb9d00da56428a94c/docs/LIBRARY_REFERENCE.md) |

## 割当・区間最適化

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 劣モジュラな二値エネルギーの最小化を最小カットに帰着（応用は実験） | [`binary-submodular-energy.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/54f9862b1def3e9129385a3acf84a6693d409a12/library/binary-submodular-energy.hpp)（依存: [`max-flow.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/54f9862b1def3e9129385a3acf84a6693d409a12/library/max-flow.hpp)） · AHC001 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/54f9862b1def3e9129385a3acf84a6693d409a12/tests/binary_submodular_energy_test.cpp) |
| 日別の禁止条件を踏まえ、1レーンの区間集合を選ぶ | [`calendar-interval-packing.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/52f76e1f5e905e5f4f11f0613285760073967281/library/calendar-interval-packing.hpp) · AHC023 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/52f76e1f5e905e5f4f11f0613285760073967281/examples/calendar_interval_packing_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/52f76e1f5e905e5f4f11f0613285760073967281/tests/calendar_interval_packing_test.cpp) |
| 重み付き区間を複数レーンに割り当てる | [`parallel-interval-packing.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/ac291d5344ff4f4eca283e694da5ba3737b14218/library/parallel-interval-packing.hpp)（依存: [`min-cost-flow.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/ac291d5344ff4f4eca283e694da5ba3737b14218/library/min-cost-flow.hpp)） · AHC023 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/ac291d5344ff4f4eca283e694da5ba3737b14218/examples/parallel_interval_packing_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/ac291d5344ff4f4eca283e694da5ba3737b14218/tests/parallel_interval_packing_test.cpp) |
| 比較回数制限のもとで順序の確定部分・未確定区間を残す | [`budgeted-order-buckets.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/21034a6404bafe091bac193b8315b0ffd1ed0686/library/budgeted-order-buckets.hpp) · AHC025 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/21034a6404bafe091bac193b8315b0ffd1ed0686/examples/budgeted_order_buckets_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/21034a6404bafe091bac193b8315b0ffd1ed0686/tests/budgeted_order_buckets_test.cpp) |
| 最小間隔と順序を守ってカット位置を復元する | [`ordered-cut-placement.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/fb4acc4255f313b63cc46c95b3965fc983b2d528/library/ordered-cut-placement.hpp) · AHC031 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/fb4acc4255f313b63cc46c95b3965fc983b2d528/examples/ordered_cut_placement_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/fb4acc4255f313b63cc46c95b3965fc983b2d528/tests/ordered_cut_placement_test.cpp) |
| 増加路の間で止められる最小費用流。部分流の扱いは呼び出し側で決める（既存拡張） | [`min-cost-flow.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/e2c29731322790e744a17033d3fa937c450cede7/library/min-cost-flow.hpp) · AHC034 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/e2c29731322790e744a17033d3fa937c450cede7/tests/min_cost_flow_stop_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/e2c29731322790e744a17033d3fa937c450cede7/docs/LIBRARY_REFERENCE.md) |
| 2次元L1距離付きの最大値と遷移元を求める | [`manhattan-max-transform.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/39ff40a8cce317a2ed00157ebb8cefefeeff0b16/library/manhattan-max-transform.hpp) · AHC039 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/39ff40a8cce317a2ed00157ebb8cefefeeff0b16/tests/manhattan_max_transform_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/39ff40a8cce317a2ed00157ebb8cefefeeff0b16/docs/LIBRARY_REFERENCE.md) |
| 正の長さで重なる区間だけを対象に距離付き遷移を最大化する | [`overlapping-interval-max-transform.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/afa01fc73184f6fe316465a5c67f924057b6f4b9/library/overlapping-interval-max-transform.hpp) · AHC039 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/afa01fc73184f6fe316465a5c67f924057b6f4b9/tests/overlapping_interval_max_transform_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/afa01fc73184f6fe316465a5c67f924057b6f4b9/docs/LIBRARY_REFERENCE.md) |
| 上下限と順序制約を満たす値へ射影する | [`bounded-order-projection.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/ef908953cb79d12f7f5f71ef4aaef53d45fc4c04/library/bounded-order-projection.hpp) · AHC045 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/ef908953cb79d12f7f5f71ef4aaef53d45fc4c04/examples/bounded_order_projection_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/ef908953cb79d12f7f5f71ef4aaef53d45fc4c04/tests/bounded_order_projection_test.cpp) |
| 整列済みの非正方割当で、最大誤差→合計の順に最小化する | [`sorted-bottleneck-assignment.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/7f0257819434e8b706195937b14ee7f0b482b3ae/library/sorted-bottleneck-assignment.hpp) · AHC053 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/7f0257819434e8b706195937b14ee7f0b482b3ae/tests/sorted_bottleneck_assignment_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/7f0257819434e8b706195937b14ee7f0b482b3ae/docs/LIBRARY_REFERENCE.md) |
| 二つの側で端点が厳密に昇順になる、交差しない組の重みを最大化する | [`noncrossing-matching.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/63185be0c9a50743d00f846f73ca5f1ffab597e7/library/noncrossing-matching.hpp) · AHC064 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/63185be0c9a50743d00f846f73ca5f1ffab597e7/tests/noncrossing_matching_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/63185be0c9a50743d00f846f73ca5f1ffab597e7/docs/LIBRARY_REFERENCE.md) |

## 差分評価・経路編集

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 重複IDを禁止し、長さ上限付きの経路を探索する | [`bounded-colored-path.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/c09a5d30611f6991d5cb3761b5a5038f5eb0316d/library/bounded-colored-path.hpp) · AHC002 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/c09a5d30611f6991d5cb3761b5a5038f5eb0316d/examples/bounded_colored_path_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/c09a5d30611f6991d5cb3761b5a5038f5eb0316d/tests/bounded_colored_path_test.cpp) |
| 両端どちらかを覆う辺の被覆数を差分更新・巻き戻しする | [`bipartite-edge-cover-counter.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/b684d67d0286d48bd4a34fb7c9136a7a8cab2d40/library/bipartite-edge-cover-counter.hpp) · AHC005 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/b684d67d0286d48bd4a34fb7c9136a7a8cab2d40/tests/bipartite_edge_cover_counter_test.cpp) |
| 先行制約を守る2点の最良挿入位置を求める（既存・変更なし、上位K未対応） | [`ordered-pair-insertion.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/0e7380c86bf2f895cc47b0f44cd976f7bb01e9cd/library/ordered-pair-insertion.hpp) · AHC006 | [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/0e7380c86bf2f895cc47b0f44cd976f7bb01e9cd/docs/LIBRARY_REFERENCE.md) |
| サイズ別の個数を上限付きスコアとして差分管理する | [`capped-size-histogram.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/dc1a18935430728a5dbca82c14f8bc84c57145e1/library/capped-size-histogram.hpp) · AHC012 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/dc1a18935430728a5dbca82c14f8bc84c57145e1/tests/capped_size_histogram_test.cpp) |
| カット移動に伴う上限付きサイズ頻度の評価を掃引で更新する | [`capped-partition-sweep.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/247becac000d9343c3d98fa15ce6062d845566df/library/capped-partition-sweep.hpp)（依存: [`capped-size-histogram.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/247becac000d9343c3d98fa15ce6062d845566df/library/capped-size-histogram.hpp)） · AHC012 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/247becac000d9343c3d98fa15ce6062d845566df/tests/capped_partition_sweep_test.cpp) |
| 疎な同グループ罰則を割当変更や交換の周辺だけで更新する | [`sparse-group-conflicts.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/6078ab465ffd5a225a464187bd443e4b62fcd38b/library/sparse-group-conflicts.hpp) · AHC017 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/6078ab465ffd5a225a464187bd443e4b62fcd38b/examples/sparse_group_conflicts_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/6078ab465ffd5a225a464187bd443e4b62fcd38b/tests/sparse_group_conflicts_test.cpp) |
| 交換列から操作を削除したときの結果を、接尾辞の効果で評価する | [`swap-suffix-pruning.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/ce5987e9b2769010a4568b3442108f2e02e29909/library/swap-suffix-pruning.hpp) · AHC021 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/ce5987e9b2769010a4568b3442108f2e02e29909/examples/swap_suffix_pruning_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/ce5987e9b2769010a4568b3442108f2e02e29909/tests/swap_suffix_pruning_test.cpp) |
| 周期巡回の訪問間隔から、周回境界込みの費用を求める | [`cyclic-visit-energy.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/3ec2d6d02ab6aea5b461002201803016ce56ecb8/library/cyclic-visit-energy.hpp) · AHC027 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/3ec2d6d02ab6aea5b461002201803016ce56ecb8/examples/cyclic_visit_energy_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/3ec2d6d02ab6aea5b461002201803016ce56ecb8/tests/cyclic_visit_energy_test.cpp) |
| 積載量依存の経路費と途中合法性を、区間の連結・反転で評価する | [`load-aware-route.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/6c2ea91bb86ef24c6c84cd99bf76df6355c16ce6/library/load-aware-route.hpp) · AHC034 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/6c2ea91bb86ef24c6c84cd99bf76df6355c16ce6/tests/load_aware_route_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/6c2ea91bb86ef24c6c84cd99bf76df6355c16ce6/docs/LIBRARY_REFERENCE.md) |
| 併合候補を安定IDと遅延削除ヒープで管理する | [`agglomerative-pair-merge.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/11e02827a706e05022f50a3f7956541fa388f9ea/library/agglomerative-pair-merge.hpp) · AHC037 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/11e02827a706e05022f50a3f7956541fa388f9ea/tests/agglomerative_pair_merge_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/11e02827a706e05022f50a3f7956541fa388f9ea/docs/LIBRARY_REFERENCE.md) |
| ペアの両端が揃った場合だけ加点し、被覆変更を差分管理する | [`paired-coverage-reward.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/2920bc508229a62f10b0070b68f3f1941fae75a4/library/paired-coverage-reward.hpp) · AHC043 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/2920bc508229a62f10b0070b68f3f1941fae75a4/examples/paired_coverage_reward_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/2920bc508229a62f10b0070b68f3f1941fae75a4/tests/paired_coverage_reward_test.cpp) |
| 積荷の現在の耐久余裕と、荷物追加後の移動可能距離を計算する | [`stack-durability-reserve.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/a4688d135b0ecabf49fe877895782ced3e0aa5ca/library/stack-durability-reserve.hpp) · AHC049 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/a4688d135b0ecabf49fe877895782ced3e0aa5ca/tests/stack_durability_reserve_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/a4688d135b0ecabf49fe877895782ced3e0aa5ca/docs/LIBRARY_REFERENCE.md) |
| 同じ分類などの制約を守って、2点を挿入する位置を比較する | [`class-constrained-pair-insertion.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/4848abd431bd612b8ab15199f2f5562fe1598974/library/class-constrained-pair-insertion.hpp) · AHC059 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/4848abd431bd612b8ab15199f2f5562fe1598974/tests/class_constrained_pair_insertion_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/4848abd431bd612b8ab15199f2f5562fe1598974/docs/LIBRARY_REFERENCE.md) |
| 区間反転する列で重み付き位置の和と順位を取得する | [`weighted-sequence-treap.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/81151ac7b6335424f69b3bbb9da842061e296fb8/library/weighted-sequence-treap.hpp) · AHC062 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/81151ac7b6335424f69b3bbb9da842061e296fb8/tests/weighted_sequence_treap_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/81151ac7b6335424f69b3bbb9da842061e296fb8/docs/LIBRARY_REFERENCE.md) |
| 配列上の散在したセルを巡回移動し、逆引き位置も更新する | [`scattered-cycle-rotation.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/37f1f97176dcc631de043411873fa74d279b9a1a/library/scattered-cycle-rotation.hpp) · AHC065 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/37f1f97176dcc631de043411873fa74d279b9a1a/tests/scattered_cycle_rotation_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/37f1f97176dcc631de043411873fa74d279b9a1a/docs/LIBRARY_REFERENCE.md) |
| 区間の前半・後半を交換すると目的位置に届く、合法な開始範囲を求める | [`half-swap-interval.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/532a0cf2a83e82849153637b8279bbd0d5c08d20/library/half-swap-interval.hpp) · AHC068 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/532a0cf2a83e82849153637b8279bbd0d5c08d20/tests/half_swap_interval_test.cpp) / [仕様と使い方](https://github.com/tokotoko7777/ahc-workspace/blob/532a0cf2a83e82849153637b8279bbd0d5c08d20/docs/LIBRARY_REFERENCE.md) |

## 確率・統計・数値計算

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 同じ変数を含む線形観測候補の情報を、共分散をコピーせずに評価する | [`gaussian-linear-information.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/0d42d1839a9ff4a1f35b2830ea4b2e2eeba682c4/library/gaussian-linear-information.hpp) · AHC003 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/0d42d1839a9ff4a1f35b2830ea4b2e2eeba682c4/examples/gaussian_linear_information_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/0d42d1839a9ff4a1f35b2830ea4b2e2eeba682c4/tests/gaussian_linear_information_test.cpp) |
| 成功率の異なる独立試行の成功回数分布を求める | [`bernoulli-count-distribution.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/b84d5b74efaa1410549101730df5a60c1cc3102e/library/bernoulli-count-distribution.hpp) · AHC016 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/b84d5b74efaa1410549101730df5a60c1cc3102e/tests/bernoulli_count_distribution_test.cpp) |
| 丸め・上下限打切りを含む正規観測の離散確率を求める | [`rounded-clipped-normal.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/d255c7a5ce936552a98346c8d2bf764f78add849/library/rounded-clipped-normal.hpp) · AHC022 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/d255c7a5ce936552a98346c8d2bf764f78add849/tests/rounded_clipped_normal_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/d255c7a5ce936552a98346c8d2bf764f78add849/docs/LIBRARY_REFERENCE.md) |
| 独立な2択の和の確率分布と、複数候補の最大値の期待値を求める | [`two-choice-sum-pmf.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/503ea8770af2217fea4f8b9f3a866c144949ac60/library/two-choice-sum-pmf.hpp) · AHC035 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/503ea8770af2217fea4f8b9f3a866c144949ac60/tests/two_choice_sum_pmf_test.cpp) |
| 雑音付き線形観測から平均と共分散を逐次更新する | [`linear-gaussian-updater.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/fcf1bbf603140f033820232afe5fbc5dc7c01e05/library/linear-gaussian-updater.hpp) · AHC040 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/fcf1bbf603140f033820232afe5fbc5dc7c01e05/tests/linear_gaussian_updater_test.cpp) |
| ピボット選択と特異判定を伴う密行列の逆行列を求める | [`dense-matrix-inverse.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/39c6fccc3544603af106c420abf983f84ded74b1/library/dense-matrix-inverse.hpp) · AHC044 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/39c6fccc3544603af106c420abf983f84ded74b1/tests/dense_matrix_inverse_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/39c6fccc3544603af106c420abf983f84ded74b1/docs/LIBRARY_REFERENCE.md) |
| マルコフ連鎖で特定パターンが一度でも出る確率を求める | [`markov-pattern-probability.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/e8aa0d41b813933b676a6b0cb442f1e51e7a92bc/library/markov-pattern-probability.hpp) · AHC047 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/e8aa0d41b813933b676a6b0cb442f1e51e7a92bc/tests/markov_pattern_probability_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/e8aa0d41b813933b676a6b0cb442f1e51e7a92bc/docs/LIBRARY_REFERENCE.md) |
| 前向き確率・後向き価値から二分岐の確率行を選ぶ | [`binary-split-fit.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/c7a55c1984e3da4992f23c94bdb1a5430db232c4/library/binary-split-fit.hpp) · AHC051 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/c7a55c1984e3da4992f23c94bdb1a5430db232c4/tests/binary_split_fit_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/c7a55c1984e3da4992f23c94bdb1a5430db232c4/docs/LIBRARY_REFERENCE.md) |
| 丸められた乗法一様ノイズから、正規事前分布の事後平均・分散を求める | [`rounded-multiplicative-normal.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/60f60000cefda6f413c1e1fb223eac5f80b8b0a3/library/rounded-multiplicative-normal.hpp) · AHC003 | [例](https://github.com/tokotoko7777/ahc-workspace/blob/60f60000cefda6f413c1e1fb223eac5f80b8b0a3/examples/rounded_multiplicative_normal_usage.cpp) / [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/60f60000cefda6f413c1e1fb223eac5f80b8b0a3/tests/rounded_multiplicative_normal_test.cpp) |

## 文字列・周期

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 相対オフセット付きの周期パターンを文字衝突を避けて併合する | [`circular-pattern-components.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/74c0189dc1bea65370a25ee45a0b74e5b8ddd591/library/circular-pattern-components.hpp)（依存: [`weighted-dsu.hpp`](https://github.com/tokotoko7777/ahc-workspace/blob/74c0189dc1bea65370a25ee45a0b74e5b8ddd591/library/weighted-dsu.hpp)） · AHC004 | [テスト](https://github.com/tokotoko7777/ahc-workspace/blob/74c0189dc1bea65370a25ee45a0b74e5b8ddd591/tests/circular_pattern_components_test.cpp) / [仕様](https://github.com/tokotoko7777/ahc-workspace/blob/74c0189dc1bea65370a25ee45a0b74e5b8ddd591/docs/LIBRARY_REFERENCE.md) |

## 追加提案：観測に基づく実行管理

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 操作が拒否・未確定になる計画で、確認済みの作業だけを進める | [`observed-task-cursor.hpp`](../library/observed-task-cursor.hpp) · AHC008の試作から独立した案（進捗ブランチ内） | [仕様・再現](OBSERVED_TASK_CURSOR_PROPOSAL.md) / [使用例](../examples/observed_task_cursor_usage.cpp) / [テスト](../tests/observed_task_cursor_test.cpp) |

## 追加提案：固定した候補セル集合の連結成分

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 同じ候補セルの組合せを切り替えて連結成分の面積・独立した重み合計を求める | [`optional-grid-components.hpp`](../library/optional-grid-components.hpp) · AHC008のライブラリ振り返りから独立した案（進捗ブランチ内） | [仕様・再現](OPTIONAL_GRID_COMPONENTS_PROPOSAL.md) / [使用例](../examples/optional_grid_components_usage.cpp) / [テスト](../tests/optional_grid_components_test.cpp) |

## 追加提案：終点固定の最短路

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 指定する辺を全最短路が通るか、多数の問い合わせで判定する | [`shortest-mandatory-arc.hpp`](../library/shortest-mandatory-arc.hpp) · AHC008から着想した独立提案 | [仕様](SHORTEST_MANDATORY_ARC_PROPOSAL.md) / [例](../examples/shortest_mandatory_arc_usage.cpp) / [テスト](../tests/shortest_mandatory_arc_test.cpp) |

## 追加提案：壁の更新と進入方向

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 観測した壁の変化から四近傍の進入方向を差分更新する | [`mutable-grid-enter-mask.hpp`](../library/mutable-grid-enter-mask.hpp) · AHC008から着想した独立提案 | [仕様](MUTABLE_GRID_ENTER_MASK_PROPOSAL.md) / [例](../examples/mutable_grid_enter_mask_usage.cpp) / [テスト](../tests/mutable_grid_enter_mask_test.cpp) |

## 追加提案：辺の順序を保つ固定グラフ最大流

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 固定した辺集合で、元の追加順を保ったまま残余流を扱う | [`ordered-csr-max-flow.hpp`](../library/ordered-csr-max-flow.hpp) · AHC008から着想した独立提案 | [仕様](ORDERED_CSR_MAX_FLOW_PROPOSAL.md) / [例](../examples/ordered_csr_max_flow_usage.cpp) / [テスト](../tests/ordered_csr_max_flow_test.cpp) |

## 追加提案：固定容量で完全一致する結果を再利用

同じ計算を繰り返し、入力の完全一致を確認して結果を再利用したいときは [bounded-exact-memo.hpp](https://github.com/tokotoko7777/ahc-workspace/blob/36e4a82f6878943a9a6e15453e7b8371949f5ec0/library/bounded-exact-memo.hpp)。[AHC008での利用例と注意点](https://github.com/tokotoko7777/ahc-workspace/blob/36e4a82f6878943a9a6e15453e7b8371949f5ec0/docs/proposals/BOUNDED_EXACT_MEMO.md) / [衝突を含むテスト](https://github.com/tokotoko7777/ahc-workspace/blob/36e4a82f6878943a9a6e15453e7b8371949f5ec0/tests/bounded-exact-memo-test.cpp)。AHC008の既定ソルバーは変更しない。

## この進捗ブランチにある追加提案

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 同じ壁配置で、多数の始点から目的地グループへの距離を何度も求める | [`grid-target-distance.hpp`](../library/grid-target-distance.hpp) · AHC008 | [仕様と適用箇所](GRID_TARGET_DISTANCE_PROPOSAL.md) / [例](../examples/grid_target_distance_usage.cpp) / [テスト](../tests/grid_target_distance_test.cpp) |
| 一人で行う作業と複数人が同時に必要な作業を一緒に割り当てる | [`grouped-mask-packing.hpp`](../library/grouped-mask-packing.hpp) · AHC008 | [仕様と適用箇所](GROUPED_MASK_PACKING_PROPOSAL.md) / [例](../examples/grouped_mask_packing_usage.cpp) / [テスト](../tests/grouped_mask_packing_test.cpp) |

## 追加提案：到達対象を変えずに空間をつなぐ

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 到達できる対象を変えず、少数の壁候補を取りやめて始点をつなぐ | [`preserve-reachability.hpp`](../library/preserve-reachability.hpp) · AHC008終盤 | [仕様・適用箇所](proposals/PRESERVE_REACHABILITY.md) / [例](../examples/preserve_reachability_usage.cpp) / [テスト](../tests/preserve_reachability_test.cpp) |
| 同じターンに柵の担当と移動を全部組み直し、全員の広い空間を保つ | [`joint-separator-step.hpp`](../library/joint-separator-step.hpp) · AHC008終盤の診断 | [仕様・適用箇所](proposals/ONE_TURN_SEPARATOR.md) / [例](../examples/joint-separator-step.cpp) / [テスト](../tests/joint-separator-step.cpp) |

## 追加提案：観測方向から見えない目的地を絞る

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 最短方向へ動く相手の目的地候補を、観測のたびに更新する | [`hidden-destination-filter.hpp`](../library/hidden-destination-filter.hpp) · AHC008の猫 | [仕様・短い使用例・適用箇所](proposals/HIDDEN_TARGET_DIRECTION_FILTER.md) / [小グラフ全列挙テスト](../tests/hidden_destination_filter_test.cpp) |

## 追加提案：広げられない区画で必須の柵

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 保護区画を変えず、同時に置いた柵のうち省けるものを求める | [`protected-component-gates.hpp`](../library/protected-component-gates.hpp) · AHC008 | [考え方・適用箇所](proposals/PROTECTED_COMPONENT_GATES.md) / [例](../examples/protected_component_gates_usage.cpp) / [全列挙テスト](../tests/protected_component_gates_test.cpp) |

## 追加提案：指定歩数で到達できる終点

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| 高々ではなく、ちょうど1歩または2歩後にあり得る頂点を列挙する | [`exact-step-frontier.hpp`](../library/exact-step-frontier.hpp) · AHC008の捕獲安全確認 | [使い方・適用箇所](proposals/EXACT_STEP_FRONTIER.md) / [例](../examples/exact_step_frontier_usage.cpp) / [テスト](../tests/exact_step_frontier_test.cpp) |

## 追加提案：最大流と最小カットをまとめて求める

| やりたいこと | 部品・由来 | 例・検証 |
| --- | --- | --- |
| Dinic で流量に加え、最小カットの始点側も必要 | [`MaxFlow::flow_and_min_cut`](../library/max-flow.hpp) · AHC008 終盤の追い込み | [仕様と計測](proposals/FLOW_AND_MIN_CUT.md) / [例](../examples/flow_and_min_cut_usage.cpp) / [全列挙とのテスト](../tests/flow_and_min_cut_test.cpp) |

## 追加提案：最後の到着を早める少人数の割当

[二段階の割当ヘッダー](../library/small-bottleneck-assignment.hpp)（AHC008の複数柵を担当する人の配置）。[仕様・適用箇所](proposals/SMALL_BOTTLENECK_ASSIGNMENT.md) / [例](../examples/small_bottleneck_assignment_usage.cpp) / [全列挙テスト](../tests/small_bottleneck_assignment_test.cpp)。

## 追加提案：制約を足したカットを少額の余裕で試す

[予算付き残余カット](../library/bounded-residual-cut.hpp) / [小さい予算で連続照会する配列版](../library/tiny-budget-residual-cut.hpp)（AHC008終盤、担当が重なる柵の差し替え）。[仕様・短い例・全列挙検証・計測](proposals/BOUNDED_RESIDUAL_CUT.md)。
