#include <algorithm>
#include <cassert>
#include <queue>
#include <string>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/grid-bfs.hpp

struct GridBfsResult {
  int height;
  int width;
  std::pair<int, int> start;
  std::vector<std::vector<int>> distance;
  std::vector<std::vector<std::pair<int, int>>> parent;

  bool reachable(std::pair<int, int> position) const {
    const auto [row, column] = position;
    assert(0 <= row && row < height && 0 <= column && column < width);
    return distance[row][column] != -1;
  }

  // start から goal までのマスを順番に返す。到達不能なら空の vector。
  std::vector<std::pair<int, int>> path_to(
      std::pair<int, int> goal) const {
    if (!reachable(goal)) return {};

    std::vector<std::pair<int, int>> path;
    for (auto position = goal;; position = parent[position.first][position.second]) {
      path.push_back(position);
      if (position == start) break;
    }
    std::reverse(path.begin(), path.end());
    return path;
  }
};

// can_enter(row, column) が true のマスだけを通る4方向BFS。
// 使い方:
// auto result = grid_bfs(height, width, {start_row, start_column},
//                        [&](int r, int c) { return grid[r][c] != '#'; });
template <class CanEnter>
GridBfsResult grid_bfs(int height, int width, std::pair<int, int> start,
                       CanEnter can_enter) {
  assert(height > 0 && width > 0);
  assert(0 <= start.first && start.first < height);
  assert(0 <= start.second && start.second < width);
  assert(can_enter(start.first, start.second));

  GridBfsResult result{
      height,
      width,
      start,
      std::vector<std::vector<int>>(height, std::vector<int>(width, -1)),
      std::vector<std::vector<std::pair<int, int>>>(
          height, std::vector<std::pair<int, int>>(width, {-1, -1}))};

  std::queue<std::pair<int, int>> queue;
  result.distance[start.first][start.second] = 0;
  result.parent[start.first][start.second] = start;
  queue.push(start);

  constexpr int DR[4] = {-1, 1, 0, 0};
  constexpr int DC[4] = {0, 0, -1, 1};
  while (!queue.empty()) {
    const auto [row, column] = queue.front();
    queue.pop();

    for (int direction = 0; direction < 4; ++direction) {
      const int next_row = row + DR[direction];
      const int next_column = column + DC[direction];
      if (next_row < 0 || next_row >= height || next_column < 0 ||
          next_column >= width) {
        continue;
      }
      if (!can_enter(next_row, next_column)) continue;
      if (result.distance[next_row][next_column] != -1) continue;

      result.distance[next_row][next_column] =
          result.distance[row][column] + 1;
      result.parent[next_row][next_column] = {row, column};
      queue.push({next_row, next_column});
    }
  }
  return result;
}

// '#' を壁として扱う、文字グリッド用の簡単な形。
inline GridBfsResult grid_bfs(const std::vector<std::string>& grid,
                              std::pair<int, int> start, char wall = '#') {
  assert(!grid.empty() && !grid[0].empty());
  const int height = static_cast<int>(grid.size());
  const int width = static_cast<int>(grid[0].size());
  for (const std::string& row : grid) {
    assert(static_cast<int>(row.size()) == width);
  }
  return grid_bfs(height, width, start,
                  [&](int row, int column) { return grid[row][column] != wall; });
}
