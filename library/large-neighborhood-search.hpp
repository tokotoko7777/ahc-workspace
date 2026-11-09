#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/large-neighborhood-search.hpp
// LNS / RRTの着想: https://atcoder.jp/contests/ahc059/editorial/15052
// 解説の考え方から独自実装。第三者の提出コードは使用していない。

enum class LnsAcceptance { HillClimbing, RecordToRecord, SimulatedAnnealing };
// 直近のtrueを返したstep()の結果（採用とは限らない）。近傍への報酬などに使う。
// Rejectedには修復失敗・閾値打ち切り・非有限scoreも含む。
enum class LnsOutcome { Rejected, Accepted, ImprovedCurrent, ImprovedBest };

struct LnsOptions {
  double time_limit_ms = 1900.0;
  std::uint64_t seed = 0;
  bool maximize = true;  // 距離・コストを小さくするならfalse。
  LnsAcceptance acceptance = LnsAcceptance::HillClimbing;
  // RRT: 「現在値」ではなく「過去最良値」から許す悪化幅。線形に変化。
  double start_margin = 2.0;
  double end_margin = 2.0;
  // SA: 指数冷却。Scoreと同じ単位で指定する。
  double start_temperature = 10.0;
  double end_temperature = 0.1;
  bool early_cutoff = true;
  int clock_interval = 1;
  // 有限値なら進捗は反復数から計算する（再現テスト用）。時計の上限も守る。
  std::uint64_t iteration_limit = std::numeric_limits<std::uint64_t>::max();
};

// 部分破壊・再構築の制御だけを担当する。これ1ファイルで使える。
// 焼きなまし/山登りで使う「部分破壊・再構築の近傍」を分けて書く形式。
// TODO付き雛形: template/search/local-search/destroy-repair.cpp
// Problemには次の2関数を書く。
//   void destroy(const State& current, State& candidate,
//                std::mt19937_64& rng, double progress);
//     currentを変更せず、candidateを「一部壊れた状態」に上書きする。
//   std::optional<Score> repair(State& candidate, std::mt19937_64& rng,
//                               double progress, long double threshold);
//     修復成功なら「完成した候補の絶対スコア」を返す。改善量ではない！
//     不可能 / 閾値を超えられないと確定した時だけnulloptを返す。
//     最大化なら score >= threshold、最小化なら score <= threshold が採用条件。
//     cutoff OFF時はそれぞれ -inf / +inf が渡される。
//
// Stateは通常の値型（コピー・swap可能、デフォルト構築不要）。Scoreは数値型。
// repairがnulloptでも元のcurrentは壊れない。candidateのvector容量は再利用する。
// destroyでcandidate=currentとすれば簡単。重い場合は必要な要素だけ詰め直す。
// ProblemはRunnerより長生きさせる。Problem側の作業bufferを共有する同時実行は不可。
// 時計は近傍の間で見る。1回のdestroy/repairが長すぎる場合は問題側でも制限する。
template <class Problem>
class LargeNeighborhoodSearch {
 public:
  using State = typename Problem::State;
  using Score = typename Problem::Score;

  LargeNeighborhoodSearch(Problem& problem, State initial, Score initial_score,
                          LnsOptions options = {})
      : problem_(problem), options_(options), current_(std::move(initial)),
        candidate_(current_), best_(current_), current_score_(initial_score),
        best_score_(initial_score), move_engine_(options.seed),
        acceptance_engine_(options.seed ^ 0xd1b54a32d192ed03ULL) {
    static_assert(std::is_arithmetic<Score>::value, "Score must be numeric");
    static_assert(!std::is_integral<Score>::value ||
                      std::numeric_limits<long double>::digits >=
                          std::numeric_limits<Score>::digits,
                  "long double must represent Score integers exactly");
    if (!(options_.time_limit_ms > 0) || !std::isfinite(options_.time_limit_ms) ||
        options_.clock_interval <= 0 ||
        !finite_nonnegative(options_.start_margin) ||
        !finite_nonnegative(options_.end_margin) ||
        !(options_.start_temperature > 0) ||
        !std::isfinite(options_.start_temperature) ||
        !(options_.end_temperature > 0) ||
        !std::isfinite(options_.end_temperature) ||
        !std::isfinite(static_cast<long double>(initial_score))) {
      throw std::invalid_argument("invalid LNS options or initial score");
    }
    switch (options_.acceptance) {
      case LnsAcceptance::HillClimbing:
      case LnsAcceptance::RecordToRecord:
      case LnsAcceptance::SimulatedAnnealing: break;
      default: throw std::invalid_argument("invalid LNS acceptance");
    }
    log_start_ = std::log(options_.start_temperature);
    log_end_ = std::log(options_.end_temperature);
    started_ = Clock::now();
  }

  // true: 近傍を1回試した（採用とは限らない）。false: 予算終了。
  bool step() {
    if (stopped_ || iterations_ >= options_.iteration_limit) return false;
    if (until_clock_ == 0) {
      const double elapsed = elapsed_ms();
      if (elapsed >= options_.time_limit_ms) {
        stopped_ = true;
        return false;
      }
      progress_ = std::clamp(elapsed / options_.time_limit_ms, 0.0, 1.0);
      until_clock_ = options_.clock_interval;
    }
    --until_clock_;
    if (options_.iteration_limit != std::numeric_limits<std::uint64_t>::max()) {
      progress_ = static_cast<double>(iterations_) /
                  static_cast<double>(options_.iteration_limit);
    }

    last_outcome_ = LnsOutcome::Rejected;
    const long double threshold = acceptance_threshold();
    const long double evaluation_threshold = options_.early_cutoff ? threshold :
        (options_.maximize ? -std::numeric_limits<long double>::infinity() :
                             std::numeric_limits<long double>::infinity());
    problem_.destroy(static_cast<const State&>(current_), candidate_,
                     move_engine_, progress_);
    const std::optional<Score> score =
        problem_.repair(candidate_, move_engine_, progress_, evaluation_threshold);
    ++iterations_;
    if (!score || !std::isfinite(static_cast<long double>(*score))) {
      ++rejected_repairs_;
      return true;
    }
    const long double value = static_cast<long double>(*score);
    if (options_.maximize ? value < threshold : value > threshold) return true;
    last_outcome_ = better(*score, current_score_) ? LnsOutcome::ImprovedCurrent : LnsOutcome::Accepted;
    using std::swap;
    swap(current_, candidate_);
    current_score_ = *score;
    ++accepted_;
    if (better(*score, best_score_)) {
      best_ = current_;
      best_score_ = *score;
      ++improved_;
      last_outcome_ = LnsOutcome::ImprovedBest;
    }
    return true;
  }

  void run() { while (step()) {} }
  const State& best_state() const { return best_; }
  const State& current_state() const { return current_; }
  Score best_score() const { return best_score_; }
  Score current_score() const { return current_score_; }
  std::uint64_t iterations() const { return iterations_; }
  std::uint64_t accepted() const { return accepted_; }
  std::uint64_t improved() const { return improved_; }
  std::uint64_t rejected_repairs() const { return rejected_repairs_; }
  LnsOutcome last_outcome() const { return last_outcome_; }
  double progress() const { return progress_; }
  double elapsed_ms() const {
    return std::chrono::duration<double, std::milli>(Clock::now() - started_).count();
  }
  // 時間・反復予算・温度進捗はリセットしない。
  void restart_from_best() { current_ = best_; current_score_ = best_score_; }

 private:
  using Clock = std::chrono::steady_clock;
  static bool finite_nonnegative(double value) {
    return value >= 0 && std::isfinite(value);
  }
  bool better(Score first, Score second) const {
    return options_.maximize ? first > second : first < second;
  }
  long double acceptance_threshold() {
    if (options_.acceptance == LnsAcceptance::HillClimbing) {
      return static_cast<long double>(current_score_);
    }
    long double base = static_cast<long double>(best_score_);
    long double margin = (1.0L - progress_) * options_.start_margin +
                         progress_ * options_.end_margin;
    if (options_.acceptance == LnsAcceptance::SimulatedAnnealing) {
      base = static_cast<long double>(current_score_);
      // 0 < u <= 1。log(0)は起きない。近傍生成用とは別の乱数列。
      const double u = static_cast<double>((acceptance_engine_() >> 11) + 1) *
                       (1.0 / 9007199254740992.0);
      const double temperature = std::exp((1.0 - progress_) * log_start_ +
                                          progress_ * log_end_);
      margin = -static_cast<long double>(temperature) * std::log(u);
    }
    // 整数Scoreの悪化量も整数。先にfloorすれば、uint64_t最大値付近でも
    // 「0.5まで許す」が絶対閾値の丸めで「1まで許す」に変わらない。
    if constexpr (std::is_integral<Score>::value) margin = std::floor(margin);
    return options_.maximize ? base - margin : base + margin;
  }
  Problem& problem_;
  LnsOptions options_;
  State current_, candidate_, best_;
  Score current_score_, best_score_;
  std::mt19937_64 move_engine_, acceptance_engine_;
  Clock::time_point started_;
  std::uint64_t iterations_ = 0, accepted_ = 0, improved_ = 0, rejected_repairs_ = 0;
  int until_clock_ = 0;
  bool stopped_ = false;
  LnsOutcome last_outcome_ = LnsOutcome::Rejected;
  double progress_ = 0, log_start_ = 0, log_end_ = 0;
};
