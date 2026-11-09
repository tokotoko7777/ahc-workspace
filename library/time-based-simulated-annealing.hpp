#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/time-based-simulated-annealing.hpp
// 前計算の着想: https://github.com/asi1024/MarathonLibrary/blob/13fe8241ac04fcdcfb8da6e84b050bd4b23ad3a9/snippets/simulated_annealing.h
// この実装は近似値だけで採否を決めず、区間内だけ通常のlogで確認する独自実装。

// タイマーを内蔵した焼きなまし。これ1ファイルだけで使える。
// 使い方:
// TimeBasedSimulatedAnnealing sa(1900.0, 100.0, 1.0, 123, 64);
// while (!sa.is_over()) {
//   double improvement = new_score - current_score;  // 最大化
//   if (sa.accept(improvement)) { ... }
// }
// 最後の 64 は時計を見る間隔。重い近傍なら1、軽い近傍なら64〜256が目安。
struct TimeBasedSimulatedAnnealing {
  double time_limit_ms;
  double start_temperature;
  double end_temperature;
  std::chrono::steady_clock::time_point start;
  std::mt19937_64 engine;
  mutable double log_start_temperature = 0.0;
  mutable double log_temperature_ratio = 0.0;
  int check_interval;
  mutable int calls_until_check = 0;
  mutable bool over = false;
  mutable double cached_elapsed_ms_value = 0.0;
  mutable double cached_progress_value = 0.0;
  mutable double cached_temperature_value;
  mutable double cached_inverse_temperature_value;

 private:
  bool linear_schedule_value = false;
  double cooling_power_value = 1.0;
  mutable double prepared_start_temperature = 0.0;
  mutable double prepared_end_temperature = 0.0;
  // 各分割点のlogを上下へ1 ULP広げた境界。未使用なら確保しない。
  std::vector<std::pair<double, double>> threshold_log_bounds_;

  static void validate_temperatures(double start_value, double end_value) {
    if (!(start_value > 0.0) || !std::isfinite(start_value) ||
        !(end_value > 0.0) || !std::isfinite(end_value)) {
      throw std::invalid_argument("temperatures must be positive and finite");
    }
  }

  double shape_progress(double progress) const {
    if (cooling_power_value == 1.0) return progress;
    return std::pow(progress, cooling_power_value);
  }

  double temperature_from_prepared_settings(double progress) const {
    if (progress <= 0.0) return start_temperature;
    if (progress >= 1.0) return end_temperature;

    const double shaped_progress = shape_progress(progress);
    if (linear_schedule_value) {
      return start_temperature +
             (end_temperature - start_temperature) * shaped_progress;
    }
    return std::exp(
        log_start_temperature + log_temperature_ratio * shaped_progress);
  }

  void refresh_cached_temperature() const {
    cached_temperature_value =
        temperature_from_prepared_settings(cached_progress_value);
    cached_inverse_temperature_value = 1.0 / cached_temperature_value;
  }

  // 以前からpublicだった温度を直接書き換えたコードにも対応する。
  void synchronize_temperature_settings() const {
    if (start_temperature == prepared_start_temperature &&
        end_temperature == prepared_end_temperature) {
      return;
    }
    validate_temperatures(start_temperature, end_temperature);
    prepared_start_temperature = start_temperature;
    prepared_end_temperature = end_temperature;
    log_start_temperature = std::log(start_temperature);
    log_temperature_ratio =
        std::log(end_temperature) - log_start_temperature;
    refresh_cached_temperature();
  }

  double acceptance_exponent(double worsening_value) const {
    // 普通の温度では速い乗算を維持する。極小温度で1/Tがinfに
    // なった時だけ除算し、worsening_valueと温度の比を失わない。
    if (std::isfinite(cached_inverse_temperature_value)) {
      return worsening_value * cached_inverse_temperature_value;
    }
    return worsening_value / cached_temperature_value;
  }

  bool update_clock() const {
    synchronize_temperature_settings();
    calls_until_check = check_interval - 1;
    cached_elapsed_ms_value = elapsed_ms();
    cached_progress_value =
        std::clamp(cached_elapsed_ms_value / time_limit_ms, 0.0, 1.0);
    refresh_cached_temperature();
    over = cached_elapsed_ms_value >= time_limit_ms;
    return over;
  }

  bool accept_worsening(double exponent) {
    // 従来と同じく、悪化手では必ず乱数をちょうど1個消費する。
    const double random_value = random_01();
    // exp(-37) は random_01() の最小の正値 2^-53 より小さい。
    // random_value==0 の時だけunderflowを含めて従来式で確認する。
    if (exponent <= -37.0 && random_value != 0.0) return false;
    return random_value < std::exp(exponent);
  }

 public:
  TimeBasedSimulatedAnnealing(
      double time_limit_ms_value,
      double start_temperature_value,
      double end_temperature_value,
      std::uint64_t seed = 0,
      int check_interval_value = 1)
      : time_limit_ms(time_limit_ms_value),
        start_temperature(start_temperature_value),
        end_temperature(end_temperature_value),
        start(std::chrono::steady_clock::now()),
        engine(seed),
        check_interval(check_interval_value),
        cached_temperature_value(start_temperature_value),
        cached_inverse_temperature_value(0.0) {
    if (!(time_limit_ms > 0.0) || !std::isfinite(time_limit_ms)) {
      throw std::invalid_argument("time_limit_ms must be positive and finite");
    }
    if (check_interval <= 0) {
      throw std::invalid_argument("check_interval must be positive");
    }
    set_temperatures(start_temperature_value, end_temperature_value);
  }

  // 探索途中で温度範囲を変える時はこちらが分かりやすい。
  // 以前どおりpublic fieldを直接変えても、次回利用時に自動同期する。
  void set_temperatures(double new_start, double new_end) {
    validate_temperatures(new_start, new_end);
    start_temperature = new_start;
    end_temperature = new_end;
    prepared_start_temperature = start_temperature;
    prepared_end_temperature = end_temperature;
    log_start_temperature = std::log(start_temperature);
    log_temperature_ratio =
        std::log(end_temperature) - log_start_temperature;
    refresh_cached_temperature();
  }

  // 指数冷却が既定。必要な時だけ線形冷却へ切り替えられる。
  void use_linear_schedule() {
    synchronize_temperature_settings();
    linear_schedule_value = true;
    refresh_cached_temperature();
  }

  void use_geometric_schedule() {
    synchronize_temperature_settings();
    linear_schedule_value = false;
    refresh_cached_temperature();
  }

  // 1より大きいと高温を長く保ち、1未満なら早めに冷える。
  void set_cooling_power(double power) {
    if (!(power > 0.0) || !std::isfinite(power)) {
      throw std::invalid_argument("cooling power must be positive and finite");
    }
    synchronize_temperature_settings();
    cooling_power_value = power;
    refresh_cached_temperature();
  }

  void set_check_interval(int new_check_interval) {
    if (new_check_interval <= 0) {
      throw std::invalid_argument("check_interval must be positive");
    }
    check_interval = new_check_interval;
    calls_until_check = 0;
  }

  void reset() {
    synchronize_temperature_settings();
    start = std::chrono::steady_clock::now();
    calls_until_check = 0;
    over = false;
    cached_elapsed_ms_value = 0.0;
    cached_progress_value = 0.0;
    cached_temperature_value = start_temperature;
    cached_inverse_temperature_value = 1.0 / start_temperature;
  }

  double elapsed_ms() const {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(now - start).count();
  }

  bool is_over() const {
    synchronize_temperature_settings();
    if (over) return true;
    if (calls_until_check > 0) {
      --calls_until_check;
      return false;
    }
    return update_clock();
  }

  // 間引きを無視して、時計・進捗率・温度を今すぐ更新する。
  bool is_over_now() const { return update_clock(); }

  // 現在時刻を読む精密な進捗率。熱いループでは cached_progress() を使う。
  double progress() const {
    return std::clamp(elapsed_ms() / time_limit_ms, 0.0, 1.0);
  }

  double cached_progress() const { return cached_progress_value; }

  double temperature(double progress) const {
    if (std::isnan(progress)) {
      throw std::invalid_argument("progress must not be NaN");
    }
    progress = std::clamp(progress, 0.0, 1.0);
    synchronize_temperature_settings();
    return temperature_from_prepared_settings(progress);
  }

  // 現在時刻を読む精密な温度。熱いループでは cached_temperature() を使う。
  double current_temperature() const { return temperature(progress()); }

  // accept() が実際に使う、最後の is_over() 確認時の温度。
  double cached_temperature() const {
    synchronize_temperature_settings();
    return cached_temperature_value;
  }

  double cached_elapsed_ms() const { return cached_elapsed_ms_value; }

  // accept() と同じ、最後に時計を確認した時の温度での採用確率。
  template <class Score>
  double acceptance_probability(Score improvement) const {
    const double value = static_cast<double>(improvement);
    if (std::isnan(value)) return 0.0;
    if (value >= 0.0) return 1.0;
    synchronize_temperature_settings();
    return std::exp(acceptance_exponent(value));
  }

  // d点悪化する手をprobabilityで採用したい時の温度を返す。
  static double temperature_for_acceptance(
      double typical_worsening,
      double probability) {
    if (!(typical_worsening > 0.0) ||
        !std::isfinite(typical_worsening) ||
        !(probability > 0.0 && probability < 1.0) ||
        !std::isfinite(probability)) {
      throw std::invalid_argument(
          "worsening must be positive and probability must be in (0, 1)");
    }
    const double result = -typical_worsening / std::log(probability);
    if (!(result > 0.0) || !std::isfinite(result)) {
      throw std::invalid_argument("calculated temperature is not finite");
    }
    return result;
  }

  // mt19937_64 の上位53bitから [0, 1) の double を作る。
  double random_01() {
    constexpr double inverse = 1.0 / 9007199254740992.0;  // 2^53
    return static_cast<double>(engine() >> 11) * inverse;
  }

  // TODO: 【任意】軽い差分評価の試行数が多い時だけ、探索前に4096等を指定する。
  // 0で無効（既定）。2のべき乗、最大2^20。約16*(bins+1) bytesを使う。
  // 構築はO(bins)。時刻・乱数列はリセットしない。accept()には影響しない。
  // 固定表を順番に巡回せず、各試行で従来と同じ新しい乱数を1個使う。
  void set_threshold_table_size(std::size_t bins) {
    if (bins == 0) {
      threshold_log_bounds_.clear();
      return;
    }
    if (bins > (std::size_t{1} << 20) || (bins & (bins - 1)) != 0) {
      throw std::invalid_argument("threshold table size must be a power of two up to 2^20");
    }
    std::vector<std::pair<double, double>> bounds(bins + 1);
    const double infinity = std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i <= bins; ++i) {
      const double value = i == 0 ? -infinity :
          std::log(static_cast<double>(i) / static_cast<double>(bins));
      bounds[i] = {std::nextafter(value, -infinity), std::nextafter(value, infinity)};
    }
    threshold_log_bounds_.swap(bounds);
  }

  std::size_t threshold_table_size() const {
    return threshold_log_bounds_.empty() ? 0 : threshold_log_bounds_.size() - 1;
  }

  // TODO: 【任意】対数表の前計算だけをON/OFFする。スコア途中打ち切りとは独立。
  // falseでもrun_with_threshold(true)で途中打ち切りできる。
  // 距離表など「問題固有の前計算」を行う設定ではない。
  void set_threshold_precomputation(bool enabled, std::size_t bins = 4096) {
    if (enabled && bins == 0) {
      throw std::invalid_argument("enabled precomputation needs a nonzero table size");
    }
    set_threshold_table_size(enabled ? bins : 0);
  }

  bool threshold_precomputation_enabled() const {
    return !threshold_log_bounds_.empty();
  }

  // 受理閾値が入る区間。lowerは安全な枝刈り用で、最終採否そのものではない。
  // TODO: evaluate_moveへlowerを渡し、最後はaccept()で確認する。
  // 値が区間内なら元のT*log(u)を計算するので、希少な悪化手も切り捨てない。
  struct AcceptanceWindow {
    double lower, upper, uniform_value, temperature;

    template <class Score>
    bool accept(Score improvement) const {
      const double value = static_cast<double>(improvement);
      if (std::isnan(value) || value <= lower) return false;
      if (value > upper) return true;
      return value > temperature * std::log(uniform_value);
    }
  };

  AcceptanceWindow draw_acceptance_window() {
    synchronize_temperature_settings();
    const double u = random_01();
    const double t = cached_temperature_value;
    if (threshold_log_bounds_.empty()) {
      const double threshold = t * std::log(u);
      return {threshold, threshold, u, t};
    }
    const std::size_t bins = threshold_table_size();
    // binsが2のべき乗なので、[0,1)の53-bit乱数の区間を正確に選べる。
    const auto index = static_cast<std::size_t>(u * static_cast<double>(bins));
    return {t * threshold_log_bounds_[index].first,
            t * threshold_log_bounds_[index + 1].second, u, t};
  }

  // この試行が採用されるために必要な最小improvementを先に返す。
  // improvement > thresholdなら採用。差分評価が重い時は、この閾値を
  // 評価関数へ渡し、超えないと証明できた時点で計算を打ち切れる。
  // 良化手を含む全試行で乱数を1個消費するためaccept()と乱数列は異なるが、
  // 各手の採用確率は同じ。
  double draw_acceptance_threshold() {
    synchronize_temperature_settings();
    return cached_temperature_value * std::log(random_01());
  }

  template <class Score>
  bool accept_with_threshold(Score improvement, double threshold) const {
    const double value = static_cast<double>(improvement);
    if (std::isnan(value) || std::isnan(threshold)) return false;
    return value > threshold;
  }

  // improvement は「変更後がどれだけ良くなるか」。
  // 最大化: new_score - current_score
  // 最小化: current_cost - new_cost
  template <class Score>
  bool accept(Score improvement) {
    const double value = static_cast<double>(improvement);
    if (std::isnan(value)) return false;
    if (value >= 0.0) return true;

    synchronize_temperature_settings();
    return accept_worsening(acceptance_exponent(value));
  }
};

// 問題依存部分をProblemへ集める、時間ベース焼きなましの薄いRunner。
//
// 【使う人がmain.cpp側へ書く場所】
// 次のTODOだけを自分の問題に合わせる。Runner本体は通常変更しない。
// 空関数を配置済みの雛形: template/search/local-search/basic.cpp
//
//   TODO: 【問題ごと】Stateへ現在解1個と差分更新用cacheを書く。
//   using State = ...;
//   TODO: 【問題ごと】Moveへ近傍1回分の小さい情報を書く。
//   using Move = ...;
//   TODO: 【問題ごと】Scoreを選ぶ。Runnerでは大きいほど良い値にする。
//   using Score = ...;
//
//   TODO: 【問題ごと】次に試す近傍を1個作る。
//   optional<Move> propose_move(const State&, mt19937_64&, double progress)
//     -> 近傍を1個作る。作れない試行はnullopt。
//   TODO: 【問題ごと】評価関数はこの1個だけ。近傍による改善量を差分計算する。
//   optional<Score> evaluate_move(
//       const State&, const Move&, double acceptance_threshold)
//     -> 正なら良化、負なら悪化となる正確な改善量を返す。
//     -> 【任意】最終改善量の上限 <= 閾値と証明できたらnulloptで途中終了。
//     -> 打ち切りが難しい問題では閾値を無視して最後まで計算してよい。
//     -> 打ち切りOFF時の閾値は-inf。不合法手は設定に関係なくnulloptでよい。
//   TODO: 【問題ごと】採用された近傍だけを現在解へ反映する。
//   void apply_move(State&, Move&)
//     -> 採用済みの手だけをStateへ反映する。Move内のvector等はmoveしてよい。
//
// Runnerは時計、温度、採否、現在解、最良解、件数統計を担当する。
// evaluate_moveはStateを書き換えない。この形なら不採用時のrevertは不要。
// ↓↓↓ ここから下はライブラリ本体。通常は編集しない。↓↓↓
template <class Problem>
struct TimeBasedAnnealingRunner {
  using State = typename Problem::State;
  using Move = typename Problem::Move;
  using Score = typename Problem::Score;

  TimeBasedAnnealingRunner(Problem& problem,
                           State initial_state,
                           Score initial_score,
                           double time_limit_ms,
                           double start_temperature,
                           double end_temperature,
                           std::uint64_t seed = 0,
                           int check_interval = 1)
      : problem_(problem),
        current_state_(std::move(initial_state)),
        best_state_(current_state_),
        current_score_(std::move(initial_score)),
        best_score_(current_score_),
        annealing_(time_limit_ms,
                   start_temperature,
                   end_temperature,
                   seed,
                   check_interval),
        move_engine_(seed ^ 0xd1b54a32d192ed03ULL) {}

  // 制限時間内なら1試行進める。時間終了後はfalse。
  // 閾値は-infで全評価。従来のaccept()の採否・乱数消費を維持する。
  bool step() {
    if (annealing_.is_over()) return false;
    ++iterations_;
    std::optional<Move> move = problem_.propose_move(
        static_cast<const State&>(current_state_),
        move_engine_,
        annealing_.cached_progress());
    if (!move.has_value()) return true;

    ++valid_moves_;
    const std::optional<Score> improvement = problem_.evaluate_move(
        static_cast<const State&>(current_state_), *move,
        -std::numeric_limits<double>::infinity());
    if (!improvement || !annealing_.accept(*improvement)) return true;

    problem_.apply_move(current_state_, *move);
    current_score_ += *improvement;
    ++accepted_moves_;
    if (best_score_ < current_score_) {
      best_score_ = current_score_;
      best_state_ = current_state_;
      ++best_updates_;
    }
    return true;
  }

  // 差分評価が重く、途中で採用不能と証明できる問題向け。
  // Problem::evaluate_moveは、improvementがthresholdを
  // 超えないと証明できた時だけnulloptを返す。分からない場合は最後まで
  // 計算し、正確なimprovementを返せば通常の焼きなましと同じ分布になる。
  // 区間表を有効にした場合は実際の閾値以下の下限を渡し、最終採否を別途確認する。
  bool step_with_threshold() {
    return step_threshold_impl<true>();
  }

  // 呼ぶ評価関数は常にevaluate_moveの1個だけ。
  // true: 安全な受理閾値を渡す。false: -infを渡して全評価。
  // 閾値の抽選・最終採否は同じ。評価側は閾値を無視してもよい。
  bool step_with_threshold(bool enable_score_early_stop) {
    return enable_score_early_stop ? step_threshold_impl<true>()
                                   : step_threshold_impl<false>();
  }

  // 山登り。Problemは焼きなましと同じで、改善量>0の手だけ採用する。
  // 同点・悪化・非有限値は棄却。evaluate_moveへ渡す閾値は0。
  // 温度による採否や受理乱数は使わない。時計・近傍・最良解管理は共通。
  bool step_hill_climbing() {
    if (annealing_.is_over()) return false;
    ++iterations_;
    std::optional<Move> move = problem_.propose_move(
        static_cast<const State&>(current_state_), move_engine_,
        annealing_.cached_progress());
    if (!move) return true;
    ++valid_moves_;
    const std::optional<Score> improvement = problem_.evaluate_move(
        static_cast<const State&>(current_state_), *move, 0.0);
    if (!improvement) { ++threshold_pruned_moves_; return true; }
    if (!std::isfinite(static_cast<long double>(*improvement)) ||
        !(*improvement > Score{0})) return true;
    problem_.apply_move(current_state_, *move);
    current_score_ += *improvement;
    ++accepted_moves_;
    if (best_score_ < current_score_) {
      best_score_ = current_score_;
      best_state_ = current_state_;
      ++best_updates_;
    }
    return true;
  }

 private:
  template <bool EnableScoreEarlyStop>
  bool step_threshold_impl() {
    if (annealing_.is_over()) return false;
    ++iterations_;
    std::optional<Move> move = problem_.propose_move(
        static_cast<const State&>(current_state_),
        move_engine_,
        annealing_.cached_progress());
    if (!move.has_value()) return true;

    ++valid_moves_;
    const auto threshold = annealing_.draw_acceptance_window();
    const double evaluation_threshold = EnableScoreEarlyStop
        ? threshold.lower : -std::numeric_limits<double>::infinity();
    const std::optional<Score> improvement = problem_.evaluate_move(
        static_cast<const State&>(current_state_), *move, evaluation_threshold);
    if (!improvement.has_value()) {
      ++threshold_pruned_moves_;
      return true;
    }
    if (!threshold.accept(*improvement)) return true;

    problem_.apply_move(current_state_, *move);
    current_score_ += *improvement;
    ++accepted_moves_;
    if (best_score_ < current_score_) {
      best_score_ = current_score_;
      best_state_ = current_state_;
      ++best_updates_;
    }
    return true;
  }

 public:
  std::uint64_t run() {
    while (step()) {
    }
    return iterations_;
  }

  std::uint64_t run_hill_climbing() {
    while (step_hill_climbing()) {}
    return iterations_;
  }

  std::uint64_t run_with_threshold() {
    while (step_with_threshold()) {
    }
    return iterations_;
  }

  // TODO: スコア途中打ち切りだけをON/OFFする。前計算とは別の設定。
  // falseでも前計算した区間表を最終採否に使える。
  // 分岐は探索開始時の1回だけ。通常のrun()とは異なり、ON/OFFで同じ乱数を消費する。
  std::uint64_t run_with_threshold(bool enable_score_early_stop) {
    if (enable_score_early_stop) return run_with_threshold();
    while (step_threshold_impl<false>()) {
    }
    return iterations_;
  }

  // 現在解が深く悪化した時、保存済みの最良解から探索を再開する。
  // 温度、時計、乱数列、試行件数はそのまま継続する。
  void restart_from_best() {
    current_state_ = best_state_;
    current_score_ = best_score_;
    ++restarts_;
  }

  const State& current_state() const { return current_state_; }
  const State& best_state() const { return best_state_; }
  const Score& current_score() const { return current_score_; }
  const Score& best_score() const { return best_score_; }
  std::uint64_t iterations() const { return iterations_; }
  std::uint64_t valid_moves() const { return valid_moves_; }
  std::uint64_t accepted_moves() const { return accepted_moves_; }
  std::uint64_t best_updates() const { return best_updates_; }
  std::uint64_t restarts() const { return restarts_; }
  // 閾値方式でevaluate_moveがnulloptを返した件数。不合法手の棄却も含む。
  std::uint64_t threshold_pruned_moves() const {
    return threshold_pruned_moves_;
  }

  TimeBasedSimulatedAnnealing& annealing() { return annealing_; }
  const TimeBasedSimulatedAnnealing& annealing() const { return annealing_; }

 private:
  Problem& problem_;
  State current_state_;
  State best_state_;
  Score current_score_;
  Score best_score_;
  TimeBasedSimulatedAnnealing annealing_;
  std::mt19937_64 move_engine_;
  std::uint64_t iterations_ = 0;
  std::uint64_t valid_moves_ = 0;
  std::uint64_t accepted_moves_ = 0;
  std::uint64_t best_updates_ = 0;
  std::uint64_t restarts_ = 0;
  std::uint64_t threshold_pruned_moves_ = 0;
};
