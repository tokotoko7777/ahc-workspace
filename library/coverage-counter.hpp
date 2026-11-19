#pragma once

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
// Source repository (created with generative AI):
// https://github.com/tokotoko7777/ahc-workspace

// 同じ対象が複数回覆われても、重みを1回だけ数える。
// weightsと試行用bufferはこのオブジェクト、現在解のcountはStateに置く。
// Scoreは符号付き数値型。重み・合計・差分がScoreに収まるように選ぶ。
//
// CoverageCounter<long long> coverage({10, 20, 30});
// auto state = coverage.make_state();
// coverage.add(state, 0, 2);             // 2件のヒットでも得点は10。
// coverage.clear_delta();
// coverage.add_delta(0, -1);             // 2→1: 得点は変わらない。
// coverage.add_delta(1, +1);             // 0→1: +20。
// auto improvement = coverage.score_delta(state); // stateは変えない。
// if (accept(improvement)) coverage.apply_delta(state);
// 次の試行の先頭でclear_deltaする。不採用時の巻き戻しは不要。
//
// 複数箇所を同時に置換する場合、旧ヒットを-1、新ヒットを+1で全て積む。
// 重複を消さず、同じIDの増減を集計してから0境界の変化を評価する。
// score_delta/apply_delta/clear_deltaは触れたID数に比例。1個のadd_deltaはO(1)。
// 1オブジェクトにつき試行bufferは1個。複数候補を同時評価する時は別々に持つ。
template <class Score = long long>
class CoverageCounter {
    static_assert(std::is_arithmetic_v<Score> && std::is_signed_v<Score>,
                  "Score must be a signed arithmetic type");

public:
    struct State {
        // 読み取り用。初期化・更新はCoverageCounter経由で行う。
        std::vector<int> counts;
        Score covered_weight{};
    };

    explicit CoverageCounter(std::vector<Score> weights)
        : weights_(std::move(weights)), delta_(weights_.size()), marked_(weights_.size()) {
        for (Score weight : weights_)
            if (!std::isfinite(static_cast<long double>(weight)))
                throw std::invalid_argument("coverage weights must be finite");
        touched_.reserve(weights_.size());
    }

    State make_state() const { return {std::vector<int>(weights_.size()), Score{}}; }
    std::size_t size() const { return weights_.size(); }

    void add(State& state, int id, int occurrences = 1) const {
        if (occurrences < 0) throw std::invalid_argument("negative occurrence count");
        change_count(state, id, occurrences);
    }

    void remove(State& state, int id, int occurrences = 1) const {
        if (occurrences < 0) throw std::invalid_argument("negative occurrence count");
        change_count(state, id, -occurrences);
    }

    void clear_delta() {
        for (int id : touched_) {
            delta_[id] = 0;
            marked_[id] = false;
        }
        touched_.clear();
    }

    void add_delta(int id, int change) {
        check_id(id);
        const int next = checked_sum(delta_[id], change);
        if (!marked_[id]) {
            touched_.push_back(id);
            marked_[id] = true;
        }
        delta_[id] = next;
    }

    Score score_delta(const State& state) const {
        check_state(state);
        Score result{};
        for (int id : touched_) {
            const int next = checked_count(state.counts[id], delta_[id]);
            if (state.counts[id] == 0 && next > 0) result += weights_[id];
            if (state.counts[id] > 0 && next == 0) result -= weights_[id];
        }
        return result;
    }

    // 全IDの件数を検証してから反映する。不正な差分ならStateは変わらない。
    // 反映後は試行bufferを空にする。
    void apply_delta(State& state) {
        const Score improvement = score_delta(state);
        for (int id : touched_) state.counts[id] += delta_[id];
        state.covered_weight += improvement;
        clear_delta();
    }

private:
    std::vector<Score> weights_;
    std::vector<int> delta_, touched_;
    std::vector<unsigned char> marked_;

    void check_id(int id) const {
        if (id < 0 || static_cast<std::size_t>(id) >= size())
            throw std::out_of_range("coverage id out of range");
    }
    void check_state(const State& state) const {
        if (state.counts.size() != size())
            throw std::invalid_argument("coverage state size mismatch");
    }
    static int checked_sum(int first, int second) {
        const long long value = static_cast<long long>(first) + second;
        if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
            throw std::overflow_error("coverage count overflow");
        return static_cast<int>(value);
    }
    static int checked_count(int count, int change) {
        const int next = checked_sum(count, change);
        if (count < 0 || next < 0) throw std::invalid_argument("negative coverage count");
        return next;
    }
    void change_count(State& state, int id, int change) const {
        check_id(id);
        check_state(state);
        const int next = checked_count(state.counts[id], change);
        if (state.counts[id] == 0 && next > 0) state.covered_weight += weights_[id];
        if (state.counts[id] > 0 && next == 0) state.covered_weight -= weights_[id];
        state.counts[id] = next;
    }
};
