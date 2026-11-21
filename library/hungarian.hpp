#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/hungarian.hpp

// 行数 <= 列数のcost[row][column]に対する、合計費用最小の1対1割り当て。
// 戻り値answer[row]は、そのrowへ割り当てたcolumn番号。
//
// 使い方:
// vector<vector<long long>> cost = {
//     {4, 1, 3}, {2, 0, 5}, {3, 2, 2}};
// vector<int> answer = hungarian_rectangular_minimum_assignment(cost);
// // answer == {1, 0, 2}、合計費用は5
//
// Costにはlong longやdoubleなどの符号付き数値型を使う。
// R 行 C 列の場合、計算量 O(R^2 C)、追加メモリ O(R+C)。
template <class Cost>
std::vector<int> hungarian_rectangular_minimum_assignment(
    const std::vector<std::vector<Cost>>& cost) {
  static_assert(std::is_signed_v<Cost> || std::is_floating_point_v<Cost>);
  const int rows = static_cast<int>(cost.size());
  if (rows == 0) return {};
  const int columns = static_cast<int>(cost[0].size());
  assert(rows <= columns);
  for (const auto& row : cost) {
    assert(static_cast<int>(row.size()) == columns);
  }

  const Cost infinity = std::numeric_limits<Cost>::max() / Cost{4};
  std::vector<Cost> row_potential(rows + 1);
  std::vector<Cost> column_potential(columns + 1);
  std::vector<int> matched_row(columns + 1);
  std::vector<int> previous_column(columns + 1);
  std::vector<Cost> minimum_slack(columns + 1);
  std::vector<char> used(columns + 1);

  for (int row = 1; row <= rows; ++row) {
    matched_row[0] = row;
    int current_column = 0;
    std::fill(minimum_slack.begin(), minimum_slack.end(), infinity);
    std::fill(used.begin(), used.end(), false);

    do {
      used[current_column] = true;
      const int current_row = matched_row[current_column];
      Cost delta = infinity;
      int next_column = 0;
      for (int column = 1; column <= columns; ++column) {
        if (used[column]) continue;
        const Cost slack = cost[current_row - 1][column - 1]
                         - row_potential[current_row]
                         - column_potential[column];
        if (slack < minimum_slack[column]) {
          minimum_slack[column] = slack;
          previous_column[column] = current_column;
        }
        if (minimum_slack[column] < delta) {
          delta = minimum_slack[column];
          next_column = column;
        }
      }

      for (int column = 0; column <= columns; ++column) {
        if (used[column]) {
          row_potential[matched_row[column]] += delta;
          column_potential[column] -= delta;
        } else {
          minimum_slack[column] -= delta;
        }
      }
      current_column = next_column;
    } while (matched_row[current_column] != 0);

    do {
      const int next_column = previous_column[current_column];
      matched_row[current_column] = matched_row[next_column];
      current_column = next_column;
    } while (current_column != 0);
  }

  std::vector<int> answer(rows);
  for (int column = 1; column <= columns; ++column) {
    if (matched_row[column]) answer[matched_row[column] - 1] = column - 1;
  }
  return answer;
}

// Keep the original square-matrix interface and tie ordering.
template <class Cost>
std::vector<int> hungarian_minimum_assignment(
    const std::vector<std::vector<Cost>>& cost) {
  for (const auto& row : cost) assert(row.size() == cost.size());
  return hungarian_rectangular_minimum_assignment(cost);
}
