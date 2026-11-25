# 向きに依存しないグリッドの門と担当レーン

元の AHC008 の閉鎖担当は「門の左右どちらかに立つ」「縦の通路を動く」「右または左へ柵を置く」が一組に固定されていた。上下に入口がある部屋を同居させると、横向きの部屋と同じ割当器を使うには、門・立つ向き・動線を一組の情報として扱う必要がある。

`RailGate` は、部屋の外側に立つマス、その門を担当する直線レーン、必要な移動数、次の移動方向、柵を置く方向を求める。番人が各門でどのレーンを担当するかを比較する、格子の入口で複数人の到着時刻を揃える、といった用途で使える。障害物や動物、同時に他の人が置く柵の合法性までは扱わない。

```cpp
// 30×30、0始まり。門は (8,9)、その西側に立つ。
RailGate left{30, 30, 8*30 + 9, 3};
int worker = 5*30 + 8;       // 同じ担当レーン
// left.stand() == 8*30+8, left.eta(worker) == 3
// left.motion(worker) == 2 (下), left.build() == 1 (右)
```

`GridIsometry` で位置と方向の両方を回転・反転しても、到着時刻や操作が同じ変換に従うことを、長方形4種類、全8向き、全入口・全担当位置で検査する。小さい2盤面は static_assert、他は実行時に比較するので、コンパイルオプションによる constexpr 上限の変更は不要。

```bash
c++ -std=gnu++23 -Wall -Wextra -pedantic-errors rail_gate_test.cpp -o /tmp/rail_gate_test
/tmp/rail_gate_test
```

## 試作ヘッダー: rail-gate.hpp

```cpp
#ifndef AHC_RAIL_GATE_HPP
#define AHC_RAIL_GATE_HPP
#include <array>
// A portal cell with an exterior standing side in U,R,D,L order.
// Workers service it along the axis perpendicular to that side.
struct RailGate {
  inline static constexpr std::array<int,4> dr{-1,0,1,0};
  inline static constexpr std::array<int,4> dc{0,1,0,-1};
  int height, width, gate, outside;
  constexpr int stand() const {
    if(gate<0||height<=0||width<=0||gate/width>=height||outside<0||outside>=4)return -1;
    int r=gate/width+dr[outside],c=gate%width+dc[outside];
    return r>=0&&r<height&&c>=0&&c<width?r*width+c:-1;
  }
  constexpr int eta(int worker) const {
    const int q=stand();
    if(q<0||worker<0||worker/width>=height)return -1;
    const int r=q/width-worker/width,c=q%width-worker%width;
    if(outside%2==1)return c==0?(r>=0?r:-r):-1;
    return r==0?(c>=0?c:-c):-1;
  }
  constexpr int motion(int worker) const {
    if(eta(worker)<=0)return -1;
    const int q=stand();
    if(outside%2==1)return worker/width>q/width?0:2;
    return worker%width<q%width?1:3;
  }
  constexpr int build() const {return (outside+2)%4;}
};
#endif
```

## テスト: rail_gate_test.cpp

```cpp
#include "rail-gate.hpp"
#include "grid-isometry.hpp"
constexpr int direction(std::pair<int,int> v){
 for(int i=0;i<4;++i)if(v==std::pair{RailGate::dr[i],RailGate::dc[i]})return i;
 return -1;
}
constexpr bool exhaustive(int h,int w){
 for(unsigned f=0;f<8;++f){
  GridIsometry x{h,w,f};
  for(int g=0;g<h*w;++g)for(int d=0;d<4;++d){
   RailGate a{h,w,g,d};
   RailGate b{x.dst_height(),x.dst_width(),x.index(g),direction(x.delta(RailGate::dr[d],RailGate::dc[d]))};
   if(a.stand()<0){if(b.stand()!=-1)return false;continue;}
   if(b.stand()!=x.index(a.stand()))return false;
   if(b.build()!=direction(x.delta(RailGate::dr[a.build()],RailGate::dc[a.build()])))return false;
   for(int worker=0;worker<h*w;++worker){
    if(a.eta(worker)!=b.eta(x.index(worker)))return false;
    int m=a.motion(worker);
    int expected=m<0?-1:direction(x.delta(RailGate::dr[m],RailGate::dc[m]));
    if(expected!=b.motion(x.index(worker)))return false;
   }
  }
 }
 return true;
}
static_assert(exhaustive(2,3)&&exhaustive(1,4));
int main(){return exhaustive(5,7)&&exhaustive(7,11)?0:1;}
```

テストで使う `grid-isometry.hpp` は[対称変換の提案](GRID_ISOMETRY.md)の試作ヘッダー。

## AHC008実測

既存の左右向きの門だけを新しい表現に交換した試作は、同じ公式配布100件で全操作・得点が一致し、6,191,777,779点、全頭捕獲と全員が同じ空間を100件とも維持。探索数を24組に固定した別計測でも、全操作・得点・408組の探索が一致。固定計測のCPU合計は37.44→37.84秒（この計測では約1.1%増）。上部・下部に新しい縦向きの区画を作る変更とは分け、まだ既定解には組み込んでいない。
