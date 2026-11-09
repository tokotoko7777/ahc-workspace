#include <algorithm>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/z-algorithm.hpp

// z[i] = sequence[0...] と sequence[i...] の最長共通接頭辞の長さ。
// string、vector<int> など、== で比較できる列に O(N) で使える。
template <class Sequence>
std::vector<int> z_algorithm(const Sequence& sequence) {
  const int n = static_cast<int>(sequence.size());
  if (n == 0) return {};
  std::vector<int> z(n, 0);
  z[0] = n;
  int left = 0;
  int right = 0;
  for (int i = 1; i < n; ++i) {
    if (i < right) z[i] = std::min(right - i, z[i - left]);
    while (i + z[i] < n && sequence[z[i]] == sequence[i + z[i]]) ++z[i];
    if (right < i + z[i]) {
      left = i;
      right = i + z[i];
    }
  }
  return z;
}
