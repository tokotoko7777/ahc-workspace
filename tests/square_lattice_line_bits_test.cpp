#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <set>
#include <utility>
#include <vector>
#include "../library/square-lattice-line-bits.hpp"

int main(){
  auto dx=SquareLatticeLineBits::dx;
  auto dy=SquareLatticeLineBits::dy;
  std::mt19937 rng(140134);
  for(int trial=0;trial<350;++trial){
    int n=trial%3==0?64:2+rng()%63;
    SquareLatticeLineBits marks(n),edges(n);
    std::vector<bool> point(n*n);
    std::set<std::pair<int,int>> unit;
    auto cell=[&](int x,int y){return x*n+y;};
    auto in=[&](int x,int y){return x>=0&&x<n&&y>=0&&y<n;};
    for(int tick=0;tick<250;++tick){
      int x=rng()%n,y=rng()%n;
      if(rng()%3==0){
        marks.mark(x,y);
        point[cell(x,y)]=true;
      }
      assert(marks.marked(x,y)==point[cell(x,y)]);
      for(int d=0;d<8;++d){
        int first=0,maxlen=0;
        while(in(x+dx[d]*(maxlen+1),y+dy[d]*(maxlen+1))){
          ++maxlen;
          if(!first&&point[cell(x+dx[d]*maxlen,y+dy[d]*maxlen)])first=maxlen;
        }
        assert(marks.nearest_steps(x,y,d)==first);
        if(!maxlen)continue;
        int len=1+rng()%maxlen;
        bool inner=false,overlap=false;
        std::vector<std::pair<int,int>> add;
        for(int z=0;z<len;++z){
          int a=cell(x+dx[d]*z,y+dy[d]*z);
          int b=cell(x+dx[d]*(z+1),y+dy[d]*(z+1));
          if(z&&point[a])inner=true;
          if(a>b)std::swap(a,b);
          overlap|=unit.contains({a,b});
          add.emplace_back(a,b);
        }
        assert(marks.inner_mark(x,y,d,len)==inner);
        assert(edges.overlaps_segment(x,y,d,len)==overlap);
        if(rng()%9==0){
          edges.add_segment(x,y,d,len);
          unit.insert(add.begin(),add.end());
        }
      }
    }
  }
}
