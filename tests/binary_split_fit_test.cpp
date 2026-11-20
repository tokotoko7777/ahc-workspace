#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>
#include <vector>
#include "../library/binary-split-fit.hpp"

int main() {
  std::mt19937 rng(2199);
  for (int scenario=0; scenario<1800; ++scenario) {
    int n=1+rng()%25, m=1+rng()%40;
    std::vector<std::vector<double>> probability(m,std::vector<double>(n));
    std::vector<double> grad(n);
    for (auto& row:probability) for (auto& v:row) v=(rng()%10001)/10000.0;
    for (auto& v:grad) v=(int(rng()%2001)-1000)/100.0;
    const auto result=best_binary_split(probability,grad);
    auto score=[&](int row,bool reverse) {
      double value=0;
      for (int i=0; i<n; ++i)
        value+=grad[i]*(reverse?1-probability[row][i]:probability[row][i]);
      return value;
    };
    for (int j=0; j<m; ++j)
      for (bool reverse:{false,true})
        assert(score(int(result.row),result.complement)+1e-10>=score(j,reverse));
  }
  auto t=best_binary_split(std::vector<std::vector<double>>{{.1},{.9}},std::vector<double>{2});
  assert(t.row==0 && t.complement);
}
