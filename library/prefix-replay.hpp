#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/prefix-replay.hpp

// 行動列の変更前を使い回し、変わった位置以降だけ再実行する。
// 焼きなまし、順序最適化、スケジュールの仮評価向け。
//
// TODO: Stateへ途中のシミュレーション状態を、Actionへ1行動を書く。
// TODO: step(State&, const Action&)に1行動の状態更新を書く。
//   状態を最後まで進めるための情報は全てStateへ置く。
//   外部入力は固定にし、stepの副作用や隠れた乱数状態に依存しないこと。
// TODO: Actionのoperator==を書く。同じ遷移を表す行動だけを等しいとする。
//   intなどなら追加実装は不要。
//
// PrefixReplay<MyState, MyAction> replay(initial, 16);
// replay.evaluate(initial_actions, step);
// replay.commit();
// const MyState& end = replay.evaluate(candidate_actions, step);
// if (better(end)) replay.commit(); // 採用時だけ確定。破棄はdiscard()または次のevaluate。
//
// evaluateは現在列・現在cacheを書き換えない。commitまで現在解を復元可能。
// 異なる長さ、末尾への追加、削除、空列も扱う。最初の相違点は自動検出。
// checkpoint_intervalを大きくするとcacheコピーが減り、再実行する手数が少し増える。
// Action列の比較・コピーはO(列長)。重いstepの再実行とStateコピーを減らす部品で、
// evaluate全体を必ずO(変更数)にするものではない。
// stepを呼ぶ範囲は、最初の相違点以前の直近checkpointから新しい列の末尾まで。
// 戻り値の参照は次のevaluate/commit/discardまで有効と考える。
// 空関数入りの使用例: template/search/local-search/prefix-replay.cpp
// ↓↓↓ ライブラリ本体。通常は編集しない。↓↓↓
template <class State, class Action>
class PrefixReplay {
 public:
  explicit PrefixReplay(State initial, int checkpoint_interval = 16)
      : current_end_(initial), trial_end_(initial) {
    if (checkpoint_interval <= 0) {
      throw std::invalid_argument("checkpoint_interval must be positive");
    }
    interval_ = static_cast<std::size_t>(checkpoint_interval);
    checkpoints_.push_back(std::move(initial));
  }

  void reserve(std::size_t action_count) {
    actions_.reserve(action_count);
    trial_actions_.reserve(action_count);
    const std::size_t count = action_count / interval_ + 1;
    checkpoints_.reserve(count);
    trial_checkpoints_.reserve(count);
  }

  template <class Step>
  const State& evaluate(const std::vector<Action>& candidate, Step&& step) {
    trial_ready_ = false;
    std::size_t common = 0;
    while (common < std::min(actions_.size(), candidate.size()) &&
           actions_[common] == candidate[common]) ++common;
    first_checkpoint_ = common / interval_;
    const std::size_t begin = first_checkpoint_ * interval_;
    trial_actions_ = candidate;
    trial_checkpoints_.clear();
    trial_end_ = checkpoints_[first_checkpoint_];
    last_replayed_actions_ = 0;
    for (std::size_t index = begin; index < candidate.size(); ++index) {
      step(trial_end_, candidate[index]);
      ++last_replayed_actions_;
      if ((index + 1) % interval_ == 0) trial_checkpoints_.push_back(trial_end_);
    }
    // stepが例外を投げたときはcommit不可。現在解は変更されていない。
    trial_ready_ = true;
    return trial_end_;
  }

  void commit() {
    if (!trial_ready_) throw std::logic_error("no completed trial to commit");
    // unchanged prefixはそのまま残す。大きいStateは新しいsuffixだけコピー。
    checkpoints_.erase(checkpoints_.begin() +
                           static_cast<std::ptrdiff_t>(first_checkpoint_ + 1),
                       checkpoints_.end());
    checkpoints_.insert(checkpoints_.end(), trial_checkpoints_.begin(),
                        trial_checkpoints_.end());
    actions_.swap(trial_actions_);
    current_end_ = trial_end_;
    trial_ready_ = false;
  }

  void discard() { trial_ready_ = false; }
  const std::vector<Action>& actions() const { return actions_; }
  const State& current_end() const { return current_end_; }
  std::size_t last_replayed_actions() const { return last_replayed_actions_; }
  std::size_t checkpoint_count() const { return checkpoints_.size(); }

 private:
  std::size_t interval_ = 1;
  std::vector<Action> actions_, trial_actions_;
  std::vector<State> checkpoints_, trial_checkpoints_;
  State current_end_, trial_end_;
  std::size_t first_checkpoint_ = 0, last_replayed_actions_ = 0;
  bool trial_ready_ = false;
};
