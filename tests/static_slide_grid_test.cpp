#include "../library/static-slide-grid.hpp"
#include <cassert>
#include <random>

int main(){
  std::mt19937 rng(462046);
  const int dh[]{-1,1,0,0},dw[]{0,0,-1,1};
  for(int t=0;t<1800;++t){
    const int h=1+rng()%20,w=1+rng()%22;
    std::vector<unsigned char> wall(h*w);
    for(auto& x:wall)x=rng()%5==0;
    StaticSlideGrid grid(h,w,wall);
    for(int p=0;p<h*w;++p)for(int d=0;d<4;++d){
      if(wall[p]){assert(grid.stop[p][d]==-1&&grid.step[p][d]==-1);continue;}
      int y=p/w,x=p%w,step=-1;
      for(int l=0;;++l){
        int j=y+dh[d],i=x+dw[d];
        if(j<0||j>=h||i<0||i>=w||wall[j*w+i])break;
        y=j;x=i;
        if(l==0)step=y*w+x;
      }
      assert(grid.step[p][d]==step&&grid.stop[p][d]==y*w+x);
    }
  }
}
