#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
constexpr int SIDE = 50, CELLS = SIDE * SIDE;

struct Random {
    uint64_t state;
    explicit Random(uint64_t seed) : state(seed + 0x9e3779b97f4a7c15ULL) {}
    uint32_t next() {
        state ^= state << 7;
        state ^= state >> 9;
        return uint32_t(state);
    }
    int integer(int n) { return int(uint64_t(next()) * uint32_t(n) >> 32); }
    double real() { return (next() + 0.5) / 4294967296.0; }
};

struct Solver {
    std::array<int, CELLS> tile{}, points{}, position{}, distance{};
    std::array<std::array<int, 4>, CELLS> adjacent{};
    std::array<int, CELLS> degree{};
    std::array<unsigned char, CELLS> used{};
    std::vector<int> path, best_path;
    Random random;
    Clock::time_point beginning;
    double seconds;
    int variant, score = 0, best_score = -1;
    long long rotations = 0, cuts = 0, rebuilds = 0, accepted = 0, dfs_nodes = 0;
    long long changed = 0, improved = 0;
    long long internal_reversals = 0;

    int target = -1, max_edges = 0, node_budget = 0, nodes = 0;
    int replacement_score = -1;
    std::vector<int> stack, replacement;
    double direction_weight = 0;
    double length_bonus = 0;
    std::array<int, CELLS> target_tile_stamp{};
    int search_stamp = 0, free_target_neighbors = 0;
    long long blocked_target_prunes = 0;

    Solver(double budget, uint64_t seed, int mode)
        : random(seed), beginning(Clock::now()), seconds(budget), variant(mode) {
        position.fill(-1);
        path.reserve(CELLS);
        best_path.reserve(CELLS);
        stack.reserve(CELLS);
        replacement.reserve(CELLS);
    }

    double elapsed() const {
        return std::chrono::duration<double>(Clock::now() - beginning).count();
    }

    void read() {
        int row, column;
        std::cin >> row >> column;
        for (int &value : tile) std::cin >> value;
        for (int &value : points) std::cin >> value;
        const int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
        for (int v = 0; v < CELLS; ++v) {
            for (int d = 0; d < 4; ++d) {
                int r = v / SIDE + dr[d], c = v % SIDE + dc[d];
                if (r < 0 || r >= SIDE || c < 0 || c >= SIDE) continue;
                int u = r * SIDE + c;
                if (tile[v] != tile[u]) adjacent[v][degree[v]++] = u;
            }
        }
        append(row * SIDE + column);
        remember();
    }

    void append(int v) {
        position[v] = int(path.size());
        path.push_back(v);
        used[tile[v]] = 1;
        score += points[v];
    }

    void reindex() {
        used.fill(0);
        position.fill(-1);
        score = 0;
        for (int i = 0; i < int(path.size()); ++i) {
            int v = path[i];
            position[v] = i;
            used[tile[v]] = 1;
            score += points[v];
        }
    }

    void remember() {
        if (score > best_score) {
            best_score = score;
            best_path = path;
        }
    }

    void extend() {
        while (true) {
            int v = path.back(), choice = -1;
            double best_key = -1e30;
            for (int d = 0; d < degree[v]; ++d) {
                int u = adjacent[v][d];
                if (used[tile[u]]) continue;
                int exits = 0;
                for (int e = 0; e < degree[u]; ++e)
                    exits += !used[tile[adjacent[u][e]]];
                double key = points[u] * 0.22 - exits * 9.0 + random.real() * 28.0;
                if (!exits) key -= 35.0;
                if (key > best_key) { best_key = key; choice = u; }
            }
            if (choice < 0) return;
            append(choice);
        }
    }

    void rotate_end() {
        int v = path.back();
        int choices[4], count = 0;
        for (int d = 0; d < degree[v]; ++d) {
            int i = position[adjacent[v][d]];
            if (i >= 0 && i + 2 < int(path.size())) choices[count++] = i;
        }
        if (!count) return;
        int first = choices[random.integer(count)] + 1;
        std::reverse(path.begin() + first, path.end());
        for (int i = first; i < int(path.size()); ++i) position[path[i]] = i;
        ++rotations;
        extend();
    }

    void reverse_internal_segment() {
        if (path.size() < 5) return;
        int first = random.integer(int(path.size()) - 1);
        int a = path[first], b = path[first + 1];
        if (!degree[a]) return;
        int c = adjacent[a][random.integer(degree[a])];
        int last = position[c];
        if (last < 0 || last + 1 >= int(path.size()) || std::abs(last - first) <= 1) return;
        int d = path[last + 1];
        if (std::abs(b / SIDE - d / SIDE) + std::abs(b % SIDE - d % SIDE) != 1) return;
        if (first > last) std::swap(first, last);
        std::reverse(path.begin() + first + 1, path.begin() + last + 1);
        for (int i = first + 1; i <= last; ++i) position[path[i]] = i;
        ++internal_reversals;
    }

    bool accept(double change, double temperature) {
        return change >= 0 || random.real() < std::exp(change / temperature);
    }

    void cut_end(double temperature) {
        if (path.size() < 3) return;
        const std::vector<int> previous = path;
        int previous_score = score;
        int limit = random.integer(8) == 0 ? 100 : 20;
        int remove = 1 + random.integer(std::min(limit, int(path.size()) - 1));
        while (remove--) {
            int v = path.back();
            path.pop_back();
            used[tile[v]] = 0;
            position[v] = -1;
            score -= points[v];
        }
        extend();
        ++cuts;
        double change = score - previous_score + length_bonus * (double(path.size()) - previous.size());
        if (score <= best_score && !accept(change, temperature)) {
            path = previous;
            reindex();
        }
    }

    void compute_distances(int start) {
        distance.fill(CELLS + 1);
        int queue[CELLS], begin = 0, end = 0;
        queue[end++] = target;
        distance[target] = 0;
        while (begin < end) {
            int v = queue[begin++];
            if (distance[v] >= max_edges) continue;
            for (int d = 0; d < degree[v]; ++d) {
                int u = adjacent[v][d];
                if ((used[tile[u]] && u != start) || distance[u] <= max_edges) continue;
                distance[u] = distance[v] + 1;
                queue[end++] = u;
            }
        }
    }

    void search_replacement(int v, int value) {
        if (++nodes > node_budget) return;
        int depth = int(stack.size());
        if (distance[v] > max_edges - depth) return;
        if (variant >= 5 && !free_target_neighbors) {
            if (distance[v] == 1 && (replacement_score < 0 ||
                value + length_bonus * stack.size() > replacement_score + length_bonus * replacement.size())) {
                replacement_score = value;
                replacement = stack;
            }
            ++blocked_target_prunes;
            return;
        }
        int choices[4], count = 0;
        double keys[4];
        for (int d = 0; d < degree[v]; ++d) {
            int u = adjacent[v][d];
            if (u == target) {
                if (depth < max_edges && (replacement_score < 0 ||
                    value + length_bonus * stack.size() > replacement_score + length_bonus * replacement.size())) {
                    replacement_score = value;
                    replacement = stack;
                }
                continue;
            }
            if (used[tile[u]] || distance[u] > max_edges - depth - 1) continue;
            double key = random.real() * 30.0 + points[u] * 0.08 - distance[u] * direction_weight;
            int at = count++;
            while (at && key > keys[at - 1]) {
                keys[at] = keys[at - 1]; choices[at] = choices[at - 1]; --at;
            }
            keys[at] = key; choices[at] = u;
        }
        for (int i = 0; i < count && nodes <= node_budget; ++i) {
            int u = choices[i];
            used[tile[u]] = 1;
            if (variant >= 5 && target_tile_stamp[tile[u]] == search_stamp) --free_target_neighbors;
            stack.push_back(u);
            search_replacement(u, value + points[u]);
            stack.pop_back();
            used[tile[u]] = 0;
            if (variant >= 5 && target_tile_stamp[tile[u]] == search_stamp) ++free_target_neighbors;
        }
    }

    void rebuild(double temperature) {
        if (path.size() < 4) return;
        int left = random.integer(int(path.size()) - 2);
        int span = 2 + random.integer(random.integer(6) ? 30 : 90);
        if (variant >= 2) {
            int range = variant == 3 || variant >= 5 ? 50 : 30;
            span = random.integer(4) == 0 ? 1 : 2 + random.integer(random.integer(6) ? range : 90);
        }
        if ((variant == 4 || variant >= 6) && random.integer(4) != 0) {
            for (int trial = 0; trial < 8; ++trial) {
                int i = random.integer(int(path.size()) - 1);
                int v = path[i];
                bool frontier = false;
                for (int d = 0; d < degree[v]; ++d) frontier |= !used[tile[adjacent[v][d]]];
                if (frontier) {
                    left = std::max(0, i - random.integer(std::max(1, span)));
                    break;
                }
            }
        }
        int right = std::min(int(path.size()) - 1, left + span);
        span = right - left;
        int previous_score = 0;
        for (int i = left + 1; i < right; ++i) {
            used[tile[path[i]]] = 0;
            previous_score += points[path[i]];
        }
        target = path[right];
        max_edges = std::min(180, span + 12 + random.integer(25));
        node_budget = 800 + 50 * span;
        if (variant == 3 || variant >= 5) {
            max_edges = std::min(200, span + 20 + random.integer(31));
            node_budget = 1500 + 80 * span;
        }
        direction_weight = random.real() * 2.5;
        nodes = 0;
        replacement_score = -1;
        replacement.clear();
        stack.clear();
        if (variant >= 5) {
            ++search_stamp;
            free_target_neighbors = 0;
            for (int d = 0; d < degree[target]; ++d) {
                int t = tile[adjacent[target][d]];
                target_tile_stamp[t] = search_stamp;
                free_target_neighbors += !used[t];
            }
        }
        compute_distances(path[left]);
        search_replacement(path[left], 0);
        ++rebuilds;
        dfs_nodes += nodes;
        if (variant >= 2 && int(replacement.size()) == right - left - 1 &&
            std::equal(replacement.begin(), replacement.end(), path.begin() + left + 1)) {
            for (int i = left + 1; i < right; ++i) used[tile[path[i]]] = 1;
            return;
        }
        double change = replacement_score - previous_score + length_bonus * (int(replacement.size()) - (right - left - 1));
        if (replacement_score >= 0 && (score + replacement_score - previous_score > best_score || accept(change, temperature))) {
            std::vector<int> next;
            next.reserve(path.size() + replacement.size());
            next.insert(next.end(), path.begin(), path.begin() + left + 1);
            next.insert(next.end(), replacement.begin(), replacement.end());
            next.insert(next.end(), path.begin() + right, path.end());
            path.swap(next);
            reindex();
            ++accepted;
            ++changed;
            improved += replacement_score > previous_score;
        } else {
            for (int i = left + 1; i < right; ++i) used[tile[path[i]]] = 1;
        }
    }

    void solve() {
        extend();
        remember();
        long long iteration = 0;
        while (true) {
            double progress = elapsed() / seconds;
            if (progress >= 1) break;
            double temperature = 180.0 * (1 - progress) + 5.0;
            if (variant == 2 || variant == 4) temperature = 90.0 * (1 - progress) + 2.0;
            if (variant == 3 || variant >= 5) temperature = 250.0 * (1 - progress) + 5.0;
            if (variant == 8) temperature = 500.0 * std::pow(0.005, progress);
            if (variant == 9) length_bonus = 30.0 * (1 - progress);
            if (variant == 10) {
                length_bonus = 60.0 * (1 - progress) * (1 - progress);
                temperature = 400.0 * (1 - progress) + 3.0;
            }
            if (variant == 0 || progress < 0.08) {
                if (random.integer(12)) rotate_end();
                else cut_end(temperature);
            } else {
                int operation = random.integer(10);
                if ((variant == 7 || variant == 8) && random.integer(8) == 0) reverse_internal_segment();
                else if (operation < 2) rotate_end();
                else if (operation == 2) cut_end(temperature);
                else rebuild(temperature);
            }
            remember();
            ++iteration;
        }
        std::string answer;
        for (int i = 1; i < int(best_path.size()); ++i) {
            int delta = best_path[i] - best_path[i - 1];
            answer += delta == -SIDE ? 'U' : delta == SIDE ? 'D' : delta == -1 ? 'L' : 'R';
        }
        std::cout << answer << '\n';
        std::cerr << "{\"score\":" << best_score << ",\"length\":" << best_path.size()
                  << ",\"iterations\":" << iteration << ",\"rotations\":" << rotations
                  << ",\"cuts\":" << cuts << ",\"rebuilds\":" << rebuilds
                  << ",\"accepted\":" << accepted << ",\"dfs_nodes\":" << dfs_nodes
                  << ",\"changed\":" << changed << ",\"improved\":" << improved
                  << ",\"blocked_target_prunes\":" << blocked_target_prunes
                  << ",\"internal_reversals\":" << internal_reversals
                  << ",\"seconds\":" << elapsed() << "}\n";
    }
};

#ifndef AHC_TEST
int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    double seconds = argc > 1 ? std::atof(argv[1]) : 1.90;
    uint64_t seed = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 0;
    int variant = argc > 3 ? std::atoi(argv[3]) : 10;
    Solver solver(seconds, seed, variant);
    solver.read();
    solver.solve();
}
#endif
