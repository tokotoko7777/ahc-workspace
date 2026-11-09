#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/radix-heap.hpp

// 取り出すキーが単調非減少になる場合の高速な優先度付きキュー。
// Dijkstra の「距離」のような 0 以上の整数キーに使える。
// 使い方:
// RadixHeap<int> queue;
// queue.push(distance, vertex);
// auto [distance, vertex] = queue.pop();
template <class Value>
struct RadixHeap {
  using Entry = std::pair<std::uint64_t, Value>;

  std::array<std::vector<Entry>, 65> buckets;
  std::uint64_t last_key = 0;
  std::size_t element_count = 0;

  bool empty() const { return element_count == 0; }
  std::size_t size() const { return element_count; }
  std::uint64_t last() const { return last_key; }

  void push(std::uint64_t key, Value value) {
    assert(last_key <= key);
    buckets[bucket_index(key ^ last_key)].push_back(
        {key, std::move(value)});
    ++element_count;
  }

  Entry pop() {
    assert(!empty());
    if (buckets[0].empty()) redistribute();

    Entry result = std::move(buckets[0].back());
    buckets[0].pop_back();
    --element_count;
    return result;
  }

  void clear() {
    for (auto& bucket : buckets) bucket.clear();
    last_key = 0;
    element_count = 0;
  }

 private:
  static int bucket_index(std::uint64_t difference) {
    if (difference == 0) return 0;
    return 64 - __builtin_clzll(difference);
  }

  void redistribute() {
    int source = 1;
    while (buckets[source].empty()) ++source;

    last_key = buckets[source][0].first;
    for (const Entry& entry : buckets[source]) {
      last_key = std::min(last_key, entry.first);
    }
    for (Entry& entry : buckets[source]) {
      buckets[bucket_index(entry.first ^ last_key)].push_back(
          std::move(entry));
    }
    buckets[source].clear();
  }
};
