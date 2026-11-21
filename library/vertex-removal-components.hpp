#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

// Preprocess a simple undirected graph. For any vertex, enumerate the pieces
// of its original connected component after removing it (other components
// are unaffected). Weight must be zero-initializable and support += and -=.
template<class Weight>
class VertexRemovalComponents {
public:
    explicit VertexRemovalComponents(int n)
        : enter_(n), leave_(n), low_(n), root_(n), next_(n),
          head_(n), sibling_(n), size_(n), residual_size_(n), active_(n),
          sub_(n), residual_(n) {
        stack_.reserve(n);
    }

    // neighbors(v) returns a span or a reference to an indexable range of
    // distinct adjacent IDs, with symmetric adjacency for active vertices.
    template<class Neighbors, class Accept, class GetWeight>
    void build(Neighbors neighbors, Accept accept, GetWeight weight) {
        const int n = static_cast<int>(enter_.size());
        std::fill(enter_.begin(), enter_.end(), -1);
        std::fill(head_.begin(), head_.end(), -1);
        int clock = 0;
        for (int v = 0; v < n; ++v) active_[v] = accept(v);
        const auto push = [&](int v, int root) {
            enter_[v] = low_[v] = clock++;
            root_[v] = root;
            size_[v] = 1;
            sub_[v] = weight(v);
            residual_[v] = Weight{};
            residual_size_[v] = 0;
            next_[v] = 0;
            stack_.push_back(v);
        };
        for (int i = 0; i < n; ++i) {
            if (!active_[i] || enter_[i] != -1) continue;
            push(i, i);
            while (!stack_.empty()) {
                const int v = stack_.back();
                const int parent = stack_.size() >= 2 ? stack_[stack_.size() - 2] : -1;
                const auto& edges = neighbors(v);
                if (next_[v] < static_cast<int>(edges.size())) {
                    const int u = edges[next_[v]++];
                    assert(0 <= u && u < n);
                    if (!active_[u] || u == parent) continue;
                    if (enter_[u] == -1) push(u, i);
                    else low_[v] = std::min(low_[v], enter_[u]);
                    continue;
                }
                leave_[v] = clock;
                stack_.pop_back();
                if (parent == -1) continue;
                low_[parent] = std::min(low_[parent], low_[v]);
                size_[parent] += size_[v];
                sub_[parent] += sub_[v];
                if (low_[v] >= enter_[parent]) {
                    sibling_[v] = head_[parent];
                    head_[parent] = v;
                    residual_[parent] -= sub_[v];
                    residual_size_[parent] -= size_[v];
                }
            }
        }
        for (int i = 0; i < n; ++i) {
            if (!active_[i]) continue;
            residual_[i] += sub_[root_[i]];
            residual_[i] -= weight(i);
            residual_size_[i] += size_[root_[i]] - 1;
        }
    }

    // fn(representative, number of vertices, weight). 'v' as representative
    // is a label for the residual side, not a vertex it contains.
    template<class Fn>
    void after_removing(int v, Fn fn) const {
        assert(active_[v]);
        for (int u = head_[v]; u != -1; u = sibling_[u]) fn(u, size_[u], sub_[u]);
        if (residual_size_[v] != 0) fn(v, residual_size_[v], residual_[v]);
    }

    // Returns the label emitted by after_removing for u, or -1 if u was
    // removed, is inactive, or was in a different component.
    int side_of(int v, int u) const {
        assert(active_[v]);
        if (!active_[u] || u == v || root_[u] != root_[v]) return -1;
        for (int c = head_[v]; c != -1; c = sibling_[c]) {
            if (enter_[c] <= enter_[u] && enter_[u] < leave_[c]) return c;
        }
        return v;
    }

    bool is_cut(int v) const {
        assert(active_[v]);
        const int c = head_[v];
        return c != -1 && (sibling_[c] != -1 || residual_size_[v] > 0);
    }

private:
    std::vector<int> enter_, leave_, low_, root_, next_, head_, sibling_, size_, residual_size_;
    std::vector<unsigned char> active_;
    std::vector<Weight> sub_, residual_;
    std::vector<int> stack_;
};
