#include <cassert>
#include <limits>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/functional-graph.hpp

// 各頂点から出る辺がちょうど1本のグラフを、O(N log steps) で前計算する。
// FunctionalGraph graph(next);
// int after = graph.jump(start, steps);
// int entry = graph.cycle_entry[start];
struct FunctionalGraph {
  static constexpr int levels =
      std::numeric_limits<unsigned long long>::digits;

  int n;
  std::vector<int> next;
  std::vector<std::vector<int>> jump_table;
  std::vector<int> cycle_id;
  std::vector<int> cycle_position;
  std::vector<int> cycle_entry;
  std::vector<int> steps_to_cycle;
  std::vector<std::vector<int>> cycles;

  explicit FunctionalGraph(std::vector<int> next)
      : n(static_cast<int>(next.size())),
        next(std::move(next)),
        jump_table(levels, std::vector<int>(n)),
        cycle_id(n, -1),
        cycle_position(n, -1),
        cycle_entry(n, -1),
        steps_to_cycle(n, -1) {
    for (int vertex = 0; vertex < n; ++vertex) {
      assert(0 <= this->next[vertex] && this->next[vertex] < n);
    }
    build_jump_table();
    analyze_cycles();
  }

  int jump(int start, unsigned long long steps) const {
    assert(0 <= start && start < n);
    int vertex = start;
    for (int bit = 0; bit < levels; ++bit) {
      if ((steps >> bit) & 1ULL) vertex = jump_table[bit][vertex];
    }
    return vertex;
  }

  int cycle_length(int vertex) const {
    assert(0 <= vertex && vertex < n);
    return static_cast<int>(cycles[cycle_id[vertex]].size());
  }

 private:
  void build_jump_table() {
    if (n == 0) return;
    jump_table[0] = next;
    for (int level = 1; level < levels; ++level) {
      for (int vertex = 0; vertex < n; ++vertex) {
        jump_table[level][vertex] =
            jump_table[level - 1][jump_table[level - 1][vertex]];
      }
    }
  }

  void analyze_cycles() {
    std::vector<int> indegree(n, 0);
    for (int vertex = 0; vertex < n; ++vertex) ++indegree[next[vertex]];
    std::vector<int> queue;
    std::vector<int> removed_order;
    queue.reserve(n);
    removed_order.reserve(n);
    for (int vertex = 0; vertex < n; ++vertex) {
      if (indegree[vertex] == 0) queue.push_back(vertex);
    }
    for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
      const int vertex = queue[head];
      removed_order.push_back(vertex);
      if (--indegree[next[vertex]] == 0) queue.push_back(next[vertex]);
    }

    for (int start = 0; start < n; ++start) {
      if (indegree[start] == 0 || cycle_id[start] != -1) continue;
      const int id = static_cast<int>(cycles.size());
      cycles.push_back({});
      int vertex = start;
      do {
        cycle_id[vertex] = id;
        cycle_position[vertex] = static_cast<int>(cycles.back().size());
        cycle_entry[vertex] = vertex;
        steps_to_cycle[vertex] = 0;
        cycles.back().push_back(vertex);
        vertex = next[vertex];
      } while (vertex != start);
    }

    for (int i = static_cast<int>(removed_order.size()) - 1; i >= 0; --i) {
      const int vertex = removed_order[i];
      const int successor = next[vertex];
      cycle_id[vertex] = cycle_id[successor];
      cycle_position[vertex] = cycle_position[successor];
      cycle_entry[vertex] = cycle_entry[successor];
      steps_to_cycle[vertex] = steps_to_cycle[successor] + 1;
    }
  }
};
