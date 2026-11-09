#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <queue>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/cost-tree-beam-search.hpp

// 1手で複数世代進める場合に使う、状態をコピーしないビームサーチ。
// 初めて使う時は、末尾のCostTreeBeamRunner<Problem>を使う。
// 同じgenerationに到着する候補ごとに上位beam_width個を残し、
// 到着予定の一番早いgenerationから順番に展開する。
//
// 空関数を配置済みの雛形: template/search/beam/variable-cost-tree.cpp
// ProblemへState、Move、Scoreと次の6関数を書く:
// generate_moves / apply_move / revert_move / evaluate / get_advance / make_key
// CostTreeBeamRunner<Problem> beam(
//     problem, initial_state, problem.evaluate(initial_state),
//     100, 50);  // 幅100、generation 50まで
// beam.run_with_key();  // 重複除去しない場合はrun()。
// vector<Move> answer = beam.restore();
//
// applyとrevertは必ず逆の操作にし、advanceは正の整数にする。
// const候補コンテナはコピーし、非const候補コンテナの要素はmoveして消費する。
// 不正な幅、最大世代、advanceにはinvalid_argumentを投げる。
// Keyは標準ではuint64_t。別の型はRunnerの第2テンプレート引数に指定する。
// 幅から落ちる候補も調べる場合はstep_and_observe、key付きなら
// step_with_key_and_observeを使う。observerは上限内の全候補について、
// MoveをapplyしたState上で選抜前にちょうど1回呼ばれる。
// この下のCostTreeBeamSearchは、関数を個別に渡したい上級者向け探索コア。
template <class State,
          class Action,
          class Score,
          class Key = std::uint64_t,
          class KeyHash = std::hash<Key>>
struct CostTreeBeamSearch {
  CostTreeBeamSearch(State initial_state,
                     Score initial_score,
                     int beam_width,
                     int max_generation,
                     bool maximize = true)
      : state_(std::move(initial_state)),
        beam_width_(beam_width),
        max_generation_(max_generation),
        maximize_(maximize) {
    if (beam_width_ <= 0) {
      throw std::invalid_argument("beam_width must be positive");
    }
    if (max_generation_ < 0) {
      throw std::invalid_argument("max_generation must be non-negative");
    }

    layers_.resize(static_cast<std::size_t>(max_generation_) + 1);
    first_candidate_.assign(
        static_cast<std::size_t>(max_generation_) + 1, -1);

    nodes_.push_back(Node{});
    nodes_[0].generation = 0;
    nodes_[0].alive = true;
    nodes_[0].scheduled = true;
    mark_.push_back(0);
    marked_first_child_.push_back(-1);
    marked_next_sibling_.push_back(-1);
    current_beam_rank_.push_back(0);

    beam_.push_back(
        Entry{0,
              std::move(initial_score),
              LogicalOrder{0, 0, 0},
              std::nullopt});
  }

  // 現在の generation を展開し、候補がある最小の generation へ進む。
  // 進めたら true。先がなければ現在のビームを残したまま false。
  template <class Expand,
            class Apply,
            class Revert,
            class Evaluate,
            class GetAdvance>
  bool step(Expand&& expand,
            Apply&& apply,
            Revert&& revert,
            Evaluate&& evaluate,
            GetAdvance&& get_advance) {
    NoKeyMaker make_key;
    NoObserver observer;
    return step_impl<false, false>(
        expand, apply, revert, evaluate, get_advance, make_key, observer);
  }

  // observer(parent_rank, action, child_state, rank_score, next_generation)
  // を、max_generation内に入る全候補について選抜前に呼ぶ。
  // observer内ではこのオブジェクトを変更せず、例外も投げないこと。
  template <class Expand,
            class Apply,
            class Revert,
            class Evaluate,
            class GetAdvance,
            class Observer>
  bool step_and_observe(Expand&& expand,
                        Apply&& apply,
                        Revert&& revert,
                        Evaluate&& evaluate,
                        GetAdvance&& get_advance,
                        Observer&& observer) {
    NoKeyMaker make_key;
    return step_impl<false, true>(
        expand, apply, revert, evaluate, get_advance, make_key, observer);
  }

  // 同じ generation かつ同じ key の候補は、最良の1個だけを残す。
  // 1個のオブジェクトでは step と step_with_key を混ぜないこと。
  template <class Expand,
            class Apply,
            class Revert,
            class Evaluate,
            class GetAdvance,
            class MakeKey>
  bool step_with_key(Expand&& expand,
                     Apply&& apply,
                     Revert&& revert,
                     Evaluate&& evaluate,
                     GetAdvance&& get_advance,
                     MakeKey&& make_key) {
    NoObserver observer;
    return step_impl<true, false>(
        expand, apply, revert, evaluate, get_advance, make_key, observer);
  }

  // 同じgeneration・keyの候補をまとめる前に、全候補をobserverで調べる。
  template <class Expand,
            class Apply,
            class Revert,
            class Evaluate,
            class GetAdvance,
            class MakeKey,
            class Observer>
  bool step_with_key_and_observe(Expand&& expand,
                                 Apply&& apply,
                                 Revert&& revert,
                                 Evaluate&& evaluate,
                                 GetAdvance&& get_advance,
                                 MakeKey&& make_key,
                                 Observer&& observer) {
    return step_impl<true, true>(
        expand, apply, revert, evaluate, get_advance, make_key, observer);
  }

  template <class Expand,
            class Apply,
            class Revert,
            class Evaluate,
            class GetAdvance>
  int run(Expand&& expand,
          Apply&& apply,
          Revert&& revert,
          Evaluate&& evaluate,
          GetAdvance&& get_advance) {
    while (step(expand, apply, revert, evaluate, get_advance)) {
    }
    return generation_;
  }

  template <class Expand,
            class Apply,
            class Revert,
            class Evaluate,
            class GetAdvance,
            class MakeKey>
  int run_with_key(Expand&& expand,
                   Apply&& apply,
                   Revert&& revert,
                   Evaluate&& evaluate,
                   GetAdvance&& get_advance,
                   MakeKey&& make_key) {
    while (step_with_key(expand, apply, revert, evaluate, get_advance,
                         make_key)) {
    }
    return generation_;
  }

  int generation() const { return generation_; }

  int size() const { return static_cast<int>(beam_.size()); }

  int beam_width() const { return beam_width_; }

  int max_generation() const { return max_generation_; }

  // 幅を小さくした場合、現在層と予約済みの全未来層を直ちに縮める。
  // 後から幅を広げても、既に捨てた候補は復元されない。
  void set_beam_width(int new_beam_width) {
    if (new_beam_width <= 0) {
      throw std::invalid_argument("beam_width must be positive");
    }
    if (new_beam_width >= beam_width_) {
      beam_width_ = new_beam_width;
      return;
    }

    beam_width_ = new_beam_width;
    trim_entries(beam_);
    for (std::vector<Entry>& entries : layers_) trim_entries(entries);
  }

  // 長い探索で再確保を避けたい場合だけ使う。使わなくても正しく動く。
  // 削除nodeは再利用するので、同時に残る現在層・未来層・祖先が目安。
  void reserve_nodes(std::size_t count) {
    nodes_.reserve(count);
    mark_.reserve(count);
    marked_first_child_.reserve(count);
    marked_next_sibling_.reserve(count);
    current_beam_rank_.reserve(count);
  }

  void reserve_candidates(std::size_t count) {
    candidates_.reserve(count);
    choices_.reserve(count);
  }

  // 現在のビームを共有履歴木のDFSで巡回する。
  // visit(rank, state) のstateはそのrankの状態。終了時はrootへ戻る。
  template <class Visit, class Apply, class Revert>
  void for_each_state(Visit&& visit, Apply&& apply, Revert&& revert) {
    begin_marking();
    traversal_stack_.clear();
    traversal_stack_.push_back(
        TraversalFrame{0, marked_first_child_[0]});

    if (nodes_[0].scheduled && nodes_[0].generation == generation_) {
      visit(current_beam_rank_[0], static_cast<const State&>(state_));
    }

    while (!traversal_stack_.empty()) {
      TraversalFrame& frame = traversal_stack_.back();
      if (frame.next_child == -1) {
        const int node = frame.node;
        traversal_stack_.pop_back();
        if (node != 0) revert(state_, *nodes_[node].action);
        continue;
      }

      const int child = frame.next_child;
      frame.next_child = marked_next_sibling_[child];
      apply(state_, *nodes_[child].action);
      if (nodes_[child].scheduled &&
          nodes_[child].generation == generation_) {
        visit(current_beam_rank_[child],
              static_cast<const State&>(state_));
      }
      traversal_stack_.push_back(
          TraversalFrame{child, marked_first_child_[child]});
    }
  }

  const Score& best_score() const {
    assert(!beam_.empty());
    return beam_.front().score;
  }

  // rank=0 が現在の generation で最良の候補。outの容量は再利用する。
  void restore(int rank, std::vector<Action>& out) const {
    assert(0 <= rank && rank < static_cast<int>(beam_.size()));
    int node = beam_[rank].node;
    out.clear();
    while (node != 0) {
      assert(nodes_[node].action.has_value());
      out.push_back(*nodes_[node].action);
      node = nodes_[node].parent;
    }
    std::reverse(out.begin(), out.end());
  }

  std::vector<Action> restore(int rank = 0) const {
    std::vector<Action> actions;
    restore(rank, actions);
    return actions;
  }

  // observerで受け取ったparent_rankとactionから、その候補の経路を作る。
  // observer内で即時に呼ぶこと。parent_rankやaction、observerが受け取る
  // 各参照を保存して、step終了後に使ってはいけない。
  void restore_candidate(int parent_rank,
                         const Action& action,
                         std::vector<Action>& out) const {
    restore(parent_rank, out);
    out.push_back(action);
  }

  std::vector<Action> restore_candidate(
      int parent_rank, const Action& action) const {
    std::vector<Action> actions;
    restore_candidate(parent_rank, action, actions);
    return actions;
  }

 private:
  struct Node {
    int parent = -1;
    int first_child = -1;
    int previous_sibling = -1;
    int next_sibling = -1;
    int generation = 0;
    bool alive = false;
    bool scheduled = false;
    std::optional<Action> action;
  };

  struct LogicalOrder {
    std::uint64_t step;
    int parent_rank;
    int action_index;
  };

  struct Entry {
    int node;
    Score score;
    LogicalOrder order;
    std::optional<Key> key;
  };

  struct Candidate {
    int parent;
    int next_same_generation;
    Action action;
    Score score;
    LogicalOrder order;
    std::optional<Key> key;
  };

  struct Choice {
    bool is_old;
    int index;
  };

  struct TraversalFrame {
    int node;
    int next_child;
  };

  struct NoKeyMaker {};
  struct NoObserver {};

  State state_;
  int beam_width_;
  int max_generation_;
  bool maximize_;
  int generation_ = 0;
  int mode_ = 0;  // 0: 未決定、1: keyなし、2: keyあり
  std::uint64_t next_step_order_ = 1;
  std::uint64_t current_step_order_ = 0;

  std::vector<Node> nodes_;
  std::vector<int> free_nodes_;
  std::vector<Entry> beam_;
  std::vector<std::vector<Entry>> layers_;
  std::priority_queue<int, std::vector<int>, std::greater<int>>
      ready_generations_;

  std::vector<Candidate> candidates_;
  std::vector<int> first_candidate_;
  std::vector<int> touched_generations_;
  std::vector<Choice> choices_;
  std::vector<int> removed_nodes_buffer_;
  std::unordered_map<Key, int, KeyHash> choice_by_key_;

  std::uint32_t mark_stamp_ = 0;
  std::vector<std::uint32_t> mark_;
  std::vector<int> marked_first_child_;
  std::vector<int> marked_next_sibling_;
  std::vector<int> current_beam_rank_;
  std::vector<TraversalFrame> traversal_stack_;

  bool score_is_nan(const Score& score) const {
    if constexpr (std::is_floating_point_v<Score>) {
      return std::isnan(score);
    } else {
      (void)score;
      return false;
    }
  }

  bool score_is_better(const Score& a, const Score& b) const {
    const bool a_nan = score_is_nan(a);
    const bool b_nan = score_is_nan(b);
    if (a_nan != b_nan) return !a_nan;
    if (a_nan) return false;
    return maximize_ ? b < a : a < b;
  }

  void trim_entries(std::vector<Entry>& entries) {
    if (entries.size() <= static_cast<std::size_t>(beam_width_)) return;

    const std::size_t first_removed = static_cast<std::size_t>(beam_width_);
    removed_nodes_buffer_.clear();
    removed_nodes_buffer_.reserve(entries.size() - first_removed);
    for (std::size_t i = first_removed; i < entries.size(); ++i) {
      removed_nodes_buffer_.push_back(entries[i].node);
    }
    // resize(n)は縮小だけでもEntryのdefault構築可能性を要求し、eraseも
    // 実装によっては末尾削除だけでmove代入をinstantiateする。pop_backなら
    // Scoreにdefault構築もmove代入も要求せず、末尾だけを確実に捨てられる。
    while (entries.size() > first_removed) entries.pop_back();
    for (int node : removed_nodes_buffer_) remove_scheduled_leaf(node);
  }

  const Score& choice_score(const Choice& choice,
                            const std::vector<Entry>& old_entries) const {
    return choice.is_old ? old_entries[choice.index].score
                         : candidates_[choice.index].score;
  }

  const LogicalOrder& choice_order(
      const Choice& choice,
      const std::vector<Entry>& old_entries) const {
    return choice.is_old ? old_entries[choice.index].order
                         : candidates_[choice.index].order;
  }

  bool order_is_earlier(const LogicalOrder& a,
                        const LogicalOrder& b) const {
    if (a.step != b.step) return a.step < b.step;
    if (a.parent_rank != b.parent_rank) {
      return a.parent_rank < b.parent_rank;
    }
    return a.action_index < b.action_index;
  }

  const Key& choice_key(const Choice& choice,
                        const std::vector<Entry>& old_entries) const {
    const std::optional<Key>& key =
        choice.is_old ? old_entries[choice.index].key
                      : candidates_[choice.index].key;
    assert(key.has_value());
    return *key;
  }

  bool choice_is_better(const Choice& a,
                        const Choice& b,
                        const std::vector<Entry>& old_entries) const {
    const Score& a_score = choice_score(a, old_entries);
    const Score& b_score = choice_score(b, old_entries);
    if (score_is_better(a_score, b_score)) return true;
    if (score_is_better(b_score, a_score)) return false;
    return order_is_earlier(choice_order(a, old_entries),
                            choice_order(b, old_entries));
  }

  int create_node(int parent, int generation, Action action) {
    int node;
    if (free_nodes_.empty()) {
      node = static_cast<int>(nodes_.size());
      nodes_.push_back(Node{});
      mark_.push_back(0);
      marked_first_child_.push_back(-1);
      marked_next_sibling_.push_back(-1);
      current_beam_rank_.push_back(-1);
    } else {
      node = free_nodes_.back();
      free_nodes_.pop_back();
    }

    Node& created = nodes_[node];
    created.parent = parent;
    created.first_child = -1;
    created.previous_sibling = -1;
    created.next_sibling = nodes_[parent].first_child;
    created.generation = generation;
    created.alive = true;
    created.scheduled = true;
    created.action.reset();
    created.action.emplace(std::move(action));
    mark_[node] = 0;
    marked_first_child_[node] = -1;
    marked_next_sibling_[node] = -1;
    current_beam_rank_[node] = -1;

    if (created.next_sibling != -1) {
      nodes_[created.next_sibling].previous_sibling = node;
    }
    nodes_[parent].first_child = node;
    return node;
  }

  void remove_scheduled_leaf(int node) {
    assert(0 <= node && node < static_cast<int>(nodes_.size()));
    assert(nodes_[node].alive);
    nodes_[node].scheduled = false;

    while (node != 0 && !nodes_[node].scheduled &&
           nodes_[node].first_child == -1) {
      const int parent = nodes_[node].parent;
      const int previous = nodes_[node].previous_sibling;
      const int next = nodes_[node].next_sibling;

      if (previous == -1) {
        nodes_[parent].first_child = next;
      } else {
        nodes_[previous].next_sibling = next;
      }
      if (next != -1) nodes_[next].previous_sibling = previous;

      nodes_[node].action.reset();
      nodes_[node].alive = false;
      nodes_[node].parent = -1;
      nodes_[node].previous_sibling = -1;
      nodes_[node].next_sibling = -1;
      free_nodes_.push_back(node);
      node = parent;
    }
  }

  void begin_marking() {
    ++mark_stamp_;
    if (mark_stamp_ == 0) {
      std::fill(mark_.begin(), mark_.end(), 0);
      mark_stamp_ = 1;
    }

    for (int rank = 0; rank < static_cast<int>(beam_.size()); ++rank) {
      current_beam_rank_[beam_[rank].node] = rank;
    }

    // 先頭（高順位）の親から展開される順になるよう、先頭挿入は逆順で行う。
    for (auto beam_it = beam_.rbegin(); beam_it != beam_.rend(); ++beam_it) {
      const Entry& entry = *beam_it;
      int node = entry.node;
      int child = -1;

      while (node != -1 && mark_[node] != mark_stamp_) {
        mark_[node] = mark_stamp_;
        marked_first_child_[node] = -1;
        marked_next_sibling_[node] = -1;
        if (child != -1) {
          marked_next_sibling_[child] = marked_first_child_[node];
          marked_first_child_[node] = child;
        }
        child = node;
        node = nodes_[node].parent;
      }

      if (child != -1 && node != -1) {
        marked_next_sibling_[child] = marked_first_child_[node];
        marked_first_child_[node] = child;
      }
    }
  }

  template <bool UseKey,
            bool Observe,
            class Expand,
            class Apply,
            class Revert,
            class Evaluate,
            class GetAdvance,
            class MakeKey,
            class Observer>
  bool expand_node(int node,
                   Expand& expand,
                   Apply& apply,
                   Revert& revert,
                   Evaluate& evaluate,
                   GetAdvance& get_advance,
                   MakeKey& make_key,
                   Observer& observer) {
    assert(current_beam_rank_[node] >= 0);
    auto&& actions = expand(static_cast<const State&>(state_));
    int action_index = 0;
    for (auto&& expanded_action : actions) {
      Action action = std::move(expanded_action);
      const int this_action_index = action_index++;
      const long long advance =
          static_cast<long long>(get_advance(static_cast<const Action&>(action)));
      if (advance <= 0) {
        return false;
      }
      if (advance >
          static_cast<long long>(max_generation_ - generation_)) {
        continue;
      }
      const int next_generation = generation_ + static_cast<int>(advance);

      apply(state_, action);
      Score score = evaluate(static_cast<const State&>(state_));
      std::optional<Key> key;
      if constexpr (UseKey) {
        key.emplace(make_key(static_cast<const State&>(state_)));
      }
      if constexpr (Observe) {
        observer(current_beam_rank_[node],
                 static_cast<const Action&>(action),
                 static_cast<const State&>(state_),
                 static_cast<const Score&>(score), next_generation);
      }
      revert(state_, action);

      if (first_candidate_[next_generation] == -1) {
        touched_generations_.push_back(next_generation);
      }
      const int candidate = static_cast<int>(candidates_.size());
      candidates_.push_back(
          Candidate{node,
                    first_candidate_[next_generation],
                    std::move(action),
                    std::move(score),
                    LogicalOrder{current_step_order_,
                                 current_beam_rank_[node],
                                 this_action_index},
                    std::move(key)});
      first_candidate_[next_generation] = candidate;
    }
    return true;
  }

  template <bool UseKey>
  void merge_generation(int generation) {
    std::vector<Entry>& old_entries = layers_[generation];
    choices_.clear();

    int new_count = 0;
    for (int candidate = first_candidate_[generation]; candidate != -1;
         candidate = candidates_[candidate].next_same_generation) {
      ++new_count;
    }
    choices_.reserve(old_entries.size() + new_count);

    if constexpr (UseKey) {
      choice_by_key_.clear();
      choice_by_key_.reserve(old_entries.size() + new_count);

      const auto add_choice = [&](Choice choice) {
        const Key& key = choice_key(choice, old_entries);
        const auto found = choice_by_key_.find(key);
        if (found == choice_by_key_.end()) {
          const int index = static_cast<int>(choices_.size());
          choice_by_key_.emplace(key, index);
          choices_.push_back(choice);
        } else if (choice_is_better(
                       choice, choices_[found->second], old_entries)) {
          choices_[found->second] = choice;
        }
      };

      for (int i = 0; i < static_cast<int>(old_entries.size()); ++i) {
        add_choice(Choice{true, i});
      }
      for (int candidate = first_candidate_[generation]; candidate != -1;
           candidate = candidates_[candidate].next_same_generation) {
        add_choice(Choice{false, candidate});
      }
    } else {
      for (int i = 0; i < static_cast<int>(old_entries.size()); ++i) {
        choices_.push_back(Choice{true, i});
      }
      for (int candidate = first_candidate_[generation]; candidate != -1;
           candidate = candidates_[candidate].next_same_generation) {
        choices_.push_back(Choice{false, candidate});
      }
    }

    assert(!choices_.empty());
    const auto is_better = [&](const Choice& a, const Choice& b) {
      return choice_is_better(a, b, old_entries);
    };
    const int kept =
        std::min(beam_width_, static_cast<int>(choices_.size()));
    if (kept < static_cast<int>(choices_.size())) {
      std::nth_element(choices_.begin(), choices_.begin() + kept,
                       choices_.end(), is_better);
      choices_.resize(kept);
    }
    std::sort(choices_.begin(), choices_.end(), is_better);

    std::vector<unsigned char> keep_old(old_entries.size(), 0);
    std::vector<Entry> next_entries;
    next_entries.reserve(choices_.size());

    for (const Choice& choice : choices_) {
      if (choice.is_old) {
        keep_old[choice.index] = 1;
        next_entries.push_back(std::move(old_entries[choice.index]));
      } else {
        Candidate& candidate = candidates_[choice.index];
        const int node = create_node(candidate.parent, generation,
                                     std::move(candidate.action));
        next_entries.push_back(
            Entry{node,
                  std::move(candidate.score),
                  candidate.order,
                  std::move(candidate.key)});
      }
    }

    removed_nodes_buffer_.clear();
    removed_nodes_buffer_.reserve(old_entries.size());
    for (int i = 0; i < static_cast<int>(old_entries.size()); ++i) {
      if (!keep_old[i]) removed_nodes_buffer_.push_back(old_entries[i].node);
    }

    const bool was_empty = old_entries.empty();
    old_entries = std::move(next_entries);
    if (was_empty) ready_generations_.push(generation);
    for (int node : removed_nodes_buffer_) remove_scheduled_leaf(node);
  }

  template <bool UseKey,
            bool Observe,
            class Expand,
            class Apply,
            class Revert,
            class Evaluate,
            class GetAdvance,
            class MakeKey,
            class Observer>
  bool step_impl(Expand& expand,
                 Apply& apply,
                 Revert& revert,
                 Evaluate& evaluate,
                 GetAdvance& get_advance,
                 MakeKey& make_key,
                 Observer& observer) {
    const int requested_mode = UseKey ? 2 : 1;
    if (mode_ == 0) mode_ = requested_mode;
    if (mode_ != requested_mode) {
      throw std::invalid_argument(
          "do not mix keyed and non-keyed search");
    }

    candidates_.clear();
    touched_generations_.clear();
    current_step_order_ = next_step_order_++;
    bool valid_advance = true;
    for_each_state(
        [&](int rank, const State&) {
          if (!valid_advance) return;
          const int node = beam_[rank].node;
          valid_advance = expand_node<UseKey, Observe>(
              node, expand, apply, revert, evaluate, get_advance, make_key,
              observer);
        },
        apply,
        revert);

    if (!valid_advance) {
      for (int next_generation : touched_generations_) {
        first_candidate_[next_generation] = -1;
      }
      candidates_.clear();
      touched_generations_.clear();
      throw std::invalid_argument("advance must be positive");
    }

    for (int next_generation : touched_generations_) {
      merge_generation<UseKey>(next_generation);
      first_candidate_[next_generation] = -1;
    }

    if (ready_generations_.empty()) return false;

    for (const Entry& entry : beam_) {
      remove_scheduled_leaf(entry.node);
    }
    beam_.clear();

    generation_ = ready_generations_.top();
    ready_generations_.pop();
    beam_ = std::move(layers_[generation_]);
    layers_[generation_].clear();
    return true;
  }
};

// 問題依存コードをProblemへ集める、世代飛ばしapply/revertビームのRunner。
//
// 【使う人がmain.cpp側へ書く場所】
// 空関数を配置済みの雛形: template/search/beam/variable-cost-tree.cpp
//
//   TODO: 【問題ごと】State、軽いMove、候補順位Scoreを書く。
//   using State, Move, Score
//   TODO: 【問題ごと】現在状態から試す合法手を列挙する。
//   generate_moves(const State&)       -> 次に試すMoveのコンテナ。
//   TODO: 【問題ごと】Stateを1手だけ進め、完全に戻す。
//   apply_move(State&, Move&)
//   revert_move(State&, const Move&)
//   TODO: 【問題ごと】子Stateの順位値そのものを返す。
//   evaluate(const State&)
//   TODO: 【問題ごと】Moveが進める正の世代数を返す。
//   get_advance(const Move&)
//   TODO: 【必要な問題だけ】同一局面を表すkeyを書く。
//   make_key(const State&)
//
// Runnerは到着世代別buffer、共有履歴木、DFS巡回、上位N件選抜、重複除去、
// 世代ループを担当する。早期terminalを拾う時はstep_and_observe系を使う。
// ↓↓↓ ここから下はライブラリ本体。通常は編集しない。↓↓↓
template <class Problem,
          class Key = std::uint64_t,
          class KeyHash = std::hash<Key>>
struct CostTreeBeamRunner {
  using State = typename Problem::State;
  using Move = typename Problem::Move;
  using Score = typename Problem::Score;

  CostTreeBeamRunner(Problem& problem,
                     State initial_state,
                     Score initial_score,
                     int beam_width,
                     int max_generation,
                     bool maximize = true)
      : problem_(problem),
        beam_(std::move(initial_state),
              std::move(initial_score),
              beam_width,
              max_generation,
              maximize) {}

  bool step() {
    return beam_.step(
        [&](const State& state) -> decltype(auto) {
          return problem_.generate_moves(state);
        },
        [&](State& state, Move& move) { problem_.apply_move(state, move); },
        [&](State& state, const Move& move) {
          problem_.revert_move(state, move);
        },
        [&](const State& state) { return problem_.evaluate(state); },
        [&](const Move& move) { return problem_.get_advance(move); });
  }

  bool step_with_key() {
    return beam_.step_with_key(
        [&](const State& state) -> decltype(auto) {
          return problem_.generate_moves(state);
        },
        [&](State& state, Move& move) { problem_.apply_move(state, move); },
        [&](State& state, const Move& move) {
          problem_.revert_move(state, move);
        },
        [&](const State& state) { return problem_.evaluate(state); },
        [&](const Move& move) { return problem_.get_advance(move); },
        [&](const State& state) { return problem_.make_key(state); });
  }

  template <class Observer>
  bool step_and_observe(Observer&& observer) {
    return beam_.step_and_observe(
        [&](const State& state) -> decltype(auto) {
          return problem_.generate_moves(state);
        },
        [&](State& state, Move& move) { problem_.apply_move(state, move); },
        [&](State& state, const Move& move) {
          problem_.revert_move(state, move);
        },
        [&](const State& state) { return problem_.evaluate(state); },
        [&](const Move& move) { return problem_.get_advance(move); },
        std::forward<Observer>(observer));
  }

  template <class Observer>
  bool step_with_key_and_observe(Observer&& observer) {
    return beam_.step_with_key_and_observe(
        [&](const State& state) -> decltype(auto) {
          return problem_.generate_moves(state);
        },
        [&](State& state, Move& move) { problem_.apply_move(state, move); },
        [&](State& state, const Move& move) {
          problem_.revert_move(state, move);
        },
        [&](const State& state) { return problem_.evaluate(state); },
        [&](const Move& move) { return problem_.get_advance(move); },
        [&](const State& state) { return problem_.make_key(state); },
        std::forward<Observer>(observer));
  }

  int run() {
    while (step()) {
    }
    return generation();
  }

  int run_with_key() {
    while (step_with_key()) {
    }
    return generation();
  }

  template <class Visit>
  void for_each_state(Visit&& visit) {
    beam_.for_each_state(
        std::forward<Visit>(visit),
        [&](State& state, Move& move) { problem_.apply_move(state, move); },
        [&](State& state, const Move& move) {
          problem_.revert_move(state, move);
        });
  }

  int generation() const { return beam_.generation(); }
  int size() const { return beam_.size(); }
  int beam_width() const { return beam_.beam_width(); }
  int max_generation() const { return beam_.max_generation(); }
  const Score& best_score() const { return beam_.best_score(); }

  std::vector<Move> restore(int rank = 0) const {
    return beam_.restore(rank);
  }
  void restore(int rank, std::vector<Move>& out) const {
    beam_.restore(rank, out);
  }
  std::vector<Move> restore_candidate(
      int parent_rank, const Move& move) const {
    return beam_.restore_candidate(parent_rank, move);
  }
  void restore_candidate(
      int parent_rank, const Move& move, std::vector<Move>& out) const {
    beam_.restore_candidate(parent_rank, move, out);
  }

  void set_width(int width) { beam_.set_beam_width(width); }
  void reserve_nodes(std::size_t count) { beam_.reserve_nodes(count); }
  void reserve_candidates(std::size_t count) {
    beam_.reserve_candidates(count);
  }

 private:
  Problem& problem_;
  CostTreeBeamSearch<State, Move, Score, Key, KeyHash> beam_;
};
