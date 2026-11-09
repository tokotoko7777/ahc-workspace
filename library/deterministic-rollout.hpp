#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/deterministic-rollout.hpp

// 候補をそれぞれ終端まで仮実行し、評価が最良の1手を返す薄いRunner。
// 未来が現在状態とActionから全て決まる問題にはこちらを使う。
//
// 【使う人がmain.cpp側へ書く場所】
// 空関数を配置済みの雛形: template/advanced/deterministic-rollout.cpp
//
//   struct Problem {
//     using State = ...;   // TODO: 現在の実状態。
//     using Action = ...;  // TODO: 今比較する1手。
//     using Score = ...;   // TODO: 仮実行の評価値。int以外でもよい。
//
//     // TODO: 今比較したい合法Actionを全て返す。
//     // vectorでなくarrayやFixedVectorを返してもよい。
//     auto generate_actions(const State&) const;
//
//     // TODO: Actionを最初に選んだ場合を、終端または指定深さまで
//     // 仮実行して評価値を返す。元のStateは変更しない。
//     Score evaluate_action(const State&, const Action&) const;
//   };
//
// Runnerは候補列と評価値の一時メモリを再利用する。
// State更新、Action履歴保存、出力は問題依存なので呼び出し側が行う。
// 同点ならgenerate_actionsで先に返されたActionを選ぶ。
// ↓↓↓ ここから下はライブラリ本体。通常は編集しない。↓↓↓
template <class Problem>
struct DeterministicRolloutRunner {
  using State = typename Problem::State;
  using Action = typename Problem::Action;
  using Score = typename Problem::Score;

  explicit DeterministicRolloutRunner(Problem& problem,
                                      bool maximize = true)
      : problem_(problem), maximize_(maximize) {}

  Action choose_action(const State& state) {
    actions_.clear();
    auto&& generated_actions = problem_.generate_actions(state);
    for (const auto& action : generated_actions) actions_.push_back(action);
    if (actions_.empty()) {
      throw std::runtime_error("generate_actions returned no action");
    }

    scores_.clear();
    for (const Action& action : actions_) {
      scores_.push_back(problem_.evaluate_action(state, action));
    }

    best_index_ = 0;
    for (std::size_t index = 1; index < scores_.size(); ++index) {
      const bool better = maximize_ ? scores_[best_index_] < scores_[index]
                                    : scores_[index] < scores_[best_index_];
      if (better) best_index_ = index;
    }
    return actions_[best_index_];
  }

  void reserve(int action_count) {
    if (action_count < 0) {
      throw std::invalid_argument("action_count must be non-negative");
    }
    actions_.reserve(static_cast<std::size_t>(action_count));
    scores_.reserve(static_cast<std::size_t>(action_count));
  }

  const std::vector<Action>& last_actions() const { return actions_; }
  const std::vector<Score>& last_scores() const { return scores_; }
  std::size_t last_best_index() const { return best_index_; }

 private:
  Problem& problem_;
  bool maximize_;
  std::vector<Action> actions_;
  std::vector<Score> scores_;
  std::size_t best_index_ = 0;
};
