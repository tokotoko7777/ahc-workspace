#include "../library/small-bottleneck-assignment.hpp"
#include <cassert>
#include <functional>
#include <random>
#include <utility>

using small_bottleneck_assignment::solve;

int main() {
  const int not_lex[] = {2,3,-1, 0,2,-1, -1,-1,4};
  const auto c = solve(3,3,not_lex);
  assert(c && c->latest == 4 && c->sum == 7);
  const int mx = std::numeric_limits<int>::max();
  const int large[] = {mx,-1, -1,mx};
  const auto l = solve(2,2,large);
  assert(l && l->latest == mx && l->sum == std::int64_t(mx)*2);
  assert(solve(0,0,{}));
  std::mt19937 rng(14387);
  for(int rep=0; rep<15000; ++rep) {
    int n=rng()%8, m=rng()%8;
    std::vector<int> mat(n*m);
    for(auto& x:mat)x=rng()%13-1;
    using P=std::pair<int,std::int64_t>;
    std::optional<P> brute;
    auto dfs=[&](auto&& self,int j,int mask,int latest,std::int64_t sum)->void{
      if(j==n){
        P p{latest,sum};
        if(!brute||p<*brute)brute=p;
        return;
      }
      for(int i=0;i<m;++i)if(!(mask>>i&1)&&mat[j*m+i]>=0)
        self(self,j+1,mask|1<<i,std::max(latest,mat[j*m+i]),sum+mat[j*m+i]);
    };
    dfs(dfs,0,0,0,0);
    const auto r=solve(n,m,mat);
    assert(bool(r)==bool(brute));
    if(!r)continue;
    assert((P{r->latest,r->sum}==*brute));
    int b=0, mx=0; std::int64_t s=0;
    for(int j=0;j<n;++j){
      const int i=r->worker_for_job[j], d=mat[j*m+i];
      assert(i>=0&&i<m&&!(b>>i&1)&&d>=0);
      b|=1<<i;s+=d;mx=std::max(mx,d);
    }
    assert(mx==r->latest&&s==r->sum);
  }
}
