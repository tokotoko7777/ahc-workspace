#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/forward-backward-dp.hpp
// 着想: https://blog.terry-u16.net/entry/ahc009 (注6)。提出コードの移植ではない。

// 有限状態の確率遷移と加算報酬に対する、固定長操作列の1操作変更評価。
// 前の確率分布×変更した遷移×後ろの期待価値で、後半を毎回再生しない。
// 状態番号は0..state_count-1。Actionは自由な値型、Realはdouble/long double等。
//
// TODO: transitions(turn, state_id, action, emit)を書く。
//   各遷移について emit(next_state, probability, immediate_reward) を呼ぶ。
//   rewardはその遷移が起きた場合の報酬（確率を掛ける前）。
//   next_state=-1は終了。この場合も即時報酬を加えるが、後続価値は加えない。
//   確率は非負・有限、各状態で合計1。外部データと遷移はbuild後に変更しない。
// TODO: 初期確率分布と、全操作終了時の状態別報酬を渡す。終端報酬省略なら全て0。
//
// ForwardBackwardDP<MyAction> cache(number_of_states);
// cache.build(initial_distribution, actions, transitions);
// double candidate = cache.score_if_changed(position, new_action, transitions);
// double improvement = candidate - cache.score(); // 最大化。最小化は逆符号。
// if (accept(improvement)) cache.commit_change(position, new_action, transitions);
//
// score_if_changedはcacheを変更しないので、不採用時のundoは不要。
// commit_changeは前向き表のsuffixと後ろ向き表のprefixだけ更新する。
// N=状態数、L=操作数、E=1時刻の全状態からの遷移数として、
// build/commit O(L*(N+E))、仮評価O(N+E)、メモリO(L*N)。
// 仮評価が軽くても採用処理はO(L*(N+E))。挿入/削除はbuildし直すこと。
// 浮動小数点の丸め順は全再計算と異なる。整数丸め前の値を比較する。
// 例: template/search/local-search/forward-backward.cpp
template <class Action, class Real = double>
class ForwardBackwardDP {
  static_assert(std::is_floating_point_v<Real>, "Real must be floating point");
 public:
  explicit ForwardBackwardDP(int state_count) : states_(state_count) {
    if (states_ <= 0) throw std::invalid_argument("state_count must be positive");
  }

  template <class Transitions>
  void build(const std::vector<Real>& initial, std::vector<Action> actions,
             Transitions&& transitions, const std::vector<Real>& terminal = {}) {
    if (initial.size() != static_cast<std::size_t>(states_) ||
        (!terminal.empty() && terminal.size() != initial.size())) {
      throw std::invalid_argument("distribution/terminal size mismatch");
    }
    ready_ = false;
    actions_ = std::move(actions);
    forward_.assign(actions_.size() + 1, std::vector<Real>(states_, Real{0}));
    backward_.assign(actions_.size() + 1, std::vector<Real>(states_, Real{0}));
    prefix_score_.assign(actions_.size() + 1, Real{0});
    forward_[0] = initial;
    if (!terminal.empty()) backward_.back() = terminal;
    for (std::size_t t = 0; t < actions_.size(); ++t) advance_forward(t, transitions);
    for (std::size_t t = actions_.size(); t > 0; --t) advance_backward(t - 1, transitions);
    refresh_score();
    ready_ = true;
  }

  template <class Transitions>
  Real score_if_changed(std::size_t position, const Action& action, Transitions&& transitions) const {
    check_position(position);
    Real result = prefix_score_[position];
    for (int from = 0; from < states_; ++from) {
      const Real probability = forward_[position][from];
      if (probability == Real{0}) continue;
      Real value = 0;
      transitions(position, from, action, [&](int to, Real p, Real reward) {
        check_edge(to, p, reward);
        if (p != Real{0}) value += p * (reward + (to < 0 ? Real{0} : backward_[position + 1][to]));
      });
      result += probability * value;
    }
    return result;
  }

  template <class Transitions>
  void commit_change(std::size_t position, Action action, Transitions&& transitions) {
    check_position(position);
    ready_ = false; // callback例外時は再buildが必要。壊れたcacheを使わせない。
    actions_[position] = std::move(action);
    for (std::size_t t = position; t < actions_.size(); ++t) advance_forward(t, transitions);
    for (std::size_t t = position + 1; t > 0; --t) advance_backward(t - 1, transitions);
    refresh_score();
    ready_ = true;
  }

  Real score() const { check_ready(); return score_; }
  const std::vector<Action>& actions() const { check_ready(); return actions_; }

 private:
  int states_;
  bool ready_ = false;
  Real score_ = 0;
  std::vector<Action> actions_;
  std::vector<std::vector<Real>> forward_, backward_;
  std::vector<Real> prefix_score_;

  void check_ready() const {
    if (!ready_) throw std::logic_error("build a valid cache first");
  }
  void check_position(std::size_t position) const {
    check_ready();
    if (position >= actions_.size()) throw std::out_of_range("action position");
  }
  void check_edge(int to, Real p, Real reward) const {
    assert(to >= -1 && to < states_);
    assert(std::isfinite(p) && p >= Real{0});
    assert(std::isfinite(reward));
    (void)to; (void)p; (void)reward;
  }
  template <class Transitions>
  void advance_forward(std::size_t turn, Transitions& transitions) {
    auto& next = forward_[turn + 1];
    std::fill(next.begin(), next.end(), Real{0});
    Real added = 0;
    for (int from = 0; from < states_; ++from) {
      const Real probability = forward_[turn][from];
      if (probability == Real{0}) continue;
      transitions(turn, from, actions_[turn], [&](int to, Real p, Real reward) {
        check_edge(to, p, reward);
        const Real mass = probability * p;
        if (mass != Real{0}) {
          added += mass * reward;
          if (to >= 0) next[to] += mass;
        }
      });
    }
    prefix_score_[turn + 1] = prefix_score_[turn] + added;
  }
  template <class Transitions>
  void advance_backward(std::size_t turn, Transitions& transitions) {
    for (int from = 0; from < states_; ++from) {
      Real value = 0;
      transitions(turn, from, actions_[turn], [&](int to, Real p, Real reward) {
        check_edge(to, p, reward);
        if (p != Real{0}) value += p * (reward + (to < 0 ? Real{0} : backward_[turn + 1][to]));
      });
      backward_[turn][from] = value;
    }
  }
  void refresh_score() {
    score_ = prefix_score_.back();
    for (int state = 0; state < states_; ++state) score_ += forward_.back()[state] * backward_.back()[state];
  }
};
