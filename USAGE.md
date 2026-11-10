# 使い方例

`library/` の各ヘッダについて、使いどころと呼び出し例をまとめています。
探索手法を選ぶところから始める場合は [探索の基本ガイド](SEARCH_GUIDE.md) を参照してください。
実際の問題で入力から出力までつなぐ例は [examples/search](examples/search/README.md) にあります。

ローカルでは必要なヘッダを `#include "library/timer.hpp"` のように読み込み、
リポジトリのルートで `-I .` を付けてコンパイルします。

```sh
g++ -O2 -std=c++17 -Wall -Wextra -I . examples/basic_usage.cpp -o /tmp/ahc_basic_usage
/tmp/ahc_basic_usage
```

上の [入門例](examples/basic_usage.cpp) は入力なしで実行できます。以下のコード片は
呼び出す部分に絞っており、必要なヘッダ、`main`、入力処理を省略しています。
`vector` や `cout` は `std::` を省略した表記です。`State`、`Move`、評価関数、
`n` などは問題に合わせて定義してください。同じ節の複数の例は、別々の使用形を示す場合があります。

単一ファイルで提出する場合は、使うヘッダの内容を `main` より上へ展開します。
必要な標準ヘッダと著作権・許諾表示を保ち、同じヘッダを二重に展開しないでください。

| 目的 | 主な項目 |
|---|---|
| 時間・乱数・探索の制御 | [Timer](#timer)、[Random](#random)、[焼きなまし](#タイマー内蔵の焼きなまし)、[BestKeeper](#bestkeeper)、[MultiStart](#multistart) |
| 配列・区間・集合 | [累積和](#累積和)、[Fenwick Tree](#fenwick-tree)、[Segment Tree](#segment-tree)、[DenseIntSet](#denseintset) |
| グラフ・経路・割当 | [Graph BFS](#graph-bfs)、[Dijkstra](#dijkstra)、[Max Flow](#max-flow)、[Hungarian Algorithm](#hungarian-algorithm) |
| 数学・文字列・列 | [StaticModInt](#staticmodint)、[Matrix](#matrix)、[Rolling Hash](#rolling-hash)、[最長増加部分列](#longest-increasing-subsequence) |
| 差分更新・巻き戻し・ビーム | [RollbackArray](#rollbackarray)、[更新の検査](#差分更新とapplyrevertの検査)、[PrefixReplay](#行動列を途中からだけ再生する)、[SimpleBeamSearch](#simplebeamsearch)、[TreeBeamSearch](#treebeamsearch) |
| 幾何・配置 | [Point2D](#point2d)、[AxisAlignedRectangle](#axisalignedrectangle)、[最大の空き長方形](#指定セルを含む最大の空き長方形)、[Convex Hull](#convex-hull) |

## Timer

ヘッダ: [`timer.hpp`](library/timer.hpp)

単位はミリ秒です。AtCoder の制限ぴったりではなく、出力用の余裕を残した値を
指定します。

```cpp
Timer timer;

while (!timer.is_over(1900.0)) {
  double progress = timer.progress(1900.0);  // 0.0 から 1.0
  double remaining = timer.remaining_ms(1900.0);
  // 探索を1回進める
}
```

## Random

ヘッダ: [`random.hpp`](library/random.hpp)

同じ seed を渡すと同じ乱数列になり、バグを再現しやすくなります。

```cpp
Random random(123);

int index = random.next_int(0, n);                  // [0, n)
long long large = random.next_int(0LL, 1000000000000LL);
double probability = random.next_real();            // [0, 1)
double temperature = random.next_real(1.0, 100.0);  // [1, 100)

random.shuffle(order);
int value = random.choice(values);

vector<double> weights = {1.0, 2.0, 7.0};
int selected = random.weighted_index(weights);
```

## AliasTable

ヘッダ: [`alias-table.hpp`](library/alias-table.hpp)

同じ重み分布から何度も抽選する場合、前計算後の1回を `O(1)` にします。

```cpp
vector<double> weights = {1.0, 2.0, 7.0};
AliasTable table(weights);

int selected = table.choose(
    random.next_int(0, table.size()), random.next_real());
```

`choose` には `[0, size())` の一様な整数と `[0, 1)` の一様実数を渡します。
重みが変わったらAliasTableを作り直す必要があります。抽選回数が少ない場合は
`random.weighted_index(weights)` の方が簡単です。

## タイマー内蔵の焼きなまし

ヘッダ: [`time-based-simulated-annealing.hpp`](library/time-based-simulated-annealing.hpp)

`time-based-simulated-annealing.hpp` は、時間計測、温度の更新、採用判定をまとめた部品です。
次は整数 `x` を目標値へ近づける小さな例です。最大化する得点を `-(x - 37)^2` と置きます。

```cpp
auto score = [](int x) { return -1LL * (x - 37) * (x - 37); };
mt19937_64 rng(123);
uniform_int_distribution<int> direction(0, 1);
int current = 0;
long long current_score = score(current);
int best = current;
long long best_score = current_score;

TimeBasedSimulatedAnnealing sa(50.0, 20.0, 0.1, 456, 1);
while (!sa.is_over()) {
  int candidate = current + (direction(rng) ? 1 : -1);
  if (candidate < -100 || candidate > 100) continue;
  long long candidate_score = score(candidate);
  if (sa.accept(candidate_score - current_score)) {
    current = candidate;
    current_score = candidate_score;
    if (current_score > best_score) {
      best = current;
      best_score = current_score;
    }
  }
}
cout << best << ' ' << best_score << '\n';
```

コンストラクタは順に「制限ミリ秒、開始温度、終了温度、seed、時計確認間隔」です。
例の50ミリ秒は動作確認用なので、実際には出力時間も残して制限を決めます。
間隔を大きくすると時計を読む回数は減りますが、その間は時間切れを確認しません。

`accept` に渡すのは **改善量** です。最大化なら `新得点 - 旧得点`、
最小化なら `旧コスト - 新コスト` にします。温度もこの改善量と同じ尺度で決めます。
現在解は悪化することがあるので、出力する最良解を別に保存してください。

同じヘッダの `TimeBasedAnnealingRunner<Problem>` を使うと、現在解・最良解・反復数も管理できます。
`Problem` に `State`、`Move`、`Score` と次の処理を用意します。

| 処理 | 役割 |
|---|---|
| `propose_move(state, rng, progress)` | 試す `Move` を `optional<Move>` で返す。生成できなければ `nullopt` |
| `evaluate_move(state, move, threshold)` | 状態を変えず、正確な改善量を `optional<Score>` で返す。非法な手なら `nullopt` |
| `apply_move(state, move)` | 採用された手を現在解へ反映する |

通常の `run()` は焼きなまし、`run_hill_climbing()` は改善量が正の手だけを採用します。
`run_with_threshold()` で評価を途中打ち切りする場合は、残りを計算しても
採用閾値を超えないと証明できる場合だけ打ち切ります。判定できなければ最後まで評価します。

## 進捗率を外から渡す焼きなまし

ヘッダ: [`simulated-annealing.hpp`](library/simulated-annealing.hpp), [`batched-timer.hpp`](library/batched-timer.hpp)

探索全体で1個の `Timer` を共有したい時や、複数フェーズに分けたい時はこちらを
使います。

```cpp
BatchedTimer timer(1200.0, 64);
SimulatedAnnealing sa(100.0, 1.0, 123);

while (!timer.is_over()) {
  sa.set_progress(timer.cached_progress());
  Move move = make_random_move(current);
  long long improvement = calculate_delta(current, move);
  if (sa.accept(improvement)) {
    apply(current, move);
    current_score += improvement;
  }
}
```

従来どおり`sa.accept(improvement, progress)`と書くこともできます。同じ
`progress`が続く間は温度を再計算しません。

## BestKeeper

ヘッダ: [`best-keeper.hpp`](library/best-keeper.hpp)

探索中の状態が最後に悪化していても、最良解を出力できます。

```cpp
long long current_score = calculate_score(answer);
BestKeeper<long long, vector<int>> best(current_score, answer);

// 状態を変更した後
best.update(current_score, answer);

// 最後は best.best_state を出力する
cout << best.best_score << '\n';
```

最小化なら第3引数を `false` にします。

```cpp
BestKeeper<double, vector<int>> best(initial_cost, answer, false);
```

## TopK

ヘッダ: [`top-k.hpp`](library/top-k.hpp)

候補をたくさん作り、良いものだけ詳しく調べたい時に使います。

```cpp
TopK<double, vector<int>> candidates(20);  // 最大化、上位20個

for (const auto& state : many_states) {
  double cheap_score = calculate_cheap_score(state);
  candidates.add(cheap_score, state);
}

for (const auto& [cheap_score, state] : candidates.entries) {
  // この20個だけを重い評価へ回す
}
```

最小化なら `TopK<double, State> candidates(20, false);` です。

## Schedule

ヘッダ: [`schedule.hpp`](library/schedule.hpp)

進捗率に応じて、近傍の大きさや試行回数を変えられます。

```cpp
double progress = timer.progress(1900.0);

int destroy_size = linear_schedule(100, 5, progress);
int trials = power_schedule(1000, 100, progress, 2.0);
double temperature = geometric_schedule(100.0, 1.0, progress);
```

## chmin / chmax

ヘッダ: [`chmin-chmax.hpp`](library/chmin-chmax.hpp)

```cpp
long long best_score = -1;
if (chmax(best_score, new_score)) {
  // best_score が更新された時だけ行う処理
}

double shortest = 1e100;
chmin(shortest, new_distance);
```

## 答えの二分探索

ヘッダ: [`binary-search-answer.hpp`](library/binary-search-answer.hpp)

「ある値以上なら可能」のように、判定結果が境界の前後で一度だけ
変わる場合に使います。

```cpp
// possible(ng) == false、possible(ok) == true にする
long long answer = binary_search_first_true<long long>(
    ng, ok, [&](long long value) { return possible(value); });

// true...true,false...false の最後のtrue
long long last = binary_search_last_true<long long>(
    ok_value, ng_value, [&](long long value) { return possible(value); });
```

実数は誤差値ではなく、回数を決めて絞ると実行時間が安定します。

```cpp
auto [false_side, true_side] = binary_search_real<double>(
    low, high, 100, [&](double value) { return possible(value); });
```

条件が途中でtrueからfalseへ戻る場合には使えません。

## 累積和

ヘッダ: [`cumulative-sum.hpp`](library/cumulative-sum.hpp), [`cumulative-sum-2d.hpp`](library/cumulative-sum-2d.hpp)

```cpp
vector<long long> values = {3, 1, 4, 1, 5};
CumulativeSum<long long> sum(values);
cout << sum.query(1, 4) << '\n';  // values[1] + values[2] + values[3]
```

2次元版の範囲は `[top, bottom) × [left, right)` です。

```cpp
vector<vector<int>> grid(h, vector<int>(w));
CumulativeSum2D<int> sum(grid);
int rectangle_sum = sum.query(top, left, bottom, right);
```

## Fenwick Tree

ヘッダ: [`fenwick-tree.hpp`](library/fenwick-tree.hpp)

値を途中で足し引きしながら区間和を何度も求める時に使います。累積和と違い、
`add` の後も高速に区間和を求められます。

```cpp
vector<long long> values = {3, 1, 4, 1, 5};
FenwickTree<long long> sum(values);

sum.add(2, 10);                 // values[2] に 10 を足す
cout << sum.query(1, 4) << '\n';  // values[1] + values[2] + values[3]
cout << sum.prefix_sum(3) << '\n';  // values[0] + values[1] + values[2]
```

各値が0以上なら、先頭からの和が指定値に達する位置も探せます。

```cpp
int index = sum.lower_bound(8LL);
// prefix_sum(index + 1) が初めて 8 以上になる index
// 全体の和が 8 未満なら sum.size() が返る
```

## Segment Tree

ヘッダ: [`segment-tree.hpp`](library/segment-tree.hpp)

区間和だけでなく、区間の最小値・最大値などを求めたい時に使います。
第2引数は空の区間を表す値、第3引数は2区間をまとめる処理です。

```cpp
vector<int> values = {3, 1, 4, 1, 5};
auto minimum = make_segment_tree(
    values,
    1000000000,
    [](int a, int b) { return min(a, b); });

cout << minimum.query(1, 4) << '\n';  // min({1, 4, 1}) = 1
minimum.set(1, 8);                    // values[1] = 8
cout << minimum.query(1, 3) << '\n';  // min({8, 4}) = 4
cout << minimum.all() << '\n';        // 全要素の最小値
```

区間和なら、空の区間は `0`、まとめる処理は足し算にします。

```cpp
auto sum = make_segment_tree(
    vector<long long>(n), 0LL,
    [](long long a, long long b) { return a + b; });
```

ラムダ式の型をそのまま保持するため、区間をまとめるたびに `std::function` の
間接呼び出しを行いません。通常は `make_segment_tree` から作ってください。

## DifferenceArray

ヘッダ: [`difference-array.hpp`](library/difference-array.hpp)

区間加算をすべて記録してから、最後に各位置の値を一度だけ作る場合に使います。

```cpp
DifferenceArray<long long> values(n);
values.add(left, right, amount);  // [left, right) に加算
values.add(another_left, another_right, another_amount);

vector<long long> result = values.build();
```

途中で区間和を質問する必要がある場合は、Fenwick TreeやRangeAddRangeSumを使います。

## DifferenceArray2D

ヘッダ: [`difference-array-2d.hpp`](library/difference-array-2d.hpp)

矩形加算をすべて記録してから、最後に各マスの値を1回だけ作る2次元いもす法です。

```cpp
DifferenceArray2D<long long> values(height, width);
values.add(top, left, bottom, right, amount);
// [top, bottom) x [left, right) に加算

vector<vector<long long>> grid = values.build();
```

大きな盤面を連続メモリで扱いたい場合は `build_flat()` を使います。

```cpp
vector<long long> flat = values.build_flat();
long long cell = flat[row * width + column];
```

矩形更新は `O(1)`、構築は `O(HW)` です。構築後にさらに `add` した場合は、
もう一度 `build` または `build_flat` を呼びます。

## RangeAddRangeSum

ヘッダ: [`range-add-range-sum.hpp`](library/range-add-range-sum.hpp)

区間全体への加算と、区間和の質問が混ざっている場合に使います。

```cpp
RangeAddRangeSum<long long> sum(initial_values);

sum.add(left, right, 10);       // [left, right) の全要素に10を足す
long long answer = sum.query(query_left, query_right);
long long total = sum.all();
```

## RangeAddRangeMinimum

ヘッダ: [`range-add-range-minimum.hpp`](library/range-add-range-minimum.hpp)

区間全体に値を足しながら、区間最小値を取得する遅延セグメント木です。

```cpp
const long long INF = (1LL << 60);
RangeAddRangeMinimum<long long> minimum(values, INF);

minimum.add(left, right, amount);       // [left, right) に加算
long long answer = minimum.query(ql, qr);  // [ql, qr) の最小値
long long one_value = minimum.get(index);
long long all = minimum.all_minimum();
```

空区間の `query` は構築時に渡した `INF` を返します。`INF + amount` が
オーバーフローしない値を選びます。

## RangeAddRangeMaximum

ヘッダ: [`range-add-range-maximum.hpp`](library/range-add-range-maximum.hpp)

区間全体に値を足しながら、区間最大値を `O(log N)` で取得します。

```cpp
const long long NEG_INF = -(1LL << 60);
RangeAddRangeMaximum<long long> maximum(values, NEG_INF);

maximum.add(left, right, amount);
long long answer = maximum.query(query_left, query_right);
long long all = maximum.all_maximum();
```

空区間の `query` は `NEG_INF` を返します。

## RangeAssignRangeSum

ヘッダ: [`range-assign-range-sum.hpp`](library/range-assign-range-sum.hpp)

「区間の全要素を同じ値に変更」と「区間和」が混ざる場合に使います。

```cpp
RangeAssignRangeSum<long long> sum(values);

sum.assign(left, right, value);  // [left, right) をvalueにする
long long answer = sum.query(query_left, query_right);
long long one = sum.get(index);
long long all = sum.all_sum();
```

代入は加算ではありません。`assign(left, right, 0)` は区間を0クリアします。

## SparseTable

ヘッダ: [`sparse-table.hpp`](library/sparse-table.hpp)

変更されない配列の区間最小値・最大値・gcdを何度も求める場合に使います。

```cpp
auto minimum = make_sparse_table(
    values, [](int a, int b) { return min(a, b); });

int answer = minimum.query(left, right);  // [left, right)、空区間不可
```

`min(x,x)=x` のように同じ値を重ねても変わらない演算専用です。区間和には
使えません。

## Sliding Window Minimum

ヘッダ: [`sliding-window-minimum.hpp`](library/sliding-window-minimum.hpp)

固定幅の全区間について、最小値・最大値をまとめて `O(N)` で求めます。

```cpp
vector<int> values = {4, 2, 2, 5, 1};
vector<int> minimums = sliding_window_minimum(values, 3);  // {2, 2, 1}
vector<int> maximums = sliding_window_maximum(values, 3);  // {4, 5, 5}

vector<int> positions =
    sliding_window_best_indices(values, 3, less<int>());  // 最小値の位置
```

## FlatGrid

ヘッダ: [`flat-grid.hpp`](library/flat-grid.hpp)

`vector<vector<T>>` と同じ感覚で使える、1本の連続した `vector` です。
大きな盤面を行方向へ何度も全走査する時に向きます。

```cpp
FlatGrid<int> distance(height, width, -1);
distance(start_row, start_column) = 0;

for (int row = 0; row < height; ++row) {
  for (int column = 0; column < width; ++column) {
    int value = distance(row, column);
  }
}

distance.fill(-1);  // 全マスを再初期化
```

マスを整数1個で持ちたい場合は `index(row, column)`、元へ戻す場合は
`position(index)` を使えます。

## 座標圧縮

ヘッダ: [`coordinate-compression.hpp`](library/coordinate-compression.hpp)

```cpp
vector<long long> xs = {100, 20, 100, 50};
CoordinateCompression<long long> compression(xs);

int compressed = compression.index(50);  // 1
long long original = compression.value(compressed);  // 50
vector<int> all = compression.compress(xs);  // {2, 0, 2, 1}
```

## DSU

ヘッダ: [`dsu.hpp`](library/dsu.hpp)

```cpp
Dsu dsu(n);
dsu.unite(a, b);

if (dsu.same(a, b)) {
  cout << "connected\n";
}
cout << dsu.size(a) << '\n';
cout << dsu.component_count() << '\n';

for (const vector<int>& group : dsu.groups()) {
  // 同じ連結成分の頂点が group に入る
}
```

## WeightedDsu

ヘッダ: [`weighted-dsu.hpp`](library/weighted-dsu.hpp)

`potential[b] - potential[a] = difference` という頂点間の差分制約を、
追加しながら管理する重み付きUnion-Findです。

```cpp
WeightedDsu<long long> dsu(n);

if (!dsu.unite(a, b, difference)) {
  // これまでの制約と矛盾する
}

if (dsu.same(a, b)) {
  long long difference_b_minus_a = dsu.difference(a, b);
}
```

`unite` は新しく結合した時だけでなく、既存の制約と整合する場合もtrueです。
矛盾する場合はfalseを返し、DSUの状態は変更しません。

## Graph BFS

ヘッダ: [`graph-bfs.hpp`](library/graph-bfs.hpp)

すべての辺を1回の移動として扱う、重みなしグラフの最短距離です。

```cpp
vector<vector<int>> graph(n);
graph[a].push_back(b);
graph[b].push_back(a);  // 無向辺なら逆向きも追加

auto shortest = graph_bfs(graph, start);
cout << shortest.distance[goal] << '\n';
vector<int> path = shortest.path_to(goal);
```

到達不能な頂点の距離は `-1`、経路は空になります。

## All-Pairs BFS

ヘッダ: [`all-pairs-bfs.hpp`](library/all-pairs-bfs.hpp)

重みなしグラフで、後から任意の2頂点間距離を何度も使う場合に全始点のBFSを先に行います。

```cpp
vector<vector<int>> graph(vertex_count);
vector<vector<unsigned short>> distance =
    all_pairs_bfs<unsigned short>(graph);

cout << distance[from][to] << '\n';
```

`Distance` を省略すると `int` です。最大距離が65,534以下なら `unsigned short` を使うと
距離表のメモリを半分程度にできます。到達不能値は `numeric_limits<Distance>::max()` です。
計算量は `O(V(V+E))`、メモリは `O(V^2)` なので、頂点数が大きい時は必要な始点だけ
`graph-bfs.hpp` で処理してください。

## Bipartite Check

ヘッダ: [`bipartite-check.hpp`](library/bipartite-check.hpp)

無向グラフのすべての辺で両端の色が異なるよう、0/1の2色に塗れるか判定します。

```cpp
auto result = bipartite_check(graph);

if (result.bipartite) {
  for (int vertex = 0; vertex < n; ++vertex) {
    int side = result.color[vertex];  // 0または1
  }
}
```

グラフが非連結でも、すべての成分を調べます。自己ループがあれば二部グラフではありません。

## Dijkstra

ヘッダ: [`dijkstra.hpp`](library/dijkstra.hpp)

辺のコストが0以上の重み付きグラフで、始点からの最短距離を求めます。
無向辺には `add_undirected_edge`、有向辺には `add_directed_edge` を使います。

```cpp
vector<vector<pair<int, long long>>> graph(n);
add_undirected_edge(graph, 0, 1, 5LL);
add_undirected_edge(graph, 1, 2, 3LL);
add_directed_edge(graph, 0, 2, 20LL);

const long long INF = (1LL << 60);
auto shortest = dijkstra(graph, 0, INF);

cout << shortest.distance[2] << '\n';  // 8
vector<int> path = shortest.path_to(2);  // {0, 1, 2}

if (!shortest.reachable(3)) {
  // 始点0から頂点3へは到達できない
}
```

`INF + 辺コスト` が型の上限を超えないよう、`INF` には少し余裕を持たせます。
負の辺があるグラフには使えません。

## Bellman–Ford

ヘッダ: [`bellman-ford.hpp`](library/bellman-ford.hpp)

負の辺を含む有向グラフの最短路と、始点から辿り着ける負閉路の
影響を求めます。

```cpp
vector<BellmanFordEdge<long long>> edges;
edges.push_back({from, to, cost});

const long long INF = (1LL << 60);
auto shortest = bellman_ford(n, edges, start, INF);

if (shortest.shortest_path_exists(goal)) {
  cout << shortest.distance[goal] << '\n';
  vector<int> path = shortest.path_to(goal);
} else if (shortest.affected_by_negative_cycle[goal]) {
  // 負閉路を何度も回って距離を下げられる
}
```

計算量は `O(NM)` です。辺コストがすべて0以上なら、Dijkstraを使う方が高速です。

## 0-1 BFS

ヘッダ: [`zero-one-bfs.hpp`](library/zero-one-bfs.hpp)

辺のコストが0か1だけなら、Dijkstraより単純なdequeで最短距離を求められます。

```cpp
vector<vector<pair<int, int>>> graph(n);
graph[from].push_back({to, 0});
graph[from].push_back({another, 1});

auto shortest = zero_one_bfs(graph, start);
cout << shortest.distance[goal] << '\n';
vector<int> path = shortest.path_to(goal);
```

コスト2以上や負のコストを入れてはいけません。

## RadixHeap

ヘッダ: [`radix-heap.hpp`](library/radix-heap.hpp)

最後に取り出したキー以上のキーだけを追加できる、高速な優先度付きキューです。
Dijkstraの非負整数距離など、条件が合う場面で `priority_queue` の代わりに使えます。

```cpp
RadixHeap<int> queue;
queue.push(0, start);

while (!queue.empty()) {
  auto [distance, vertex] = queue.pop();
  // 新しくpushするキーは distance 以上にする
}
```

キーは `uint64_t` です。負数、小数、取り出したキーより小さいキーを後から
追加する用途には使えません。その場合は通常の `priority_queue` を使います。

## Grid BFS

ヘッダ: [`grid-bfs.hpp`](library/grid-bfs.hpp)

文字グリッドなら `'#'` を壁として、上下左右の最短距離を求められます。

```cpp
vector<string> grid = {
    "...#",
    ".#..",
    "....",
};

auto shortest = grid_bfs(grid, {0, 0});
cout << shortest.distance[2][3] << '\n';

vector<pair<int, int>> path = shortest.path_to({2, 3});
for (auto [row, column] : path) {
  grid[row][column] = 'o';
}
```

通れる条件が複雑なら、判定関数を直接渡します。

```cpp
auto shortest = grid_bfs(height, width, start, [&](int row, int column) {
  return height_map[row][column] <= limit;
});
```

## Topological Sort

ヘッダ: [`topological-sort.hpp`](library/topological-sort.hpp)

仕事の依存関係などを、必ず前提が先に来る順番へ並べます。

```cpp
vector<vector<int>> graph(n);
graph[before].push_back(after);

auto result = topological_sort(graph);
if (result.has_cycle) {
  // 循環する依存関係がある
} else {
  for (int vertex : result.order) {
    // この順番なら、すべての辺は前から後ろへ向く
  }
}
```

## Functional Graph

ヘッダ: [`functional-graph.hpp`](library/functional-graph.hpp)

各頂点から出る辺がちょうど1本のグラフを解析します。各頂点は最終的にどれかの周期に入ります。

```cpp
vector<int> next(n);
FunctionalGraph graph(next);

int after = graph.jump(start, steps);  // stepsはunsigned long longまで
int entry = graph.cycle_entry[start];
int distance = graph.steps_to_cycle[start];
int length = graph.cycle_length(start);
int id = graph.cycle_id[start];

for (int vertex : graph.cycles[id]) {
  // startが最終的に入る周期の頂点
}
```

64段のダブリング表を作るため、構築は `O(64N)`、1回の `jump` は `O(64)` です。

## Strongly Connected Components

ヘッダ: [`strongly-connected-components.hpp`](library/strongly-connected-components.hpp)

有向グラフを、互いに行き来できる頂点のグループへ分けます。再帰を使わないので、
長い一本道でも再帰の深さを気にせず使えます。

```cpp
auto components = strongly_connected_components(graph);

cout << components.component_count() << '\n';
if (components.same(a, b)) {
  // aからbへも、bからaへも到達できる
}

int id = components.component_id[vertex];
for (int member : components.groups[id]) {
  // vertex と同じ強連結成分
}
```

`groups` は成分間の辺が前から後ろへ向く順番です。

## Floyd–Warshall

ヘッダ: [`floyd-warshall.hpp`](library/floyd-warshall.hpp)

小さなグラフの全頂点間最短距離をまとめて求めます。`O(N^3)` なので、頂点数が
大きい場合はBFSやDijkstraを必要な始点からだけ実行します。

```cpp
const long long INF = (1LL << 60);
vector<vector<long long>> distance(n, vector<long long>(n, INF));
for (int i = 0; i < n; ++i) distance[i][i] = 0;

distance[from][to] = cost;
floyd_warshall(distance, INF);

cout << distance[start][goal] << '\n';
if (has_negative_cycle(distance)) {
  // 負閉路が存在する
}
```

`INF + 有限距離` がオーバーフローしないよう、`INF` は型の最大値より余裕を
持たせてください。

## Kruskal

ヘッダ: [`kruskal.hpp`](library/kruskal.hpp)

無向グラフを最小コストで連結する辺を選びます。

```cpp
vector<KruskalEdge<long long>> edges;
edges.push_back({a, b, cost});

auto tree = kruskal(n, edges);
if (tree.connected()) {
  cout << tree.total_cost << '\n';
  for (auto edge : tree.edges) {
    // edge.from, edge.to, edge.cost
  }
}
```

元のグラフが連結でなければ、各成分の最小全域木を合わせた森を返します。

## Lowest Common Ancestor

ヘッダ: [`lowest-common-ancestor.hpp`](library/lowest-common-ancestor.hpp)

木を根付き木として前計算し、LCA・頂点間距離・パス上の頂点を求めます。

```cpp
LowestCommonAncestor tree_lca(tree, root);

int common = tree_lca.lca(a, b);
int edge_count = tree_lca.distance(a, b);
int parent = tree_lca.kth_ancestor(vertex, steps);
int on_path = tree_lca.jump(a, b, steps_from_a);
```

木は連結な無向隣接リストで渡します。再帰を使わないため、一本道でも
再帰スタックを消費しません。

## Tree Diameter

ヘッダ: [`tree-diameter.hpp`](library/tree-diameter.hpp)

木の中で最も距離が長い2頂点と、その間の経路を `O(N)` で求めます。
辺の重みは0以上です。

```cpp
vector<vector<pair<int, long long>>> tree(n);
tree[a].push_back({b, cost});
tree[b].push_back({a, cost});

auto diameter = tree_diameter(tree);
cout << diameter.length << '\n';
for (int vertex : diameter.path) {
  // endpoint_a から endpoint_b までの頂点列
}
```

重みなしの木では、各辺の `cost` を1にします。再帰は使いません。

## Max Flow

ヘッダ: [`max-flow.hpp`](library/max-flow.hpp)

辺ごとの容量を超えないように、始点から終点まで運べる最大量を
Dinic法で求めます。容量が大きい時は `long long` を使います。

```cpp
MaxFlow<long long> flow(n);
flow.add_edge(0, 1, 10);
flow.add_edge(0, 2, 5);
flow.add_edge(1, 3, 7);
flow.add_edge(2, 3, 8);

long long maximum = flow.flow(0, 3);
```

最大流を流した後、`min_cut(source)` が `true` の頂点と `false` の頂点の
間に最小カットがあります。`get_edge(index)` で元の容量と実際の流量も読めます。

## Min-Cost Flow

ヘッダ: [`min-cost-flow.hpp`](library/min-cost-flow.hpp)

流す量だけでなく、合計コストも最小にしたい場合に使います。
このパーツで `add_edge` に渡せる辺コストは0以上です。

```cpp
MinCostFlow<int, long long> flow(n);
flow.add_edge(0, 1, 2, 5);  // 容量2、1単位あたりコスト5
flow.add_edge(1, 3, 2, 4);
flow.add_edge(0, 2, 1, 1);
flow.add_edge(2, 3, 1, 2);

auto [sent, minimum_cost] = flow.flow(0, 3, 3);
// 3単位すべて流せなければ sent < 3
```

`Cost` には逆向き残余辺の負コストを保存するため、`long long` など符号付き型を
使います。`flow` は1つのインスタンスに対し1回だけ呼びます。

## Bipartite Matching

ヘッダ: [`bipartite-matching.hpp`](library/bipartite-matching.hpp)

人と仕事のような左右の集合から、可能な組を1対1で最大数選びます。

```cpp
BipartiteMatching matching(person_count, job_count);
matching.add_edge(person, job);  // この組は可能

auto result = matching.solve();
cout << result.size() << '\n';
for (auto [person, job] : result.pairs()) {
  // 選ばれた組
}
```

コストのない最大マッチングなら、最大流に変換するよりこちらが簡単です。

## Hungarian Algorithm

ヘッダ: [`hungarian.hpp`](library/hungarian.hpp)

左右の数が同じ費用行列で、全てを1対1に対応させた時の合計費用を最小化します。

```cpp
vector<vector<long long>> cost = {
    {4, 1, 3},
    {2, 0, 5},
    {3, 2, 2},
};
vector<int> assigned_column = hungarian_minimum_assignment(cost);
// assigned_column == {1, 0, 2}、合計費用は5
```

戻り値の `assigned_column[row]` が、その行へ割り当てた列です。行列は正方形にし、費用型には
`long long` や `double` などの符号付き型を使います。計算量は `O(N^3)` です。

## Two-SAT

ヘッダ: [`two-sat.hpp`](library/two-sat.hpp)

各変数がtrue/falseのどちらかで、すべての条件を「AまたはB」で書ける時に
使います。変数番号は0からです。

```cpp
TwoSat sat(variable_count);
sat.add_clause(0, true, 1, false);  // x0 == true OR x1 == false
sat.add_implication(2, true, 0, true);  // x2 == true なら x0 == true
sat.set_value(3, false);                // x3をfalseに固定

if (sat.solve()) {
  const vector<bool>& answer = sat.answer();
} else {
  // すべてを満たす割り当てはない
}
```

## StaticModInt

ヘッダ: [`static-mod-int.hpp`](library/static-mod-int.hpp)

modをコンパイル時に固定し、剰余の四則演算を普通の数のように書けます。

```cpp
using Mint = StaticModInt<998244353>;

Mint a = 10;
Mint b = 3;
Mint answer = (a + b) * b;
answer /= 2;

cout << answer.value() << '\n';
cout << Mint(2).pow(100).value() << '\n';
```

割り算では、割る数とmodが互いに素である必要があります。値がすでに
`[0, mod)` と保証できる熱いループでは `Mint::raw(value)` を使うと剰余計算を
省けますが、範囲が不明なら通常のコンストラクタを使ってください。

## PrimeTable

ヘッダ: [`prime-table.hpp`](library/prime-table.hpp)

上限までの素数判定と最小素因数を線形時間で前計算します。

```cpp
PrimeTable primes(1000000);

if (primes.is_prime(x)) {
  // xは素数
}

vector<pair<int, int>> factors = primes.factorize(x);
vector<int> divisors = primes.divisors(x);
```

質問する値は構築時の上限以下である必要があります。1の素因数分解は空です。

## ModCombination

ヘッダ: [`mod-combination.hpp`](library/mod-combination.hpp)

素数mod上の `nCk`、`nPk`、重複組合せを事前計算後 `O(1)` で取得します。
このファイル単体で使えるため、`static-mod-int.hpp` は必要ありません。

```cpp
ModCombination<998244353> combination(max_n);

int choose = combination.choose(n, k);          // nCk
int arrange = combination.permutation(n, k);    // nPk
int repeated = combination.multichoose(n, k);   // 重複を許してk個
```

`MOD` は素数、扱う `n` は `MOD` 未満にします。後からより大きい `n` を渡すと、
必要なところまで自動的に事前計算を延長します。

## Extended GCD

ヘッダ: [`extended-gcd.hpp`](library/extended-gcd.hpp)

拡張Euclid互除法で `a*x + b*y = gcd(a,b)` となる `x`, `y` を求めます。

```cpp
auto result = extended_gcd(a, b);
cout << result.gcd << ' ' << result.x << ' ' << result.y << '\n';
```

mod逆元も取得できます。逆元が存在しない場合は `nullopt` です。

```cpp
optional<long long> inverse = modular_inverse(value, modulus);
if (inverse.has_value()) {
  // value * *inverse ≡ 1 (mod modulus)
}
```

## Floor Sum

ヘッダ: [`floor-sum.hpp`](library/floor-sum.hpp)

次の和を `O(log modulus)` で求めます。

```text
floor((a*0+b)/modulus) + ... + floor((a*(n-1)+b)/modulus)
```

```cpp
long long answer = floor_sum(n, modulus, a, b);
```

`n >= 0`, `modulus > 0` が必要で、`a`, `b` は負でも扱えます。
答えだけでなく `a*n+b` などの中間計算も `long long` に収まる範囲で使います。

## Integer Square Root

ヘッダ: [`integer-square-root.hpp`](library/integer-square-root.hpp)

距離の二乗などから、浮動小数の丸め誤差なしで整数平方根を得ます。

```cpp
long long floor_root = floor_integer_square_root(value);
long long ceil_root = ceil_integer_square_root(value);
```

内部では `sqrt(long double)` を初期値にしますが、最後に整数の割り算で補正します。
`value` は0以上の整数型にしてください。

## Matrix

ヘッダ: [`matrix.hpp`](library/matrix.hpp)

動的サイズの行列で、加算・乗算・累乗ができます。要素は連続メモリに保存します。

```cpp
Matrix<long long> matrix({{1, 1}, {1, 0}});
Matrix<long long> powered = matrix_power(matrix, exponent);

cout << powered(0, 1) << '\n';
```

`Matrix<StaticModInt<998244353>>` のように、四則演算と `0`, `1` を扱える型にも
使えます。行列乗算は `i-k-j` 順で、左側要素が0の計算は飛ばします。

## Sequence Overlap

ヘッダ: [`sequence-overlap.hpp`](library/sequence-overlap.hpp)

1つ目の列の末尾と2つ目の列の先頭を、最も長く一致する位置で重ねます。

```cpp
string first = "ABCDE";
string second = "CDEFG";

int overlap = suffix_prefix_overlap(first, second);  // 3
string merged = merge_with_overlap(first, second);  // "ABCDEFG"
```

`vector<int>`など、`size()`、`operator[]`、末尾への`insert`を使える列にも
同じ関数を使えます。短い列向けの分かりやすい実装で、計算量は
`O(min(N,M)^2)`です。長い列を大量に比較する場合はZ algorithmやrolling hashを
使ってください。

## Probability Move DP

ヘッダ: [`probability-move-dp.hpp`](library/probability-move-dp.hpp)

移動が確率 `0.8` で成功し、失敗するとその場に残る3状態の例です。状態2をゴールとして
吸収するので、一度着いた確率は次のターンへ残りません。

```cpp
ProbabilityMoveDP<double> dp(3, 0);  // 状態数3、開始状態0
vector<int> move_right = {1, 2, 2};  // 成功時: 0->1, 1->2, 2->2

double arrived_now = dp.step(move_right, 0.8, 2);
double not_arrived = dp.remaining_probability();
```

コマンドごとに遷移先が違う場合は、`move_up`、`move_down` のような配列を先に作り、
選んだ配列を `step` へ渡します。壁へ進む状態では遷移先を自分自身にすれば、成功側と
失敗側の確率が正しく同じ状態へ足されます。確率型は標準の `double` のほか、必要なら
`long double` や `float` に変更できます。

## Rolling Hash

ヘッダ: [`rolling-hash.hpp`](library/rolling-hash.hpp)

文字列や整数列の部分列が同じかを高速に比べます。

```cpp
RollingHash hash(text);

if (hash.hash(first_left, first_right) ==
    hash.hash(second_left, second_right)) {
  // 2つの部分文字列は高確率で同じ
}

RollingHash another(other_text);
int common = hash.longest_common_prefix(
    0, hash.size(), another, 0, another.size());
```

64bit hashなので衝突可能性は0ではありません。誤判定が絶対に許されない最終確認では、
元の文字列も比較してください。異なる `base` で作った2個を比較してはいけません。

## Z Algorithm

ヘッダ: [`z-algorithm.hpp`](library/z-algorithm.hpp)

列の各位置について、「その位置からの列」と列全体の先頭が何要素一致するかを
`O(N)` で求めます。

```cpp
vector<int> z = z_algorithm(text);
// z[i] = text[0...] と text[i...] の最長共通接頭辞
```

`string` だけでなく、`vector<int>` など `==` で比較できる列にも使えます。

## Prefix Function / KMP

ヘッダ: [`prefix-function.hpp`](library/prefix-function.hpp)

パターンの出現位置をすべて、文字列の長さに比例する時間で探します。

```cpp
vector<int> positions = find_pattern_occurrences(text, pattern);

vector<int> prefix = prefix_function(pattern);
// prefix[i] = pattern[0..i]の、接頭辞でも接尾辞でもある最長長さ
```

こちらはhash衝突がなく、完全に正確な一致検索です。`string` と `vector<int>` などに
使えます。空のpatternは0から `text.size()` までの全位置に一致します。

## Manacher

ヘッダ: [`manacher.hpp`](library/manacher.hpp)

列の各中心に対する奇数長・偶数長の最長回文を、全体 `O(N)` で求めます。

```cpp
auto palindromes = palindrome_radii(text);

bool yes = palindromes.is_palindrome(left, right);  // [left, right)
auto [longest_left, longest_right] = palindromes.longest_interval();

int odd_length = 2 * palindromes.odd[center] - 1;
int even_length = 2 * palindromes.even[center];
```

`string` だけでなく、`vector<int>` など `==` で比較できる列に使えます。

## Longest Increasing Subsequence

ヘッダ: [`longest-increasing-subsequence.hpp`](library/longest-increasing-subsequence.hpp)

最長増加部分列の長さだけでなく、選んだ位置と値も復元します。

```cpp
auto strict = longest_increasing_subsequence(values);        // a < b
auto non_strict = longest_increasing_subsequence(values, false);  // a <= b

cout << strict.length() << '\n';
for (int index : strict.indices) cout << index << ' ';
for (auto value : strict.values) cout << value << ' ';
```

## Inversion Count

ヘッダ: [`inversion-count.hpp`](library/inversion-count.hpp)

`i < j` かつ `values[i] > values[j]` となる組の個数を `O(N log N)` で数えます。

```cpp
long long inversions = inversion_count(values);
```

同じ値同士は逆転に数えません。値は内部で座標圧縮されるため、負数や大きな値も
そのまま渡せます。

## Fast I/O

ヘッダ: [`fast-io.hpp`](library/fast-io.hpp)

入力が非常に大きく、通常の `cin` がボトルネックになった場合だけ使います。

```cpp
FastInput input;
FastOutput output;

int n = input.next<int>();
long long x = input.next<long long>();
string name = input.next<string>();

output.write_integer(n, ' ');
output.write_integer(x, '\n');
output.write_string(name);
output.write_character('\n');
```

整数、空白区切り文字列、空白以外の1文字に対応します。小数や1行全体の入力には
通常の入出力を使ってください。`FastOutput` は終了時に残りをまとめて出力します。

## RollbackArray

ヘッダ: [`rollback-array.hpp`](library/rollback-array.hpp)

変更案を実際に配列へ反映して評価し、不採用なら元へ戻せます。

```cpp
RollbackArray<int> state(initial_state);

int snapshot = state.snapshot();
state.set(i, new_value_i);
state.set(j, new_value_j);

if (accept_move) {
  // 変更後の状態が残る。もう以前へ戻さない時だけ履歴を破棄する
  state.clear_history();
} else {
  state.rollback(snapshot);
}
```

`clear_history()`は現在値を変えず、蓄積した履歴の容量を再利用できる状態で
履歴件数だけを0にします。それ以前に取得したsnapshot番号は全て無効になります。
入れ子の試行中や、外側のsnapshotへまだ戻る可能性がある時は呼ばないでください。

## 差分更新とapply/revertの検査

ヘッダ: [`debug-state-check.hpp`](library/debug-state-check.hpp)

`AhcDebugStateCheck`は、差分更新後の値を独立な全再計算と照合した時に、再現に必要な
seed、反復番号、Move列、最初に違った項目をまとめて報告します。

```cpp
#ifndef NDEBUG
State before = state;
#endif

apply(state, move);

#ifndef NDEBUG
AhcDebugStateCheck check(seed, iteration, moves_text);
check.require_equal("score", state.score, calculate_score_from_scratch(state));
check.require_equal("hash", state.hash, calculate_hash_from_board(state));
check.require(is_legal(state), "legality");
#endif
```

浮動小数点は`require_close(name, actual, expected, absolute, relative)`を使います。
全Stateの`memcmp`はせず、盤面、score、hash、cache、候補集合を個別に比較します。
実行可能な経路swapとグリッド更新の検査は
[`examples/debug/`](examples/debug/README.md) にあります。

## 行動列を途中からだけ再生する

ヘッダ: [`prefix-replay.hpp`](library/prefix-replay.hpp)

`PrefixReplay<State, Action>` は、確定済みの行動列の途中状態を保存し、
次の候補と共通する前半を再利用します。重いシミュレーションで行動列を変更する時に使います。

```cpp
struct State { long long sum = 0; };
auto step = [](State& state, const int& action) { state.sum += action; };
PrefixReplay<State, int> replay(State{}, 2);  // 2行動ごとに途中状態を保存

vector<int> actions{1, 2, 3, 4};
replay.evaluate(actions, step);
replay.commit();  // 現在解を確定。合計は10

actions[2] = 8;
const State& trial = replay.evaluate(actions, step);
cout << trial.sum << '\n';                       // 15
cout << replay.last_replayed_actions() << '\n';  // 後半の2行動だけを実行
if (trial.sum > replay.current_end().sum) {
  replay.commit();
} else {
  replay.discard();
}
```

`evaluate` だけでは現在解は変わりません。採用時に `commit`、不採用時に `discard` を呼びます。
返された参照は、次の `evaluate` / `commit` / `discard` より前だけ使ってください。
途中状態の保存間隔を大きくすると保存量は減り、再実行する行動が増えます。
行動列の比較やコピーは `O(列長)` なので、処理全体が変更箇所数だけの時間になるわけではありません。

`step` は同じ状態と行動から同じ結果を作る必要があります。乱数を使う場合は乱数器の状態も
`State` に含め、外部入力は固定してください。独自の `Action` 型には、
同じ遷移を表すかを正しく判定する `operator==` を用意します。

## RollbackDsu

ヘッダ: [`rollback-dsu.hpp`](library/rollback-dsu.hpp)

辺を仮に追加して連結性を調べ、追加前へ戻せます。

```cpp
RollbackDsu dsu(n);

int snapshot = dsu.snapshot();
dsu.unite(a, b);
dsu.unite(c, d);

bool connected = dsu.same(x, y);
dsu.rollback(snapshot);
```

## StampArray

ヘッダ: [`stamp-array.hpp`](library/stamp-array.hpp)

BFSごとに大きな配列全体を `fill` したくない時に使います。

```cpp
StampArray<int> distance(n, -1);

for (int start : starts) {
  distance.clear();
  queue<int> que;
  distance[start] = 0;
  que.push(start);

  while (!que.empty()) {
    int v = que.front();
    que.pop();
    // distance[to] は未訪問なら -1
  }
}
```

## DenseIntSet

ヘッダ: [`dense-int-set.hpp`](library/dense-int-set.hpp)

値の範囲が `0 <= value < universe_size` と分かっている整数集合です。
`unordered_set` と違い、hash計算や実行中のメモリ確保がありません。

```cpp
DenseIntSet active(vertex_count);
active.insert(vertex);
active.erase(vertex);

if (active.contains(vertex)) {
  // 集合に入っている
}

for (int i = 0; i < active.size(); ++i) {
  int vertex = active[i];
}

active.clear();  // O(1)
```

削除で要素の順番は変わります。乱数で `[0, active.size())` のindexを選べば、
要素を `O(1)` でランダム選択できます。

## BinaryTrie

ヘッダ: [`binary-trie.hpp`](library/binary-trie.hpp)

非負整数の多重集合から、指定値とのXORが最小・最大になる要素を探します。

```cpp
BinaryTrie<unsigned> values;
values.insert(x);
values.insert(x);  // 重複も保存できる
values.erase(x);   // 1個だけ削除

unsigned nearest = values.minimum_xor_element(query);
unsigned farthest = values.maximum_xor_element(query);
unsigned minimum_xor = values.minimum_xor_value(query);
```

値が24bit未満と分かっている時は `BinaryTrie<unsigned, 24>` のようにbit数を
狭めると、時間とメモリを削減できます。空の時にXOR検索は呼べません。

## XorBasis

ヘッダ: [`xor-basis.hpp`](library/xor-basis.hpp)

追加した整数から任意の部分集合を選んだ時のXORを、最大bit数個の基底で表します。

```cpp
XorBasis<unsigned long long> basis;

basis.insert(value);
bool possible = basis.contains(target);
unsigned long long maximum = basis.maximum_xor();
unsigned long long best_with_seed = basis.maximum_xor(seed);
unsigned long long minimum_with_seed = basis.minimum_xor(seed);
```

`rank()` は独立な基底数で、作れる異なるXOR値は `2^rank()` 個です。
値が20bit未満なら `XorBasis<unsigned, 20>` のようにbit数を狭められます。
`BinaryTrie` が「集合に実際に入っている1要素」を探すのに対し、`XorBasis` は
「複数要素の部分集合XOR」を扱います。

## BatchedTimer

ヘッダ: [`batched-timer.hpp`](library/batched-timer.hpp)

1反復が非常に軽い時、毎回時計を見るコストを減らします。指定回数の間は時間切れを
確認しないので、1反復が重い処理には通常の `Timer` を使ってください。

```cpp
BatchedTimer timer(1900.0, 256);

while (!timer.is_over()) {
  // 軽い探索を1回進める
  double progress = timer.cached_progress();
}
```

## MultiStart

ヘッダ: [`multi-start.hpp`](library/multi-start.hpp)

ランダム初期解を複数作り、一番良いものを選びます。

```cpp
auto generate = [&]() {
  State state;
  // random を使って state を作る
  return state;
};

auto evaluate = [](const State& state) {
  return state.score;
};

State best = multi_start<State>(100, generate, evaluate);  // 100回、最大化
State best_by_time =
    time_based_multi_start<State>(500.0, generate, evaluate);  // 500ms
```

最小化では最後の引数へ `false` を渡します。

## SimpleBeamSearch

ヘッダ: [`simple-beam-search.hpp`](library/simple-beam-search.hpp)

最初に使うための、状態を丸ごとコピーするビームサーチです。`State` の中へ操作履歴も
入れておけば、返された状態からそのまま答えを取り出せます。

```cpp
struct State {
  int position = 0;
  long long score = 0;
  vector<int> answer;
};

auto expand = [](const State& state) {
  vector<State> next_states;
  for (int direction : {-1, 1}) {
    State next = state;
    next.position += direction;
    next.score = next.position;
    next.answer.push_back(direction);
    next_states.push_back(std::move(next));
  }
  return next_states;
};

auto evaluate = [](const State& state) { return state.score; };

State answer = simple_beam_search(State{}, 100, 200, expand, evaluate);
```

`evaluate`は候補を残す順位用です。提出解は問題本来の得点で別に
比べます。一部の状態だけ早くterminalになる場合、子を持たない状態は
次の層で消えます。さらに幅から落ちると現在層にも一度も現れないため、
生成直後に全候補を見る`step_and_observe`で別保存します。

```cpp
SimpleBeamSearch<State, long long> beam(State{}, 200);
beam.step_and_observe(
    expand, evaluate,
    [&](const State& child, const long long&) {
      if (is_terminal(child)) save_if_better(child);
    });
```

同じ層の同じ状態へ別経路から到達するなら、`step_with_key`を使えます。

```cpp
SimpleBeamSearch<State, long long> beam(State{}, 200);
for (int turn = 0; turn < 100; ++turn) {
  if (!beam.step_with_key(expand, evaluate, make_key)) break;
}
```

一時`vector`を作らず子を直接渡す場合は、`step_each`を使います。

```cpp
beam.step_each(
    [&](const State& parent, auto&& emit) {
      for (Move move : make_moves(parent)) {
        State child = parent;
        apply(child, move);
        emit(std::move(child));
      }
    },
    evaluate);
```

## TreeBeamSearch

ヘッダ: [`tree-beam-search.hpp`](library/tree-beam-search.hpp)

状態が大きくコピーが重い時の発展版です。1手進める `apply` と、その1手だけを
正確に戻す `revert` を用意します。

```cpp
struct State {
  int position = 0;
  long long score = 0;
};

struct Move {
  int difference;
};

auto expand = [](const State&) {
  return vector<Move>{{-1}, {1}};
};

auto apply = [](State& state, const Move& move) {
  state.position += move.difference;
  state.score += move.difference;
};

auto revert = [](State& state, const Move& move) {
  state.position -= move.difference;
  state.score -= move.difference;
};

auto evaluate = [](const State& state) { return state.score; };

State initial;
TreeBeamSearch<State, Move, long long> beam(initial, initial.score, 200);

for (int turn = 0; turn < 100; ++turn) {
  if (!beam.step(expand, apply, revert, evaluate)) break;
}

vector<Move> answer = beam.restore();
```

`apply` でscore、hash、補助配列も変更したなら、`revert` ではそれらを全部戻します。
`Move`には上書き前の値など、復元に必要な情報も入れます。
現在幅の状態を確認する時は`for_each_state(visit, apply, revert)`を使えます。
`visit(rank, state)`内では`beam.restore(rank)`でその状態の手順を復元できます。
幅から落ちる生成直後の候補まで見る場合は`step_and_observe`を使い、
`restore_candidate(parent_rank, move)`でその候補の手順を復元します。

同じ盤面へ別の手順で到達する問題では、`step_with_key` を使えます。
`state.hash` が同じ候補は、評価値が一番良い1件だけ残ります。

```cpp
for (int turn = 0; turn < 100; ++turn) {
  bool advanced = beam.step_with_key(
      expand,
      apply,
      revert,
      evaluate,
      [](const State& state) { return state.hash; });
  if (!advanced) break;
}
```

`hash` は自分で更新します。盤面のswapなら `zobrist-hash.hpp` と組み合わせると
差分更新しやすいです。64bit hashにはごく小さい衝突可能性があります。

## CostTreeBeamSearch

ヘッダ: [`cost-tree-beam-search.hpp`](library/cost-tree-beam-search.hpp)

状態コピーを避けたい点は`TreeBeamSearch`と同じですが、1行動の
消費手数が1、2、3と異なる時に使います。同じ到着世代の候補だけを
比べます。

```cpp
struct Move {
  int difference;
  int advance;
};

auto get_advance = [](const Move& move) { return move.advance; };

CostTreeBeamSearch<State, Move, long long> beam(
    initial, initial.score, 200, 100);  // 幅200、最大世代100

while (beam.step(expand, apply, revert, evaluate, get_advance)) {
}

vector<Move> answer = beam.restore();
```

`advance`は正の整数にします。最大世代を超える行動は自動で候補から
外れます。重複を消す場合は、同じ1つのbeamで`step`と`step_with_key`を
混ぜず、最初から最後までどちらか一方を使います。
途中で幅を縮める時は`set_beam_width`を使います。現在層だけでなく、
既に予約された未来層も同じ幅まで縮みます。

順位評価、terminal保存、keyの作り方、`apply / revert`の確認方法は
[`SEARCH_GUIDE.md`](SEARCH_GUIDE.md)にまとめています。

## SharedHistory

ヘッダ: [`shared-history.hpp`](library/shared-history.hpp)

候補ごとに長い操作列をコピーせず、最後の操作と親番号だけを保存します。

```cpp
SharedHistory<Move> history;
int root = history.root();

int first = history.add(root, first_move);
int second = history.add(first, second_move);
int another = history.add(first, another_move);

vector<Move> answer = history.restore(second);
```

## BestByKey

ヘッダ: [`best-by-key.hpp`](library/best-by-key.hpp)

複数の操作列が同じ状態に到達する時、同じ状態キーの中で一番良い候補だけ残します。

```cpp
BestByKey<uint64_t, long long, State> unique_candidates;

for (State state : candidates) {
  unique_candidates.add(state.hash, state.score, std::move(state));
}

for (auto& entry : unique_candidates.entries) {
  // entry.key, entry.score, entry.state を使う
}
```

最小化なら `BestByKey<uint64_t, long long, State> unique(false);` です。

## ZobristHash

ヘッダ: [`zobrist-hash.hpp`](library/zobrist-hash.hpp)

配列の1か所を変更した時、配列全体を見直さずhashを更新できます。

```cpp
vector<int> values(n);  // 各値は [0, value_kinds)
ZobristHash zobrist(n, value_kinds, 123);
uint64_t hash = zobrist.build(values);

int old_value = values[position];
int new_value = 3;
zobrist.change(hash, position, old_value, new_value);
values[position] = new_value;
```

64bit hashにも衝突の可能性はあります。デバッグ中はhashが同じ候補について、可能なら
元の状態も比較してください。

## FixedVector

ヘッダ: [`fixed-vector.hpp`](library/fixed-vector.hpp)

要素数の最大値が分かっている小さな配列で、動的メモリ確保を避けたい時に使います。

```cpp
FixedVector<int, 100> values;
values.push_back(10);
values.push_back(20);

for (int value : values) {
  cout << value << '\n';
}
```

100個を超えて追加するとassertで停止します。

## MoveStatistics

ヘッダ: [`move-statistics.hpp`](library/move-statistics.hpp)

近傍の種類ごとに用意すると、どの近傍が働いているか確認できます。

```cpp
vector<MoveStatistics> statistics(number_of_move_types);

statistics[type].add(accepted, improvement > 0);

cerr << "accept = " << statistics[type].acceptance_rate() << '\n';
cerr << "improve = " << statistics[type].improvement_rate() << '\n';
```

## RouteUtils

ヘッダ: [`route-utils.hpp`](library/route-utils.hpp)

巡回路や配送順の距離を計算します。距離関数を渡すので、点や距離の型は自由です。

```cpp
vector<pair<int, int>> route{{0, 0}, {10, 5}, {20, 5}, {0, 0}};

auto manhattan = [](auto a, auto b) {
  return abs(a.first - b.first) + abs(a.second - b.second);
};

int cost = route_length(route, manhattan);

// route[2] の直前へ (12, 8) を挿入した場合の距離変化
int insert_delta =
    route_insertion_delta(route, 2, pair{12, 8}, manhattan);

// route[1] を削除した場合の距離変化
int remove_delta = route_removal_delta(route, 1, manhattan);

// route[1]〜route[2] をreverseした場合の距離変化
int reverse_delta = route_reverse_delta(route, 1, 2, manhattan);
```

差分はすべて `変更後 - 変更前` です。`route_reverse_delta` はManhattan距離など、
行きと帰りの距離が同じ場合にだけ使えます。

## Point2D

ヘッダ: [`point-2d.hpp`](library/point-2d.hpp)

整数2次元座標の足し引き、内積、外積、距離をまとめて使えます。

```cpp
Point2D<long long> a{x1, y1};
Point2D<long long> b{x2, y2};

auto difference = b - a;
long long distance2 = squared_distance(a, b);
long long manhattan = manhattan_distance(a, b);
long long cross_value = cross(origin, a, b);
int turn = orientation(origin, a, b);  // 左1、一直線0、右-1
```

乗算の結果も座標型になるため、座標が大きい時は `long long` を使います。

## AxisAlignedRectangle

ヘッダ: [`axis-aligned-rectangle.hpp`](library/axis-aligned-rectangle.hpp)

軸に平行な長方形を `{left, bottom, right, top}` の順で持ちます。
範囲は半開区間 `[left, right) × [bottom, top)` です。

```cpp
using Rect = AxisAlignedRectangle<int>;
Rect a{0, 0, 4, 3};
Rect touching{4, 0, 6, 3};
cout << a.area() << '\n';                // 12
cout << a.contains(3, 2) << '\n';         // true
cout << a.contains(4, 2) << '\n';         // false: 右端は含まない
cout << a.overlaps(touching) << '\n';     // false: 辺が接するだけ
```

`left < right` かつ `bottom < top` の時だけ `is_valid()` が真です。
`width()`、`height()`、`area()` を呼ぶ前に、有効な長方形であることを確かめます。
面積の戻り値は `long long` なので、幅・高さ・面積が使用する整数型に収まる範囲で使います。

## 指定セルを含む最大の空き長方形

ヘッダ: [`axis-aligned-rectangle.hpp`](library/axis-aligned-rectangle.hpp), [`largest-empty-rectangle.hpp`](library/largest-empty-rectangle.hpp)

整数座標の範囲内で、指定セルを含み、障害物に重ならない最大面積の長方形を求めます。
`axis-aligned-rectangle.hpp` と `largest-empty-rectangle.hpp` を読み込みます。

```cpp
using Rect = AxisAlignedRectangle<int>;
Rect bounds{0, 0, 10, 8};
vector<Rect> obstacles{{4, 0, 6, 8}};
LargestEmptyRectangleWorkspace<Rect> workspace;
Rect empty = largest_empty_rectangle(bounds, 1, 2, obstacles, workspace);
cout << empty.left << ' ' << empty.bottom << ' '
     << empty.right << ' ' << empty.top << '\n';  // 0 0 4 8
cout << empty.area() << '\n';                    // 32
```

対象セル `(1, 2)` は `[1, 2) × [2, 3)` を表します。セルが範囲外、障害物の内部、
または長方形が無効な場合には `invalid_argument` が送出されます。
障害物同士は重なっていても構いません。配置し直す長方形自身は障害物リストから外します。

障害物数を `M` とすると時間は `O(M² + M log M)`、追加メモリは `O(M)` です。
繰り返し呼ぶ場合は `workspace` を再利用できます。最大化するのは面積なので、
別の評価式を使う問題では、返った長方形をその評価式でも確認してください。

## Farthest Point Sampling

ヘッダ: [`farthest-point-sampling.hpp`](library/farthest-point-sampling.hpp)

全要素を毎回評価できない時に、互いに離れた代表を選びます。距離関数を渡すので、座標型や
距離型は自由です。

```cpp
vector<Point2D<long long>> points;

vector<int> representatives = farthest_point_sampling(
    static_cast<int>(points.size()), 16,
    [&](int a, int b) { return squared_distance(points[a], points[b]); });

for (int index : representatives) {
  // points[index] を代表として重い評価を行う
}
```

最初の代表は既定で添字0です。中心などから始めたい場合は第4引数へその添字を渡します。
これはk-center問題の厳密解ではなく、現在の代表集合から最も遠い要素を順に足すgreedyです。
計算量は要素数を `N`、代表数を `K` として `O(NK)` です。

## Convex Hull

ヘッダ: [`convex-hull.hpp`](library/convex-hull.hpp)

`x`, `y` メンバを持つ整数座標の点から凸包を求めます。`point-2d.hpp` の
`Point2D<long long>` のほか、同じメンバを持つ自作structでも使えます。

```cpp
vector<Point2D<long long>> points;
vector<Point2D<long long>> hull = convex_hull(points);
// 辞書順最小の点から反時計回り。先頭点の重複はない
```

辺の途中にある一直線上の点も残したい場合は `convex_hull(points, true)` です。
`convex-hull.hpp` 自体は `point-2d.hpp` をincludeしないため、自作の点型と組み合わせても
単独で貼り付けられます。

## Segment Intersection

ヘッダ: [`segment-intersection.hpp`](library/segment-intersection.hpp)

閉線分同士が交わるかを、整数の外積だけで正確に判定します。

```cpp
bool touch_or_cross = segments_intersect(a, b, c, d);
bool cross_inside = segments_properly_intersect(a, b, c, d);
bool on_segment = point_on_segment(a, b, point);
```

`segments_intersect` は端点の接触や線分の重なりもtrueです。
`segments_properly_intersect` は両線分の内部で交差する場合だけtrueです。

## Interval Union

ヘッダ: [`interval-union.hpp`](library/interval-union.hpp)

壁や時間帯などの半開区間 `[left, right)` をまとめて扱います。接している区間も1本へ
併合されます。

```cpp
vector<pair<int, int>> first{{1, 5}, {8, 10}};
vector<pair<int, int>> second{{3, 9}};

auto merged = merge_half_open_intervals(first);  // [1,5), [8,10)
long long covered = interval_union_length(first);  // 6
long long changed = interval_symmetric_difference_length(first, second);  // 6
```

`interval_symmetric_difference_length` は片方だけに含まれる長さです。日をまたいで壁が
変わった長さ、2つの予約集合の差などを端点走査で正確に計算できます。座標型は `int`、
`long long` などから自由に選べます。

## Hilbert Order

ヘッダ: [`hilbert-order.hpp`](library/hilbert-order.hpp)

2次元の近い点を、1次元でも近くへ並べたい時に使います。座標が0以上 `2^bits` 未満に
なるよう平行移動・拡大してから、返り値でsortします。

```cpp
struct Point { int x, y, id; };
vector<Point> points;

sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
  return hilbert_order_2d(a.x, a.y, 10) <
         hilbert_order_2d(b.x, b.y, 10);
});
```

`bits=10` なら各座標は `0..1023` です。厳密な最近傍順ではありませんが、空間的に
まとまったcluster作成、query順、cache localityの改善候補を1行のkeyで作れます。
