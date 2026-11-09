#include <functional>
#include <utility>
#include <unordered_map>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/best-by-key.hpp

// 同じキーの候補が複数ある時、一番良いものだけ残す。
// Key は int、long long、string など unordered_map で使える型。
//
// 使い方:
// BestByKey<long long, double, State> unique_candidates;
// unique_candidates.add(state_hash, score, state);
template <class Key,
          class Score,
          class State,
          class Hash = std::hash<Key>>
struct BestByKey {
  struct Entry {
    Key key;
    Score score;
    State state;
  };

  bool maximize;
  std::vector<Entry> entries;
  std::unordered_map<Key, int, Hash> index_by_key;

  explicit BestByKey(bool maximize = true) : maximize(maximize) {}

  bool add(Key key, Score score, State state) {
    const auto iterator = index_by_key.find(key);
    if (iterator == index_by_key.end()) {
      const int index = static_cast<int>(entries.size());
      index_by_key.emplace(key, index);
      entries.push_back(
          {std::move(key), std::move(score), std::move(state)});
      return true;
    }

    Entry& old = entries[iterator->second];
    const bool better = maximize ? old.score < score : score < old.score;
    if (!better) return false;
    old.score = std::move(score);
    old.state = std::move(state);
    return true;
  }

  int size() const { return static_cast<int>(entries.size()); }

  void clear() {
    entries.clear();
    index_by_key.clear();
  }
};
