# 改修提案: 同じ格子で距離を繰り返し求める BFS

同じ壁配置で複数の始点から BFS する場合に、各マスから移動可能な
4 方向を 1 バイトに一度だけまとめる案。盤面が変わったら再準備する。
1 回だけの探索には前処理を伴わない従来の BFS を使う。

`Grid<V, INF>` は 4 近傍の表（盤外は -1、盤内は 0〜V-1）と壁配列を
受ける。距離を int16_t で計算するため `V < INF <= 32767` とする。
範囲外の始点は全マス INF。互換性のため、始点だけ壁でも隣の空きマスに
出られる。壁を始点にできない問題では呼び出し側で弾く。

```sh
g++ -std=gnu++23 example.cpp -o example && ./example
g++ -std=gnu++23 differential.cpp -o diff && ./diff
```

未採用の独立案。前処理を含む損益分岐は、候補の問題のデータと全体実行時間で測る。
合成した400盤面で、全始点・盤外始点と愚直なキューBFSを比較し、14,800件すべて一致。

## `prepared-bfs.hpp`

```cpp
#pragma once
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

namespace ahc_prepared_bfs {
// Four directional offsets as measured; excludes wall destinations, not
// origins (the stock contract permits leaving a blocked BFS origin).
template<std::size_t V,int Inf>
struct Grid {
 std::array<std::uint8_t,V> adj;
 template<class Walls,class Neighbors>
 void prepare(const Walls& wall,const Neighbors& nb){
  std::array<std::uint8_t,V> op;
  for(int p=0;p<int(V);++p)op[p]=!wall[p];
  for(int p=0;p<int(V);++p){
   unsigned bits=0;
   for(int k=0;k<4;++k){int q=nb[p][k];if(q>=0)bits|=unsigned(op[q])<<k;}
   adj[p]=bits;
  }
 }
 template<class Neighbors,class Dist>
 void bfs(int s,Dist& d,const Neighbors& nb)const{
  d.fill(Inf);
  if(s<0||s>=int(V))return;
  int q[V],e=1;q[0]=s;d[s]=0;
  for(int i=0;i<e;++i){
   int p=q[i];int16_t step=d[p]+1;
   for(unsigned bits=adj[p];bits;bits&=bits-1){
    int u=nb[p][std::countr_zero(bits)];
    if(d[u]==Inf){d[u]=step;q[e++]=u;}
   }
  }
 }
};
}
```

## `example.cpp`

```cpp
#include "prepared-bfs.hpp"
#include <array>
#include <bitset>
#include <iostream>
int main() {
    // 0 -- 1
    // |    |
    // 2 -- 3     Directions ordered U, R, D, L.
    const std::array<std::array<int,4>,4> nb{{
        {-1,1,2,-1},{-1,-1,3,0},{0,3,-1,-1},{1,-1,-1,2}
    }};
    std::bitset<4> walls;
    walls.set(1);
    ahc_prepared_bfs::Grid<4,100> graph;
    graph.prepare(walls, nb);
    std::array<int16_t,4> dist{};
    graph.bfs(0, dist, nb);
    std::cout << dist[3] << '\n'; // 2 via 0 -> 2 -> 3
}
```

## `differential.cpp`

```cpp
#include "prepared-bfs.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <iostream>
#include <queue>
#include <random>
int main() {
    constexpr int W=7,H=5,V=W*H,INF=1000;
    std::array<std::array<int,4>,V> nb{};
    for (int p=0; p<V; ++p) {
        nb[p]={p/W ? p-W : -1,p%W<W-1 ? p+1 : -1,
               p/W<H-1 ? p+W : -1,p%W ? p-1 : -1};
    }
    std::mt19937 rng(4172);
    unsigned checks=0;
    for (int world=0; world<400; ++world) {
        std::bitset<V> walls;
        for (int p=0;p<V;++p) walls[p]=(rng()%100)<unsigned(world%100);
        ahc_prepared_bfs::Grid<V,INF> graph;
        graph.prepare(walls,nb);
        for (int s=-1;s<=V;++s) {
            std::array<int16_t,V> got{},exp{};
            exp.fill(INF);
            if (0<=s && s<V) {
                std::queue<int> q;
                q.push(s);
                exp[s]=0;
                while (!q.empty()) {
                    int p=q.front(); q.pop();
                    for (int u:nb[p]) if (u>=0 && !walls[u] && exp[u]==INF) {
                        exp[u]=exp[p]+1;
                        q.push(u);
                    }
                }
            }
            graph.bfs(s,got,nb);
            assert(got==exp);
            ++checks;
        }
    }
    std::cout << checks << " exact source/board comparisons\n";
}
```
