#include <cassert>
#include <utility>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/binary-search-answer.hpp

// predicateが false...false,true...true の時、[false_side,true_side]の最初のtrueを返す。
// predicate(false_side)==false、predicate(true_side)==trueが必要。
template <class Integer, class Predicate>
Integer binary_search_first_true(Integer false_side, Integer true_side,
                                 Predicate predicate) {
  assert(false_side < true_side);
  assert(!predicate(false_side) && predicate(true_side));
  while (true_side - false_side > 1) {
    const Integer middle = false_side + (true_side - false_side) / 2;
    if (predicate(middle)) {
      true_side = middle;
    } else {
      false_side = middle;
    }
  }
  return true_side;
}

// predicateが true...true,false...false の時、[true_side,false_side]の最後のtrueを返す。
template <class Integer, class Predicate>
Integer binary_search_last_true(Integer true_side, Integer false_side,
                                Predicate predicate) {
  assert(true_side < false_side);
  assert(predicate(true_side) && !predicate(false_side));
  while (false_side - true_side > 1) {
    const Integer middle = true_side + (false_side - true_side) / 2;
    if (predicate(middle)) {
      true_side = middle;
    } else {
      false_side = middle;
    }
  }
  return true_side;
}

// 実数の単調判定の境界を固定回数で絞る。{false側,true側}を返す。
template <class Real, class Predicate>
std::pair<Real, Real> binary_search_real(Real false_side, Real true_side,
                                         int iterations,
                                         Predicate predicate) {
  assert(false_side < true_side && iterations >= 0);
  assert(!predicate(false_side) && predicate(true_side));
  for (int iteration = 0; iteration < iterations; ++iteration) {
    const Real middle = (false_side + true_side) / static_cast<Real>(2);
    if (predicate(middle)) {
      true_side = middle;
    } else {
      false_side = middle;
    }
  }
  return {false_side, true_side};
}
