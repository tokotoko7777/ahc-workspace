#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/two-sat.hpp

// 各変数がtrue/falseのどちらかで、各条件が「AまたはB」の形になる問題を解く。
// 使い方:
// TwoSat sat(variable_count);
// sat.add_clause(a, true, b, false);  // aがtrue、またはbがfalse
// if (sat.solve()) { auto answer = sat.answer(); }
struct TwoSat {
  int variable_count;
  std::vector<std::vector<int>> graph;
  std::vector<bool> assignment;
  bool solved = false;
  bool satisfiable = false;

  explicit TwoSat(int variable_count)
      : variable_count(variable_count),
        graph(variable_count * 2),
        assignment(variable_count, false) {
    assert(variable_count >= 0);
  }

  // (variable_a == value_a) OR (variable_b == value_b) を追加する。
  void add_clause(int variable_a, bool value_a, int variable_b,
                  bool value_b) {
    check_variable(variable_a);
    check_variable(variable_b);
    solved = false;
    const int a = literal(variable_a, value_a);
    const int b = literal(variable_b, value_b);
    graph[a ^ 1].push_back(b);
    graph[b ^ 1].push_back(a);
  }

  // (from == from_value) ならば (to == to_value)。
  void add_implication(int from, bool from_value, int to, bool to_value) {
    add_clause(from, !from_value, to, to_value);
  }

  void set_value(int variable, bool value) {
    add_clause(variable, value, variable, value);
  }

  bool solve() {
    const std::vector<int> component = component_ids();
    satisfiable = true;
    for (int variable = 0; variable < variable_count; ++variable) {
      const int is_true = literal(variable, true);
      const int is_false = literal(variable, false);
      if (component[is_true] == component[is_false]) {
        satisfiable = false;
        break;
      }
      assignment[variable] = component[is_true] > component[is_false];
    }
    solved = true;
    return satisfiable;
  }

  const std::vector<bool>& answer() const {
    assert(solved && satisfiable);
    return assignment;
  }

 private:
  void check_variable(int variable) const {
    assert(0 <= variable && variable < variable_count);
  }

  int literal(int variable, bool value) const {
    return variable * 2 + (value ? 0 : 1);
  }

  std::vector<int> component_ids() const {
    const int n = variable_count * 2;
    std::vector<std::vector<int>> reverse_graph(n);
    std::vector<int> reverse_degree(n, 0);
    for (const auto& edges : graph) {
      for (int next : edges) ++reverse_degree[next];
    }
    for (int vertex = 0; vertex < n; ++vertex) {
      reverse_graph[vertex].reserve(reverse_degree[vertex]);
    }
    for (int vertex = 0; vertex < n; ++vertex) {
      for (int next : graph[vertex]) reverse_graph[next].push_back(vertex);
    }

    std::vector<char> visited(n, false);
    std::vector<int> next_edge(n, 0);
    std::vector<int> finish_order;
    finish_order.reserve(n);
    std::vector<int> stack;
    stack.reserve(n);
    for (int start = 0; start < n; ++start) {
      if (visited[start]) continue;
      visited[start] = true;
      stack.push_back(start);
      while (!stack.empty()) {
        const int vertex = stack.back();
        if (next_edge[vertex] < static_cast<int>(graph[vertex].size())) {
          const int next = graph[vertex][next_edge[vertex]++];
          if (!visited[next]) {
            visited[next] = true;
            stack.push_back(next);
          }
        } else {
          finish_order.push_back(vertex);
          stack.pop_back();
        }
      }
    }

    std::vector<int> component(n, -1);
    int component_count = 0;
    for (int i = n - 1; i >= 0; --i) {
      const int start = finish_order[i];
      if (component[start] != -1) continue;
      component[start] = component_count;
      stack.push_back(start);
      while (!stack.empty()) {
        const int vertex = stack.back();
        stack.pop_back();
        for (int next : reverse_graph[vertex]) {
          if (component[next] != -1) continue;
          component[next] = component_count;
          stack.push_back(next);
        }
      }
      ++component_count;
    }
    return component;
  }
};
