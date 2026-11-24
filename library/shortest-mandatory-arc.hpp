#pragma once

#include <bit>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace ahc {

// Fixed target in a unit-length digraph. Arc identity is its input index.
class shortest_mandatory_arc {
public:
    struct arc { int tail, head; };

    shortest_mandatory_arc(int vertices, std::span<const arc> arcs, int target) {
        if (vertices <= 0 || target < 0 || target >= vertices) {
            throw std::invalid_argument("invalid size or target");
        }
        if (arcs.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::invalid_argument("too many arcs for int identifiers");
        }
        mandatory_tail_.assign(arcs.size(), -1);
        distance_.assign(vertices, -1);
        entry_.assign(vertices, -1);
        exit_.assign(vertices, -1);
        std::vector<std::vector<int>> incoming(vertices), outgoing(vertices);
        for (int i = 0; i < static_cast<int>(arcs.size()); ++i) {
            const auto [u, v] = arcs[i];
            if (u < 0 || v < 0 || u >= vertices || v >= vertices) {
                throw std::invalid_argument("arc endpoint out of range");
            }
            incoming[v].push_back(u);
            outgoing[u].push_back(i);
        }
        std::vector<int> order{target};
        order.reserve(vertices);
        distance_[target] = 0;
        for (std::size_t i = 0; i < order.size(); ++i) {
            const int v = order[i];
            for (int u : incoming[v]) if (distance_[u] < 0) {
                distance_[u] = distance_[v] + 1;
                order.push_back(u);
            }
        }

        const int bits = static_cast<int>(std::bit_width(static_cast<unsigned>(order.size())));
        std::vector<std::vector<int>> up(bits, std::vector<int>(vertices, target));
        std::vector<int> depth(vertices);
        std::vector<std::vector<int>> children(vertices);
        const auto lca = [&](int a, int b) {
            if (depth[a] < depth[b]) { int x = a; a = b; b = x; }
            int difference = depth[a] - depth[b];
            for (int k = 0; k < bits; ++k) if ((difference >> k) & 1) a = up[k][a];
            if (a == b) return a;
            for (int k = bits - 1; k >= 0; --k) if (up[k][a] != up[k][b]) {
                a = up[k][a]; b = up[k][b];
            }
            return up[0][a];
        };
        for (std::size_t i = 1; i < order.size(); ++i) {
            const int u = order[i];
            int par = -1, count = 0, single_arc = -1;
            for (int e : outgoing[u]) {
                const int v = arcs[e].head;
                if (distance_[v] != distance_[u] - 1) continue;
                par = par < 0 ? v : lca(par, v);
                ++count;
                single_arc = e;
            }
            // A reachable non-target has at least one descending arc.
            if (count == 1) mandatory_tail_[single_arc] = u;
            depth[u] = depth[par] + 1;
            up[0][u] = par;
            for (int k = 1; k < bits; ++k) up[k][u] = up[k - 1][up[k - 1][u]];
            children[par].push_back(u);
        }
        struct Frame { int node; std::size_t next; };
        std::vector<Frame> stack{{target, 0}};
        int timer = 0;
        entry_[target] = timer++;
        while (!stack.empty()) {
            auto& f = stack.back();
            if (f.next == children[f.node].size()) {
                exit_[f.node] = timer;
                stack.pop_back();
                continue;
            }
            const int v = children[f.node][f.next++];
            entry_[v] = timer++;
            stack.push_back({v, 0});
        }
    }

    // -1 if source cannot reach the fixed target.
    [[nodiscard]] int distance(int source) const {
        validate_source(source);
        return distance_[source];
    }

    // nullopt if there is no source-to-target path. False for source == target,
    // or when at least one shortest path avoids this particular arc.
    [[nodiscard]] std::optional<bool> mandatory(int source, int arc_id) const {
        validate_source(source);
        if (arc_id < 0 || static_cast<std::size_t>(arc_id) >= mandatory_tail_.size()) {
            throw std::out_of_range("arc id out of range");
        }
        if (distance_[source] < 0) return std::nullopt;
        const int u = mandatory_tail_[arc_id];
        return u >= 0 && entry_[u] <= entry_[source] && entry_[source] < exit_[u];
    }

private:
    std::vector<int> distance_, entry_, exit_, mandatory_tail_;

    void validate_source(int u) const {
        if (u < 0 || static_cast<std::size_t>(u) >= distance_.size()) {
            throw std::out_of_range("source out of range");
        }
    }
};
} // namespace ahc
