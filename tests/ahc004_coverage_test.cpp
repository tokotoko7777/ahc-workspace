#define AHC004_TEST
#define AHC_DISABLE_GCC_OPTIMIZE
#include "examples/search/ahc004_genome_sa.cpp"

void require(bool condition) {
    if (!condition) throw runtime_error("AHC004 coverage check failed");
}

// AhoCorasick・差分cacheを使わず、全開始位置から文字を直接照合する。
pair<vector<int>, int> full_counts(const vector<string>& board, const vector<Target>& targets) {
    const int n = board.size();
    vector<int> counts(targets.size());
    int empty = 0;
    for (const auto& row : board) empty += count(row.begin(), row.end(), '.');
    for (int r = 0; r < n; ++r) for (int c = 0; c < n; ++c)
        for (int direction = 0; direction < 2; ++direction)
            for (int id = 0; id < static_cast<int>(targets.size()); ++id) {
                bool match = true;
                for (int k = 0; k < static_cast<int>(targets[id].text.size()); ++k) {
                    const int rr = (r + (direction == 1 ? k : 0)) % n;
                    const int cc = (c + (direction == 0 ? k : 0)) % n;
                    if (board[rr][cc] != targets[id].text[k]) { match = false; break; }
                }
                counts[id] += match;
            }
    return {counts, empty};
}

int main() {
    mt19937 rng(271828);
    vector<Target> targets{{"AA", 2}, {"AAA", 3}, {"AB", 1}, {"BA", 4}, {"BB", 2}};
    const int n = 20;
    int m = 0;
    for (const auto& target : targets) m += target.frequency;
    GenomeProblem problem(n, m, targets);
    vector<string> board(n, string(n, 'A'));
    auto state = problem.make_state(board);
    const auto covered_weight = [&](const vector<int>& counts) {
        int result = 0;
        for (int id = 0; id < static_cast<int>(targets.size()); ++id)
            if (counts[id]) result += targets[id].frequency;
        return result;
    };
    for (int trial = 0; trial < 300; ++trial) {
        auto [old_counts, old_empty] = full_counts(state.board, targets);
        require(state.coverage.counts == old_counts);
        require(state.coverage.covered_weight == covered_weight(old_counts));
        require(state.empty == old_empty);
        const auto saved = state;
        GenomeProblem::Move move;
        auto candidate = state.board;
        const int updates = 1 + rng() % 25;
        for (int i = 0; i < updates; ++i) {
            const int cell = rng() % (n * n);
            const char value = "AB."[rng() % 3];
            move.changes.emplace_back(cell, value);
            candidate[cell / n][cell % n] = value;
        }
        const auto [next_counts, next_empty] = full_counts(candidate, targets);
        const double expected = problem.score(covered_weight(next_counts), next_empty)
                              - problem.score(covered_weight(old_counts), old_empty);
        const auto difference = problem.evaluate_move(state, move, -INFINITY);
        require(difference && abs(*difference - expected) < 1e-12);
        require(state.board == saved.board && state.line_hits == saved.line_hits);
        require(state.coverage.counts == saved.coverage.counts);
        require(state.coverage.covered_weight == saved.coverage.covered_weight);
        require(state.line_code == saved.line_code && state.line_empty == saved.line_empty);
        require(state.empty == saved.empty);
        if (rng() % 2) {
            problem.apply_move(state, move);
            require(state.board == candidate);
            require(state.coverage.counts == next_counts);
            require(state.coverage.covered_weight == covered_weight(next_counts));
            require(state.empty == next_empty);
            const auto rebuilt = problem.make_state(candidate);
            require(state.line_hits == rebuilt.line_hits);
            require(state.line_code == rebuilt.line_code && state.line_empty == rebuilt.line_empty);
        }
    }
    cout << "AHC004 full-recomputation checks passed\n";
}
