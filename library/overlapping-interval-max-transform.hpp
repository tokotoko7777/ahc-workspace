#pragma once
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

// State (lo,hi) represents [cut[lo],cut[hi]], lo<hi. Returns the best
// attainable source with strictly positive overlap, penalized by endpoint
// L1 distance. cut is strictly increasing. src=-1 marks unavailable.
template<class Score,class Coordinate>
void overlapping_interval_max_transform(
    const std::vector<Coordinate>& cut,Score penalty,
    std::vector<Score>& value,std::vector<int>& source) {
  const int n=int(cut.size());
  assert(penalty>=0 && value.size()==cut.size()*cut.size() && source.size()==value.size());
  const auto original=value;
  const auto available=source;
  std::fill(source.begin(),source.end(),-1);
  std::vector<int> best(n*n);
  auto compare=[&](int& a,int b,int sign_l,int sign_h){
    auto offset=[&](int i){return original[i]+penalty*(sign_l*Score(cut[i/n])+sign_h*Score(cut[i%n]));};
    if(b>=0 && (a<0 || offset(b)>offset(a)))a=b;
  };
  auto emit=[&](int l,int h,int q){
    if(l>=h||q<0)return;
    int k=l*n+h;
    assert(std::max(l,q/n)<std::min(h,q%n));
    auto v=original[q]-penalty*(std::max(cut[l],cut[q/n])-std::min(cut[l],cut[q/n])
                              +std::max(cut[h],cut[q%n])-std::min(cut[h],cut[q%n]));
    if(source[k]<0||v>value[k]){source[k]=q;value[k]=v;}
  };
  auto init=[&]{for(int i=0;i<n*n;++i)best[i]=(i/n<i%n&&available[i]>=0)?i:-1;};
  // predecessor lower <= l, upper >= h
  init();
  for(int l=0;l<n;++l)for(int h=n-1;h>=0;--h){
    auto& q=best[l*n+h];
    if(l)compare(q,best[(l-1)*n+h],1,-1);
    if(h+1<n)compare(q,best[l*n+h+1],1,-1);
    emit(l,h,q);
  }
  // predecessor lower <= l, and l < upper <= h
  init();
  for(int l=0;l<n;++l){
    for(int h=0;h<n;++h)if(l)compare(best[l*n+h],best[(l-1)*n+h],1,1);
    int q=-1;
    for(int h=l+1;h<n;++h){compare(q,best[l*n+h],1,1);emit(l,h,q);}
  }
  // l <= predecessor lower < h, upper >= h
  init();
  for(int h=n-1;h>=0;--h){
    for(int l=0;l<n;++l)if(h+1<n)compare(best[l*n+h],best[l*n+h+1],-1,-1);
    int q=-1;
    for(int l=h-1;l>=0;--l){compare(q,best[l*n+h],-1,-1);emit(l,h,q);}
  }
  // predecessor lower >= l, upper <= h, with nonempty predecessor
  init();
  for(int l=n-1;l>=0;--l)for(int h=0;h<n;++h){
    auto& q=best[l*n+h];
    if(l+1<n)compare(q,best[(l+1)*n+h],-1,1);
    if(h)compare(q,best[l*n+h-1],-1,1);
    emit(l,h,q);
  }
  for(int i=0;i<n*n;++i)if(source[i]>=0)source[i]=available[source[i]];
}
