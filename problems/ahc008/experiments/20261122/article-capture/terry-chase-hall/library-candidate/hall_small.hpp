#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace ahc {

using WorkerMask = std::uint32_t;
inline constexpr int hall_small_max_workers = 16;

struct HallDeficiency {
    // The exact union of neighbors of these tasks, not merely a superset.
    WorkerMask worker_neighbors;
    std::vector<std::size_t> tasks;
};

// A worker can serve at most one task. nullopt iff all tasks can be
// assigned to distinct eligible workers. This does not return a matching.
inline std::optional<HallDeficiency> find_hall_deficiency(
    int workers, std::span<const WorkerMask> task_neighbors) {
    if (workers < 0 || workers > hall_small_max_workers)
        throw std::out_of_range("Hall worker count outside [0,16]");
    const auto subsets = WorkerMask{1} << workers;
    for (auto mask : task_neighbors)
        if (mask >= subsets)
            throw std::invalid_argument("task points to worker outside declared range");
    if (task_neighbors.empty()) return std::nullopt;

    std::vector<std::size_t> confined(subsets);
    for (auto mask : task_neighbors) ++confined[mask];
    // Subset zeta transform: confined[S] becomes the count of tasks
    // whose whole neighborhood is contained in S.
    for (WorkerMask bit = 1; bit < subsets; bit <<= 1)
        for (WorkerMask mask = 0; mask < subsets; ++mask)
            if (mask & bit) confined[mask] += confined[mask ^ bit];

    int best_size = workers + 1;
    WorkerMask best = 0;
    for (WorkerMask mask = 0; mask < subsets; ++mask) {
        const auto size = std::popcount(mask);
        if (size < best_size && confined[mask] > static_cast<std::size_t>(size)) {
            best = mask;
            best_size = size;
        }
    }
    if (best_size == workers + 1) return std::nullopt;

    HallDeficiency proof{0, {}};
    const auto needed = static_cast<std::size_t>(best_size + 1);
    proof.tasks.reserve(needed);
    for (std::size_t i = 0; i < task_neighbors.size(); ++i) {
        if ((task_neighbors[i] & ~best) != 0) continue;
        proof.tasks.push_back(i);
        proof.worker_neighbors |= task_neighbors[i];
        if (proof.tasks.size() == needed) break;
    }
    return proof;
}

} // namespace ahc
