#pragma once

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ahc {
/// Track a hidden goal on a unit-weight graph with uniformly tied departures.
/// A negative distance denotes an unreachable vertex. A zero root distance
/// denotes the current position; it is not eligible as a new target.
/// The caller computes distances and supplies the problem's completion rule.
/// A disconnected stored target is redrawn uniformly among reachable others.
template<std::size_t Vertices>
struct HiddenDestinationFilter {
    std::array<double, Vertices> mass{};
    double redrawn = 1.0;

    template<std::size_t Moves, class Distance, class Finished>
    std::array<double, Moves> observe(
        const std::array<Distance, Vertices>& root_distance,
        const std::array<std::array<Distance, Vertices>, Moves>& neighbor_distance,
        std::uint64_t enabled, std::size_t observed, Finished finished) {
        static_assert(Moves > 0 && Moves <= 64);
        static_assert(std::is_signed_v<Distance>, "unreachable is negative");
        if constexpr (Moves < 64) enabled &= (std::uint64_t{1} << Moves) - 1;
        std::array<double, Moves> likelihood{};
        std::array<double, Vertices> posterior{};
        std::size_t eligible = 0;
        double resampled = redrawn;
        for (std::size_t v = 0; v < Vertices; ++v) {
            if (root_distance[v] > 0) ++eligible;
            else resampled += mass[v];
        }
        if (eligible == 0) {
            if (observed < Moves) *this = {};
            return likelihood;
        }
        const double prior = resampled / eligible;
        double next_redrawn = 0.;
        for (std::size_t v = 0; v < Vertices; ++v) {
            if (root_distance[v] <= 0) continue;
            std::uint64_t shortest = 0;
            for (std::uint64_t left = enabled; left; left &= left - 1) {
                const std::size_t k = std::countr_zero(left);
                assert(k < Moves);
                if (neighbor_distance[k][v] >= 0 &&
                    neighbor_distance[k][v] < root_distance[v]) {
                    shortest |= std::uint64_t{1} << k;
                }
            }
            assert(shortest && "distances and enabled neighbors must be consistent");
            if (!shortest) continue;
            const double share = (mass[v] + prior) / std::popcount(shortest);
            for (std::uint64_t left = shortest; left; left &= left - 1) {
                likelihood[std::countr_zero(left)] += share;
            }
            if (observed < Moves && (shortest & (std::uint64_t{1} << observed))) {
                if (finished(v)) next_redrawn += share;
                else posterior[v] = share;
            }
        }
        const double likelihood_of_observation = observed < Moves ? likelihood[observed] : 0.;
        // In an inconsistent or unobserved turn, do not invent evidence.
        // Out-of-range observation provides the forecast without changing the state.
        if (observed >= Moves) return likelihood;
        if (likelihood_of_observation == 0.) {
            *this = {};
            return likelihood;
        }
        const double inverse = 1. / likelihood_of_observation;
        redrawn = next_redrawn * inverse;
        for (std::size_t v = 0; v < Vertices; ++v) mass[v] = posterior[v] * inverse;
        return likelihood;
    }
};
}
