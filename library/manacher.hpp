#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/manacher.hpp

struct PalindromeRadii {
  // odd[i]=i中心の奇数長回文の半径。長さは 2*odd[i]-1。
  std::vector<int> odd;
  // even[i]=i-1とiの間が中心の偶数長回文の半径。長さは 2*even[i]。
  std::vector<int> even;

  bool is_palindrome(int left, int right) const {
    const int n = static_cast<int>(odd.size());
    assert(0 <= left && left <= right && right <= n);
    const int length = right - left;
    if (length == 0) return true;
    if (length & 1) {
      const int center = (left + right) / 2;
      return odd[center] >= length / 2 + 1;
    }
    const int center = (left + right) / 2;
    return even[center] >= length / 2;
  }

  // 最長回文の半開区間 [left, right) を返す。
  std::pair<int, int> longest_interval() const {
    std::pair<int, int> result{0, 0};
    for (int center = 0; center < static_cast<int>(odd.size()); ++center) {
      const int radius = odd[center];
      const std::pair<int, int> candidate{center - radius + 1,
                                          center + radius};
      if (result.second - result.first < candidate.second - candidate.first) {
        result = candidate;
      }
    }
    for (int center = 0; center < static_cast<int>(even.size()); ++center) {
      const int radius = even[center];
      const std::pair<int, int> candidate{center - radius, center + radius};
      if (result.second - result.first < candidate.second - candidate.first) {
        result = candidate;
      }
    }
    return result;
  }
};

// すべての中心の最長回文を O(N) で求めるManacher法。
// string、vector<int>など == で比較できる列に使える。
// auto radii = palindrome_radii(text);
// auto [left, right] = radii.longest_interval();
template <class Sequence>
PalindromeRadii palindrome_radii(const Sequence& sequence) {
  const int n = static_cast<int>(sequence.size());
  PalindromeRadii result{std::vector<int>(n), std::vector<int>(n)};

  for (int i = 0, left = 0, right = -1; i < n; ++i) {
    int radius = i > right ? 1 : std::min(result.odd[left + right - i],
                                          right - i + 1);
    while (0 <= i - radius && i + radius < n &&
           sequence[i - radius] == sequence[i + radius]) {
      ++radius;
    }
    result.odd[i] = radius;
    if (i + radius - 1 > right) {
      left = i - radius + 1;
      right = i + radius - 1;
    }
  }

  for (int i = 0, left = 0, right = -1; i < n; ++i) {
    int radius = i > right ? 0 : std::min(result.even[left + right - i + 1],
                                          right - i + 1);
    while (0 <= i - radius - 1 && i + radius < n &&
           sequence[i - radius - 1] == sequence[i + radius]) {
      ++radius;
    }
    result.even[i] = radius;
    if (i + radius - 1 > right) {
      left = i - radius;
      right = i + radius - 1;
    }
  }
  return result;
}
