#pragma once
#include <cassert>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>
#include "capped-size-histogram.hpp"

// 既存グループを共通閾値で2分し、サイズ別上限制約の得点を最大化。
// events はソート済みの (座標, グループID)。同一座標をまとめて低い側へ移す。
// group_sizes[g] は events に現れる g の数。cap[0] は0とする。
struct CappedPartitionSplit {
  int initial_value, value;
  std::optional<int> threshold; // 厳密な改善がないなら閾値なし
};

inline CappedPartitionSplit best_capped_partition_split(
    const std::vector<std::pair<int,int>>& events,
    const std::vector<int>& group_sizes, const std::vector<int>& cap) {
  assert(!cap.empty() && cap[0] == 0);
  auto high=group_sizes;
  std::vector<int> low(high.size());
  CappedSizeHistogram hist(cap);
  for(int n:high) if(n>0) hist.replace_size(0,n);
  CappedPartitionSplit result{hist.value(),hist.value(),std::nullopt};
  for(std::size_t i=0;i<events.size();) {
    const int key=events[i].first;
    do {
      int g=events[i].second;
      assert(g>=0 && g<int(high.size()) && high[g]>0);
      hist.replace_size(high[g],high[g]-1);
      hist.replace_size(low[g],low[g]+1);
      --high[g];
      ++low[g];
      ++i;
    } while(i<events.size() && events[i].first==key);
    if(hist.value()>result.value) result={result.initial_value,hist.value(),key};
  }
  return result;
}
