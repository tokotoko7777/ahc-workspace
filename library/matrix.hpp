#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/matrix.hpp

// 動的サイズの行列。要素は連続メモリに保存する。
template <class Value>
struct Matrix {
  int rows = 0;
  int columns = 0;
  std::vector<Value> values;

  Matrix() = default;

  Matrix(int rows, int columns, Value initial = Value{})
      : rows(rows), columns(columns) {
    assert(rows >= 0 && columns >= 0);
    values.assign(static_cast<std::size_t>(rows) * columns, initial);
  }

  explicit Matrix(const std::vector<std::vector<Value>>& source)
      : rows(static_cast<int>(source.size())),
        columns(source.empty() ? 0 : static_cast<int>(source[0].size())) {
    values.reserve(static_cast<std::size_t>(rows) * columns);
    for (const auto& row : source) {
      assert(static_cast<int>(row.size()) == columns);
      values.insert(values.end(), row.begin(), row.end());
    }
  }

  Value& operator()(int row, int column) {
    check_index(row, column);
    return values[static_cast<std::size_t>(row) * columns + column];
  }

  const Value& operator()(int row, int column) const {
    check_index(row, column);
    return values[static_cast<std::size_t>(row) * columns + column];
  }

  static Matrix identity(int size) {
    assert(size >= 0);
    Matrix result(size, size);
    for (int i = 0; i < size; ++i) result(i, i) = Value{1};
    return result;
  }

  Matrix operator+(const Matrix& other) const {
    assert(rows == other.rows && columns == other.columns);
    Matrix result(rows, columns);
    for (std::size_t i = 0; i < values.size(); ++i) {
      result.values[i] = values[i] + other.values[i];
    }
    return result;
  }

  Matrix operator*(const Matrix& other) const {
    assert(columns == other.rows);
    Matrix result(rows, other.columns);
    for (int row = 0; row < rows; ++row) {
      for (int middle = 0; middle < columns; ++middle) {
        const Value left = (*this)(row, middle);
        if (left == Value{}) continue;
        for (int column = 0; column < other.columns; ++column) {
          result(row, column) += left * other(middle, column);
        }
      }
    }
    return result;
  }

  bool operator==(const Matrix& other) const {
    return rows == other.rows && columns == other.columns &&
           values == other.values;
  }

 private:
  void check_index(int row, int column) const {
    assert(0 <= row && row < rows);
    assert(0 <= column && column < columns);
  }
};

// 正方行列のn乗。O(size^3 log exponent)。
template <class Value>
Matrix<Value> matrix_power(Matrix<Value> base, std::uint64_t exponent) {
  assert(base.rows == base.columns);
  Matrix<Value> result = Matrix<Value>::identity(base.rows);
  while (exponent > 0) {
    if (exponent & 1) result = result * base;
    base = base * base;
    exponent >>= 1;
  }
  return result;
}
