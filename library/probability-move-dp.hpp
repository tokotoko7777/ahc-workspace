#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/probability-move-dp.hpp

// 「確率move_probabilityで指定先へ進み、失敗すると同じ状態に残る」を1手進める。
// goalを指定すると、そこへ初めて入った確率を返し、以後の分布から取り除く。
//
// 使い方:
// ProbabilityMoveDP<double> dp(3, 0);  // 3状態、初期状態0
// vector<int> move_to = {1, 2, 2};
// double arrived = dp.step(move_to, 0.8, 2);
template <class Real = double>
struct ProbabilityMoveDP {
  std::vector<Real> probability;
  std::vector<Real> next_probability;

  ProbabilityMoveDP(int state_count, int start_state)
      : probability(state_count, Real(0)),
        next_probability(state_count, Real(0)) {
    assert(state_count > 0);
    assert(0 <= start_state && start_state < state_count);
    probability[start_state] = Real(1);
  }

  // destination[state] は、移動に成功した時の遷移先。
  // absorbing_state=-1なら吸収せず、全確率を次の分布へ残す。
  Real step(const std::vector<int>& destination,
            Real move_probability,
            int absorbing_state = -1) {
    const int state_count = static_cast<int>(probability.size());
    assert(static_cast<int>(destination.size()) == state_count);
    assert(Real(0) <= move_probability && move_probability <= Real(1));
    assert(-1 <= absorbing_state && absorbing_state < state_count);

    std::fill(next_probability.begin(), next_probability.end(), Real(0));
    const Real stay_probability = Real(1) - move_probability;
    Real absorbed = Real(0);

    for (int state = 0; state < state_count; ++state) {
      const int next_state = destination[state];
      assert(0 <= next_state && next_state < state_count);
      const Real mass = probability[state];

      next_probability[state] += mass * stay_probability;
      if (next_state == absorbing_state) {
        absorbed += mass * move_probability;
      } else {
        next_probability[next_state] += mass * move_probability;
      }
    }

    probability.swap(next_probability);
    return absorbed;
  }

  Real remaining_probability() const {
    return std::accumulate(
        probability.begin(), probability.end(), Real(0));
  }
};
