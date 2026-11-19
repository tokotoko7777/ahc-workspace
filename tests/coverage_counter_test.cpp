#include <climits>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
#include "library/coverage-counter.hpp"

void require(bool condition) {
    if (!condition) throw std::runtime_error("coverage check failed");
}

template <class Error, class F>
void expect_error(F action) {
    bool caught = false;
    try { action(); } catch (const Error&) { caught = true; }
    require(caught);
}

int main() {
    CoverageCounter<long long> coverage({10, 20, 0});
    auto state = coverage.make_state();
    coverage.add(state, 0, 2);
    coverage.remove(state, 0);
    require(state.counts[0] == 1 && state.covered_weight == 10);
    coverage.remove(state, 0);
    require(state.covered_weight == 0);

    // 途中の-1ではなく、IDごとに全て集計した後の件数で評価する。
    coverage.add_delta(0, -1);
    coverage.add_delta(0, 1);
    coverage.add_delta(0, 2);
    coverage.add_delta(1, 3);
    require(coverage.score_delta(state) == 30);
    require(state.covered_weight == 0);
    coverage.apply_delta(state);
    require(state.counts == std::vector<int>({2, 3, 0}));
    require(coverage.score_delta(state) == 0);  // applyでbufferは空になる。

    // あるIDが不正でも、それ以前のIDだけが反映されることはない。
    const auto before = state;
    coverage.add_delta(0, 1);
    coverage.add_delta(1, -4);
    expect_error<std::invalid_argument>([&] { coverage.apply_delta(state); });
    require(state.counts == before.counts && state.covered_weight == before.covered_weight);
    coverage.clear_delta();
    expect_error<std::out_of_range>([&] { coverage.add_delta(3, 1); });
    expect_error<std::invalid_argument>([&] { coverage.remove(state, 0, 3); });
    expect_error<std::invalid_argument>([&] { coverage.add(state, 0, -1); });
    CoverageCounter<long long>::State wrong_size;
    expect_error<std::invalid_argument>([&] { coverage.score_delta(wrong_size); });
    auto huge = coverage.make_state();
    coverage.add(huge, 0, INT_MAX);
    expect_error<std::overflow_error>([&] { coverage.add(huge, 0); });
    coverage.add_delta(0, INT_MAX);
    expect_error<std::overflow_error>([&] { coverage.add_delta(0, 1); });
    coverage.clear_delta();
    CoverageCounter<long long> empty({});
    auto empty_state = empty.make_state();
    empty.apply_delta(empty_state);
    require(empty_state.covered_weight == 0);

    // 集合の被覆を全て数え直すoracleと比較。重複・複数group変更・棄却を含む。
    std::mt19937 rng(314159);
    std::vector<long long> weights(17);
    for (auto& weight : weights) weight = static_cast<int>(rng() % 20) - 3;
    CoverageCounter<long long> counter(weights);
    auto current = counter.make_state();
    std::vector<std::vector<int>> groups(14);
    const auto new_hits = [&] {
        std::vector<int> hits(rng() % 15);
        for (int& id : hits) id = rng() % weights.size();
        return hits;
    };
    const auto full_counts = [&](const auto& all_groups) {
        std::vector<int> counts(weights.size());
        for (const auto& hits : all_groups) for (int id : hits) ++counts[id];
        return counts;
    };
    const auto full_weight = [&](const auto& counts) {
        long long result = 0;
        for (std::size_t id = 0; id < weights.size(); ++id)
            if (counts[id] > 0) result += weights[id];
        return result;
    };
    for (auto& hits : groups) {
        hits = new_hits();
        for (int id : hits) counter.add(current, id);
    }
    for (int trial = 0; trial < 3000; ++trial) {
        const auto saved = current;
        auto next_groups = groups;
        counter.clear_delta();
        for (int i = 0; i < 3; ++i) {
            const int group = rng() % groups.size();
            for (int id : next_groups[group]) counter.add_delta(id, -1);
            next_groups[group] = new_hits();
            for (int id : next_groups[group]) counter.add_delta(id, 1);
        }
        const auto next_counts = full_counts(next_groups);
        const auto score = full_weight(next_counts);
        require(counter.score_delta(current) == score - full_weight(full_counts(groups)));
        require(current.counts == saved.counts && current.covered_weight == saved.covered_weight);
        if (rng() % 2) {
            counter.apply_delta(current);
            groups = std::move(next_groups);
        }
        require(current.counts == full_counts(groups));
        require(current.covered_weight == full_weight(current.counts));
    }
    std::cout << "coverage counter checks passed\n";
}
