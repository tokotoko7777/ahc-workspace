#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/coordinate-compression.hpp

// 座標圧縮。int、long long、string など、比較できる型なら使える。
// 使い方:
// CoordinateCompression<long long> xs(values);
// int compressed = xs.index(original_value);
// long long original = xs.value(compressed);
template <class T>
struct CoordinateCompression {
  std::vector<T> values;

  explicit CoordinateCompression(std::vector<T> values)
      : values(std::move(values)) {
    std::sort(this->values.begin(), this->values.end());
    this->values.erase(
        std::unique(this->values.begin(), this->values.end()),
        this->values.end());
  }

  int size() const { return static_cast<int>(values.size()); }

  int index(const T& value) const {
    const auto iterator =
        std::lower_bound(values.begin(), values.end(), value);
    assert(iterator != values.end() && *iterator == value);
    return static_cast<int>(iterator - values.begin());
  }

  const T& value(int index) const {
    assert(0 <= index && index < size());
    return values[index];
  }

  std::vector<int> compress(const std::vector<T>& original) const {
    std::vector<int> result;
    result.reserve(original.size());
    for (const T& value : original) result.push_back(index(value));
    return result;
  }
};
