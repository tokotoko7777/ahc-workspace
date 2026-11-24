# 提案: 辺の追加順を保つ CSR 最大流

[`ordered-csr-max-flow.hpp`](../library/ordered-csr-max-flow.hpp) は
辺をすべて追加してから確定できるグラフ向けの Dinic 法です。
各頂点の正辺・逆辺を元の追加順のまま連続配置します。平行辺・逆向き辺・自己ループも
同じ順序で扱い、`add_edge(u,v,cap)` の返す辺 ID は凍結後も変わりません。

1. `ahc::OrderedCsrMaxFlow<long long> g(n)` を作り、辺を追加します。
2. `g.freeze()` で辺配置を確定します。再呼び出しは何もしません。
3. `g.flow(s,t[,limit])` で、現在の残余グラフに**追加で**流した量を得ます。
   `get_edge(id)` / `edges()` は元の容量と現在の流量を返します。

`min_cut(s)` は、その時点の残余グラフで `s` から到達できる頂点の集合です。
流量上限に達しただけでは `t` がまだ到達可能なことがあります。上限を省略しても
1回に返せる量は容量型の上限までです。まとめて最大流量を求める場合は、総流量を
表せる容量型を選んでください。容量は非負の符号付き整数、頂点数と辺 ID は
`int` の範囲です。凍結は時間・領域ともに O(V+E)、DFS の再帰深さは O(V) です。

既存の [`MaxFlow`](../library/max-flow.hpp) は計算後にも辺を追加でき、内部配列も
公開しています。本提案はその置換ではありません。凍結後の辺追加、および凍結前の
探索・辺照会は例外になります。

[使用例](../examples/ordered_csr_max_flow_usage.cpp) は独立した合成グラフで、答えは `5`。
[テスト](../tests/ordered_csr_max_flow_test.cpp) は固定シードを含む小グラフ2,003例で、
既存実装と各辺の流量・全始点の残余到達集合を照合し、全カット列挙と流量保存も確認します。
同じ流量でも挿入順で最初に選ばれる経路が変わるひし形グラフと、狭い符号付き容量型では
既定の上限に達しても残余経路が残る場合を別途検査します。

リポジトリのルートで実行します。

```sh
flags='-std=gnu++23 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all'
g++ $flags -I library tests/ordered_csr_max_flow_test.cpp -o /tmp/ordered-csr-test
/tmp/ordered-csr-test
g++ $flags -I library examples/ordered_csr_max_flow_usage.cpp -o /tmp/ordered-csr-example
/tmp/ordered-csr-example
```

実問題への適用候補: AHC008 のように盤面から固定したフローネットワークを構成する場合。
この汎用ヘッダと使用例は保存済みソルバへ組み込んでいません。性能は測定していません。
