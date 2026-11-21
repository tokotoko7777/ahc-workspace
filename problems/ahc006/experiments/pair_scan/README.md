# 注文置換の挿入位置走査を短縮

従来の `best_ordered_pair_insertion` と同じ最小増分・同点規則。対称なマンハッタン距離を表で保持する場合だけ、新しい集荷・配達の2行を固定し、各位置における前の値を隣の辺に引き継ぐ。短縮前後で近傍の選択、挿入の同点規則、その他の処理は変更しない。非対称距離や表を作らないビルドは従来の汎用関数のまま。

合成2,048候補の距離差分・両gapが従来と一致。各307,200回を6回、前後を交互に測定。旧1,950〜2,219 ms → 新414〜471 ms。これは置換内の最良挿入走査に限る。探索全体や時間内の得点は別測定。

```sh
g++ -std=c++23 problems/ahc006/experiments/pair_scan/fixed_work.cpp -o fixed
./fixed
python3 scripts/bundle.py problems/ahc006/experiments/pair_scan/submission_entry.cpp -o submission.cpp
g++ -std=c++23 submission.cpp -o candidate
```
