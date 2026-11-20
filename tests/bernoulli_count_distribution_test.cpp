#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <numeric>
#include <random>
#include "../library/bernoulli-count-distribution.hpp"

int main() {
  std::mt19937 rng(160981);
  for(int t=0;t<450;++t){
    const int n=rng()%13;
    std::vector<double> q(n);
    for(double& p:q)p=(rng()%11)*0.1;
    auto got=bernoulli_count_distribution(q);
    std::vector<double> brute(n+1);
    for(unsigned b=0;b<(1u<<n);++b){
      double mass=1;
      for(int j=0;j<n;++j)mass*=((b>>j&1)?q[j]:1-q[j]);
      brute[std::popcount(b)]+=mass;
    }
    for(int j=0;j<=n;++j)assert(std::abs(brute[j]-got[j])<1e-10);
    std::reverse(q.begin(),q.end());
    auto reversed=bernoulli_count_distribution(q);
    for(int j=0;j<=n;++j)assert(std::abs(reversed[j]-got[j])<1e-10);
    assert(std::abs(std::accumulate(got.begin(),got.end(),0.0)-1.0)<1e-10);
  }
}
