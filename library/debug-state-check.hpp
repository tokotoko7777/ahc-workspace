#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/debug-state-check.hpp

// 差分更新やapply/revertを、独立に全再計算した値と照合するdebug用補助。
// 1回のrequireは比較自体を除いてO(1)、保持メモリはメッセージ分だけ。
// 失敗時にseed・反復番号・Move列・最初に違った項目を例外へ入れる。
//
// 使い方:
// #ifndef NDEBUG
// AhcDebugStateCheck check(seed, iteration, move_text);
// check.require_equal("score", state.score, calculate_score_from_scratch(state));
// check.require_close("average", state.average, oracle_average, 1e-9, 1e-9);
// #endif
//
// State全体のmemcmpには使わず、盤面、score、hash、cache、候補集合などを
// 意味のある順に個別比較する。NDEBUG版で全再計算やStateコピーも消したい時は、
// 呼び出しだけでなく検査用の計算全体を#ifndef NDEBUGで囲む。
struct AhcDebugStateCheck {
  std::uint64_t seed = 0;
  long long iteration = 0;
  std::string moves;

  AhcDebugStateCheck(
      std::uint64_t seed_value,
      long long iteration_value,
      std::string moves_value)
      : seed(seed_value),
        iteration(iteration_value),
        moves(std::move(moves_value)) {}

  [[noreturn]] void fail(
      const std::string& first_difference,
      const std::string& detail = "") const {
    std::ostringstream message;
    message << "debug state mismatch: seed=" << seed
            << " iteration=" << iteration
            << " moves=" << (moves.empty() ? "[]" : moves)
            << " first_difference=" << first_difference;
    if (!detail.empty()) message << " detail=" << detail;
    throw std::runtime_error(message.str());
  }

  void require(
      bool condition,
      const std::string& first_difference,
      const std::string& detail = "") const {
    if (!condition) fail(first_difference, detail);
  }

  template <class Actual, class Expected>
  void require_equal(
      const std::string& first_difference,
      const Actual& actual,
      const Expected& expected) const {
    if (!(actual == expected)) fail(first_difference);
  }

  void require_close(
      const std::string& first_difference,
      double actual,
      double expected,
      double absolute_tolerance,
      double relative_tolerance) const {
    if (std::isnan(actual) || std::isnan(expected) ||
        absolute_tolerance < 0.0 || relative_tolerance < 0.0 ||
        !std::isfinite(absolute_tolerance) ||
        !std::isfinite(relative_tolerance)) {
      fail(first_difference, "NaN or invalid tolerance");
    }
    if (actual == expected) return;
    if (!std::isfinite(actual) || !std::isfinite(expected)) {
      fail(first_difference, "non-finite values differ");
    }
    const double difference = std::abs(actual - expected);
    const double scale = std::max(std::abs(actual), std::abs(expected));
    if (difference > absolute_tolerance + relative_tolerance * scale) {
      std::ostringstream detail;
      detail << "actual=" << actual << " expected=" << expected;
      fail(first_difference, detail.str());
    }
  }
};
