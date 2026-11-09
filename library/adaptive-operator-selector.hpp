#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/adaptive-operator-selector.hpp
// ALNSの着想（過去の成果に応じて近傍の選択頻度を変える）:
// https://doi.org/10.1287/trsc.1050.0135
// 概念を参考にした独自実装。論文の実験設定や第三者コードの再現ではない。

struct AdaptiveOperatorOptions {
  bool adaptive = true; // falseなら常に等確率。比較実験用。
  int update_interval = 128; // この回数のrecordごとに重みを更新する。
  double learning_rate = 0.2; // 0: 学習なし、1: 直近区間の平均報酬で置換。
  double exploration = 0.1; // 選択確率のうち一様分布を混ぜる割合。(0,1]
};

// 少数個の近傍向け。選択O(近傍数)、通常の記録O(1)、区間更新O(近傍数)。
// select/record中はメモリ確保しない。時計も盤面hashも一致判定も不要。
// SA/LNS等から独立。select(rng)の戻り値0..size()-1で壊し方を選び、
// 試行後にrecord(id, reward)へ「0以上1以下の成果」を渡す。
// 失敗・枝刈りも報酬0で必ず記録する。成功だけ記録すると成功率を学べない。
// 報酬は問題依存。採用数≠得点改善なので、設定は実問題scoreで検証する。
class AdaptiveOperatorSelector {
 public:
  explicit AdaptiveOperatorSelector(int count, AdaptiveOperatorOptions options = {})
      : options_(options) {
    if (count <= 0 || options.update_interval <= 0 ||
        !std::isfinite(options.learning_rate) || options.learning_rate < 0 || options.learning_rate > 1 ||
        !std::isfinite(options.exploration) || options.exploration <= 0 || options.exploration > 1) {
      throw std::invalid_argument("invalid adaptive operator options");
    }
    weights_.assign(count, 1.0);
    rewards_.assign(count, 0.0);
    used_.assign(count, 0);
    probabilities_.resize(count);
    cumulative_.resize(count);
    rebuild();
  }
  int size() const { return static_cast<int>(weights_.size()); }
  template <class Random> int select(Random& rng) const {
    const double value = std::generate_canonical<double, 53>(rng); // [0,1)
    for (int id = 0; id + 1 < size(); ++id) if (value < cumulative_[id]) return id;
    return size() - 1; // 丸め誤差でも範囲外を返さない。
  }
  void record(int id, double reward) {
    if (id < 0 || id >= size() || !std::isfinite(reward) || reward < 0 || reward > 1) {
      throw std::invalid_argument("operator id/reward out of range");
    }
    if (!options_.adaptive) return;
    rewards_[id] += reward;
    ++used_[id];
    if (++pending_ == options_.update_interval) update();
  }
  double probability(int id) const { return probabilities_.at(id); }
  // 未完の区間を反映したい時のみ手動で呼ぶ。通常はrecordが自動実行する。
  void update() {
    if (!pending_) return;
    for (int id = 0; id < size(); ++id) {
      if (used_[id]) {
        const double mean = rewards_[id] / used_[id];
        weights_[id] = (1 - options_.learning_rate) * weights_[id] + options_.learning_rate * mean;
      } // 未試行の近傍は「失敗」と見なさず、以前の重みを維持する。
      used_[id] = 0;
      rewards_[id] = 0;
    }
    pending_ = 0;
    rebuild();
  }
 private:
  void rebuild() {
    double sum = 0;
    for (double weight : weights_) sum += weight;
    double cumulative = 0;
    for (int id = 0; id < size(); ++id) {
      const double learned = sum > 0 ? weights_[id] / sum : 1.0 / size();
      probabilities_[id] = options_.adaptive ?
          options_.exploration / size() + (1 - options_.exploration) * learned : 1.0 / size();
      cumulative_[id] = cumulative += probabilities_[id];
    }
    cumulative_.back() = 1.0;
  }
  AdaptiveOperatorOptions options_;
  std::vector<double> weights_, rewards_, probabilities_, cumulative_;
  std::vector<int> used_;
  int pending_ = 0;
};
