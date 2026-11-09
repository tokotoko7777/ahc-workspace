#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

// Jointly fit line breakpoints and small independent edge corrections.
struct PiecewiseModel {
    static constexpr int LINES = 60, WIDTH = 29, EDGE_COUNT = 1740;
    static constexpr double LINE_PRECISION = 1.0 / 3.001111111111111;
    struct Observation {
        int query;
        uint32_t mask;
        std::array<unsigned char, WIDTH + 1> prefix{};
    };
    struct Line {
        std::array<double, WIDTH> profile{};
        std::array<double, WIDTH> variance{};
        std::array<double, WIDTH> a00{}, a01{}, a11{};
        std::vector<Observation> observations;
        double second_moment = 3.001111111111111;
        Line() {
            profile.fill(5.0);
            variance.fill(3.001111111111111);
            a00.fill(LINE_PRECISION);
            a11.fill(LINE_PRECISION);
        }
    };
    std::array<Line, LINES> lines;
    std::array<double, EDGE_COUNT> correction{}, edge_precision{};
    std::array<std::vector<int>, EDGE_COUNT> edge_queries;
    std::vector<double> residual, weight;
    std::vector<std::vector<int>> history;
    int sweeps = 0;
    bool constant_lines = false;
    double edge_prior_precision = 2.0;
    double line_prior_precision = LINE_PRECISION;
    double estimated_d = 1.05;
    int adaptation = 0;
    double local_constant_prior = 0;

    explicit PiecewiseModel(double edge_variance = 0.5) {
        edge_prior_precision = 1.0 / edge_variance;
        edge_precision.fill(edge_prior_precision);
    }

    static int line_of(int edge) { return edge < 870 ? edge / 29 : 30 + (edge - 870) % 30; }
    static int offset_of(int edge) { return edge < 870 ? edge % 29 : (edge - 870) / 30; }

    double predict(int edge) const {
        return lines[line_of(edge)].profile[offset_of(edge)] + correction[edge];
    }

    double uncertainty(int edge) const {
        return lines[line_of(edge)].variance[offset_of(edge)] + 1.0 / edge_precision[edge];
    }

    static double contribution(const std::array<double, WIDTH> &profile, uint32_t mask) {
        double value = 0;
        while (mask) {
            int bit = __builtin_ctz(mask);
            value += profile[bit];
            mask &= mask - 1;
        }
        return value;
    }

    void add(const std::vector<int> &edges, int observed) {
        std::array<uint32_t, LINES> masks{};
        double prediction = 0;
        for (int edge : edges) {
            masks[line_of(edge)] |= uint32_t(1) << offset_of(edge);
            prediction += predict(edge);
        }
        int query = int(residual.size());
        double w = 1.0 / std::max(1.0, prediction * prediction / 300.0);
        residual.push_back(observed / 1000.0 - prediction);
        weight.push_back(w);
        history.push_back(edges);
        for (int edge : edges) {
            edge_queries[edge].push_back(query);
            edge_precision[edge] += w;
        }
        for (int id = 0; id < LINES; ++id) if (masks[id]) {
            Observation observation;
            observation.query = query;
            observation.mask = masks[id];
            for (int k = 1; k <= WIDTH; ++k)
                observation.prefix[k] = observation.prefix[k - 1] + ((masks[id] >> (k - 1)) & 1);
            int total = observation.prefix[WIDTH];
            Line &line = lines[id];
            for (int cut = 1; cut < WIDTH; ++cut) {
                int left = observation.prefix[cut], right = total - left;
                line.a00[cut] += w * left * left;
                line.a01[cut] += w * left * right;
                line.a11[cut] += w * right * right;
            }
            line.observations.push_back(observation);
        }
    }

    void fit_line(int id) {
        Line &line = lines[id];
        if (line.observations.empty()) return;
        std::array<double, WIDTH> b0{};
        double total_b = 0;
        for (const Observation &observation : line.observations) {
            int q = observation.query;
            double centered = residual[q] + contribution(line.profile, observation.mask) - 5.0 * observation.prefix[WIDTH];
            double w = weight[q] * centered;
            total_b += w * observation.prefix[WIDTH];
            for (int cut = 1; cut < WIDTH; ++cut) b0[cut] += w * observation.prefix[cut];
        }
        if (constant_lines && local_constant_prior == 0) {
            double precision = line.a00[1] + 2 * line.a01[1] + line.a11[1] - line_prior_precision;
            double limit = adaptation >= 2 ? 4.0 - estimated_d : 4.0;
            double mean = 5.0 + std::clamp(total_b / precision, -limit, limit);
            line.second_moment = (mean - 5.0) * (mean - 5.0) + 1.0 / precision;
            for (const Observation &observation : line.observations)
                residual[observation.query] -= mean * observation.prefix[WIDTH] - contribution(line.profile, observation.mask);
            line.profile.fill(mean);
            line.variance.fill(1.0 / precision);
            return;
        }
        std::array<double, WIDTH> left_mean{}, right_mean{}, evidence{}, second_moment{};
        double maximum = -1e100;
        for (int cut = 1; cut < WIDTH; ++cut) {
            double a = line.a00[cut], b = line.a01[cut], d = line.a11[cut];
            double determinant = a * d - b * b;
            double x = b0[cut], y = total_b - x;
            double limit = adaptation >= 2 ? 4.0 - estimated_d : 4.0;
            double left = std::clamp((d * x - b * y) / determinant, -limit, limit);
            double right = std::clamp((a * y - b * x) / determinant, -limit, limit);
            left_mean[cut] = left + 5;
            right_mean[cut] = right + 5;
            double improvement = 2 * (x * left + y * right) - (a * left * left + 2 * b * left * right + d * right * right);
            evidence[cut] = 0.5 * (improvement - std::log(determinant / (line_prior_precision * line_prior_precision)));
            second_moment[cut] = 0.5 * (left * left + right * right + (a + d) / determinant);
            maximum = std::max(maximum, evidence[cut]);
        }
        std::array<double, WIDTH> next{};
        std::array<double, WIDTH> squares{};
        double total_weight = 0;
        line.second_moment = 0;
        double constant_mean = 0, constant_variance = 0, constant_evidence = -1e100;
        if (local_constant_prior > 0) {
            double precision = line.a00[1] + 2 * line.a01[1] + line.a11[1] - line_prior_precision;
            double shift = std::clamp(total_b / precision, -4.0, 4.0);
            constant_mean = 5.0 + shift;
            constant_variance = 1.0 / precision;
            constant_evidence = 0.5 * (2 * total_b * shift - precision * shift * shift - std::log(precision / line_prior_precision)) +
                std::log(28.0 * local_constant_prior);
            maximum = std::max(maximum, constant_evidence);
            double w = std::exp(constant_evidence - maximum);
            total_weight = w;
            next.fill(w * constant_mean);
            squares.fill(w * (constant_mean * constant_mean + constant_variance));
            line.second_moment = w * (shift * shift + constant_variance);
        }
        for (int cut = 1; cut < WIDTH; ++cut) {
            double w = std::exp(evidence[cut] - maximum);
            total_weight += w;
            line.second_moment += w * second_moment[cut];
            double determinant = line.a00[cut] * line.a11[cut] - line.a01[cut] * line.a01[cut];
            for (int j = 0; j < WIDTH; ++j) {
                double value = j < cut ? left_mean[cut] : right_mean[cut];
                double v = (j < cut ? line.a11[cut] : line.a00[cut]) / determinant;
                next[j] += w * value;
                squares[j] += w * (value * value + v);
            }
        }
        for (double &value : next) value /= total_weight;
        for (int j = 0; j < WIDTH; ++j) line.variance[j] = std::max(0.0, squares[j] / total_weight - next[j] * next[j]);
        line.second_moment /= total_weight;
        std::array<double, WIDTH> change{};
        for (int j = 0; j < WIDTH; ++j) change[j] = next[j] - line.profile[j];
        for (const Observation &observation : line.observations)
            residual[observation.query] -= contribution(change, observation.mask);
        line.profile = next;
    }

    void fit_edges() {
        for (int edge = 0; edge < EDGE_COUNT; ++edge) {
            if (edge_queries[edge].empty()) continue;
            double sum = correction[edge] * (edge_precision[edge] - edge_prior_precision);
            for (int q : edge_queries[edge]) sum += weight[q] * residual[q];
            double next = std::clamp(sum / edge_precision[edge], -2.0, 2.0);
            double change = next - correction[edge];
            for (int q : edge_queries[edge]) residual[q] -= change;
            correction[edge] = next;
        }
    }

    void fit(int passes) {
        if (adaptation && residual.size() >= 200 && residual.size() % 100 == 0) {
            double variance = 0;
            for (const Line &line : lines) variance += line.second_moment / LINES;
            double next_d = std::clamp(4.0 - std::sqrt(3.0 * variance), 0.1, 2.0);
            estimated_d += 0.5 * (next_d - estimated_d);
            double next_line_precision = 3.0 / ((4.0 - estimated_d) * (4.0 - estimated_d));
            double next_edge_precision = 3.0 / (estimated_d * estimated_d + 0.1);
            for (Line &line : lines) for (int cut = 1; cut < WIDTH; ++cut) {
                line.a00[cut] += next_line_precision - line_prior_precision;
                line.a11[cut] += next_line_precision - line_prior_precision;
            }
            for (double &precision : edge_precision) precision += next_edge_precision - edge_prior_precision;
            line_prior_precision = next_line_precision;
            edge_prior_precision = next_edge_precision;
        }
        for (int pass = 0; pass < passes; ++pass) {
            for (int i = 0; i < LINES; ++i) fit_line((i + 7 * sweeps) % LINES);
            fit_edges();
            ++sweeps;
        }
    }
};


constexpr int SIDE = 30, VERTICES = SIDE * SIDE, EDGES = 2 * SIDE * (SIDE - 1);

struct GaussianModel {
    int bins, size;
    double exploration, extra_noise;
    std::vector<double> mean, covariance, product, planning_cost;
    std::array<int, EDGES> feature{};

    GaussianModel(int groups, double optimism, double noise, int line_model = 0)
        : bins(groups), size(60 * groups), exploration(optimism), extra_noise(noise),
          mean(size, 5.0), covariance(size * size), product(size), planning_cost(size) {
        std::vector<std::vector<int>> offsets(bins);
        for (int j = 0; j < 29; ++j) offsets[j * bins / 29].push_back(j);
        for (int line = 0; line < 60; ++line) {
            for (int i = 0; i < bins; ++i) for (int j = 0; j < bins; ++j) {
                double sum = 0;
                for (int x : offsets[i]) for (int y : offsets[j]) {
                    double separation = line_model == 1 ? 0.0 : std::abs(x - y) / (line_model == 2 ? 28.0 : 56.0);
                    sum += 3.001111111111111 * (1.0 - separation);
                    if (x == y) sum += 0.4677777777777778;
                }
                covariance[(line * bins + i) * size + line * bins + j] =
                    sum / (offsets[i].size() * offsets[j].size());
            }
        }
        for (int r = 0; r < SIDE; ++r) for (int c = 0; c < SIDE - 1; ++c)
            feature[r * 29 + c] = r * bins + c * bins / 29;
        for (int r = 0; r < SIDE - 1; ++r) for (int c = 0; c < SIDE; ++c)
            feature[870 + r * 30 + c] = (30 + c) * bins + r * bins / 29;
    }

    void prepare(int turn) {
        double optimism = exploration * std::exp(-turn / 200.0) + 0.02;
        for (int i = 0; i < size; ++i) {
            double uncertainty = std::sqrt(std::max(0.0, covariance[i * size + i]));
            planning_cost[i] = std::clamp(mean[i] - optimism * uncertainty, 0.8, 9.0);
        }
    }

    double cost(int edge) const { return planning_cost[feature[edge]]; }

    double update(const std::vector<int> &path, int observed) {
        std::vector<int> counts(size), active;
        active.reserve(path.size());
        for (int edge : path) {
            int f = feature[edge];
            if (!counts[f]++) active.push_back(f);
        }
        double prediction = 0;
        for (int f : active) prediction += counts[f] * mean[f];
        double variance = std::max(1.0, prediction * prediction / 300.0 + extra_noise * path.size());
        for (int i = 0; i < size; ++i) {
            const double *row = covariance.data() + i * size;
            double sum = 0;
            for (int f : active) sum += row[f] * counts[f];
            product[i] = sum;
        }
        for (int f : active) variance += counts[f] * product[f];
        double residual = observed / 1000.0 - prediction;
        double inverse_variance = 1.0 / variance;
        for (int i = 0; i < size; ++i) {
            double gain = product[i] * inverse_variance;
            mean[i] = std::clamp(mean[i] + gain * residual, 1.0, 9.0);
            double *row = covariance.data() + i * size;
            for (int j = 0; j < size; ++j) row[j] -= gain * product[j];
        }
        return -0.5 * (std::log(variance) + residual * residual / variance);
    }
};

struct Grid {
    struct Arc { int to, edge; char direction; };
    std::array<std::vector<Arc>, VERTICES> arcs;
    Grid() {
        for (int r = 0; r < SIDE; ++r) for (int c = 0; c < SIDE; ++c) {
            int v = r * SIDE + c;
            if (c + 1 < SIDE) {
                int edge = r * 29 + c;
                arcs[v].push_back({v + 1, edge, 'R'});
                arcs[v + 1].push_back({v, edge, 'L'});
            }
            if (r + 1 < SIDE) {
                int edge = 870 + r * 30 + c;
                arcs[v].push_back({v + SIDE, edge, 'D'});
                arcs[v + SIDE].push_back({v, edge, 'U'});
            }
        }
    }

    std::pair<std::string, std::vector<int>> one_bend_route(int start, int target, bool horizontal_first) const {
        int row = start / SIDE, column = start % SIDE;
        int target_row = target / SIDE, target_column = target % SIDE;
        std::string moves;
        std::vector<int> edges;
        auto horizontal = [&]() {
            while (column < target_column) { moves += 'R'; edges.push_back(row * 29 + column); ++column; }
            while (column > target_column) { moves += 'L'; --column; edges.push_back(row * 29 + column); }
        };
        auto vertical = [&]() {
            while (row < target_row) { moves += 'D'; edges.push_back(870 + row * 30 + column); ++row; }
            while (row > target_row) { moves += 'U'; --row; edges.push_back(870 + row * 30 + column); }
        };
        if (horizontal_first) { horizontal(); vertical(); }
        else { vertical(); horizontal(); }
        return {moves, edges};
    }

    template<class Costs>
    std::pair<std::string, std::vector<int>> route(int start, int target, const Costs &model) const {
        std::array<double, VERTICES> distance;
        std::array<int, VERTICES> previous{}, previous_edge{};
        std::array<char, VERTICES> direction{};
        distance.fill(std::numeric_limits<double>::infinity());
        distance[start] = 0;
        using Item = std::pair<double, int>;
        std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;
        queue.push({0, start});
        while (!queue.empty()) {
            auto [cost, v] = queue.top(); queue.pop();
            if (cost != distance[v]) continue;
            if (v == target) break;
            for (const auto &arc : arcs[v]) {
                double next = cost + model.cost(arc.edge);
                if (next < distance[arc.to]) {
                    distance[arc.to] = next;
                    previous[arc.to] = v;
                    previous_edge[arc.to] = arc.edge;
                    direction[arc.to] = arc.direction;
                    queue.push({next, arc.to});
                }
            }
        }
        std::string moves;
        std::vector<int> edges;
        for (int v = target; v != start; v = previous[v]) {
            moves.push_back(direction[v]); edges.push_back(previous_edge[v]);
        }
        std::reverse(moves.begin(), moves.end());
        std::reverse(edges.begin(), edges.end());
        return {moves, edges};
    }
};

struct EdgeCosts {
    std::array<double, EDGES> values{};
    double cost(int edge) const { return values[edge]; }
};

#ifndef AHC_TEST
int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int bins = argc > 1 ? std::atoi(argv[1]) : 8;
    double exploration = argc > 2 ? std::atof(argv[2]) : 0.6;
    double extra_noise = argc > 3 ? std::atof(argv[3]) : 0.3;
    bool mixture = argc > 4 ? std::atoi(argv[4]) != 0 : true;
    int split_bins = argc > 5 ? std::atoi(argv[5]) : 20;
    if (!split_bins) split_bins = bins;
    int lock_after = argc > 6 ? std::atoi(argv[6]) : 250;
    int piecewise_mode = argc > 7 ? std::atoi(argv[7]) : 3;
    int fit_passes = argc > 8 ? std::atoi(argv[8]) : 3;
    int adaptation = argc > 9 ? std::atoi(argv[9]) : 1;
    double lock_threshold = argc > 10 ? std::atof(argv[10]) : 8.0;
    double exploration_floor = argc > 11 ? std::atof(argv[11]) : 0.3;
    bool piecewise_uncertainty = argc > 12 ? std::atoi(argv[12]) != 0 : true;
    bool soft_type = argc > 13 ? std::atoi(argv[13]) != 0 : false;
    double local_constant_prior = argc > 14 ? std::atof(argv[14]) : 0.1;
    int straight_start = argc > 15 ? std::atoi(argv[15]) : 0;
    if (bins < 1 || bins > 29 || split_bins < 1 || split_bins > 29) return 1;
    const auto start = std::chrono::steady_clock::now();
    const std::clock_t cpu_start = std::clock();
    GaussianModel model(bins, exploration, extra_noise, mixture ? 1 : 0);
    std::unique_ptr<GaussianModel> split_model;
    if (mixture) split_model = std::make_unique<GaussianModel>(split_bins, exploration, extra_noise, 2);
    double model_log_odds = 0;
    int locked = 0, locked_at = -1;
    EdgeCosts edge_costs;
    std::unique_ptr<PiecewiseModel> piecewise;
    if (piecewise_mode) piecewise = std::make_unique<PiecewiseModel>(piecewise_mode == 4 ? 1.0 : piecewise_mode == 5 ? 0.2 : 0.5);
    if (piecewise) piecewise->adaptation = adaptation;
    if (piecewise) piecewise->local_constant_prior = local_constant_prior;
    Grid grid;
    for (int turn = 0; turn < 1000; ++turn) {
        int sr, sc, tr, tc;
        if (!(std::cin >> sr >> sc >> tr >> tc)) return 1;
        if (locked != 2) model.prepare(turn);
        if (mixture) {
            if (locked != 1) split_model->prepare(turn);
            double probability = locked ? double(locked == 1) : 1.0 / (1.0 + std::exp(-model_log_odds));
            double optimism = exploration * std::exp(-turn / 200.0) + exploration_floor;
            for (int edge = 0; edge < EDGES; ++edge) {
                int i = model.feature[edge], j = split_model->feature[edge];
                double piecewise_weight = piecewise ? std::clamp((turn - 150) / 250.0, 0.0, 1.0) : 0;
                if (piecewise_mode == 2) piecewise_weight *= 0.6;
                if (locked == 1) {
                    double variance = model.covariance[i * model.size + i];
                    if (piecewise_uncertainty && piecewise) variance += piecewise_weight * (piecewise->uncertainty(edge) - variance);
                    double value = model.mean[i] - optimism * std::sqrt(std::max(0.0, variance));
                    if (piecewise_mode >= 3) value += piecewise_weight * (piecewise->predict(edge) - model.mean[i]);
                    edge_costs.values[edge] = std::clamp(value, 0.8, 9.0);
                    continue;
                }
                double first = model.mean[i], second = split_model->mean[j];
                if (piecewise_weight && local_constant_prior == 0) {
                    if (soft_type && piecewise->constant_lines) first += piecewise_weight * (piecewise->predict(edge) - first);
                    else second += piecewise_weight * (piecewise->predict(edge) - second);
                }
                double average = probability * first + (1 - probability) * second;
                double variance = probability * model.covariance[i * model.size + i] +
                    (1 - probability) * split_model->covariance[j * split_model->size + j] +
                    probability * (1 - probability) * (first - second) * (first - second);
                if (local_constant_prior > 0) average += piecewise_weight * (piecewise->predict(edge) - average);
                if (piecewise_uncertainty && piecewise) variance += piecewise_weight * (piecewise->uncertainty(edge) - variance);
                edge_costs.values[edge] = std::clamp(average - optimism * std::sqrt(std::max(0.0, variance)), 0.8, 9.0);
            }
        } else for (int edge = 0; edge < EDGES; ++edge) edge_costs.values[edge] = model.cost(edge);
        auto [moves, edges] = grid.route(sr * SIDE + sc, tr * SIDE + tc, edge_costs);
        if (turn < straight_start) {
            double best = 1e100;
            for (bool horizontal_first : {false, true}) {
                auto candidate = grid.one_bend_route(sr * SIDE + sc, tr * SIDE + tc, horizontal_first);
                double cost = 0;
                for (int edge : candidate.second) cost += edge_costs.cost(edge);
                if (cost < best) {
                    best = cost;
                    moves = std::move(candidate.first);
                    edges = std::move(candidate.second);
                }
            }
        }
        std::cout << moves << '\n' << std::flush;
        int observed;
        if (!(std::cin >> observed)) return 1;
        double evidence = locked != 2 ? model.update(edges, observed) : 0;
        if (mixture && locked != 1) {
            double split_evidence = split_model->update(edges, observed);
            if (!locked) model_log_odds = std::clamp(model_log_odds + 0.5 * (evidence - split_evidence), -20.0, 20.0);
        }
        if (mixture && !locked && lock_after && turn + 1 >= lock_after && std::abs(model_log_odds) > lock_threshold) {
            locked = model_log_odds > 0 ? 1 : 2;
            locked_at = turn + 1;
        }
        if (piecewise && (locked != 1 || piecewise_mode >= 3)) {
            piecewise->add(edges, observed);
            bool constant_lines = locked == 1 || (soft_type && turn >= 249 && model_log_odds > 4.6);
            bool changed = piecewise->constant_lines != constant_lines;
            piecewise->constant_lines = constant_lines;
            if ((turn + 1) % 10 == 0 || changed) piecewise->fit(fit_passes);
        }
    }
    std::cerr << "{\"bins\":" << bins << ",\"parameters\":" << model.size
              << ",\"mixture\":" << (mixture ? "true" : "false") << ",\"constant_line_probability\":" << 1.0 / (1.0 + std::exp(-model_log_odds))
              << ",\"split_bins\":" << split_bins << ",\"locked_model\":" << locked << ",\"locked_at\":" << locked_at
              << ",\"piecewise_mode\":" << piecewise_mode << ",\"piecewise_sweeps\":" << (piecewise ? piecewise->sweeps : 0)
              << ",\"adaptation\":" << adaptation << ",\"estimated_d\":" << (piecewise ? piecewise->estimated_d : 0)
              << ",\"lock_threshold\":" << lock_threshold << ",\"exploration_floor\":" << exploration_floor << ",\"piecewise_uncertainty\":" << piecewise_uncertainty
              << ",\"soft_type\":" << soft_type
              << ",\"local_constant_prior\":" << local_constant_prior
              << ",\"straight_start\":" << straight_start
              << ",\"cpu_seconds\":" << double(std::clock() - cpu_start) / CLOCKS_PER_SEC
              << ",\"wall_seconds\":" << std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count()
              << "}\n";
}
#endif
