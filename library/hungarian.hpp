#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/hungarian.hpp

// 正方形のcost[row][column]に対する、合計費用最小の1対1割り当て。
// 戻り値answer[row]は、そのrowへ割り当てたcolumn番号。
//
// 使い方:
// vector<vector<long long>> cost = {
//     {4, 1, 3}, {2, 0, 5}, {3, 2, 2}};
// vector<int> answer = hungarian_minimum_assignment(cost);
// // answer == {1, 0, 2}、合計費用は5
//
// Costにはlong longやdoubleなどの符号付き数値型を使う。
// 計算量は行数をNとして O(N^3)、追加メモリは O(N)。
template <class Cost>
std::vector<int> hungarian_minimum_assignment(
    const std::vector<std::vector<Cost>>& cost) {
  static_assert(std::is_signed_v<Cost> || std::is_floating_point_v<Cost>);
  const int size = static_cast<int>(cost.size());
  if (size == 0) return {};
  for (const auto& row : cost) {
    assert(static_cast<int>(row.size()) == size);
  }

  const Cost infinity = std::numeric_limits<Cost>::max() / Cost{4};
  std::vector<Cost> row_potential(size + 1);
  std::vector<Cost> column_potential(size + 1);
  std::vector<int> matched_row(size + 1);
  std::vector<int> previous_column(size + 1);

  for (int row = 1; row <= size; ++row) {
    matched_row[0] = row;
    int current_column = 0;
    std::vector<Cost> minimum_slack(size + 1, infinity);
    std::vector<char> used(size + 1, false);

    do {
      used[current_column] = true;
      const int current_row = matched_row[current_column];
      Cost delta = infinity;
      int next_column = 0;
      for (int column = 1; column <= size; ++column) {
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

      for (int column = 0; column <= size; ++column) {
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

  std::vector<int> answer(size);
  for (int column = 1; column <= size; ++column) {
    answer[matched_row[column] - 1] = column - 1;
  }
  return answer;
}
