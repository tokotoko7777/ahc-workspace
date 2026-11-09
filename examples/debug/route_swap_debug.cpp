// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "library/debug-state-check.hpp"

// 経路swapの差分score・差分hash・cache・候補集合・revertを検査する完成例。
// 実際の提出コードでは、検査用コピーと全再計算を#ifndef NDEBUGごと外す。

struct RouteDebugPoint {
  int x;
  int y;
};

struct RouteDebugMove {
  int left;
  int right;
};

struct RouteDebugState {
  std::vector<int> route;
  std::vector<int> position_of;
  long long score = 0;
  std::uint64_t hash = 0;
  std::set<std::pair<int, int>> swap_candidates;
};

long long route_debug_distance(
    const RouteDebugPoint& a, const RouteDebugPoint& b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::uint64_t route_debug_token(int position, int value) {
  std::uint64_t x =
      static_cast<std::uint64_t>(position + 1) * 0x9e3779b97f4a7c15ULL ^
      static_cast<std::uint64_t>(value + 11);
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

long long route_debug_full_score(
    const RouteDebugState& state,
    const std::vector<RouteDebugPoint>& points) {
  long long result = 0;
  for (int i = 1; i < static_cast<int>(state.route.size()); ++i) {
    result += route_debug_distance(
        points[state.route[i - 1]], points[state.route[i]]);
  }
  return result;
}

std::uint64_t route_debug_full_hash(const RouteDebugState& state) {
  std::uint64_t result = 0;
  for (int i = 0; i < static_cast<int>(state.route.size()); ++i) {
    result ^= route_debug_token(i, state.route[i]);
  }
  return result;
}

std::vector<int> route_debug_full_positions(const RouteDebugState& state) {
  std::vector<int> result(state.route.size(), -1);
  for (int i = 0; i < static_cast<int>(state.route.size()); ++i) {
    const int value = state.route[i];
    if (0 <= value && value < static_cast<int>(result.size())) {
      result[value] = i;
    }
  }
  return result;
}

bool route_debug_candidate(const RouteDebugState& state, int left, int right) {
  return (state.route[left] + 2 * state.route[right]) % 5 != 0;
}

std::set<std::pair<int, int>> route_debug_full_candidates(
    const RouteDebugState& state) {
  std::set<std::pair<int, int>> result;
  const int n = static_cast<int>(state.route.size());
  for (int left = 1; left + 1 < n; ++left) {
    for (int right = left + 1; right + 1 < n; ++right) {
      if (route_debug_candidate(state, left, right)) {
        result.insert({left, right});
      }
    }
  }
  return result;
}

RouteDebugState route_debug_make_state(
    std::vector<int> route,
    const std::vector<RouteDebugPoint>& points) {
  RouteDebugState state;
  state.route = std::move(route);
  state.position_of = route_debug_full_positions(state);
  state.score = route_debug_full_score(state, points);
  state.hash = route_debug_full_hash(state);
  state.swap_candidates = route_debug_full_candidates(state);
  return state;
}

long long route_debug_swap_delta(
    const RouteDebugState& state,
    const RouteDebugMove& move,
    const std::vector<RouteDebugPoint>& points) {
  std::vector<int> changed_edges{
      move.left, move.left + 1, move.right, move.right + 1};
  std::sort(changed_edges.begin(), changed_edges.end());
  changed_edges.erase(
      std::unique(changed_edges.begin(), changed_edges.end()),
      changed_edges.end());

  const auto value_after = [&](int position) {
    if (position == move.left) return state.route[move.right];
    if (position == move.right) return state.route[move.left];
    return state.route[position];
  };
  long long before = 0;
  long long after = 0;
  for (int edge_right : changed_edges) {
    if (edge_right <= 0 || edge_right >= static_cast<int>(state.route.size())) {
      continue;
    }
    before += route_debug_distance(
        points[state.route[edge_right - 1]], points[state.route[edge_right]]);
    after += route_debug_distance(
        points[value_after(edge_right - 1)], points[value_after(edge_right)]);
  }
  return after - before;
}

void route_debug_refresh_candidate_pairs(
    RouteDebugState& state, int first, int second, bool erase_only) {
  const int n = static_cast<int>(state.route.size());
  for (int other = 1; other + 1 < n; ++other) {
    if (other == first || other == second) continue;
    for (int changed : {first, second}) {
      const auto pair = std::minmax(changed, other);
      state.swap_candidates.erase(pair);
      if (!erase_only && route_debug_candidate(state, pair.first, pair.second)) {
        state.swap_candidates.insert(pair);
      }
    }
  }
  const auto changed_pair = std::minmax(first, second);
  state.swap_candidates.erase(changed_pair);
  if (!erase_only &&
      route_debug_candidate(state, changed_pair.first, changed_pair.second)) {
    state.swap_candidates.insert(changed_pair);
  }
}

void route_debug_apply(
    RouteDebugState& state,
    const RouteDebugMove& move,
    const std::vector<RouteDebugPoint>& points) {
  const long long delta = route_debug_swap_delta(state, move, points);
  route_debug_refresh_candidate_pairs(state, move.left, move.right, true);
  const int left_value = state.route[move.left];
  const int right_value = state.route[move.right];
  state.hash ^= route_debug_token(move.left, left_value);
  state.hash ^= route_debug_token(move.right, right_value);
  state.hash ^= route_debug_token(move.left, right_value);
  state.hash ^= route_debug_token(move.right, left_value);
  std::swap(state.route[move.left], state.route[move.right]);
  state.position_of[left_value] = move.right;
  state.position_of[right_value] = move.left;
#ifndef AHC_DEBUG_INJECT_SCORE_BUG
  state.score += delta;
#else
  (void)delta;
#endif
#ifdef AHC_DEBUG_INJECT_HASH_BUG
  state.hash ^= 1;
#endif
  route_debug_refresh_candidate_pairs(state, move.left, move.right, false);
}

void route_debug_revert(
    RouteDebugState& state,
    const RouteDebugMove& move,
    const std::vector<RouteDebugPoint>& points) {
  route_debug_apply(state, move, points);
#ifdef AHC_DEBUG_INJECT_REVERT_BUG
  state.position_of[state.route[move.left]] = -1;
#endif
}

std::string route_debug_moves_text(const std::vector<RouteDebugMove>& moves) {
  std::ostringstream output;
  output << '[';
  for (int i = 0; i < static_cast<int>(moves.size()); ++i) {
    if (i != 0) output << ',';
    output << '(' << moves[i].left << ',' << moves[i].right << ')';
  }
  output << ']';
  return output.str();
}

#ifndef NDEBUG
void route_debug_verify(
    const RouteDebugState& state,
    const std::vector<RouteDebugPoint>& points,
    std::uint64_t seed,
    int iteration,
    const std::vector<RouteDebugMove>& moves) {
  AhcDebugStateCheck check(
      seed, iteration, route_debug_moves_text(moves));
  check.require_equal(
      "score", state.score, route_debug_full_score(state, points));
  check.require_equal("hash", state.hash, route_debug_full_hash(state));
  check.require_equal(
      "position_cache", state.position_of,
      route_debug_full_positions(state));
  check.require_equal(
      "candidate_order", state.swap_candidates,
      route_debug_full_candidates(state));
  check.require(
      !state.route.empty() && state.route.front() == 0 &&
          state.route.back() == static_cast<int>(state.route.size()) - 1,
      "fixed_endpoints");
  std::vector<int> sorted_route = state.route;
  std::sort(sorted_route.begin(), sorted_route.end());
  bool is_permutation = true;
  for (int i = 0; i < static_cast<int>(sorted_route.size()); ++i) {
    is_permutation = is_permutation && sorted_route[i] == i;
  }
  check.require(is_permutation, "permutation");
}

void route_debug_require_same(
    const RouteDebugState& actual,
    const RouteDebugState& expected,
    std::uint64_t seed,
    int iteration,
    const std::vector<RouteDebugMove>& moves) {
  AhcDebugStateCheck check(
      seed, iteration, route_debug_moves_text(moves));
  check.require_equal("revert.route", actual.route, expected.route);
  check.require_equal("revert.score", actual.score, expected.score);
  check.require_equal("revert.hash", actual.hash, expected.hash);
  check.require_equal(
      "revert.position_cache", actual.position_of, expected.position_of);
  check.require_equal(
      "revert.candidate_order", actual.swap_candidates,
      expected.swap_candidates);
}
#endif

int main() {
  try {
    const std::vector<RouteDebugPoint> points{
        {0, 0}, {4, 1}, {8, 2}, {2, 7}, {7, 8},
        {12, 5}, {5, 12}, {11, 11}, {15, 6}, {16, 0}};
    const std::vector<std::uint64_t> seeds{1, 7, 20260907};
    for (std::uint64_t seed : seeds) {
      std::mt19937_64 random(seed);
      std::vector<int> route(points.size());
      std::iota(route.begin(), route.end(), 0);
      std::shuffle(route.begin() + 1, route.end() - 1, random);
      RouteDebugState state = route_debug_make_state(route, points);
      std::vector<RouteDebugMove> accepted_moves;
#ifndef NDEBUG
      route_debug_verify(state, points, seed, -1, accepted_moves);
#endif

      // score/hash/revertの故障fixtureが必ず最初の1手で検出される経路。
      const RouteDebugMove first_move{1, 2};
#ifndef NDEBUG
      const RouteDebugState before_first = state;
#endif
      route_debug_apply(state, first_move, points);
      accepted_moves.push_back(first_move);
#ifndef NDEBUG
      route_debug_verify(state, points, seed, 0, accepted_moves);
#endif
      route_debug_revert(state, first_move, points);
      accepted_moves.pop_back();
#ifndef NDEBUG
      route_debug_verify(state, points, seed, 0, accepted_moves);
      route_debug_require_same(
          state, before_first, seed, 0, accepted_moves);
#endif

      for (int iteration = 1; iteration <= 600; ++iteration) {
        int left = 1 + static_cast<int>(random() % (route.size() - 2));
        int right = 1 + static_cast<int>(random() % (route.size() - 2));
        if (left == right) right = 1 + right % (route.size() - 2);
        if (right < left) std::swap(left, right);
        const RouteDebugMove move{left, right};
#ifndef NDEBUG
        const RouteDebugState before = state;
#endif
        route_debug_apply(state, move, points);
        accepted_moves.push_back(move);
#ifndef NDEBUG
        route_debug_verify(state, points, seed, iteration, accepted_moves);
#endif
        if (random() % 3 == 0) {
          route_debug_revert(state, move, points);
          accepted_moves.pop_back();
#ifndef NDEBUG
          route_debug_verify(state, points, seed, iteration, accepted_moves);
          route_debug_require_same(
              state, before, seed, iteration, accepted_moves);
#endif
        }

        // 連続Moveを適用し、必ず逆順に戻す。同じ場所の複数更新も含む。
        if (iteration % 100 == 0) {
#ifndef NDEBUG
          const RouteDebugState before_chain = state;
#endif
          std::vector<RouteDebugMove> chain;
          for (int count = 0; count < 5; ++count) {
            int a = 1 + static_cast<int>(random() % (route.size() - 2));
            int b = 1 + static_cast<int>(random() % (route.size() - 2));
            if (a == b) b = 1 + b % (route.size() - 2);
            if (b < a) std::swap(a, b);
            chain.push_back({a, b});
            route_debug_apply(state, chain.back(), points);
            accepted_moves.push_back(chain.back());
#ifndef NDEBUG
            route_debug_verify(
                state, points, seed, iteration, accepted_moves);
#endif
          }
          for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
            route_debug_revert(state, *it, points);
            accepted_moves.pop_back();
          }
#ifndef NDEBUG
          route_debug_verify(state, points, seed, iteration, accepted_moves);
          route_debug_require_same(
              state, before_chain, seed, iteration, accepted_moves);
#endif
        }
      }
    }
    std::cout << "route_swap_debug: all checks passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
