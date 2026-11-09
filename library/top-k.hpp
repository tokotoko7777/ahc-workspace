#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/top-k.hpp

// 良い候補を上位 K 個だけ残す。1回の追加は O(K)。
// 毎回全体を sort しないので、小さな K で何度も追加する用途に向く。
// 使い方:
// TopK<double, vector<int>> candidates(20);        // 大きいほど良い
// TopK<double, vector<int>> candidates(20, false); // 小さいほど良い
// candidates.add(score, state);
template <class Score, class State>
struct TopK {
  int limit;
  bool maximize;
  std::vector<std::pair<Score, State>> entries;

  explicit TopK(int limit, bool maximize = true)
      : limit(limit), maximize(maximize) {
    assert(limit > 0);
    entries.reserve(limit + 1);
  }

  void add(Score score, State state) {
    const auto is_better = [&](const Score& a, const Score& b) {
      return maximize ? b < a : a < b;
    };

    int position = 0;
    while (position < static_cast<int>(entries.size()) &&
           !is_better(score, entries[position].first)) {
      ++position;
    }
    if (position >= limit) return;

    entries.insert(entries.begin() + position,
                   {std::move(score), std::move(state)});
    if (static_cast<int>(entries.size()) > limit) entries.pop_back();
  }

  int size() const { return static_cast<int>(entries.size()); }

  const Score& best_score() const {
    assert(!entries.empty());
    return entries.front().first;
  }

  const State& best_state() const {
    assert(!entries.empty());
    return entries.front().second;
  }
};
