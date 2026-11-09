#include <algorithm>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/sequence-overlap.hpp

// 1つ目の末尾と2つ目の先頭が一致する最大長を返す。O(min(N,M)^2)。
// stringだけでなくvector<int>などにも使える。
// int length = suffix_prefix_overlap(first, second);
template <class Sequence>
int suffix_prefix_overlap(const Sequence& first, const Sequence& second) {
  const int limit = static_cast<int>(std::min(first.size(), second.size()));
  for (int length = limit; length >= 1; --length) {
    bool same = true;
    for (int index = 0; index < length; ++index) {
      if (first[first.size() - length + index] != second[index]) {
        same = false;
        break;
      }
    }
    if (same) return length;
  }
  return 0;
}

// 最大限重ねて連結する。
// auto merged = merge_with_overlap(string("ABCDE"), string("CDEFG"));
template <class Sequence>
Sequence merge_with_overlap(const Sequence& first, const Sequence& second) {
  Sequence result = first;
  const int overlap = suffix_prefix_overlap(first, second);
  result.insert(result.end(), second.begin() + overlap, second.end());
  return result;
}
