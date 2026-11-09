#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/alias-table.hpp

// 変わらない重み分布から、前計算 O(N)・1回 O(1) で抽選するAlias法。
// AliasTable table(weights);
// int selected = table.choose(random.next_int(0, table.size()),
//                             random.next_real());
struct AliasTable {
  std::vector<double> probability;
  std::vector<int> alias;

  explicit AliasTable(const std::vector<double>& weights) {
    const int n = static_cast<int>(weights.size());
    assert(n > 0);
    double total = 0.0;
    for (double weight : weights) {
      assert(weight >= 0.0);
      total += weight;
    }
    assert(total > 0.0);

    probability.resize(n);
    alias.resize(n);
    std::vector<double> scaled(n);
    std::vector<int> small;
    std::vector<int> large;
    small.reserve(n);
    large.reserve(n);
    for (int i = 0; i < n; ++i) {
      scaled[i] = weights[i] * n / total;
      if (scaled[i] < 1.0) {
        small.push_back(i);
      } else {
        large.push_back(i);
      }
    }

    while (!small.empty() && !large.empty()) {
      const int low = small.back();
      small.pop_back();
      const int high = large.back();
      large.pop_back();
      probability[low] = std::clamp(scaled[low], 0.0, 1.0);
      alias[low] = high;
      scaled[high] -= 1.0 - scaled[low];
      if (scaled[high] < 1.0) {
        small.push_back(high);
      } else {
        large.push_back(high);
      }
    }
    for (int index : small) {
      probability[index] = 1.0;
      alias[index] = index;
    }
    for (int index : large) {
      probability[index] = 1.0;
      alias[index] = index;
    }
  }

  int size() const { return static_cast<int>(probability.size()); }

  // columnは[0,size())の一様乱数、unitは[0,1)の一様乱数。
  int choose(int column, double unit) const {
    assert(0 <= column && column < size());
    assert(0.0 <= unit && unit < 1.0);
    return unit < probability[column] ? column : alias[column];
  }
};
