#ifndef AHC_INCREMENTAL_MANHATTAN_DISTANCE_HPP
#define AHC_INCREMENTAL_MANHATTAN_DISTANCE_HPP
#include <algorithm>
#include <cassert>
#include <vector>

// Non-wrapping rectangular Manhattan distance from every cell to the
// nearest source. Sources are only added. A new source propagates only
// strict improvements; O(changed cells), O(height*width) scratch.
class IncrementalManhattanDistance {
  int height,width;
  std::vector<int> d,queue;
public:
  IncrementalManhattanDistance(int h,int w)
      :height(h),width(w),d(h*w,h+w),queue(h*w) {
    assert(h>0&&w>0);
  }
  int distance(int id)const{return d[id];}
  bool add(int source){
    assert(0<=source&&source<int(d.size()));
    if(d[source]==0)return false;
    int end=1;queue[0]=source;d[source]=0;
    for(int i=0;i<end;++i){
      const int u=queue[i],y=u/width,x=u%width,value=d[u]+1;
      auto update=[&](int v){if(d[v]>value){d[v]=value;queue[end++]=v;}};
      if(y)update(u-width);
      if(y+1<height)update(u+width);
      if(x)update(u-1);
      if(x+1<width)update(u+1);
    }
    return true;
  }
};
#endif
