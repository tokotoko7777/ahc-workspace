#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <iterator>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/chokudai-search.hpp
// 着想: https://chokudai.hatenablog.com/entry/2017/04/12/055515
// 考え方から独自実装。深さ別の候補を残し、浅い層から繰り返し展開する。

struct ChokudaiOptions {
  int max_depth = 100;
  std::size_t capacity_per_depth = 256; // メモリ上限。超過時は最悪候補を捨てる近似。
  double time_limit_ms = 1900;
  bool maximize = true;
  std::uint64_t expansion_limit = std::numeric_limits<std::uint64_t>::max();
};

// ActionBeamと同じgenerate_actions/evaluate_action/apply_actionを使える。
// 追加: is_terminal(state)、final_score(state)。順位値と最終得点を分ける。
// Stateはコピー可能な値型。答えの復元情報もStateに保持する。
// 親を1個ずつ取り出し、Actionを先に評価。層の上限に入る子だけStateをコピー。
// 同点は先着優先。重複除去はしない。容量制限付きなので完全探索の保証はない。
template <class Problem> class ChokudaiSearch {
 public:
  using State = typename Problem::State;
  using Score = typename Problem::Score;
  ChokudaiSearch(Problem& problem, State initial, Score rank, ChokudaiOptions options = {})
      : problem_(problem), options_(options) {
    if (options.max_depth < 0 || !options.capacity_per_depth ||
        !std::isfinite(options.time_limit_ms) || options.time_limit_ms <= 0 || !finite(rank))
      throw std::invalid_argument("invalid chokudai options/rank");
    layers_.reserve(static_cast<std::size_t>(options.max_depth) + 1);
    for (int d = 0; d <= options.max_depth; ++d) layers_.emplace_back(Order{options.maximize});
    layers_[0].insert(Entry{rank, next_id_++, std::move(initial)});
    started_ = Clock::now();
  }
  // false: 時間/回数上限または全候補を消費。途中解を完成解として返さない。
  bool step() {
    if (stopped_ || expansions_ >= options_.expansion_limit || expired()) return false;
    for (std::size_t visited = 0; visited < layers_.size(); ++visited) {
      const std::size_t depth = cursor_;
      cursor_ = (cursor_ + 1) % layers_.size();
      if (layers_[depth].empty()) continue;
      auto entry = layers_[depth].extract(layers_[depth].begin());
      State& parent = entry.value().state;
      ++expansions_;
      if (problem_.is_terminal(parent)) { consider(parent); return true; }
      if (depth + 1 == layers_.size()) return true;
      auto& next = layers_[depth + 1];
      for (auto action : problem_.generate_actions(parent)) {
        // 候補数が多い親でも時間を大きく超えないよう64評価ごとに確認。
        if ((evaluations_ & 63U) == 0 && expired()) { stopped_ = true; break; }
        ++evaluations_;
        const Score rank = problem_.evaluate_action(parent, action);
        if (!finite(rank)) continue;
        if (next.size() == options_.capacity_per_depth &&
            !better(rank, next.rbegin()->rank)) continue;
        State child = parent;
        problem_.apply_action(child, action);
        if (problem_.is_terminal(child)) { consider(child); continue; }
        next.insert(Entry{rank, next_id_++, std::move(child)});
        if (next.size() > options_.capacity_per_depth) next.erase(std::prev(next.end()));
      }
      return true;
    }
    stopped_ = true;
    return false;
  }
  void run() { while (step()) {} }
  const std::optional<State>& best_state() const { return best_; }
  const std::optional<Score>& best_score() const { return best_score_; }
  std::uint64_t expansions() const { return expansions_; }
  std::uint64_t evaluations() const { return evaluations_; }
  double elapsed_ms() const {
    return std::chrono::duration<double, std::milli>(Clock::now() - started_).count();
  }
 private:
  using Clock = std::chrono::steady_clock;
  struct Entry { Score rank; std::uint64_t id; State state; };
  struct Order {
    bool maximize;
    bool operator()(const Entry& a, const Entry& b) const {
      if (a.rank != b.rank) return maximize ? a.rank > b.rank : a.rank < b.rank;
      return a.id < b.id;
    }
  };
  static bool finite(Score x) { return std::isfinite(static_cast<long double>(x)); }
  bool better(Score a, Score b) const { return options_.maximize ? a > b : a < b; }
  bool expired() const { return elapsed_ms() >= options_.time_limit_ms; }
  void consider(const State& state) {
    const Score score = problem_.final_score(state);
    if (finite(score) && (!best_score_ || better(score, *best_score_))) {
      best_ = state; best_score_ = score;
    }
  }
  Problem& problem_;
  ChokudaiOptions options_;
  std::vector<std::multiset<Entry, Order>> layers_;
  std::optional<State> best_;
  std::optional<Score> best_score_;
  Clock::time_point started_;
  std::size_t cursor_ = 0;
  std::uint64_t next_id_ = 0, expansions_ = 0, evaluations_ = 0;
  bool stopped_ = false;
};
