#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>

// X~Normal(mean,sigma), observation = clamp(round(X),lo,hi) の対数確率。
// 両端には範囲外の裾を含める。極小の確率は1e-280で下駄を履かせる。
inline double rounded_clipped_normal_logmass(int observation,double mean,double sigma,
                                             int lo,int hi) {
  assert(lo<=observation && observation<=hi && sigma>0);
  if(lo==hi)return 0;
  const double d=sigma*std::sqrt(2.0);
  const double a=(observation-0.5-mean)/d;
  const double b=(observation+0.5-mean)/d;
  double p;
  if(observation==lo)p=0.5*std::erfc(-b);
  else if(observation==hi)p=0.5*std::erfc(a);
  else if(a>=0)p=0.5*(std::erfc(a)-std::erfc(b));
  else p=0.5*(std::erfc(-b)-std::erfc(-a));
  return std::log(std::max(p,1e-280));
}
