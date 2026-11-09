#include <cassert>
#include <cstddef>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/difference-array-2d.hpp

// 矩形加算をすべて記録し、最後に O(HW) で各マスの値を作る2次元いもす法。
// DifferenceArray2D<long long> values(height, width);
// values.add(top, left, bottom, right, amount);  // 半開矩形
// vector<vector<long long>> result = values.build();
template <class Value>
struct DifferenceArray2D {
  int height;
  int width;
  std::vector<Value> difference;

  DifferenceArray2D(int height, int width)
      : height(height), width(width) {
    assert(height >= 0 && width >= 0);
    difference.assign(static_cast<std::size_t>(height + 1) * (width + 1),
                      Value{});
  }

  void add(int top, int left, int bottom, int right, Value amount) {
    assert(0 <= top && top <= bottom && bottom <= height);
    assert(0 <= left && left <= right && right <= width);
    difference[index(top, left)] += amount;
    difference[index(top, right)] -= amount;
    difference[index(bottom, left)] -= amount;
    difference[index(bottom, right)] += amount;
  }

  // row * width + column の1次元配列で返す。大きな盤面ではこちらが高速。
  std::vector<Value> build_flat() const {
    std::vector<Value> result(static_cast<std::size_t>(height) * width,
                              Value{});
    for (int row = 0; row < height; ++row) {
      Value row_sum{};
      for (int column = 0; column < width; ++column) {
        row_sum += difference[index(row, column)];
        result[static_cast<std::size_t>(row) * width + column] =
            row_sum +
            (row == 0
                 ? Value{}
                 : result[static_cast<std::size_t>(row - 1) * width + column]);
      }
    }
    return result;
  }

  std::vector<std::vector<Value>> build() const {
    const std::vector<Value> flat = build_flat();
    std::vector<std::vector<Value>> result(
        height, std::vector<Value>(width, Value{}));
    for (int row = 0; row < height; ++row) {
      for (int column = 0; column < width; ++column) {
        result[row][column] =
            flat[static_cast<std::size_t>(row) * width + column];
      }
    }
    return result;
  }

 private:
  std::size_t index(int row, int column) const {
    return static_cast<std::size_t>(row) * (width + 1) + column;
  }
};
