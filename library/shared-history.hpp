#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/shared-history.hpp

// 操作列の共通部分を親番号で共有する。
// root() は「まだ操作がない状態」を表す。
//
// 使い方:
// SharedHistory<Move> history;
// int node = history.root();
// node = history.add(node, move);
// vector<Move> answer = history.restore(node);
template <class Action>
struct SharedHistory {
  struct Node {
    int parent;
    Action action;
  };

  std::vector<Node> nodes;

  int root() const { return -1; }

  int add(int parent, Action action) {
    assert(-1 <= parent && parent < static_cast<int>(nodes.size()));
    nodes.push_back({parent, std::move(action)});
    return static_cast<int>(nodes.size()) - 1;
  }

  std::vector<Action> restore(int node) const {
    assert(-1 <= node && node < static_cast<int>(nodes.size()));
    std::vector<Action> actions;
    while (node != root()) {
      actions.push_back(nodes[node].action);
      node = nodes[node].parent;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
  }
};
