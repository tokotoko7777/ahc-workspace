#pragma once
#include <cassert>
#include <vector>

// グループのサイズ頻度 f[d] と sum_d min(f[d], cap[d]) を差分更新する。
// cap の範囲外のサイズは無視する。replace_size(0,new_size) で空から作るなら
// cap[0] を 0 にする。正の cap に対応するサイズは削除前に追加済みであること。
class CappedSizeHistogram {
 public:
  explicit CappedSizeHistogram(const std::vector<int>& cap)
      : cap_(cap), frequency_(cap.size()) {}

  void replace_size(int before, int after) {
    if (before == after) return;
    if (tracked(before)) {
      assert(frequency_[before] > 0);
      value_ -= frequency_[before] <= cap_[before];
      --frequency_[before];
    }
    if (tracked(after)) {
      value_ += frequency_[after] < cap_[after];
      ++frequency_[after];
    }
  }

  int value() const { return value_; }
  int frequency(int size) const {
    assert(size >= 0 && size < static_cast<int>(cap_.size()));
    return frequency_[size]; // cap=0 のサイズは記録しない
  }

 private:
  bool tracked(int size) const {
    assert(size >= 0);
    return size < static_cast<int>(cap_.size()) && cap_[size] > 0;
  }
  std::vector<int> cap_, frequency_;
  int value_ = 0;
};
