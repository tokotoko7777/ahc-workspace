# 提案: 変更された壁だけを反映する進入方向マスク

`ahc::MutableGridEnterMask(h,w)` は四近傍の長方形盤面を管理する。
正の寸法、`h*w <= INT_MAX` が必要。初期状態は全セル開放。
行優先でセル番号を付け、壁セル `p` は `snapshot[p/64]` の `p%64` ビットを立てる。
`word_count()` と一致するワード数で余りビットを 0 にした全スナップショットを
`update(snapshot)` に渡す。不正な長さ・余りビットは状態変更前に例外で通知する。

`out_mask(p)` の下位4ビットは上・右・下・左の順。`neighbor(p,d)` は壁を
考慮せず対応する座標を返す（盤外なら -1）。進入可能な方向だけを列挙するには
`out_mask` と組み合わせる。壁の**行き先**へは進入できないが、壁上を始点に
した場合は隣の開放セルへ出られる。通常の「壁上を始点にできない」探索は
`is_blocked(start)` で明示的に拒否すること。逆向き探索でそのまま使うと、
壁上の始点の扱いが異なる。BFSや経路復元は既存ライブラリ側で行う。

全準備式の prepared BFS と違い、前回との XOR により変更先セルだけを抽出し、
その隣接元の逆方向ビットを更新する。初期化は O(HW)、更新は
O(ceil(HW/64) + 4K)（K は実際に変化したセル数）、領域 O(HW)。
一度の探索や全盤変更が続く場合にも速くなるとは限らない。性能・既存解法統合は未測定。

```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -fsanitize=undefined -fno-sanitize-recover=all -I library tests/mutable_grid_enter_mask_test.cpp -o /tmp/grid-mask-test
/tmp/grid-mask-test
g++ -std=c++23 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -fsanitize=undefined -fno-sanitize-recover=all -I library examples/mutable_grid_enter_mask_usage.cpp -o /tmp/grid-mask-example
/tmp/grid-mask-example  # 2, 4, 0（各1行）
```

固定シードの合成盤面のみで、全セルのマスクと全始点の距離・同点時を含む最初の一手を
素朴な座標版と照合する。細い盤面、63–65セルのワード境界、同じ隣接元に対する
同時反転、同一セルの相殺反転、全変更と変更なし、不正長・余りビット、
壁始点の非対称性を検査する。

実問題への適用候補: AHC008 のように柵設置後の盤面で同じ4近傍を繰り返し調べる場合。解答への統合や実行時間測定は、この独立した部品では行っていません。
