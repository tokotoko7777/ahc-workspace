// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/move-statistics.hpp

// 近傍ごとの試行・採用・改善回数を数える。
// vector<MoveStatistics> statistics(number_of_move_types); のように使える。
//
// 使い方:
// statistics[type].add(accepted, improvement > 0);
// cerr << statistics[type].acceptance_rate() << '\n';
struct MoveStatistics {
  long long tried = 0;
  long long accepted = 0;
  long long improved = 0;

  void add(bool was_accepted, bool was_improved) {
    ++tried;
    if (was_accepted) ++accepted;
    if (was_improved) ++improved;
  }

  double acceptance_rate() const {
    return tried == 0 ? 0.0 : static_cast<double>(accepted) / tried;
  }

  double improvement_rate() const {
    return tried == 0 ? 0.0 : static_cast<double>(improved) / tried;
  }

  void clear() { tried = accepted = improved = 0; }
};
