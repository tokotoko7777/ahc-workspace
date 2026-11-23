# 固定歩行から柵の設置可否を判定する案

## AHC008での使いどころ

先に配置を決めてから設営経路を探すと、「先に通り、通った後に塞ぐ」経路を見落としやすい。
各人の歩行と空きターンを先に固定できる場合、どの柵をその経路から設置できるかを
動物なしで厳密に判定できる。これはライブラリ採用案であり、AHC008の提出版には未統合。

- 壁セルごとに、いずれかの作業者がそのセルから出発するか、そこへ到着した最後のターンを保存する。
- 空きターンから隣接最大4セルだけを調べ、最後の利用より後かつ締切内なら作業候補にする。
- 壁タスクと人・ターンの二部マッチングが全タスクを覆えば、その固定歩行のまま施行できる。
- 不可能なら、必要タスク数より利用可能な枠が少ない衝突集合も得られる。
- これは固定経路・動物なし・既存行動の合法性確認済みという前提に限る。
  動物が近いときの柵設置禁止や、状況に応じて歩行を変える場合は別途検証が必要。

## 使用例

3×3を行優先で番号付け。作業者1がセル1を通過するので、隣接する作業者0が置けるのはターン2から。

~~~cpp
#include "certificate.hpp"
using fixed_walk_walls::certify;
auto r = certify(3, 3, {{0,0,0,0},{4,1,2,2}}, {{1,1,1},{0,0,1}}, {{1,2}});
// r.feasible == true; r.placement[0] == {0, 2}
// 締切を1に変えると infeasible
~~~

頂点数をV、作業者数をA、ターン数をT、壁数をW、候補辺数をEとすると、
グラフ作成は O(V + AT + W)、単純増加路は O(WE)。
各空き枠から調べる候補は最大4つ。大規模化する場合は既存の二部マッチング実装と差し替える。

## 再現

以下をそれぞれ certificate.hpp / check.cpp に保存して実行する。

~~~sh
g++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror check.cpp -o check && ./check
g++ -std=c++23 -O1 -g -fsanitize=undefined -fno-sanitize-recover=all check.cpp -o check_ubsan && ./check_ubsan
~~~

3×3の人工10,000例（実現可能2,269）と手作り3例で、物理的に逐次シミュレーションする
全探索と一致。返された配置および失敗時の衝突集合も独立に検証した。
コンテスト入力、他者のコード、学習・調整済み係数、採点結果を含まない。

### certificate.hpp

~~~cpp
#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

namespace fixed_walk_walls {
struct Task { int cell, deadline; };  // deadline inclusive, turns start at zero
struct Slot { int actor, turn; };
struct Certificate {
    bool feasible = false;
    std::vector<Slot> placement;     // task order, {-1,-1} for unmatched
    std::vector<int> deficient_tasks;
    std::vector<Slot> neighbor_slots;
};

// Route[a][t] is the position at the start of turn t; routes also carry
// their last destinations. available[a][t] marks waits free for placements.
// Preconditions: rectangular cardinal grid, unique target cells, initially
// legal prescribed moves and no other sources of obstacles/placement veto.
// Actors may share cells. Targets must not be occupied at either endpoint of
// a placement turn. Result is exact for these fixed routes, not for animals.
inline Certificate certify(int height, int width,
        const std::vector<std::vector<int>>& route,
        const std::vector<std::vector<char>>& available,
        const std::vector<Task>& tasks) {
    assert(height > 0 && width > 0 && route.size() == available.size());
    const int vertices = height * width;
    const int actors = static_cast<int>(route.size());
    const int horizon = route.empty() ? 0 : static_cast<int>(route[0].size()) - 1;
    const int n = static_cast<int>(tasks.size());
    assert(horizon >= 0);
    std::vector<int> last_touch(vertices, -1), task_index(vertices, -1);
    for (int a = 0; a < actors; ++a) {
        assert(static_cast<int>(route[a].size()) == horizon+1);
        assert(static_cast<int>(available[a].size()) == horizon);
        for (int t = 0; t < horizon; ++t) {
            int u = route[a][t], v = route[a][t+1];
            assert(u >= 0 && u < vertices && v >= 0 && v < vertices);
            last_touch[u] = std::max(last_touch[u], t);
            last_touch[v] = std::max(last_touch[v], t);
        }
    }
    for (int i = 0; i < n; ++i) {
        const auto [p, d] = tasks[i];
        (void)d;
        assert(p >= 0 && p < vertices && task_index[p] < 0);
        task_index[p] = i;
    }
    std::vector<Slot> slots;
    std::vector<std::vector<int>> edges(n);
    auto connect = [&](int t, int p, int j) {
        int i = task_index[p];
        if (i >= 0 && t > last_touch[p] && t <= tasks[i].deadline)
            edges[i].push_back(j);
    };
    for (int a = 0; a < actors; ++a) for (int t = 0; t < horizon; ++t) {
        const int u = route[a][t];
        if (!available[a][t] || route[a][t+1] != u) continue;
        const int j = static_cast<int>(slots.size());
        slots.push_back({a,t});
        if (u >= width) connect(t,u-width,j);
        if (u + width < vertices) connect(t,u+width,j);
        if (u % width) connect(t,u-1,j);
        if (u % width + 1 < width) connect(t,u+1,j);
    }
    std::vector<int> right(slots.size(),-1), left(n,-1), seen(slots.size(),-1);
    auto add = [&](auto&& self, int i, int stamp) -> bool {
        for (int j : edges[i]) {
            if (seen[j] == stamp) continue;
            seen[j] = stamp;
            if (right[j] < 0 || self(self,right[j],stamp)) {
                right[j] = i;
                left[i] = j;
                return true;
            }
        }
        return false;
    };
    int matched = 0;
    for (int i = 0; i < n; ++i) matched += add(add,i,i);
    Certificate out;
    out.placement.resize(tasks.size(),Slot{-1,-1});
    for (int i = 0; i < n; ++i)
        if (left[i] >= 0) out.placement[i] = slots[left[i]];
    out.feasible = matched == n;
    if (out.feasible) return out;
    // All neighbors of the alternating-reachable task set. Every such slot
    // is matched, and its owner is in the set; at least one task is unmatched.
    std::vector<char> found(n), reached(slots.size());
    for (int i = 0; i < n; ++i) if (left[i] < 0) {
        found[i] = true;
        out.deficient_tasks.push_back(i);
    }
    for (std::size_t h = 0; h < out.deficient_tasks.size(); ++h) {
        for (int j : edges[out.deficient_tasks[h]]) if (!reached[j]) {
            reached[j] = true;
            out.neighbor_slots.push_back(slots[j]);
            assert(right[j] >= 0);
            if (!found[right[j]]) {
                found[right[j]] = true;
                out.deficient_tasks.push_back(right[j]);
            }
        }
    }
    assert(out.neighbor_slots.size() < out.deficient_tasks.size());
    return out;
}
} // namespace fixed_walk_walls
~~~

### check.cpp

~~~cpp
#include "certificate.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <iostream>
#include <random>
#include <set>
#include <utility>
using fixed_walk_walls::Task;
using V = std::vector<std::vector<int>>;
using A = std::vector<std::vector<char>>;
bool neighbor(int x,int y) { return std::abs(x/3-y/3)+std::abs(x%3-y%3)==1; }

bool check(const V& route, const A& free, const std::vector<Task>& tasks) {
    int m = int(route.size()), tmax = m ? int(route[0].size())-1 : 0;
    const int full = (1 << tasks.size())-1;
    // Independent chronological exhaustive simulation. Unlike the matcher,
    // branch decisions are ordinary physical wall maps, not last-touch edges.
    std::set<int> old{0};
    for (int t=0; t<tmax; ++t) {
        std::set<int> next;
        for (int mask : old) {
            bool move_allowed=true;
            for (int i=0; i<int(tasks.size()); ++i) if (mask>>i&1)
                for (int a=0;a<m;++a)
                    if (route[a][t+1]==tasks[i].cell || route[a][t]==tasks[i].cell) move_allowed=false;
            if (!move_allowed) continue;
            auto choices=[&](auto&& self,int a,int visit)->void {
                if (a==m) { next.insert(visit);return; }
                self(self,a+1,visit);
                if (!free[a][t] || route[a][t]!=route[a][t+1]) return;
                for (int i=0;i<int(tasks.size());++i) if (!(visit>>i&1)) {
                    const int p=tasks[i].cell;
                    if (t>tasks[i].deadline || !neighbor(route[a][t],p)) continue;
                    bool clear=true;
                    for (int z=0;z<m;++z) if (route[z][t]==p || route[z][t+1]==p) clear=false;
                    if (clear) self(self,a+1,visit|(1<<i));
                }
            };
            choices(choices,0,mask);
        }
        old.swap(next);
    }
    auto got=fixed_walk_walls::certify(3,3,route,free,tasks);
    assert(got.feasible==old.contains(full));
    if (got.feasible) {
        std::set<std::pair<int,int>> used;
        for (int i=0;i<int(tasks.size());++i) {
            const auto [a,t]=got.placement[i];
            assert(a>=0 && a<m && t>=0 && t<tmax);
            assert(used.emplace(a,t).second && free[a][t] && route[a][t]==route[a][t+1]);
            assert(t<=tasks[i].deadline && neighbor(route[a][t],tasks[i].cell));
            for (int b=0;b<m;++b) for(int j=t;j<=tmax;++j)
                assert(route[b][j]!=tasks[i].cell);
        }
    } else {
        std::set<std::pair<int,int>> expected, found;
        for (auto s:got.neighbor_slots) assert(found.emplace(s.actor,s.turn).second);
        for (int i:got.deficient_tasks) for(int a=0;a<m;++a) for(int t=0;t<tmax;++t) {
            if (t>tasks[i].deadline || !free[a][t] || route[a][t]!=route[a][t+1] ||
                !neighbor(route[a][t],tasks[i].cell)) continue;
            bool never=true;
            for(int b=0;b<m;++b) for(int j=t;j<=tmax;++j)
                if (route[b][j]==tasks[i].cell) never=false;
            if(never) expected.emplace(a,t);
        }
        assert(found==expected && found.size()<got.deficient_tasks.size());
    }
    return got.feasible;
}
int main() {
    // Another actor crosses before construction. A t=0 closure is forbidden.
    const V route{{0,0,0,0},{4,1,2,2}};
    const A available{{1,1,1},{0,0,1}};
    assert(check(route,available,{{1,2}}));
    assert(!check(route,available,{{1,1}}));
    // Distinct wall tasks compete for the only free adjacent slot.
    assert(!check({{4,4}},{{1}},{{1,0},{3,0}}));
    std::mt19937 gen(94431);
    auto pick=[&](int n){return int(gen()%static_cast<unsigned>(n));};
    int successful=0;
    for(int rep=0;rep<10000;++rep) {
        int m=1+pick(3), t=pick(6), nw=pick(5);
        V p(m,std::vector<int>(t+1));
        A a(m,std::vector<char>(t));
        for(int i=0;i<m;++i) {
            p[i][0]=pick(9);
            for(int j=0;j<t;++j) {
                std::vector<int> next{p[i][j]};
                for(int q=0;q<9;++q) if(neighbor(p[i][j],q)) next.push_back(q);
                p[i][j+1]=pick(2)?p[i][j]:next[pick(int(next.size()))];
                a[i][j]=static_cast<char>(pick(3)!=0);
            }
        }
        std::array<int,9> cells{0,1,2,3,4,5,6,7,8};
        std::shuffle(cells.begin(),cells.end(),gen);
        std::vector<Task> tasks;
        for(int i=0;i<nw;++i) tasks.push_back({cells[i],pick(t+1)-1});
        successful+=check(p,a,tasks);
    }
    std::cout<<10000<<" synthetic cases, "<<successful<<" feasible, plus 3 crafted cases\n";
}
~~~
