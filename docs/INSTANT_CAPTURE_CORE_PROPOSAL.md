# ライブラリ改修提案: 同一ターンで隔離可能な区画を求める

**状態:** C++23の独立候補。既存ソルバーへの組み込み・速度比較は未実施。

長方形の4近傍グリッドを扱う。各作業員は隣接セルを1つ壁にできるが、人・対象がいるセルと対象に隣接するセルには置けない。作業員の移動先は壁にできない。与えられたターン開始状態から、対象を含み人のいない区画を同ターンで作れるかと、誰がどの壁を置くかを返す。

## なぜ全探索が要らないか

置ける壁セルの集合を L とする。隔離可能な区画 W に、開始時に作業員がいてはいけない。1歩で外へ出るには、塞ぐ予定の境界セルに入る必要があるため。同じ理由で、作業員は開いている境界にもいられない。開始時に人がいるセルは壁にできないからだ。したがって W 内には L のセルがない。一方、W の開境界はすべてそのターンに塞ぐ必要があるので L に属する。よって W はグリッドから L を除いたときの連結成分に一意に定まる。その境界を担当できる作業員へ重複なく割り当てられれば実現可能。

- 区画ごとの判定。複数区画を同時に閉じる場合は境界の和集合で割当を再確認する。
- 既に隔離済みなら `already_isolated()`。他人の行動を予約する制約、採点、動物の動きは含めない。
- 面積上限外の探索も最後まで続け、同じ大きな成分を対象ごとに探索し直さない。速度の測定値はない。

## 短い使用例

`wall` は1セル1バイト、対象の位置と担い手の位置は0始まりの行優先番号。`owners[i]` は `boundary[i]` を置ける人の元の配列インデックス。

```cpp
#include "instant_core.hpp"
#include <iostream>
int main() {
    std::vector<unsigned char> wall(5); // 1×5; no existing walls
    std::vector<int> people{0}, pets{3};
    for (const auto& core : instant_core::find(1, 5, wall, people, pets, 1, 4)) {
        if (core.already_isolated()) continue;
        for (std::size_t i = 0; i < core.boundary.size(); ++i)
            std::cout << core.owners[i] << " " << core.boundary[i] << "\n"; // 0 1
    }
}
```

## 検証

以下2ブロックのファイルを同じディレクトリに保存して実行する。

```sh
g++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror synthetic.cpp -o /tmp/instant-core-test
/tmp/instant-core-test
```

合成小盤面29,738件と個別9件は合法な移動・待機・柵の全列挙と一致。30×30の人工盤面でも確認し、UBSan付きで同じ検査を通過済み。実戦ベンチではない。

## `instant_core.hpp`

```cpp
#ifndef AHC008_INSTANT_CORE_HPP
#define AHC008_INSTANT_CORE_HPP
#include <algorithm>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
#include <utility>
namespace instant_core {
struct Core {
 std::vector<int> cells, pet_ids;
 std::vector<int> boundary, owners; // aligned; each owner is a distinct worker index
 bool already_isolated() const { return boundary.empty(); }
};
inline std::vector<Core> find(int rows, int cols, std::span<const unsigned char> wall,
                    std::span<const int> humans, std::span<const int> pets,
                    int min_area=1,int max_area=std::numeric_limits<int>::max()){
 constexpr int imax=std::numeric_limits<int>::max();
 if(rows<=0||cols<=0||rows>imax/cols||min_area<1||max_area<min_area||
    humans.size()>static_cast<unsigned>(imax)||pets.size()>static_cast<unsigned>(imax))
  throw std::invalid_argument("invalid grid dimensions, area range, or actor count");
 int n=rows*cols;
 if(wall.size()!=static_cast<unsigned>(n))throw std::invalid_argument("wrong wall bitmap size");
 auto valid=[&](int p){return p>=0&&p<n&&!wall[p];};
 auto adj=[&](int p,auto fn){
  if(p>=cols)fn(p-cols);
  if(p<n-cols)fn(p+cols);
  if(p%cols)fn(p-1);
  if(p%cols<cols-1)fn(p+1);
 };
 std::vector<unsigned char> human(n),pet_ban(n);
 for(int p:humans){if(!valid(p))throw std::invalid_argument("invalid worker position");human[p]=1;}
 for(int p:pets){if(!valid(p))throw std::invalid_argument("invalid animal position");pet_ban[p]=1;adj(p,[&](int q){pet_ban[q]=1;});}
 std::vector<std::vector<int>> builders(n);
 for(int i=0;i<(int)humans.size();++i)adj(humans[i],[&](int q){
  if(!wall[q]&&!human[q]&&!pet_ban[q])builders[q].push_back(i);
 });
 std::vector<int> comp(n,-1),queue;
 std::vector<Core> result;
 for(int j=0;j<(int)pets.size();++j){
  int s=pets[j];
  if(comp[s]!=-1)continue;
  queue.assign(1,s);comp[s]=-2;
  bool held=false;
  for(std::size_t z=0;z<queue.size();++z){
   int p=queue[z];held|=human[p];
   adj(p,[&](int q){if(comp[q]==-1&&!wall[q]&&builders[q].empty()){
    comp[q]=-2;queue.push_back(q);
   }});
  }
  // Exhaust the component even if too large: every repeated root is then
  // known to belong to this rejected component.
  if(held||queue.size()<(unsigned)min_area||queue.size()>(unsigned)max_area)continue;
  Core c;
  for(int p:queue)adj(p,[&](int q){if(!builders[q].empty())c.boundary.push_back(q);});
  std::sort(c.boundary.begin(),c.boundary.end());
  c.boundary.erase(std::unique(c.boundary.begin(),c.boundary.end()),c.boundary.end());
  if(c.boundary.size()>humans.size())continue;
  c.owners.assign(c.boundary.size(),-1);
  std::vector<int> worker_to_boundary(humans.size(),-1), seen(humans.size(),-1);
  auto match=[&](auto&& self,int q,int stamp)->bool{
   for(int i:builders[c.boundary[q]])if(seen[i]!=stamp){
    seen[i]=stamp;
    if(worker_to_boundary[i]<0||self(self,worker_to_boundary[i],stamp)){
     worker_to_boundary[i]=q;c.owners[q]=i;return true;
    }
   }
   return false;
  };
  bool good=true;
  for(int i=0;i<(int)c.boundary.size();++i)if(!match(match,i,i)){good=false;break;}
  if(!good)continue;
  for(int p:queue)comp[p]=static_cast<int>(result.size());
  c.cells=queue;std::sort(c.cells.begin(),c.cells.end());result.push_back(std::move(c));
 }
 for(int j=0;j<(int)pets.size();++j)if(comp[pets[j]]>=0)result[comp[pets[j]]].pet_ids.push_back(j);
 return result;
}
}
#endif
```

## `synthetic.cpp`

```cpp
#include "instant_core.hpp"
#include <bits/stdc++.h>
using namespace std;
using U=uint32_t;
U bit(int i){return 1u<<i;}
vector<int> cells(U x){vector<int> v;while(x){v.push_back(countr_zero(x));x&=x-1;}return v;}
struct Grid{
 int R,C;U all;vector<U> adj;
 Grid(int r,int c):R(r),C(c),all(bit(r*c)-1),adj(r*c){
  for(int i=0;i<r*c;++i)for(int j=0;j<r*c;++j)
   if(abs(i/c-j/c)+abs(i%c-j%c)==1)adj[i]|=bit(j);
 }
 U wave(U m)const{U a=0;for(int q:cells(m))a|=adj[q];return a;}
 U flood(U m,U free)const{for(;;){U b=m|(wave(m)&free);if(b==m)return m;m=b;}}
};
U mask(vector<int> a){U m=0;for(int i:a)m|=bit(i);return m;}
struct Core{U body;bool already_isolated;vector<pair<int,int>> witness;};
vector<Core> instant(const Grid& g,U free,const vector<int>& h,U pets,int cap=INT_MAX){
 vector<unsigned char>w(g.R*g.C,1);for(int q:cells(free))w[q]=0;
 auto out=instant_core::find(g.R,g.C,w,h,cells(pets),1,cap);
 vector<Core> ans;
 for(auto const& c:out){
  Core a{mask(c.cells),c.already_isolated(),{}};
  for(size_t i=0;i<c.boundary.size();++i)a.witness.emplace_back(c.owners[i],c.boundary[i]);
  ans.push_back(a);
 }
 return ans;
}
vector<set<U>> brute(const Grid& g,U free,vector<int> h,vector<int> p){
 vector<set<U>> res(p.size());vector<int> end=h;
 auto rec=[&](auto&& self,int i,U added)->void{
  if(i==(int)h.size()){
   U moved=mask(end),left=free&~added;if(moved&added)return;
   for(int j=0;j<(int)p.size();++j){
    U comp=g.flood(bit(p[j]),left);if(!(comp&moved))res[j].insert(comp);
   }
   return;
  }
  end[i]=h[i];self(self,i+1,added);
  for(int q:cells(g.adj[h[i]])){
   if(free&bit(q)){end[i]=q;self(self,i+1,added);end[i]=h[i];}
   bool ok=true;for(int z:h)if(z==q)ok=false;
   for(int z:p)if(z==q||(g.adj[z]&bit(q)))ok=false;
   if(ok)self(self,i+1,added|bit(q));
  }
 };
 rec(rec,0,0);return res;
}
void check(const Grid& g,U f,vector<int> h,vector<int> ps){
 auto truth=brute(g,f,h,ps);
 for(int cap:{3,INT_MAX}){
  auto cs=instant(g,f,h,mask(ps),cap);vector<set<U>> got(ps.size());
  for(auto [a,isolated,ws]:cs){
   assert(isolated==ws.empty());
   U add=0,people=mask(h),used=0;for(auto [i,q]:ws){
    assert(!(used&bit(i))&&!(add&bit(q))&&(g.adj[h[i]]&bit(q))
      &&(f&bit(q))&&!(bit(q)&(people|mask(ps)|g.wave(mask(ps)))));
    used|=bit(i);add|=bit(q);
   }
   assert(g.flood(a&-a,f&~add)==a&&!(a&people));
   for(int i=0;i<(int)ps.size();++i)if(a&bit(ps[i]))got[i].insert(a);
  }
  for(int i=0;i<(int)ps.size();++i){
   assert(truth[i].size()<=1);
   set<U> allowed;for(U a:truth[i])if(popcount(a)<=cap)allowed.insert(a);
   assert(got[i]==allowed);
  }
 }
}
int main(){
 check(Grid(1,5),31,{0},{3}); // legal single cut
 check(Grid(1,5),31,{0,1},{3}); // occupied gate cannot close
 check(Grid(1,6),63,{0,5},{2,3}); // adjacent pets protect would-be targets
 check(Grid(1,7),119,{0},{6}); // preisolated core
 {Grid g(3,3); check(g,511,{0},{8});assert(instant(g,511,{0},bit(8)).empty());}
 {Grid g(4,4);U f=mask({0,1,2,4,5,6,8,9,10,15});
  check(g,f,{0,15},{10});assert(instant(g,f,{0,15},bit(10)).empty());} // enough actors, still Hall failure
 {Grid g(1,7); check(g,127,{3},{0,6});
  auto c=instant(g,127,{3},bit(0)|bit(6));assert(c.size()==2);
  assert(c[0].witness[0].first==c[1].witness[0].first && c[0].witness[0].second!=c[1].witness[0].second);
 } // each core feasible; simultaneous union infeasible
 check(Grid(1,5),31,{0,0},{3,3});
 check(Grid(1,5),31,{0},{0,3});
 int checked=0;
 mt19937 rng(872654);
 for(int rep=0;rep<30000;++rep){
  Grid g(2+rng()%2,2+rng()%3); U f=0;
  for(int i=0;i<(int)g.adj.size();++i)if(rng()%4)f|=bit(i);
  auto p=cells(f);if(p.size()<2)continue;shuffle(p.begin(),p.end(),rng);
  int m=1+rng()%min<int>(3,p.size()-1),n=1+rng()%min<int>(3,p.size()-m);
  vector<int> h(p.begin(),p.begin()+m),ps(p.begin()+m,p.begin()+m+n);
  if(rep%7==0)h.back()=h.front();
  if(rep%5==0)ps.back()=ps.front();
  if(rep%11==0)ps.back()=h.front();
  check(g,f,h,ps);++checked;
  vector<unsigned char> w(g.R*g.C,1);for(int q:cells(f))w[q]=0;
  auto all=instant_core::find(g.R,g.C,w,h,ps);
  set<pair<int,U>> want, got;
  for(auto& c:all)if(c.cells.size()>=2&&c.cells.size()<=4)for(int j:c.pet_ids)want.emplace(j,mask(c.cells));
  auto limited=instant_core::find(g.R,g.C,w,h,ps,2,4);
  for(auto& c:limited)for(int j:c.pet_ids)got.emplace(j,mask(c.cells));
  assert(got==want);
 }
 {
  vector<unsigned char> w(900,1);
  for(int c=0;c<10;++c)w[c]=0;
  for(int c=0;c<30;++c)w[870+c]=0;
  vector<int> h{0,0,875},p{3,3,4,899,899,875};
  auto cores=instant_core::find(30,30,w,h,p,1,8);
  assert(cores.size()==1&&cores[0].boundary==vector<int>{1}&&cores[0].pet_ids==vector<int>({0,1,2}));
  assert(instant_core::find(30,30,w,h,p,1,7).empty());
  assert(instant_core::find(30,30,w,h,p,9,30).empty());
  auto wide=instant_core::find(30,30,w,vector<int>{0,0},p,9,30);
  assert(wide.size()==1&&wide[0].already_isolated()&&wide[0].pet_ids==vector<int>({3,4,5}));
  w[10]=0; // first corridor's right endpoint becomes one step larger
  assert(instant_core::find(30,30,w,h,p,1,8).empty());
  bool caught=false;
  try { auto invalid=instant_core::find(30,30,w,vector<int>{-1},p); }
  catch(const invalid_argument&){caught=true;}
  assert(caught);
 }
 cout<<"PASS synthetic cross-check; "<<checked<<" tested randomized draws (of 30000), plus 9 tiny crafted and a 30x30 fixture; caps and min area exercised\n";
}
```
