#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/prefix-function.hpp

// result[i] = sequence[0..i]の、全体とは異なる最長の接頭辞=接尾辞の長さ。
// string、vector<int>など == で比較できる列に使える。
template <class Sequence>
std::vector<int> prefix_function(const Sequence& sequence) {
  const int n = static_cast<int>(sequence.size());
  std::vector<int> result(n, 0);
  for (int i = 1; i < n; ++i) {
    int length = result[i - 1];
    while (length > 0 && sequence[i] != sequence[length]) {
      length = result[length - 1];
    }
    if (sequence[i] == sequence[length]) ++length;
    result[i] = length;
  }
  return result;
}

// text内でpatternが始まる位置を O(text.size() + pattern.size()) ですべて返す。
// 空patternは0からtext.size()までの全位置にマッチする。
template <class Sequence>
std::vector<int> find_pattern_occurrences(const Sequence& text,
                                          const Sequence& pattern) {
  const int text_size = static_cast<int>(text.size());
  const int pattern_size = static_cast<int>(pattern.size());
  if (pattern_size == 0) {
    std::vector<int> result(text_size + 1);
    for (int i = 0; i <= text_size; ++i) result[i] = i;
    return result;
  }

  const std::vector<int> prefix = prefix_function(pattern);
  std::vector<int> result;
  int length = 0;
  for (int i = 0; i < text_size; ++i) {
    while (length > 0 && text[i] != pattern[length]) {
      length = prefix[length - 1];
    }
    if (text[i] == pattern[length]) ++length;
    if (length == pattern_size) {
      result.push_back(i + 1 - pattern_size);
      length = prefix[length - 1];
    }
  }
  return result;
}
