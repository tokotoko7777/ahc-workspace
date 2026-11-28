#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <span>

namespace ahc {
enum class TinyComponentStatus { invalid_start, complete, over_limit };

template <std::size_t Limit>
struct TinyComponent {
    TinyComponentStatus status = TinyComponentStatus::invalid_start;
    std::array<int, Limit> ids{};
    std::size_t count = 0;

    // Partial traversal is never a certified component.
    constexpr std::span<const int> complete_vertices() const & {
        return status == TinyComponentStatus::complete
            ? std::span<const int>(ids.data(), count) : std::span<const int>{};
    }
    std::span<const int> complete_vertices() const && = delete;
};

// Enumerate directed or undirected out-reachability without a |V|-sized
// visited array. Small Limit (usually <= 8) is the intended use.
template <std::size_t Vertices, std::size_t Limit, class Neighbors, class Allowed>
constexpr TinyComponent<Limit> collect_tiny_component(
        int start, Neighbors&& neighbors, Allowed&& allowed) {
    static_assert(0 < Limit && Limit <= Vertices);
    auto valid = [](int p) {
        return p >= 0 && static_cast<std::size_t>(p) < Vertices;
    };
    TinyComponent<Limit> result;
    if (!valid(start) || !allowed(start)) return result;
    result.ids[0] = start;
    result.count = 1;
    for (std::size_t head = 0; head < result.count; ++head) {
        for (int next : neighbors(result.ids[head])) {
            if (!valid(next) || !allowed(next)) continue;
            auto end = result.ids.begin() + result.count;
            if (std::find(result.ids.begin(), end, next) != end) continue;
            if (result.count == Limit) {
                result.status = TinyComponentStatus::over_limit;
                return result;
            }
            result.ids[result.count++] = next;
        }
    }
    result.status = TinyComponentStatus::complete;
    return result;
}
}  // namespace ahc
