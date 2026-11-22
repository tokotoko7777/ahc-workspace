# ロールアウトの共通する初手を再利用する

同じ状態、同じ設定、同じ決定器から始まるロールアウトでは、決定器の
初手が決定論的なら、その行動だけを複数試行に再利用できる。状態遷移や
乱数付きの相手応答を共有するのではない。決定器に可変キャッシュがある
場合は、後続の行動に影響しないことを別途確認する。

```cpp
std::optional<Action> first;
for (int i = 0; i < trial_count; ++i) {
    State rollout = live;
    if (!first) first = policy(live);  // 副作用なし。試行0回なら計算しない。
    rollout.apply(*first);
    sample_response(rollout, rng);
    // 2手目以降は、各試行の状態から毎回選ぶ。
}
```

実装例は [AHC008 の検証記録](DEFAULT_VALIDATION.md) の `make.py`。
同じ試行数で行動列、評価、乱数状態が一致することを確認し、別途、
実時間制限で試行数が変わったときの得点と合法性を測る。既存の汎用API
には組み込まず、明示的な初手の再利用を推奨する提案に留める。
