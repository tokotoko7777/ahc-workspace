#include <algorithm>
#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/lowest-common-ancestor.hpp

// 根付き木のLCA、頂点間距離、パス上のk歩先を O(log N) で求める。
// 構築は O(N log N)。再帰を使わない。
struct LowestCommonAncestor {
  int n;
  int root;
  int levels = 1;
  std::vector<int> depth;
  std::vector<std::vector<int>> ancestor;

  LowestCommonAncestor(const std::vector<std::vector<int>>& tree, int root = 0)
      : n(static_cast<int>(tree.size())), root(root), depth(n, -1) {
    assert(0 <= root && root < n);
    while ((1LL << levels) <= n) ++levels;
    ancestor.assign(levels, std::vector<int>(n, root));

    std::vector<int> queue;
    queue.reserve(n);
    depth[root] = 0;
    ancestor[0][root] = root;
    queue.push_back(root);
    for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
      const int vertex = queue[head];
      for (int next : tree[vertex]) {
        assert(0 <= next && next < n);
        if (depth[next] != -1) continue;
        depth[next] = depth[vertex] + 1;
        ancestor[0][next] = vertex;
        queue.push_back(next);
      }
    }
    assert(static_cast<int>(queue.size()) == n);

    for (int level = 1; level < levels; ++level) {
      for (int vertex = 0; vertex < n; ++vertex) {
        ancestor[level][vertex] =
            ancestor[level - 1][ancestor[level - 1][vertex]];
      }
    }
  }

  int kth_ancestor(int vertex, int steps) const {
    assert(0 <= vertex && vertex < n);
    assert(0 <= steps && steps <= depth[vertex]);
    for (int level = 0; level < levels; ++level) {
      if ((steps >> level) & 1) vertex = ancestor[level][vertex];
    }
    return vertex;
  }

  int lca(int a, int b) const {
    assert(0 <= a && a < n && 0 <= b && b < n);
    if (depth[a] < depth[b]) std::swap(a, b);
    a = kth_ancestor(a, depth[a] - depth[b]);
    if (a == b) return a;
    for (int level = levels - 1; level >= 0; --level) {
      if (ancestor[level][a] == ancestor[level][b]) continue;
      a = ancestor[level][a];
      b = ancestor[level][b];
    }
    return ancestor[0][a];
  }

  int distance(int a, int b) const {
    const int common = lca(a, b);
    return depth[a] + depth[b] - 2 * depth[common];
  }

  // aからbへの単純パスをsteps歩進んだ頂点。0ならa、distance(a,b)ならb。
  int jump(int a, int b, int steps) const {
    const int common = lca(a, b);
    const int up_steps = depth[a] - depth[common];
    const int total_distance = up_steps + depth[b] - depth[common];
    assert(0 <= steps && steps <= total_distance);
    if (steps <= up_steps) return kth_ancestor(a, steps);
    return kth_ancestor(b, total_distance - steps);
  }
};
