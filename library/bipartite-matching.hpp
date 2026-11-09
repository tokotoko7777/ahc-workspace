#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/bipartite-matching.hpp

struct BipartiteMatchingResult {
  std::vector<int> left_match;
  std::vector<int> right_match;

  int size() const {
    int result = 0;
    for (int match : left_match) result += match != -1;
    return result;
  }

  std::vector<std::pair<int, int>> pairs() const {
    std::vector<std::pair<int, int>> result;
    result.reserve(size());
    for (int left = 0; left < static_cast<int>(left_match.size()); ++left) {
      if (left_match[left] != -1) result.push_back({left, left_match[left]});
    }
    return result;
  }
};

// Hopcroft-Karp法。左側と右側の頂点を1対1に対応させる最大マッチング。
// 使い方:
// BipartiteMatching matching(left_size, right_size);
// matching.add_edge(left, right);
// auto result = matching.solve();
struct BipartiteMatching {
  int left_size;
  int right_size;
  std::vector<std::vector<int>> graph;

  BipartiteMatching(int left_size, int right_size)
      : left_size(left_size), right_size(right_size), graph(left_size) {
    assert(left_size >= 0 && right_size >= 0);
  }

  void add_edge(int left, int right) {
    assert(0 <= left && left < left_size);
    assert(0 <= right && right < right_size);
    graph[left].push_back(right);
  }

  BipartiteMatchingResult solve() const {
    BipartiteMatchingResult result{std::vector<int>(left_size, -1),
                                   std::vector<int>(right_size, -1)};
    std::vector<int> level(left_size);
    std::vector<int> next_edge(left_size);

    while (build_levels(result, level)) {
      std::fill(next_edge.begin(), next_edge.end(), 0);
      for (int left = 0; left < left_size; ++left) {
        if (result.left_match[left] == -1) {
          augment(left, result, level, next_edge);
        }
      }
    }
    return result;
  }

 private:
  bool build_levels(const BipartiteMatchingResult& matching,
                    std::vector<int>& level) const {
    std::fill(level.begin(), level.end(), -1);
    std::vector<int> queue;
    queue.reserve(left_size);
    for (int left = 0; left < left_size; ++left) {
      if (matching.left_match[left] == -1) {
        level[left] = 0;
        queue.push_back(left);
      }
    }

    bool found_free_right = false;
    for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
      const int left = queue[head];
      for (int right : graph[left]) {
        const int next_left = matching.right_match[right];
        if (next_left == -1) {
          found_free_right = true;
        } else if (level[next_left] == -1) {
          level[next_left] = level[left] + 1;
          queue.push_back(next_left);
        }
      }
    }
    return found_free_right;
  }

  bool augment(int left, BipartiteMatchingResult& matching,
               std::vector<int>& level, std::vector<int>& next_edge) const {
    for (int& index = next_edge[left];
         index < static_cast<int>(graph[left].size()); ++index) {
      const int right = graph[left][index];
      const int next_left = matching.right_match[right];
      if (next_left != -1 &&
          (level[next_left] != level[left] + 1 ||
           !augment(next_left, matching, level, next_edge))) {
        continue;
      }
      matching.left_match[left] = right;
      matching.right_match[right] = left;
      return true;
    }
    level[left] = -1;
    return false;
  }
};
