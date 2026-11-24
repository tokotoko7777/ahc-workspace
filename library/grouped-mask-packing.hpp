#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace ahc {

struct MaskPackingOption {
  std::uint32_t resources;
  std::int64_t value;
};

struct GroupedMaskPackingResult {
  std::int64_t value;
  std::uint32_t resources;
  std::vector<int> option;  // chosen index per group; -1 if skipped
};

// Choose up to one option per group, with no overlapping resource bits.
// All options can be skipped; thus zero or negative values are never needed.
inline GroupedMaskPackingResult grouped_mask_packing(
    int resource_count,
    std::span<const std::vector<MaskPackingOption>> groups) {
  if (resource_count < 0 || resource_count > 16)
    throw std::invalid_argument("resource count must be in [0,16]");
  const auto states = 1u << resource_count;
  std::vector<std::int64_t> dp(states, -1), next(states);
  std::vector<std::vector<int>> choices;
  choices.reserve(groups.size());
  dp[0] = 0;
  for (const auto& group : groups) {
    if (group.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
      throw std::invalid_argument("too many options in a group");
    next = dp;
    choices.emplace_back(states, -1);
    auto& chosen = choices.back();
    for (int i = 0; i < static_cast<int>(group.size()); ++i) {
      const auto [mask, value] = group[i];
      if (mask >= states) throw std::invalid_argument("option uses an unknown resource");
      if (value <= 0) continue;
      for (auto free = (states - 1) ^ mask, prev = free;; prev = (prev - 1) & free) {
        if (dp[prev] >= 0) {
          if (dp[prev] > std::numeric_limits<std::int64_t>::max() - value)
            throw std::overflow_error("packing value exceeds int64_t");
          const auto dst = prev | mask;
          if (const auto v = dp[prev] + value; v > next[dst]) {
            next[dst] = v;
            chosen[dst] = i;
          }
        }
        if (prev == 0) break;
      }
    }
    dp.swap(next);
  }
  const auto best = std::max_element(dp.begin(), dp.end());
  auto mask = static_cast<std::uint32_t>(best - dp.begin());
  GroupedMaskPackingResult result{*best, mask, std::vector<int>(groups.size())};
  for (auto g = groups.size(); g > 0; --g) {
    const int i = choices[g - 1][mask];
    result.option[g - 1] = i;
    if (i >= 0) mask ^= groups[g - 1][i].resources;
  }
  return result;
}

}  // namespace ahc
