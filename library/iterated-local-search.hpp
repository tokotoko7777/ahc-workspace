#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/iterated-local-search.hpp
// 着想: https://arxiv.org/abs/math/0102188
// 摂動→局所探索→受理を繰り返すILSの独自実装。

// 重い局所探索の内側でも同じ締切を確認するための読み取り専用budget。
class IlsBudget {
 public:
  explicit IlsBudget(double milliseconds) : limit_(milliseconds), start_(Clock::now()) {
    if (!std::isfinite(limit_) || limit_ <= 0) throw std::invalid_argument("invalid ILS time limit");
  }
  double elapsed_ms() const { return std::chrono::duration<double, std::milli>(Clock::now() - start_).count(); }
  double remaining_ms() const { return std::max(0.0, limit_ - elapsed_ms()); }
  bool expired() const { return elapsed_ms() >= limit_; }
  double progress() const { return std::clamp(elapsed_ms() / limit_, 0.0, 1.0); }
 private:
  using Clock = std::chrono::steady_clock;
  double limit_;
  Clock::time_point start_;
};
struct IlsOptions {
  double time_limit_ms = 1900;
  std::uint64_t seed = 0;
  bool maximize = true;
  bool accept_worse = false; // trueなら局所探索後の候補を常に次の出発点にする。
  std::uint64_t restart_after = 20; // 最良更新がこの回数ないとbestへ戻る。0なら無効。
  std::uint64_t iteration_limit = std::numeric_limits<std::uint64_t>::max();
};

// Problem::perturb(current,candidate,rng,budget): candidateへ合法な大きい変更を書く。
// Problem::local_search(candidate,rng,budget)->Score: 細かい改善を重ね、完成解の絶対値を返す。
// local_searchは締切時にも「現在の合法な候補」を残す。途中の不正状態を返さない。
// 最初のstepは初期解の局所探索のみ。以後は摂動から再度局所探索する。
template <class Problem> class IteratedLocalSearch {
 public:
  using State = typename Problem::State;
  using Score = typename Problem::Score;
  IteratedLocalSearch(Problem& problem, State initial, Score score, IlsOptions options = {})
      : problem_(problem), options_(options), current_(std::move(initial)), candidate_(current_),
        best_(current_), current_score_(score), best_score_(score), rng_(options.seed), budget_(options.time_limit_ms) {
    if (!finite(score)) throw std::invalid_argument("nonfinite initial ILS score");
  }
  bool step() {
    if (iterations_ >= options_.iteration_limit || budget_.expired()) return false;
    if (iterations_) problem_.perturb(static_cast<const State&>(current_), candidate_, rng_, budget_);
    else candidate_ = current_;
    const Score score = problem_.local_search(candidate_, rng_, budget_);
    ++iterations_; ++stale_;
    if (finite(score)) {
      if (better(score, best_score_)) { best_ = candidate_; best_score_ = score; stale_ = 0; ++improved_; }
      if (options_.accept_worse || !better(current_score_, score)) {
        using std::swap; swap(current_, candidate_); current_score_ = score; ++accepted_;
      }
    }
    if (options_.restart_after && stale_ >= options_.restart_after) {
      current_ = best_; current_score_ = best_score_; stale_ = 0;
    }
    return true;
  }
  void run() { while (step()) {} }
  const State& best_state() const { return best_; }
  Score best_score() const { return best_score_; }
  Score current_score() const { return current_score_; }
  std::uint64_t iterations() const { return iterations_; }
  std::uint64_t accepted() const { return accepted_; }
  std::uint64_t improved() const { return improved_; }
 private:
  static bool finite(Score score) { return std::isfinite(static_cast<long double>(score)); }
  bool better(Score a, Score b) const { return options_.maximize ? a > b : a < b; }
  Problem& problem_; IlsOptions options_;
  State current_, candidate_, best_;
  Score current_score_, best_score_;
  std::mt19937_64 rng_; IlsBudget budget_;
  std::uint64_t iterations_ = 0, accepted_ = 0, improved_ = 0, stale_ = 0;
};
