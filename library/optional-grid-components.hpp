#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace ahc {
enum class grid_cell : std::uint8_t { blocked, fixed_open, optional };

template <std::size_t Dimensions>
struct grid_component_weight {
    std::size_t cells = 0;
    std::array<std::int64_t, Dimensions> weight{};
    bool operator==(const grid_component_weight&) const = default;
};

// Fixed four-neighbor grid and cell weights. Each query independently enables
// some of the optional cells. Cell and mask ordering is row-major.
template <std::size_t Dimensions = 0>
class optional_grid_components {
public:
    using summary_type = grid_component_weight<Dimensions>;
    using weight_type = std::array<std::int64_t, Dimensions>;
private:
    using index = std::size_t;
    static constexpr index nil = std::numeric_limits<index>::max();
    struct edge { index to, next; };
    std::vector<index> optional_, head_;
    std::vector<edge> edges_;
    std::vector<summary_type> summaries_, unaffected_;

    index node(summary_type weight) {
        const index id = summaries_.size();
        summaries_.push_back(weight);
        head_.push_back(nil);
        return id;
    }
    void link(index a, index b) {
        edges_.push_back({b, head_[a]});
        head_[a] = edges_.size() - 1;
        edges_.push_back({a, head_[b]});
        head_[b] = edges_.size() - 1;
    }
public:
    optional_grid_components(index height, index width,
                             std::span<const grid_cell> cells,
                             std::span<const weight_type> weights = {}) {
        if ((height && width > nil / height) ||
            cells.size() != height * width || (Dimensions != 0 && weights.size() != cells.size()) ||
            (Dimensions == 0 && !weights.empty())) {
            throw std::invalid_argument("grid dimensions or weight length");
        }
        const index n = cells.size();
        std::vector<index> id(n, nil), queue;
        std::vector<std::uint8_t> seen(n);
        for (index p = 0; p < n; ++p) {
            switch (cells[p]) {
            case grid_cell::blocked:
            case grid_cell::fixed_open:
                break;
            case grid_cell::optional: {
                optional_.push_back(p);
                summary_type one{1, {}};
                if constexpr (Dimensions != 0) one.weight = weights[p];
                id[p] = node(one);
                break;
            }
            default:
                throw std::invalid_argument("invalid grid cell kind");
            }
        }
        const auto each_neighbor = [&](index p, auto&& fn) {
            if (p >= width) fn(p - width);
            if (p % width + 1 < width) fn(p + 1);
            if (p < n - width) fn(p + width);
            if (p % width != 0) fn(p - 1);
        };
        for (index p = 0; p < n; ++p) {
            if (cells[p] != grid_cell::fixed_open || seen[p]) continue;
            queue.clear();
            queue.push_back(p);
            seen[p] = 1;
            summary_type sum;
            bool touches_optional = false;
            for (index i = 0; i < queue.size(); ++i) {
                const index u = queue[i];
                ++sum.cells;
                for (index d = 0; d < Dimensions; ++d) sum.weight[d] += weights[u][d];
                each_neighbor(u, [&](index v) {
                    if (cells[v] == grid_cell::optional) touches_optional = true;
                    if (cells[v] == grid_cell::fixed_open && !seen[v]) {
                        seen[v] = 1;
                        queue.push_back(v);
                    }
                });
            }
            if (!touches_optional) {
                unaffected_.push_back(sum);
            } else {
                const index j = node(sum);
                for (const index u : queue) id[u] = j;
            }
        }
        for (const index p : optional_) {
            each_neighbor(p, [&](index q) {
                if (id[q] == nil) return;
                // Both endpoints iterate an optional-optional edge. A fixed
                // component is never an outer-loop endpoint, so add all
                // optional-to-fixed incidences (parallel edges are harmless).
                if (cells[q] != grid_cell::optional || p < q) link(id[p], id[q]);
            });
        }
    }

    [[nodiscard]] std::span<const index> optional_cells() const noexcept {
        return optional_;
    }
    // Fixed components not adjacent to any optional cell. Use once in an
    // additive objective; append once per result only if a full list is needed.
    [[nodiscard]] std::span<const summary_type> unaffected() const noexcept {
        return unaffected_;
    }

    // enabled[i] is 0 (closed) or 1 (open) for optional_cells()[i].
    // Includes affected fixed components even when every optional cell is
    // closed. Result order is unspecified; never includes unaffected().
    [[nodiscard]] std::vector<summary_type>
    affected(std::span<const std::uint8_t> enabled) const {
        if (enabled.size() != optional_.size()) {
            throw std::invalid_argument("optional mask length");
        }
        std::vector<std::uint8_t> seen(summaries_.size());
        for (index p = 0; p < enabled.size(); ++p) {
            if (enabled[p] > 1) throw std::invalid_argument("optional flag must be 0 or 1");
            seen[p] = static_cast<std::uint8_t>(1 - enabled[p]); // disabled nodes never enter the queue
        }
        std::vector<summary_type> out;
        std::vector<index> queue;
        for (index p = 0; p < seen.size(); ++p) {
            if (seen[p]) continue;
            queue.clear();
            queue.push_back(p);
            seen[p] = 1;
            summary_type sum;
            for (index i = 0; i < queue.size(); ++i) {
                const index u = queue[i];
                sum.cells += summaries_[u].cells;
                for (index d = 0; d < Dimensions; ++d) sum.weight[d] += summaries_[u].weight[d];
                for (index j = head_[u]; j != nil; j = edges_[j].next) {
                    const index v = edges_[j].to;
                    if (!seen[v]) {
                        seen[v] = 1;
                        queue.push_back(v);
                    }
                }
            }
            out.push_back(sum);
        }
        return out;
    }
};
} // namespace ahc
