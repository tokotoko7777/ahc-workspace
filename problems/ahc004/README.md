# AHC004: 文字列の盤面復元

[公式問題文](https://atcoder.jp/contests/ahc004/tasks/ahc004_a) ·
[公式配布入力ZIP](https://img.atcoder.jp/ahc004/a45fa3f18ab177158bf5961b12872f93.zip) ·
[公式採点ツールZIP](https://img.atcoder.jp/ahc004/222362f13a30b1342bf79d0041bd4d39.zip)

## 検証結果

公式配布 seed 0–99 の100件で **8,996,036,656点**。全語収録は 56 件、
ローカル最長 2.788662 秒。各出力で公式ツールと独立採点が一致した。
配布100件での目標9,000,000,000点は 未達（残り 3,963,344 点）。これは本番順位との比較ではない。
研究初期に同条件で測った単一ファイルは7,489,107,407点。
同じ実行ファイルでの先行計測は8,994,781,021点（最長2.790913秒）。
ソース変更はなく、時計で探索を打ち切るため得点や実行時間は変動する。

- [ケース別の測定結果](benchmark-results.csv)
- [この版の研究用アーカイブ](https://github.com/tokotoko7777/ahc-workspace/blob/851552a10dc1ac63210597ebaf5aa928b54a9073/problems/ahc004/experiments/medium_mixed_extend/submission.single.cpp)
- [編集用ソースのエントリ](https://github.com/tokotoko7777/ahc-workspace/blob/b0a0799f5a3c701cc054da02a3188188ee23bdc1/problems/ahc004/experiments/medium_mixed_extend/submission_entry.cpp)
- [検証過程と比較](https://github.com/tokotoko7777/ahc-workspace/blob/codex/ahc004-focus-20261121/problems/ahc004/experiments/local100/README.md)
- 単一ファイル SHA-256: `b5ce9a20f2bb20414f39be58314827bd1e8d4b63c970977a50d59bd26c3c81bb`
- ケース別結果 SHA-256: `53cc61143d9777f9f0bca0b4065b0c201f19a9d9ea66ddbbb8f805fd85626d4a`
- 公式入力ZIP SHA-256: `b0d1cd27284d96af3f409ee58304b661ddd2711400fbfbb37ffa60ad29d38cf9`

## 解法

入力に出た部分文字列から、行・列の循環と交差の候補を復元する。
完全な復元が難しい場合は、曖昧な部分を残して組み直す経路と、行・列の
まとまりを修復する経路も試す。候補の実際の収録語数を採点したうえで、
残り時間を焼きなましに割り当て、最良盤面を保持する。
短い単語が多く元の盤面の手掛かりが弱い場合は、復元だけで全語を収録するのが難しい。

## 未使用の追加100件

別の公式ジェネレータ seed 20,000,200〜20,000,299 では **8,714,472,941点**。
直前版の8,641,361,238点から+73,111,703点、全語収録48件→50件。
これは配布100件・目標点の判定とは別集計で、全200出力の採点は一致した。

## 再現

採点ツールを展開し、そのディレクトリで `cargo build --release --locked`。
`IN` に公式入力ZIPを展開した `in/`、`VIS` に公式 `vis` のパスを設定する。
このディレクトリで次を実行。ビルドに追加の最適化フラグは不要。

```sh
g++ -std=c++23 main.cpp -o solver
python3 benchmark.py "$IN" result --vis "$VIS" --entry "solver=$PWD/solver"
```

採点は直列。実行時間の比較は同じマシン・負荷条件で行う。
