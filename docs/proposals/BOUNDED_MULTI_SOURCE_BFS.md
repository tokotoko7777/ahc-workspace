# 距離上限つき多始点 BFS の試作

複数の始点から指定距離以内に到達できる頂点の和集合だけが必要なら、始点ごとの距離配列の生成と走査は不要。始点をまとめてキューに投入し、指定した層まで探索する。キューは固定長、返り値はビット集合。重複始点は一度だけ処理する。

AHC008では終盤の柵の候補を「少なくとも一人が締切までに行けるマス」に限定するときに使う。人間が同じマスに重なっていても、BFSの始点集合は重複を除ける。得点評価やフロー側の人間の多重度は変更しない。

使用例：

```cpp
auto reachable = bounded_multi_source_bfs<900>(starts, remaining_turns,
    [&](int v) { return adj[v]; }, [&](int v) { return wall[v]; });
```

隣接先は負の値（辺なし）または `[0, MaxVertices)`。始点は常に到達扱いにし、`blocked` は辺を渡る際の行先について評価する。距離配列や経路が必要な場合は既存BFSを使う。

単体テスト：重複始点・空始点・通行止め・空グラフを含む乱択1,500件で、始点ごとの単純なBFSの和集合と比較。

```bash
c++ -std=gnu++23 -Wall -Wextra -pedantic-errors test.cpp -o /tmp/bounded_bfs_test
/tmp/bounded_bfs_test
```

## 試作ヘッダー: bounded-multi-source-bfs.hpp

```cpp
#ifndef AHC_BOUNDED_MULTI_SOURCE_BFS_HPP
#define AHC_BOUNDED_MULTI_SOURCE_BFS_HPP
#include <array>
#include <bitset>
#include <cassert>
#include <span>

// Union of radius-limited BFS neighborhoods. Roots are always included,
// even when blocked as a destination; negative neighbor IDs are absent.
// Does not allocate, and visits each vertex at most once.
template <std::size_t MaxVertices, class Neighbors, class Blocked>
std::bitset<MaxVertices> bounded_multi_source_bfs(
    std::span<const int> roots, int radius, Neighbors neighbors, Blocked blocked) {
  assert(radius >= 0);
  std::bitset<MaxVertices> visited;
  std::array<int, MaxVertices> queue;
  std::size_t head = 0, tail = 0;
  for (int v : roots) {
    assert(0 <= v && std::size_t(v) < MaxVertices);
    if (visited[v]) continue;
    visited[v] = true;
    queue[tail++] = v;
  }
  for (int depth = 0; depth < radius && head < tail; ++depth) {
    const std::size_t level_end = tail;
    for (; head < level_end; ++head) for (int v : neighbors(queue[head])) {
      if (v < 0 || visited[v] || blocked(v)) continue;
      visited[v] = true;
      queue[tail++] = v;
    }
  }
  return visited;
}
#endif
```

## 単体テスト: test.cpp

```cpp
#include "bounded-multi-source-bfs.hpp"
#include <queue>
#include <random>
#include <vector>
int main() {
  constexpr int n=73;
  std::mt19937 rng(8147);
  for(int it=0;it<1500;++it){
    std::array<std::vector<int>,n> g;
    for(int k=0;k<it%150;++k){
      int u=rng()%n,v=rng()%n;
      g[u].push_back(v);
    }
    std::bitset<n> walls,expected;
    for(int i=0;i<it%30;++i)walls.set(rng()%n);
    std::vector<int> roots;
    for(int i=0;i<it%10;++i)roots.push_back(rng()%n);
    int radius=it%12;
    for(int u:roots){
      std::queue<int> q;
      std::array<int,n> d;d.fill(-1);
      d[u]=0;q.push(u);expected.set(u);
      while(!q.empty()){
        int v=q.front();q.pop();
        if(d[v]>=radius)continue;
        for(int w:g[v])if(!walls[w]&&d[w]==-1){
          expected.set(w);d[w]=d[v]+1;q.push(w);
        }
      }
    }
    auto result=bounded_multi_source_bfs<n>(roots,radius,[&](int u)->const auto&{return g[u];},
      [&](int x){return walls[x];});
    if(result!=expected)return 1;
  }
}
```

## AHC008での実測

公式配布100件の通常制限で、得点 6,191,777,779、全頭捕獲100、全員同じ空間100は両版で同じ。全100件の操作列も一致し、比較できた終盤シミュレーションの組数は993→1,020。1件最大24組に固定した別計測でも全操作列・得点・探索組数が一致し、終盤探索が走った同じ17件のソルバーCPU合計は22.79→21.81秒（約4.3%減）。測定は同じローカル環境の逐次実行で、ジャッジ上の時間や点の伸びを示すものではない。
