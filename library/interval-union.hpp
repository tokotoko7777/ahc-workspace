#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/interval-union.hpp

// 半開区間 [left, right) の重なりと接触をまとめる。
//
// 使い方:
// vector<pair<long long, long long>> intervals = {{1, 4}, {3, 7}, {9, 10}};
// auto merged = merge_half_open_intervals(intervals);
// // merged == {{1, 7}, {9, 10}}
template <class Value>
std::vector<std::pair<Value, Value>> merge_half_open_intervals(
    std::vector<std::pair<Value, Value>> intervals) {
  for (const auto& [left, right] : intervals) assert(!(right < left));
  std::sort(intervals.begin(), intervals.end());

  std::vector<std::pair<Value, Value>> merged;
  for (const auto& interval : intervals) {
    if (interval.first == interval.second) continue;
    if (merged.empty() || merged.back().second < interval.first) {
      merged.push_back(interval);
    } else if (merged.back().second < interval.second) {
      merged.back().second = interval.second;
    }
  }
  return merged;
}

// 区間の和集合の長さを返す。入力は未整列・重複ありでよい。
template <class Value>
Value interval_union_length(
    std::vector<std::pair<Value, Value>> intervals) {
  const auto merged = merge_half_open_intervals(std::move(intervals));
  Value result{};
  for (const auto& [left, right] : merged) result += right - left;
  return result;
}

// 2つの区間集合のうち、片方だけに含まれる部分の合計長を返す。
// 壁の追加・削除量や、時間差分の変化量を数える時に使える。
template <class Value>
Value interval_symmetric_difference_length(
    std::vector<std::pair<Value, Value>> first,
    std::vector<std::pair<Value, Value>> second) {
  first = merge_half_open_intervals(std::move(first));
  second = merge_half_open_intervals(std::move(second));

  Value result{};
  for (const auto& [left, right] : first) result += right - left;
  for (const auto& [left, right] : second) result += right - left;

  int first_index = 0;
  int second_index = 0;
  while (first_index < static_cast<int>(first.size())
         && second_index < static_cast<int>(second.size())) {
    const auto& a = first[first_index];
    const auto& b = second[second_index];
    const Value overlap_left = std::max(a.first, b.first);
    const Value overlap_right = std::min(a.second, b.second);
    if (overlap_left < overlap_right) {
      result -= (overlap_right - overlap_left) * Value{2};
    }
    if (a.second < b.second) {
      ++first_index;
    } else {
      ++second_index;
    }
  }
  return result;
}
