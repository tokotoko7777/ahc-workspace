#pragma once
#include <cassert>
#include <vector>

// 独立なベルヌーイ試行の成功数の確率質量関数。
// probs は各試行の成功確率。O(n^2) 時間、O(n) メモリ。
inline std::vector<double> bernoulli_count_distribution(const std::vector<double>& probs) {
  std::vector<double> mass(probs.size()+1);
  mass[0]=1.0;
  int seen=0;
  for(double p:probs) {
    assert(0.0<=p && p<=1.0);
    for(int k=++seen;k>0;--k) mass[k]=mass[k]*(1-p)+mass[k-1]*p;
    mass[0]*=1-p;
  }
  return mass;
}
