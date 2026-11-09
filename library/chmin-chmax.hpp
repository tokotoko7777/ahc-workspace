// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/chmin-chmax.hpp

// 使い方:
// chmin(distance, new_distance);  // 小さくなれば更新して true
// chmax(score, new_score);        // 大きくなれば更新して true
template <class T>
bool chmin(T& current, const T& candidate) {
  if (candidate >= current) return false;
  current = candidate;
  return true;
}

template <class T>
bool chmax(T& current, const T& candidate) {
  if (candidate <= current) return false;
  current = candidate;
  return true;
}
