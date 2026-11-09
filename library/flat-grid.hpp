#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/flat-grid.hpp

// 2次元データを1本の vector に置き、行方向へ連続アクセスできる配列。
// vector<vector<T>> より動的確保が少なく、全マス走査でキャッシュに載りやすい。
// 使い方:
// FlatGrid<int> distance(height, width, -1);
// distance(row, column) = 0;
template <class T>
struct FlatGrid {
  int height;
  int width;
  std::vector<T> data;

  FlatGrid(int height, int width, const T& initial_value = T{})
      : height(height),
        width(width),
        data(height < 0 || width < 0
                 ? 0
                 : static_cast<std::size_t>(height) * width,
             initial_value) {
    assert(height >= 0 && width >= 0);
  }

  explicit FlatGrid(const std::vector<std::vector<T>>& values)
      : height(static_cast<int>(values.size())),
        width(values.empty() ? 0 : static_cast<int>(values[0].size())) {
    data.reserve(static_cast<std::size_t>(height) * width);
    for (const auto& row : values) {
      assert(static_cast<int>(row.size()) == width);
      data.insert(data.end(), row.begin(), row.end());
    }
  }

  int size() const { return static_cast<int>(data.size()); }

  int index(int row, int column) const {
    assert(0 <= row && row < height && 0 <= column && column < width);
    return row * width + column;
  }

  std::pair<int, int> position(int index) const {
    assert(width > 0 && 0 <= index && index < size());
    return {index / width, index % width};
  }

  decltype(auto) operator()(int row, int column) {
    return data[index(row, column)];
  }

  decltype(auto) operator()(int row, int column) const {
    return data[index(row, column)];
  }

  void fill(const T& value) { std::fill(data.begin(), data.end(), value); }

  auto begin() { return data.begin(); }
  auto end() { return data.end(); }
  auto begin() const { return data.begin(); }
  auto end() const { return data.end(); }
};
