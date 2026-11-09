#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/cumulative-sum-2d.hpp

// 2次元累積和。上・左を含み、下・右を含まない長方形の合計を返す。
// 使い方:
// CumulativeSum2D<long long> sum(grid);
// long long total = sum.query(top, left, bottom, right);
template <class T>
struct CumulativeSum2D {
  int height;
  int width;
  std::vector<std::vector<T>> sum;

  explicit CumulativeSum2D(const std::vector<std::vector<T>>& grid)
      : height(static_cast<int>(grid.size())),
        width(height == 0 ? 0 : static_cast<int>(grid[0].size())),
        sum(height + 1, std::vector<T>(width + 1, T{})) {
    for (const auto& row : grid) {
      assert(static_cast<int>(row.size()) == width);
    }
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        sum[y + 1][x + 1] = grid[y][x] + sum[y][x + 1] +
                            sum[y + 1][x] - sum[y][x];
      }
    }
  }

  T query(int top, int left, int bottom, int right) const {
    assert(0 <= top && top <= bottom && bottom <= height);
    assert(0 <= left && left <= right && right <= width);
    return sum[bottom][right] - sum[top][right] - sum[bottom][left] +
           sum[top][left];
  }
};
