#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/simple-beam-search.hpp

// 状態を丸ごと持つ、分かりやすさ優先のビームサーチ。
// State が小さい時や、まずビームサーチを試したい時に向いている。
// 状態が大きくコピーが重い時は、actionから順位を差分計算できるなら
// action-beam-search.hpp、apply/revertを書けるならtree-beam-search.hppも検討する。
//
// 空関数を配置済みの雛形: template/search/beam/simple.cpp
// 一番簡単な使い方。TODOが付いた3か所だけ問題に合わせる:
// TODO: 【問題ごと】盤面、現在手数、score、必要なら操作履歴をStateへ書く。
// SimpleBeamSearch<State, long long> beam(initial_state, 100);
// for (int turn = 0; turn < turns; ++turn) {
//   if (!beam.step(
//           // TODO: 【問題ごと】stateから次の子Stateを全て作って返す。
//           [](const State& state) { return state.next_states(); },
//           // TODO: 【問題ごと】子Stateの順位値そのものを返す。
//           [](const State& state) { return state.score; })) {
//     break;
//   }
// }
// State answer = beam.best();
// この下のSimpleBeamSearch本体は通常編集しない。
//
// 一時 vector を作りたくない時は、子を emit で直接渡せる。
// beam.step_each(
//     [](const State& state, auto&& emit) {
//       for (int action : state.actions()) {
//         State child = state.next(action);
//         emit(std::move(child));
//       }
//     },
//     [](const State& state) { return state.score; });
//
// step は子が1つもなければ false を返し、現在の層と depth をそのまま残す。
// この時、直近stepの件数は generated / unique / kept の全てが0になる。
// 一部の状態だけ子を持たない場合、その状態は次の層には残らない。
// 幅から落ちる終端状態も保存する時は step_and_observe を使う。
// observer は重複除去と幅選抜より前に、全ての生成子へちょうど1回呼ばれる。
// StateとScoreの参照はobserver呼出中だけ有効。必要な値はその場でコピーし、
// observerから同じbeamのstep/reset/set_widthなどを呼ばないこと。
// beam.step_and_observe(expand, rank_score,
//     [&](const State& child, const Score&) {
//       if (is_terminal(child)) save_if_better(child);
//     });
template <class State, class Score>
struct SimpleBeamSearch {
  // 古いコードとの互換用に公開したまま残す。
  // 探索内部では State と順位情報を別配列に置き、選抜中に State を動かさない。
  struct Candidate {
    Score score;
    State state;
    std::size_t order;
  };

  explicit SimpleBeamSearch(
      State initial_state,
      int beam_width,
      bool maximize = true)
      : beam_width_(beam_width), maximize_(maximize) {
    if (beam_width <= 0) {
      throw std::invalid_argument("beam_width must be positive");
    }
    beam_.push_back(std::move(initial_state));
  }

  // 1層進める。expand(state) は次の State を並べたコンテナを返す。
  // constコンテナはコピーし、非constコンテナの要素はmoveして消費する。
  // evaluate(state) はビーム内での順位を決める値を返す。
  template <class Expand, class Evaluate>
  bool step(Expand&& expand, Evaluate&& evaluate) {
    NoObserver observer;
    return step_range_impl(
        std::forward<Expand>(expand),
        std::forward<Evaluate>(evaluate), observer);
  }

  // 全ての生成子を、重複除去・幅選抜より前にobserverへ渡す。
  // on_generated(const State&, const Score&) は各生成子へちょうど1回呼ばれる。
  // 受け取った参照を保存したり、同じbeamへ再入したりしないこと。
  template <class Expand, class Evaluate, class OnGenerated>
  bool step_and_observe(
      Expand&& expand,
      Evaluate&& evaluate,
      OnGenerated&& on_generated) {
    return step_range_impl(
        std::forward<Expand>(expand),
        std::forward<Evaluate>(evaluate), on_generated);
  }

  // generate(parent, emit) の形で、子Stateを一時コンテナなしで直接渡す。
  // lvalueはコピーする。moveしたい時は emit(std::move(child)) と書く。
  // emit は generate の呼び出し中だけ使い、保存して後から呼ばないこと。
  template <class Generate, class Evaluate>
  bool step_each(Generate&& generate, Evaluate&& evaluate) {
    NoObserver observer;
    return step_each_impl(
        std::forward<Generate>(generate),
        std::forward<Evaluate>(evaluate), observer);
  }

  template <class Generate, class Evaluate, class OnGenerated>
  bool step_each_and_observe(
      Generate&& generate,
      Evaluate&& evaluate,
      OnGenerated&& on_generated) {
    return step_each_impl(
        std::forward<Generate>(generate),
        std::forward<Evaluate>(evaluate), on_generated);
  }

  // 同じ key の状態は、評価値が一番良い候補だけを残してから幅を絞る。
  // make_key(state) は int、uint64_t、string などを返すようにする。
  template <class Expand, class Evaluate, class MakeKey>
  bool step_with_key(
      Expand&& expand,
      Evaluate&& evaluate,
      MakeKey&& make_key) {
    using Key =
        std::decay_t<decltype(make_key(std::declval<const State&>()))>;
    NoObserver observer;
    return step_range_key_impl(
        std::forward<Expand>(expand),
        std::forward<Evaluate>(evaluate),
        std::forward<MakeKey>(make_key),
        std::hash<Key>{}, std::equal_to<Key>{}, observer);
  }

  // std::hash<Key>を用意しにくい型では、hashと等値比較も直接渡せる。
  template <class Expand,
            class Evaluate,
            class MakeKey,
            class Hash,
            class KeyEqual>
  bool step_with_key(
      Expand&& expand,
      Evaluate&& evaluate,
      MakeKey&& make_key,
      Hash&& hash,
      KeyEqual&& key_equal) {
    NoObserver observer;
    return step_range_key_impl(
        std::forward<Expand>(expand),
        std::forward<Evaluate>(evaluate),
        std::forward<MakeKey>(make_key),
        std::forward<Hash>(hash),
        std::forward<KeyEqual>(key_equal), observer);
  }

  template <class Expand, class Evaluate, class MakeKey, class OnGenerated>
  bool step_with_key_and_observe(
      Expand&& expand,
      Evaluate&& evaluate,
      MakeKey&& make_key,
      OnGenerated&& on_generated) {
    using Key =
        std::decay_t<decltype(make_key(std::declval<const State&>()))>;
    return step_range_key_impl(
        std::forward<Expand>(expand),
        std::forward<Evaluate>(evaluate),
        std::forward<MakeKey>(make_key),
        std::hash<Key>{}, std::equal_to<Key>{}, on_generated);
  }

  template <class Expand,
            class Evaluate,
            class MakeKey,
            class Hash,
            class KeyEqual,
            class OnGenerated>
  bool step_with_key_and_observe(
      Expand&& expand,
      Evaluate&& evaluate,
      MakeKey&& make_key,
      Hash&& hash,
      KeyEqual&& key_equal,
      OnGenerated&& on_generated) {
    return step_range_key_impl(
        std::forward<Expand>(expand),
        std::forward<Evaluate>(evaluate),
        std::forward<MakeKey>(make_key),
        std::forward<Hash>(hash),
        std::forward<KeyEqual>(key_equal), on_generated);
  }

  template <class Generate, class Evaluate, class MakeKey>
  bool step_each_with_key(
      Generate&& generate,
      Evaluate&& evaluate,
      MakeKey&& make_key) {
    using Key =
        std::decay_t<decltype(make_key(std::declval<const State&>()))>;
    NoObserver observer;
    return step_each_key_impl(
        std::forward<Generate>(generate),
        std::forward<Evaluate>(evaluate),
        std::forward<MakeKey>(make_key),
        std::hash<Key>{}, std::equal_to<Key>{}, observer);
  }

  template <class Generate,
            class Evaluate,
            class MakeKey,
            class Hash,
            class KeyEqual>
  bool step_each_with_key(
      Generate&& generate,
      Evaluate&& evaluate,
      MakeKey&& make_key,
      Hash&& hash,
      KeyEqual&& key_equal) {
    NoObserver observer;
    return step_each_key_impl(
        std::forward<Generate>(generate),
        std::forward<Evaluate>(evaluate),
        std::forward<MakeKey>(make_key),
        std::forward<Hash>(hash),
        std::forward<KeyEqual>(key_equal), observer);
  }

  template <class Generate,
            class Evaluate,
            class MakeKey,
            class OnGenerated>
  bool step_each_with_key_and_observe(
      Generate&& generate,
      Evaluate&& evaluate,
      MakeKey&& make_key,
      OnGenerated&& on_generated) {
    using Key =
        std::decay_t<decltype(make_key(std::declval<const State&>()))>;
    return step_each_key_impl(
        std::forward<Generate>(generate),
        std::forward<Evaluate>(evaluate),
        std::forward<MakeKey>(make_key),
        std::hash<Key>{}, std::equal_to<Key>{}, on_generated);
  }

  template <class Generate,
            class Evaluate,
            class MakeKey,
            class Hash,
            class KeyEqual,
            class OnGenerated>
  bool step_each_with_key_and_observe(
      Generate&& generate,
      Evaluate&& evaluate,
      MakeKey&& make_key,
      Hash&& hash,
      KeyEqual&& key_equal,
      OnGenerated&& on_generated) {
    return step_each_key_impl(
        std::forward<Generate>(generate),
        std::forward<Evaluate>(evaluate),
        std::forward<MakeKey>(make_key),
        std::forward<Hash>(hash),
        std::forward<KeyEqual>(key_equal), on_generated);
  }

  // 現在の層。step 成功後は評価値が良い順に並んでいる。
  const std::vector<State>& states() const { return beam_; }

  const State& best() const {
    assert(!beam_.empty());
    return beam_.front();
  }

  // 探索途中で変更すると順位順が崩れるため、通常は探索終了後だけ使う。
  State& best() {
    assert(!beam_.empty());
    return beam_.front();
  }

  std::size_t size() const { return beam_.size(); }

  int depth() const { return depth_; }

  int width() const { return beam_width_; }

  // 直近に試したstepの件数。keyなしでは generated == unique。
  // stepがfalseを返した時は3つとも0になる。
  std::size_t last_generated_count() const { return last_generated_count_; }
  std::size_t last_unique_count() const { return last_unique_count_; }
  std::size_t last_kept_count() const { return last_kept_count_; }

  // 1層に出る候補数の目安が分かる時は、再確保を避けられる。
  void reserve_candidates(std::size_t count) {
    candidate_states_.reserve(count);
    candidate_metadata_.reserve(count);
    candidate_ids_.reserve(count);
  }

  // 幅を小さくした時は、現在の層もその場で切り詰める。
  void set_width(int beam_width) {
    if (beam_width <= 0) {
      throw std::invalid_argument("beam_width must be positive");
    }
    beam_width_ = beam_width;
    while (beam_.size() > static_cast<std::size_t>(beam_width_)) {
      beam_.pop_back();
    }
  }

  // 新しい初期状態から探索し直す。確保済みのbuffer容量は再利用する。
  void reset(State initial_state) {
    beam_.clear();
    beam_.push_back(std::move(initial_state));
    next_beam_.clear();
    candidate_states_.clear();
    candidate_metadata_.clear();
    candidate_ids_.clear();
    depth_ = 0;
    clear_last_counts();
  }

  // 現在のbeamは保ち、使っていない作業bufferのメモリを解放する。
  // beam自身の余分なcapacityにもshrink_to_fitを試みる。
  void release_memory() {
    std::vector<State>().swap(next_beam_);
    std::vector<State>().swap(candidate_states_);
    std::vector<CandidateMeta>().swap(candidate_metadata_);
    std::vector<std::size_t>().swap(candidate_ids_);
    beam_.shrink_to_fit();
  }

 private:
  struct CandidateMeta {
    Score score;
    std::size_t state_index;
    std::size_t order;
  };

  struct NoObserver {
    void operator()(const State&, const Score&) const {}
  };

  int beam_width_;
  bool maximize_;
  int depth_ = 0;
  std::vector<State> beam_;
  std::vector<State> next_beam_;
  std::vector<State> candidate_states_;
  std::vector<CandidateMeta> candidate_metadata_;
  std::vector<std::size_t> candidate_ids_;
  std::size_t last_generated_count_ = 0;
  std::size_t last_unique_count_ = 0;
  std::size_t last_kept_count_ = 0;

  void clear_last_counts() {
    last_generated_count_ = 0;
    last_unique_count_ = 0;
    last_kept_count_ = 0;
  }

  void begin_step() {
    candidate_states_.clear();
    candidate_metadata_.clear();
    candidate_ids_.clear();
    clear_last_counts();
  }

  template <class NextState, class Evaluate, class OnGenerated>
  void add_candidate(
      NextState&& next_state,
      Evaluate& evaluate,
      OnGenerated& on_generated) {
    const std::size_t order = last_generated_count_++;
    Score score = evaluate(next_state);
    const State& generated_state = next_state;
    const Score& generated_score = score;
    on_generated(generated_state, generated_score);

    const std::size_t state_index = candidate_states_.size();
    candidate_states_.push_back(std::forward<NextState>(next_state));
    candidate_metadata_.push_back(
        {std::move(score), state_index, order});
    ++last_unique_count_;
  }

  template <class NextState,
            class Evaluate,
            class MakeKey,
            class OnGenerated,
            class IndexByKey>
  void add_keyed_candidate(
      NextState&& next_state,
      Evaluate& evaluate,
      MakeKey& make_key,
      OnGenerated& on_generated,
      IndexByKey& index_by_key) {
    static_assert(std::is_assignable_v<State&, State&&>,
                  "keyed search requires assignable State");
    static_assert(std::is_assignable_v<Score&, Score&&>,
                  "keyed search requires assignable Score");
    const std::size_t order = last_generated_count_++;
    Score score = evaluate(next_state);
    const State& generated_state = next_state;
    const Score& generated_score = score;
    on_generated(generated_state, generated_score);
    using Key = typename IndexByKey::key_type;
    Key key = make_key(generated_state);

    const auto found = index_by_key.find(key);
    if (found == index_by_key.end()) {
      const std::size_t metadata_index = candidate_metadata_.size();
      index_by_key.emplace(std::move(key), metadata_index);
      const std::size_t state_index = candidate_states_.size();
      candidate_states_.push_back(std::forward<NextState>(next_state));
      candidate_metadata_.push_back(
          {std::move(score), state_index, order});
      ++last_unique_count_;
      return;
    }

    CandidateMeta& old = candidate_metadata_[found->second];
    if (score_is_better(score, old.score)) {
      // constコンテナ由来の候補は、まずcopy構築してからmove代入する。
      // 旧実装と同じく、Stateにcopy代入までは要求しない。
      State replacement(std::forward<NextState>(next_state));
      candidate_states_[old.state_index] = std::move(replacement);
      old.score = std::move(score);
      old.order = order;
    } else {
      // 旧APIは非constコンテナの全要素をmoveして消費していた。
      // 重複で落ちる候補についても、その互換性を維持する。
      State consumed(std::forward<NextState>(next_state));
      (void)consumed;
    }
  }

  template <class Expand, class Evaluate, class OnGenerated>
  bool step_range_impl(
      Expand&& expand,
      Evaluate&& evaluate,
      OnGenerated& on_generated) {
    begin_step();
    for (const State& state : beam_) {
      auto&& next_states = expand(state);
      for (auto&& next_state : next_states) {
        // 従来どおり、非constコンテナはmove、constコンテナはcopyする。
        add_candidate(
            std::move(next_state), evaluate, on_generated);
      }
    }
    return finish_step();
  }

  template <class Generate, class Evaluate, class OnGenerated>
  bool step_each_impl(
      Generate&& generate,
      Evaluate&& evaluate,
      OnGenerated& on_generated) {
    begin_step();
    for (const State& state : beam_) {
      auto emit = [&](auto&& next_state) {
        using EmittedState = std::decay_t<decltype(next_state)>;
        static_assert(std::is_same_v<EmittedState, State>,
                      "emit must be called with State");
        add_candidate(
            std::forward<decltype(next_state)>(next_state),
            evaluate, on_generated);
      };
      generate(state, emit);
    }
    return finish_step();
  }

  template <class Expand,
            class Evaluate,
            class MakeKey,
            class Hash,
            class KeyEqual,
            class OnGenerated>
  bool step_range_key_impl(
      Expand&& expand,
      Evaluate&& evaluate,
      MakeKey&& make_key,
      Hash&& hash,
      KeyEqual&& key_equal,
      OnGenerated& on_generated) {
    using Key =
        std::decay_t<decltype(make_key(std::declval<const State&>()))>;
    using HashType = std::decay_t<Hash>;
    using KeyEqualType = std::decay_t<KeyEqual>;
    std::unordered_map<Key, std::size_t, HashType, KeyEqualType> index_by_key(
        0, std::forward<Hash>(hash), std::forward<KeyEqual>(key_equal));

    begin_step();
    index_by_key.reserve(std::max(
        candidate_states_.capacity(),
        static_cast<std::size_t>(beam_width_)));
    for (const State& state : beam_) {
      auto&& next_states = expand(state);
      for (auto&& next_state : next_states) {
        add_keyed_candidate(
            std::move(next_state), evaluate, make_key,
            on_generated, index_by_key);
      }
    }
    return finish_step();
  }

  template <class Generate,
            class Evaluate,
            class MakeKey,
            class Hash,
            class KeyEqual,
            class OnGenerated>
  bool step_each_key_impl(
      Generate&& generate,
      Evaluate&& evaluate,
      MakeKey&& make_key,
      Hash&& hash,
      KeyEqual&& key_equal,
      OnGenerated& on_generated) {
    using Key =
        std::decay_t<decltype(make_key(std::declval<const State&>()))>;
    using HashType = std::decay_t<Hash>;
    using KeyEqualType = std::decay_t<KeyEqual>;
    std::unordered_map<Key, std::size_t, HashType, KeyEqualType> index_by_key(
        0, std::forward<Hash>(hash), std::forward<KeyEqual>(key_equal));

    begin_step();
    index_by_key.reserve(std::max(
        candidate_states_.capacity(),
        static_cast<std::size_t>(beam_width_)));
    for (const State& state : beam_) {
      auto emit = [&](auto&& next_state) {
        using EmittedState = std::decay_t<decltype(next_state)>;
        static_assert(std::is_same_v<EmittedState, State>,
                      "emit must be called with State");
        add_keyed_candidate(
            std::forward<decltype(next_state)>(next_state),
            evaluate, make_key, on_generated, index_by_key);
      };
      generate(state, emit);
    }
    return finish_step();
  }

  // 浮動小数点の NaN は最大化・最小化のどちらでも最悪とする。
  // これにより、候補比較が strict weak ordering を壊さない。
  bool score_is_better(const Score& a, const Score& b) const {
    if constexpr (std::is_floating_point_v<Score>) {
      const bool a_is_nan = std::isnan(a);
      const bool b_is_nan = std::isnan(b);
      if (a_is_nan != b_is_nan) return !a_is_nan;
      if (a_is_nan) return false;
    }
    return maximize_ ? b < a : a < b;
  }

  bool metadata_is_better(const CandidateMeta& left,
                          const CandidateMeta& right) const {
    if (score_is_better(left.score, right.score)) return true;
    if (score_is_better(right.score, left.score)) return false;
    return left.order < right.order;
  }

  bool finish_step() {
    if (candidate_metadata_.empty()) return false;
    keep_best_candidates();
    return true;
  }

  void keep_best_candidates() {
    const std::size_t kept = std::min(
        static_cast<std::size_t>(beam_width_),
        candidate_metadata_.size());
    candidate_ids_.resize(candidate_metadata_.size());
    std::iota(candidate_ids_.begin(), candidate_ids_.end(), std::size_t{0});
    const auto is_better_id = [&](std::size_t left, std::size_t right) {
      return metadata_is_better(candidate_metadata_[left],
                                candidate_metadata_[right]);
    };

    // 小さいIDだけを並べ替える。State本体だけでなくScoreも動かさないため、
    // 通常stepではScoreにmove代入やswapを要求しない。
    if (kept < candidate_ids_.size()) {
      std::nth_element(candidate_ids_.begin(),
                       candidate_ids_.begin() + kept,
                       candidate_ids_.end(), is_better_id);
      candidate_ids_.resize(kept);
    }
    std::sort(candidate_ids_.begin(), candidate_ids_.end(), is_better_id);

    next_beam_.clear();
    next_beam_.reserve(kept);
    for (std::size_t id : candidate_ids_) {
      const CandidateMeta& candidate = candidate_metadata_[id];
      next_beam_.push_back(
          std::move(candidate_states_[candidate.state_index]));
    }
    beam_.swap(next_beam_);
    // swap後のnext_beam_には1つ前の層が入っている。容量だけ再利用し、
    // Stateが内部に持つvectorなどのメモリはすぐ解放する。
    next_beam_.clear();
    candidate_states_.clear();
    candidate_metadata_.clear();
    candidate_ids_.clear();
    last_kept_count_ = kept;
    ++depth_;
  }
};

// 指定した深さまで進める簡単なラッパー。
// State の中に操作履歴も入れておくと、返り値から答えを復元できる。
// 子を持たない状態は捨て、最も深い空でない層の評価値最大/最小の状態を返す。
template <class State, class Expand, class Evaluate>
State simple_beam_search(
    State initial_state,
    int turns,
    int beam_width,
    Expand expand,
    Evaluate evaluate,
    bool maximize = true) {
  if (turns < 0) {
    throw std::invalid_argument("turns must be non-negative");
  }

  using Score = std::decay_t<decltype(evaluate(initial_state))>;
  SimpleBeamSearch<State, Score> beam(
      std::move(initial_state), beam_width, maximize);
  for (int turn = 0; turn < turns; ++turn) {
    if (!beam.step(expand, evaluate)) break;
  }
  return std::move(beam.best());
}

// 同じ key の候補を1つにまとめる固定深さ版。
template <class State, class Expand, class Evaluate, class MakeKey>
State simple_beam_search_with_key(
    State initial_state,
    int turns,
    int beam_width,
    Expand expand,
    Evaluate evaluate,
    MakeKey make_key,
    bool maximize = true) {
  if (turns < 0) {
    throw std::invalid_argument("turns must be non-negative");
  }

  using Score = std::decay_t<decltype(evaluate(initial_state))>;
  SimpleBeamSearch<State, Score> beam(
      std::move(initial_state), beam_width, maximize);
  for (int turn = 0; turn < turns; ++turn) {
    if (!beam.step_with_key(expand, evaluate, make_key)) break;
  }
  return std::move(beam.best());
}
