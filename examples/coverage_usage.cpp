// AHC_SOURCE: examples/coverage_usage.cpp
// Source repository: https://github.com/tokotoko7777/ahc-workspace
// 入力不要の接続例。実問題ではgroupsの生成だけを、行のヒット・視界・住民被覆等へ変える。
#include <cassert>
#include <iostream>
#include <vector>
#include "library/coverage-counter.hpp"

int main() {
    // 対象0..2の重み。groupsには同じ対象の複数出現を残す。
    CoverageCounter<long long> coverage({10, 20, 30});
    auto state = coverage.make_state();
    std::vector<std::vector<int>> groups{{0, 0, 1}, {0, 2}};
    for (const auto& hits : groups)
        for (int id : hits) coverage.add(state, id);
    assert(state.covered_weight == 60);

    // TODO(実問題): 変更した行・アンテナ等の、新しいヒット一覧を作る。
    // group 0だけを {0, 2, 2} に置換。対象0は3→2なので得点維持。
    const std::vector<int> next_hits{0, 2, 2};
    coverage.clear_delta();
    for (int id : groups[0]) coverage.add_delta(id, -1);
    for (int id : next_hits) coverage.add_delta(id, +1);
    const auto improvement = coverage.score_delta(state);
    assert(improvement == -20);  // 対象1だけが1→0になり失点。
    assert(state.covered_weight == 60);  // 仮評価は現在解を変更しない。

    // TODO(実問題): sa.accept(improvement)等で採否を決める。
    const bool accepted = true;  // この例では反映結果を確認するため採用。
    if (accepted) {
        coverage.apply_delta(state);
        groups[0] = next_hits;
    }
    // 不採用ならstate・groupsを触らず、次の試行をclear_deltaから始める。
    assert(state.covered_weight == 40);
    std::cout << "covered_weight=" << state.covered_weight << '\n';
}
