#ifndef AHC_BINARY_SPLIT_FIT_HPP
#define AHC_BINARY_SPLIT_FIT_HPP
#include <cassert>
#include <cmath>
#include <cstddef>

struct BinarySplitFit {
  std::size_t row;
  bool complement;
  double centered_value;
};

// Pick one row of probabilities, optionally exchanging its two outcomes,
// to maximize the objective constant + sum_i gradient[i] * probability[i].
// RowRange and Gradient must support size() and indexed access, and each row
// must contain at least gradient.size() components. Extra components are ignored.
// Example: [[.8,.3],[.3,.8]] with gradient {1,-1} selects row 0 unchanged
// (up to floating-point ties). Complexity O(rows * gradient.size()).
template<class RowRange, class Gradient>
BinarySplitFit best_binary_split(const RowRange& rows, const Gradient& gradient) {
  assert(rows.size() > 0);
  BinarySplitFit best{0,false,-1.0};
  for (std::size_t j=0; j<rows.size(); ++j) {
    double value=0;
    for (std::size_t i=0; i<gradient.size(); ++i)
      value += gradient[i] * (rows[j][i] - .5);
    if (std::abs(value) > best.centered_value)
      best = {j, value < 0, std::abs(value)};
  }
  return best;
}
#endif
