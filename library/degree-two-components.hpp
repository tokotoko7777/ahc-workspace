#pragma once
#include <algorithm>
#include <cassert>
#include <vector>

// 無向次数2以下（並列辺も可）の成分を、成分ごとのvector生成なしで列挙。
// n頂点を行き来する動的グラフでは、毎回の訪問・スタック領域を再利用する。
// links(v) は2要素の範囲。辺がなければ -1、活性頂点間で対称であること。
class DegreeTwoComponents {
 public:
  explicit DegreeTwoComponents(int n) : used_(n) { pending_.reserve(n); }

  template<class Active, class Links, class Visitor>
  void scan(Active active, Links links, Visitor visit) {
    std::fill(used_.begin(), used_.end(), 0);
    pending_.clear();
    for (int s = 0; s < int(used_.size()); ++s) {
      if (used_[s] || !active(s)) continue;
      used_[s] = 1;
      pending_.push_back(s);
      int size = 0, end_a = -1, end_b = -1;
      while (!pending_.empty()) {
        const int v = pending_.back();
        pending_.pop_back();
        ++size;
        for (int u : links(v)) {
          if (u < 0) {
            (end_a < 0 ? end_a : end_b) = v;
          } else if (!used_[u]) {
            used_[u] = 1;
            pending_.push_back(u);
          }
        }
      }
      assert((end_a < 0) == (end_b < 0));
      visit(size, end_a, end_b); // (size,-1,-1) は閉路。孤立点の両端は同じ。
    }
  }

 private:
  std::vector<unsigned char> used_;
  std::vector<int> pending_;
};
