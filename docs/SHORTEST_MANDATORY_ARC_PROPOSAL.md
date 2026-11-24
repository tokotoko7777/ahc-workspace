# 無重み有向グラフの最短路で必ず通る辺

終点を固定し、任意の始点と入力辺IDについて、その辺が全最短路に含まれるかを O(1) で判定します。平行辺は別々のIDで扱うため、同じ端点間に同じ長さの辺が複数あれば、個々の辺は必須ではありません。未到達は `std::nullopt`、始点と終点が同じなら空の最短路があるので全辺が `false` です。

逆向きBFSで得た距離が1減る辺だけを残し、終点から逆向きに見た最短路DAGの支配木を作ります。各頂点の直接支配頂点は、先に処理したDAG上の全親の支配木LCAです。ある辺が必須なのは、始点からの全最短路がその辺の始点を通り、かつそこから出る最短方向の辺がそのIDの1本だけの場合です。前処理は O((V+E) log(V+1)) 時間、ピーク追加メモリ O(E+V log(V+1))、作成後は O(V+E) です。処理は非再帰です。

削除する辺が一つだけなら、辺を除外して逆向きBFSを一度行う O(V+E) の方法が簡単です。この部品は、同じ終点で多数の辺削除候補を照会する場合向けです。頂点集合の「どれか」を全最短路が通るかは別問題です。例えば分岐の2頂点を集合とすると、どちらも単独では必須でなくても、最短路は必ずどちらかを通ります。その場合は最短路DAGから集合全体を除外し、到達可能性を確認します。

[使用例](../examples/shortest_mandatory_arc_usage.cpp) が独立した利用例です。小さな有向グラフの全列挙とランダム多重グラフを、対象辺を1本ずつ削除して逆向きBFSする独立実装と照合しています。実ソルバへの組み込み・実測速度比較は行っていません。

```sh
g++ -std=gnu++23 -O2 -Wall -Wextra -Werror -pedantic-errors -fsanitize=undefined -I library tests/shortest_mandatory_arc_test.cpp -o /tmp/shortest_arc_check
/tmp/shortest_arc_check
g++ -std=gnu++23 -O2 -Wall -Wextra -Werror -pedantic-errors -fsanitize=undefined -I library examples/shortest_mandatory_arc_usage.cpp -o /tmp/shortest_arc_example
/tmp/shortest_arc_example
```

AHC008の試作で、固定した見張り位置に向かう最短路が指定した通路の辺を必ず横切るか調べる用途から着想しました。実際の捕獲判定には動物のランダムな歩行・担当者の位置・柵の合法性を別に検証します。
