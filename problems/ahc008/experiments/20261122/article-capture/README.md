# AHC008 全頭捕獲の研究記録

## 現在のベスト（2026-11-23）

| 検証条件 | 公式配布100ケース | 別seedで独立生成100ケース |
|---|---:|---:|
| スコア合計 | **6,152,805,559** | 5,998,857,137 |
| 全頭捕獲 | **100 / 100** | 97 / 100 |
| 終了時に未捕獲だった動物 | 0 | 4 |
| 公式テスター込み最大実測 | 2.741秒未満 | 2.754秒未満 |

目標 6,263,333,336 まで **110,527,777点**。本番と同一ケースではなく、順位比較ではありません。mainへのマージ・公式提出なし。

- [採用版の通常時計での検証・再現用記録](terry-virtual-chase-absorb/NATIVE_RESULT.md)（固定作業量は [こちら](terry-virtual-chase-absorb/FIXED_RESULT.md)）。正式採点と独立リプレイを照合。以前の版と全200ケースの行動と得点が一致し、終盤の比較探索回数を増やせています。
- 現在の破壊的変更のない基準は GNU C++23、内部締切 2670ms。検証したソース SHA-256：`a237f7986e03ba94e98cfb6f1d2059f50d87305f6c8a82b1fa6d4ccd5f013f22`。許可された資料と特定提出のみを参考に検証。配布しているのは自作の差分・テスト・再現ログで、参考実装のソースやモデルは同梱しません。**下の旧版再現手順は現在のベストではありません。**

## 今の調査

| 課題 | 分かったこと |
|---|---|
| もっと狭い区画に入るまで待つ | [実際の捕獲直前を起点にした比較](terry-map1-passage/RESULT.md)では、9マス区画で捕まった牛・豚・ウサギ95頭のうち、25ターン以内に4マス以下へ入って入口を閉められる位置まで届く確率が80%以上だった個体は0。これは固定した柵・他の動物なし・担当即時のモデル。 |
| 少人数で狭い区画を閉める | [1人で二口を順に閉める](terry-one-guard-dual-fallback/RESULT.md)と、豚の偶奇両方とウサギに対応するには、この調査では5マス以上が必要。既存の9マス区画をそのまま6マスに置換する案は[14区画とも不可](terry-map1-single-guard-embedding/RESULT.md)。 |
| 設営の柵を増やさない | Map3の [片口を先に閉める案](terry-map3-pig-parity-caps/RESULT.md)は豚に有利な静的形でも、[配備に時間がかかり不採用](terry-map3-caps-splice/RESULT.md)。[同じ柵数の千鳥配置](terry-map3-parity-seam/TRAFFIC_RESULT.md)は平均到達率の基準未満。妻側に二口を寄せる配置は[一部で成立](terry-map3-staggered-same-end/RESULT.md)するが、通路一つを失う。[盤面上端で既存2区画を残す省柵案](terry-map1-top-edge-pair/PRESET.md)は616候補中、両方を4マス以下で独立に閉められるものがゼロ。 |
| 汎用部品 | [少人数の割当不可理由を返す部品](terry-chase-hall/library-candidate/README.md)、[固定グラフのCSR最大流](terry-chase-csr/library-candidate/README.md)。どちらも使用例・独立テスト付きの改修候補。 |

[全問の進捗表](https://github.com/tokotoko7777/ahc-workspace/blob/codex/ahc-research-progress/docs/AHC_PROGRESS.md) ・ [ライブラリの不便と改修案](https://github.com/tokotoko7777/ahc-workspace/blob/codex/ahc-research-progress/docs/LIBRARY_FEEDBACK.md)

---

## 初期の独自実装（履歴・旧版のみ）

The tested standard is an exact GNU C++23 single-file solver with built-in
optimization; run with no arguments under the official interactive tester.
Keep this research branch separate from main and from the historical AHC008
leaderboard, whose original input cases were not reproduced.

The policy is an independent implementation using only the two references
authorized by the user: [TERRY](https://blog.terry-u16.net/entry/ahc008),
[siman](https://simanman.hatenablog.com/entry/2022/02/28/074838), and the
[official rules and tools](https://atcoder.jp/contests/ahc008/tasks/ahc008_a).
No participant submission code is used.

1. Build the tested body-only 1×4 banks; ready real two-door captures take
   priority. Dynamic pair assignments compare actual exterior travel with
   expected legal arrival-time occupancy instead of fixed longitudinal roles.
   Use uniform substeps for the passive species, a short causal dog posterior
   followed by a cheap approximation, and an approximate cat heading model.
2. Keep the validated visible residual handoff (early t160 if over four
   remain, otherwise t225). For late one or two actual residual animals,
   plan a real irregular vertex cut and preserve the connected human exterior.
   Begin legally available fences as other guards walk; synchronize remaining
   doors. This is not a strategy of directing humans into small refuges.
3. Wait after full separation rather than reducing retained human area.

## Frozen comparison against previous global-clear standard

| Metric | Official distributed 100 old → new | Independent generated 100 old → new |
|---|---:|---:|
| All animals disconnected from every human | 30→**33** | 33→**41** |
| Score | 2,117,108,303→**2,598,735,635 (+22.75%)** | 1,748,184,767→**2,519,824,436 (+44.14%)** |
| Real final exact-four cages (animals) | 706→814 | 710→830 |
| Final animals still reachable | 220→166 | 222→152 |
| First separations into nonrectangular 4–48-cell enclosures | 224→207 | 237→218 |
| Final sterile area union | 11,821→15,223 | 9,112→15,488 |
| Score wins/ties/losses | 57/0/43 | 66/0/34 |
| Relative all-clear gained/lost | 19/16 | 25/17 |
| Observed tester wall seconds for new, median/max | .648/.979 | .616/.870 |

On practice, pair the earlier global-clear v2 ledger and the v2→v4 ledger
through v2: all 100 v2 full hashes and exactly adjudicated scores are
identical. Independent100 has a direct full paired run. All trajectories
were official tester v3 legal and the independent scoring replay agreed.
This is not complete capture, and the improvement does not apply per
case; the user's 6,263,333,336-point goal remains unmet. In particular,
last research version's practice final area-only capacity is 4.8598B:
even hypothetically no pets in the final human components, this fixed
layout could not reach the aggregate target.

- [Earlier A/B global-clear→v2](https://github.com/tokotoko7777/ahc-workspace/commit/10149e1a4ba15eeb389cd90d952542ec0b038267)
- [Final wall-preparation A/B v2→v4](https://github.com/tokotoko7777/ahc-workspace/commit/635afeff8931a2a6382e7c4af970e2f3c7cbe3a2)
- [Developed v4 planner and fixtures](https://github.com/tokotoko7777/ahc-workspace/commit/ea0d16ee48f64972bd62f7266e13de76aaeaab16)

## Reproduce

```sh
D=problems/ahc008/experiments/20261122
A="$D/article-capture"
g++ -std=c++23 "$A/solve.cpp" -o /tmp/ahc008-article
g++ -std=c++23 "$D/parallel-banks-narrow-global-clear.cpp" -o /tmp/ahc008-old
PYTHONPATH="$D" python3 "$A/bench_article.py" problems/ahc008 "$TESTER" /tmp/ahc008-old /tmp/ahc008-article "$PRACTICE100" /tmp/article.csv --n 100
python3 "$A/compare_articles.py" /tmp/article.csv
```

The supplied independent seed file was fixed using the first 8 little-endian
digest bytes of SHA256(`ahc008/article-policy/full-validation/20261122/v1/{i}`)
for i=0,…,99. Regenerate in a clean folder using official tester v3's generator.
Source SHA256 `7d2d6fd24886dd4fad442da78a3aff6ee31f4d206270d9ee220bcdc474b24783`.
