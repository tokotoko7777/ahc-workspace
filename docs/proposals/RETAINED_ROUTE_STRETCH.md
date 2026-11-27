# 頂点を閉じたとき、残る領域の最大迂回距離を測る

利用例: 格子上に複数の壁を追加する探索で、連結性と面積が保たれていても、後続の移動が遠回りになる候補を検出する。入力は無向・単位長の隣接リスト、今回削除する頂点、残す成分の始点。既存の壁は隣接辺を持たない頂点にできる。

削除後の連結成分を C とする。C 内にあり、今回の削除頂点に隣接する頂点を境界とする。C 内の任意の最短経路が追加壁の影響を受ける場合、削除前経路が最初に C を出る直前と、最後に C に戻った直後はともに境界にある。この区間を C 内の最短経路に置き換えると、増加量は境界頂点対の最大増加量以下。境界も C 内なので、この上界はその頂点対で達成される。削除が1頂点なら、境界の異なる2点の削除前距離は2（直接辺があれば1）。

- 部品: [retained-route-stretch.hpp](../../library/retained-route-stretch.hpp)
- 使用例・独立な全点対テスト: [retained_route_stretch_test.cpp](../../tests/retained_route_stretch_test.cpp)
- 計算量: 頂点・辺数を V、E、境界サイズを b とすると O((b+1)(V+E)+b log b)、追加メモリ O(V)。境界が1点以下なら成分取得のBFSのみ。1頂点の削除では削除前のBFSは不要。
- 最小使用例: `auto r = retained_route::maximum_stretch(adj, std::vector{gate}, anchor);`。結果の `max_extra` が最大増加、正なら `from` と `to` が証拠点、`before` と `after` がその距離。
- 検証: GCC C++23で無向一般グラフと格子の乱択4,500件をFloyd–Warshallと全点対で突き合わせ。8頂点サイクルの1点削除で2→6も確認。単体実行: `g++ -std=gnu++23 -O3 tests/retained_route_stretch_test.cpp -o /tmp/stretch-test && /tmp/stretch-test`。
- 限界: 最大増加0はすべての距離が保たれる意味で、最短経路の本数や実際に選ばれる経路、乱歩の分岐、後続の行動が同じという意味ではない。部分的な遠回りを理由に既存の捕獲を一律に止めない。
