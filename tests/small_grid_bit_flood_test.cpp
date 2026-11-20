#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <vector>
#include "../library/small-grid-bit-flood.hpp"

template<int H,int W>
void check(std::mt19937& rng){
  using Flood=SmallGridBitFlood<H,W>;
  using Mask=typename Flood::Mask;
  constexpr int dr[]={-1,1,0,0},dc[]={0,0,-1,1};
  for(int t=0;t<1200;++t){
    std::array<bool,H*W> allowed{},used{};
    Mask mask=0;
    for(int i=0;i<H*W;++i)if((allowed[i]=rng()%8<t%8))
      mask|=Flood::cell(i/W,i%W);
    std::vector<Mask> found,expected;
    Flood::components(mask,[&](Mask part){found.push_back(part);});
    for(int i=0;i<H*W;++i)if(allowed[i]&&!used[i]){
      std::vector<int> q{i};
      used[i]=true;
      Mask part=0;
      for(int head=0;head<int(q.size());++head){
        int j=q[head];
        part|=Flood::cell(j/W,j%W);
        for(int d=0;d<4;++d){
          int x=j/W+dr[d],y=j%W+dc[d];
          if(x>=0&&x<H&&y>=0&&y<W&&allowed[x*W+y]&&!used[x*W+y]){
            used[x*W+y]=true;q.push_back(x*W+y);
          }
        }
      }
      assert(Flood::count(part)==int(q.size()));
      expected.push_back(part);
    }
    std::sort(found.begin(),found.end());
    std::sort(expected.begin(),expected.end());
    assert(found==expected);
  }
}
int main(){
  std::mt19937 rng(150635);
  check<10,10>(rng);
  check<8,15>(rng);
  check<1,61>(rng);
  check<64,1>(rng);
}
