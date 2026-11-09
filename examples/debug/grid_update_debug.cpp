// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "library/debug-state-check.hpp"

// 1マス色変更の差分score・境界・接触辺・hash・候補集合を検査する完成例。
// oracleは差分式を使わず盤面全体を走査し、連結性も毎回BFSで確認する。

struct GridDebugMove {
  int cell;
  int new_color;
  int old_color = -1;
};

struct GridDebugState {
  int height = 0;
  int width = 0;
  int color_count = 0;
  std::vector<int> color;
  std::vector<int> count;
  std::vector<int> contact_edges;
  int boundary_edges = 0;
  long long score = 0;
  std::uint64_t hash = 0;
  std::set<int> boundary_cells;
};

constexpr std::array<int, 4> GRID_DEBUG_DY{-1, 1, 0, 0};
constexpr std::array<int, 4> GRID_DEBUG_DX{0, 0, -1, 1};

bool grid_debug_inside(const GridDebugState& state, int y, int x) {
  return 0 <= y && y < state.height && 0 <= x && x < state.width;
}

int grid_debug_index(const GridDebugState& state, int y, int x) {
  return y * state.width + x;
}

std::vector<int> grid_debug_neighbors(const GridDebugState& state, int cell) {
  const int y = cell / state.width;
  const int x = cell % state.width;
  std::vector<int> result;
  result.reserve(4);
  for (int direction = 0; direction < 4; ++direction) {
    const int next_y = y + GRID_DEBUG_DY[direction];
    const int next_x = x + GRID_DEBUG_DX[direction];
    if (grid_debug_inside(state, next_y, next_x)) {
      result.push_back(grid_debug_index(state, next_y, next_x));
    }
  }
  return result;
}

int grid_debug_cell_value(int cell, int color) {
  return static_cast<int>(
      (static_cast<long long>(cell + 3) * (color + 5) + color * color) % 23);
}

std::uint64_t grid_debug_token(int cell, int color) {
  std::uint64_t x =
      static_cast<std::uint64_t>(cell + 17) * 0x9e3779b97f4a7c15ULL ^
      static_cast<std::uint64_t>(color + 1);
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

int grid_debug_contact_index(
    const GridDebugState& state, int first_color, int second_color) {
  if (second_color < first_color) std::swap(first_color, second_color);
  return first_color * state.color_count + second_color;
}

bool grid_debug_is_boundary_cell(const GridDebugState& state, int cell) {
  for (int neighbor : grid_debug_neighbors(state, cell)) {
    if (state.color[cell] != state.color[neighbor]) return true;
  }
  return false;
}

std::vector<int> grid_debug_full_counts(const GridDebugState& state) {
  std::vector<int> result(state.color_count, 0);
  for (int color : state.color) {
    if (0 <= color && color < state.color_count) ++result[color];
  }
  return result;
}

int grid_debug_full_boundary_edges(const GridDebugState& state) {
  int result = 0;
  for (int y = 0; y < state.height; ++y) {
    for (int x = 0; x < state.width; ++x) {
      const int cell = grid_debug_index(state, y, x);
      if (y + 1 < state.height &&
          state.color[cell] != state.color[grid_debug_index(state, y + 1, x)]) {
        ++result;
      }
      if (x + 1 < state.width &&
          state.color[cell] != state.color[grid_debug_index(state, y, x + 1)]) {
        ++result;
      }
    }
  }
  return result;
}

std::vector<int> grid_debug_full_contact_edges(const GridDebugState& state) {
  std::vector<int> result(state.color_count * state.color_count, 0);
  for (int y = 0; y < state.height; ++y) {
    for (int x = 0; x < state.width; ++x) {
      const int cell = grid_debug_index(state, y, x);
      for (const auto& [dy, dx] :
           std::array<std::pair<int, int>, 2>{{{1, 0}, {0, 1}}}) {
        const int next_y = y + dy;
        const int next_x = x + dx;
        if (!grid_debug_inside(state, next_y, next_x)) continue;
        const int neighbor = grid_debug_index(state, next_y, next_x);
        if (state.color[cell] == state.color[neighbor]) continue;
        ++result[grid_debug_contact_index(
            state, state.color[cell], state.color[neighbor])];
      }
    }
  }
  return result;
}

std::set<int> grid_debug_full_boundary_cells(const GridDebugState& state) {
  std::set<int> result;
  for (int cell = 0; cell < static_cast<int>(state.color.size()); ++cell) {
    if (grid_debug_is_boundary_cell(state, cell)) result.insert(cell);
  }
  return result;
}

std::uint64_t grid_debug_full_hash(const GridDebugState& state) {
  std::uint64_t result = 0;
  for (int cell = 0; cell < static_cast<int>(state.color.size()); ++cell) {
    result ^= grid_debug_token(cell, state.color[cell]);
  }
  return result;
}

long long grid_debug_full_score(const GridDebugState& state) {
  long long result = 0;
  for (int cell = 0; cell < static_cast<int>(state.color.size()); ++cell) {
    result += grid_debug_cell_value(cell, state.color[cell]);
  }
  return result - 3LL * grid_debug_full_boundary_edges(state);
}

bool grid_debug_all_colors_connected(const GridDebugState& state) {
  for (int target = 0; target < state.color_count; ++target) {
    int start = -1;
    int expected_count = 0;
    for (int cell = 0; cell < static_cast<int>(state.color.size()); ++cell) {
      if (state.color[cell] == target) {
        if (start == -1) start = cell;
        ++expected_count;
      }
    }
    if (start == -1) return false;
    std::vector<char> visited(state.color.size(), false);
    std::queue<int> queue;
    visited[start] = true;
    queue.push(start);
    int reached = 0;
    while (!queue.empty()) {
      const int cell = queue.front();
      queue.pop();
      ++reached;
      for (int neighbor : grid_debug_neighbors(state, cell)) {
        if (!visited[neighbor] && state.color[neighbor] == target) {
          visited[neighbor] = true;
          queue.push(neighbor);
        }
      }
    }
    if (reached != expected_count) return false;
  }
  return true;
}

GridDebugState grid_debug_make_state(
    int height, int width, int color_count, std::vector<int> color) {
  GridDebugState state;
  state.height = height;
  state.width = width;
  state.color_count = color_count;
  state.color = std::move(color);
  state.count = grid_debug_full_counts(state);
  state.contact_edges = grid_debug_full_contact_edges(state);
  state.boundary_edges = grid_debug_full_boundary_edges(state);
  state.score = grid_debug_full_score(state);
  state.hash = grid_debug_full_hash(state);
  state.boundary_cells = grid_debug_full_boundary_cells(state);
  return state;
}

void grid_debug_recolor(GridDebugState& state, int cell, int new_color) {
  const int old_color = state.color[cell];
  const std::vector<int> neighbors = grid_debug_neighbors(state, cell);
  std::vector<int> affected = neighbors;
  affected.push_back(cell);
  for (int affected_cell : affected) {
    state.boundary_cells.erase(affected_cell);
  }

  const int old_boundary_edges = state.boundary_edges;
  for (int neighbor : neighbors) {
    const int neighbor_color = state.color[neighbor];
    if (old_color != neighbor_color) {
      --state.boundary_edges;
      --state.contact_edges[grid_debug_contact_index(
          state, old_color, neighbor_color)];
    }
  }
  --state.count[old_color];
  ++state.count[new_color];
  state.hash ^= grid_debug_token(cell, old_color);
  state.hash ^= grid_debug_token(cell, new_color);
  state.score +=
      grid_debug_cell_value(cell, new_color) -
      grid_debug_cell_value(cell, old_color);
  state.color[cell] = new_color;

  for (int neighbor : neighbors) {
    const int neighbor_color = state.color[neighbor];
    if (new_color != neighbor_color) {
      ++state.boundary_edges;
      ++state.contact_edges[grid_debug_contact_index(
          state, new_color, neighbor_color)];
    }
  }
  state.score -= 3LL * (state.boundary_edges - old_boundary_edges);
  for (int affected_cell : affected) {
    if (grid_debug_is_boundary_cell(state, affected_cell)) {
      state.boundary_cells.insert(affected_cell);
    }
  }
}

void grid_debug_apply(GridDebugState& state, GridDebugMove& move) {
  move.old_color = state.color[move.cell];
  grid_debug_recolor(state, move.cell, move.new_color);
}

void grid_debug_revert(GridDebugState& state, const GridDebugMove& move) {
  grid_debug_recolor(state, move.cell, move.old_color);
}

bool grid_debug_legal_move(const GridDebugState& state, GridDebugMove move) {
  if (move.cell < 0 || move.cell >= static_cast<int>(state.color.size()) ||
      move.new_color < 0 || move.new_color >= state.color_count ||
      move.new_color == state.color[move.cell]) {
    return false;
  }
  GridDebugState candidate = state;
  grid_debug_apply(candidate, move);
  return grid_debug_all_colors_connected(candidate);
}

std::string grid_debug_moves_text(const std::vector<GridDebugMove>& moves) {
  std::ostringstream output;
  output << '[';
  for (int i = 0; i < static_cast<int>(moves.size()); ++i) {
    if (i != 0) output << ',';
    output << '(' << moves[i].cell << ':' << moves[i].old_color
           << "->" << moves[i].new_color << ')';
  }
  output << ']';
  return output.str();
}

#ifndef NDEBUG
void grid_debug_verify(
    const GridDebugState& state,
    std::uint64_t seed,
    int iteration,
    const std::vector<GridDebugMove>& moves) {
  AhcDebugStateCheck check(seed, iteration, grid_debug_moves_text(moves));
  check.require_equal("score", state.score, grid_debug_full_score(state));
  check.require_equal("hash", state.hash, grid_debug_full_hash(state));
  check.require_equal("counts", state.count, grid_debug_full_counts(state));
  check.require_equal(
      "boundary_edges", state.boundary_edges,
      grid_debug_full_boundary_edges(state));
  check.require_equal(
      "contact_edges", state.contact_edges,
      grid_debug_full_contact_edges(state));
  check.require_equal(
      "candidate_order", state.boundary_cells,
      grid_debug_full_boundary_cells(state));
  check.require(grid_debug_all_colors_connected(state), "connectivity");
}

void grid_debug_require_same(
    const GridDebugState& actual,
    const GridDebugState& expected,
    std::uint64_t seed,
    int iteration,
    const std::vector<GridDebugMove>& moves) {
  AhcDebugStateCheck check(seed, iteration, grid_debug_moves_text(moves));
  check.require_equal("revert.color", actual.color, expected.color);
  check.require_equal("revert.score", actual.score, expected.score);
  check.require_equal("revert.hash", actual.hash, expected.hash);
  check.require_equal("revert.counts", actual.count, expected.count);
  check.require_equal(
      "revert.contact_edges", actual.contact_edges, expected.contact_edges);
  check.require_equal(
      "revert.boundary_edges", actual.boundary_edges,
      expected.boundary_edges);
  check.require_equal(
      "revert.candidate_order", actual.boundary_cells,
      expected.boundary_cells);
}
#endif

bool grid_debug_try_repair(
    GridDebugState& state,
    std::vector<GridDebugMove> proposed,
    int maximum_applied,
    std::vector<GridDebugMove>& trace) {
  std::vector<GridDebugMove> applied;
  for (GridDebugMove move : proposed) {
    if (static_cast<int>(applied.size()) >= maximum_applied ||
        !grid_debug_legal_move(state, move)) {
      for (auto it = applied.rbegin(); it != applied.rend(); ++it) {
        grid_debug_revert(state, *it);
        trace.pop_back();
      }
      return false;
    }
    grid_debug_apply(state, move);
    applied.push_back(move);
    trace.push_back(move);
  }
  return true;
}

int main() {
  try {
    constexpr int height = 5;
    constexpr int width = 8;
    constexpr int color_count = 3;
    std::vector<int> initial_color(height * width);
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        initial_color[y * width + x] = x < 3 ? 0 : (x < 6 ? 1 : 2);
      }
    }

    const std::vector<std::uint64_t> seeds{3, 11, 20260907};
    for (std::uint64_t seed : seeds) {
      std::mt19937_64 random(seed);
      GridDebugState state = grid_debug_make_state(
          height, width, color_count, initial_color);
      std::vector<GridDebugMove> trace;
#ifndef NDEBUG
      grid_debug_verify(state, seed, -1, trace);
#endif

      // 修復途中で展開上限へ達しても、既に適用したMoveを全て戻す。
#ifndef NDEBUG
      const GridDebugState before_budget_failure = state;
#endif
      const std::vector<GridDebugMove> repair{{10, 1}, {18, 0}};
      const bool repaired = grid_debug_try_repair(state, repair, 1, trace);
      if (repaired) throw std::runtime_error("repair fixture unexpectedly succeeded");
#ifndef NDEBUG
      grid_debug_verify(state, seed, 0, trace);
      grid_debug_require_same(
          state, before_budget_failure, seed, 0, trace);
#endif

      for (int iteration = 1; iteration <= 1200; ++iteration) {
        GridDebugMove move{
            static_cast<int>(random() % state.color.size()),
            static_cast<int>(random() % color_count)};
#ifndef NDEBUG
        const GridDebugState before = state;
#endif
        if (!grid_debug_legal_move(state, move)) {
#ifndef NDEBUG
          grid_debug_require_same(state, before, seed, iteration, trace);
#endif
          continue;
        }
        grid_debug_apply(state, move);
        trace.push_back(move);
#ifndef NDEBUG
        grid_debug_verify(state, seed, iteration, trace);
#endif
        if (random() % 2 == 0) {
          grid_debug_revert(state, move);
          trace.pop_back();
#ifndef NDEBUG
          grid_debug_verify(state, seed, iteration, trace);
          grid_debug_require_same(state, before, seed, iteration, trace);
#endif
        }
      }
    }
    std::cout << "grid_update_debug: all checks passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
