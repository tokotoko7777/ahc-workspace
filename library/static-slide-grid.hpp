#pragma once
#include <array>
#include <cassert>
#include <vector>

// Movement endpoints among static blockers, flattened row-major.
// Direction order is U, D, L, R. -1 denotes no adjacent passable
// step; a valid slide with no movement has the original index.
struct StaticSlideGrid {
  int h,w;
  std::vector<std::array<int,4>> step,stop;
  StaticSlideGrid(int h,int w,const std::vector<unsigned char>& wall)
      :h(h),w(w),step(h*w),stop(h*w){
    assert(h>0&&w>0&&int(wall.size())==h*w);
    const int dr[]{-1,1,0,0},dc[]{0,0,-1,1};
    for(int r=0;r<h;++r)for(int c=0;c<w;++c){
      int p=r*w+c;
      step[p].fill(-1);stop[p].fill(-1);
      if(wall[p])continue;
      for(int d=0;d<4;++d){
        int y=r+dr[d],x=c+dc[d];
        if(0<=y&&y<h&&0<=x&&x<w&&!wall[y*w+x])step[p][d]=y*w+x;
      }
    }
    for(int d=0;d<4;++d){
      for(int y=0;y<h;++y)for(int x=0;x<w;++x){
        int r=d==1?h-1-y:y,c=d==3?w-1-x:x,p=r*w+c;
        if(wall[p])continue;
        const int q=step[p][d];
        stop[p][d]=q<0?p:stop[q][d];
      }
    }
  }
};
