#pragma once

#include <algorithm>
#include <cassert>
#include <ranges>
#include <span>
#include <vector>

namespace retained_route {

struct Stretch {
    int retained_size = 0;
    int boundary_size = 0;
    int max_extra = 0;
    int from = -1;
    int to = -1;
    int before = 0;
    int after = 0;
};

// An unweighted undirected graph whose vertices are 0 .. adj.size()-1.
// Existing walls may be represented by isolated vertices. "Removed" contains
// newly deleted vertices only and may contain duplicates. An unremoved anchor
// specifies which connected component to retain. All neighbor IDs must be valid.
template<class Adjacency>
Stretch maximum_stretch(const Adjacency& adj, std::span<const int> removed, int anchor) {
    const int n = static_cast<int>(adj.size());
    assert(anchor >= 0 && anchor < n);
    std::vector<unsigned char> blocked(n, 0), marked(n, 0);
    std::vector<int> gates;
    for (const int u : removed) {
        assert(u >= 0 && u < n && u != anchor);
        if (!blocked[u]) {
            blocked[u] = 1;
            gates.push_back(u);
        }
    }
    std::vector<int> old(n), now(n), queue(n), boundary;
    auto bfs = [&](int start, bool cut, std::vector<int>& dist) {
        std::ranges::fill(dist, -1);
        int end = 1;
        queue[0] = start;
        dist[start] = 0;
        for (int at = 0; at < end; ++at) {
            const int u = queue[at];
            for (const int v : adj[u]) {
                if ((cut && blocked[v]) || dist[v] >= 0) continue;
                dist[v] = dist[u] + 1;
                queue[end++] = v;
            }
        }
        return end;
    };
    Stretch out;
    out.retained_size = bfs(anchor, true, now);
    for (const int gate : gates) {
        for (const int u : adj[gate]) {
            if (now[u] < 0 || marked[u]) continue;
            marked[u] = 1;
            boundary.push_back(u);
        }
    }
    std::ranges::sort(boundary);
    out.boundary_size = static_cast<int>(boundary.size());
    for (int i = 0; i + 1 < out.boundary_size; ++i) {
        const int a = boundary[i];
        bfs(a, true, now);
        if (gates.size() == 1) {
            // Distinct boundary vertices share the removed neighbor and had
            // original distance two, except when there is a direct edge.
            for (const int b : boundary) old[b] = 2;
            for (const int b : adj[a]) old[b] = 1;
        } else {
            bfs(a, false, old);
        }
        for (int j = i + 1; j < out.boundary_size; ++j) {
            const int b = boundary[j];
            const int delta = now[b] - old[b];
            if (delta <= out.max_extra) continue;
            out.max_extra = delta;
            out.from = a;
            out.to = b;
            out.before = old[b];
            out.after = now[b];
        }
    }
    return out;
}

} // namespace retained_route
