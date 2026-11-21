# AHC007: Online MST
[公式問題文・採点規則](https://atcoder.jp/contests/ahc007/tasks/ahc007_a) /
[公式ツールと練習150ケース](https://img.atcoder.jp/ahc007/a855b6a456c9892b747e147001b0f89.zip)。

## 本番同一の150ケースでの比較

| 実装 | 150件の絶対点合計 |
|---|---:|
| 本番最終1位 | 14,241,733,941 |
| 本版（入力をローカル再現） | **14,248,878,730** |
| 差 | **+7,144,789** |

[AtCoder提供ALE-Benchデータ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc007.zip)
の full private 150件を使った。full が本番同一集合である根拠は
[論文Appendix A.2「Submission」](https://openreview.net/pdf?id=JCjGvbsOmQ)。
順位表の1行目、ケース数、2秒／1,024 MiB、各ケースの
`round(10^8 × 最小全域木長 / 採用辺長)` の和を確認した。
解を選ぶ前に本番入力やケース別結果を見ず、候補固定後に採点した。
AtCoderに再提出したわけではない。

全150件で添付の対話テスター・vis・独立整数採点が一致。
最長0.308726秒（逐次入出力込み）、solver CPU最長0.276794秒。
[集計とハッシュ](private-evaluation-summary.json)。
検証ソース: [d93d5e1](https://github.com/tokotoko7777/ahc-workspace/blob/d93d5e1851c8c973708e691d1ad97d70fffc1679/problems/ahc007/main.cpp)。

## 配布・未調整ケースの検証
- 公式配布150件: **14,255,819,331点**（研究開始時14,184,305,386から+71,513,945）。
  単一ファイルの全採否が研究版と一致、全150件が公式 vis と独立採点で一致。
  Linux上の逐次入出力込み最長0.223076秒、solver CPU最長0.211615秒。
- 公式生成器の別の未調整150件（seed 27,155,000〜27,155,149）:
  研究版 **14,249,050,741点**。同じ入力の旧版より+92,850,050。
  この生成集合も本番とは別。
- [練習ケース別結果](public-measurements.csv)、
  [別解・失敗した仮説・最適化の全記録](https://github.com/tokotoko7777/ahc-workspace/tree/fc71719c1e03c6f5d0aba21829892e887ff6c066/problems/ahc007/experiments/focus)。

## 解法
採択済みの森を縮約し、現在の両端が同じ成分なら棄却し、未来だけでは
両端を結べないなら採択する。この不変条件で最後まで連結な木を作る。
未来辺の各長さを一様分布から層化抽出し、最大辺が最小となる代替経路の
ボトルネック重みと、その重みを決めた辺を双方向探索で求める。
現在の長さが推定閾値と拮抗するときだけ、12標本から最大96標本へ増やす。

全ての未来を知った最適な入替え値には、オンラインで見送りを取り消せない
分の偏りがある。ボトルネックに選ばれた辺の標本長が幾何期待値より短く、
複数の候補が競っているほど、待つと選択肢を失うので閾値を上げる。
逆に、長い辺が直列のボトルネックとして現れやすいときは下げる。
これは補正モデルであり、本番の真の未公開の値を先読みしない。
固定乱数はアルゴリズムの標本生成用で、入力のケース番号には依存しない。
期限切れ辺を各頂点の隣接列からカーソルで外し、縮約更新も採択時だけ行う。
150件の採否は全件同一のままCPU中央値は0.183142→0.151289秒
（[同仕事量の比較](public-sweep-ab.json)）。

## 再現
公式ZIPを展開し、`tools/` で `cargo build --release --locked --bin vis`。
ルートから、配布150入力を `PUBLIC_INPUTS`、公式 `vis` のパスを `VIS` として:

```sh
g++ -std=c++23 problems/ahc007/main.cpp -o ahc007
python3 problems/ahc007/benchmark.py "$PUBLIC_INPUTS" result "$VIS" ahc007 ahc007
g++ -std=c++23 tests/weighted_bottleneck_witness_test.cpp -o witness-test
./witness-test
python3 -m unittest discover -s tests
```

`main.cpp` は GNU C++23 単一ファイルで、最適化指定はソース内。
編集用は [solver.cpp](solver.cpp) と [汎用ボトルネック探索](../../library/weighted-bottleneck-witness.hpp)。
生成コマンドは
`python3 scripts/bundle.py problems/ahc007/solver.cpp -o problems/ahc007/main.cpp`
の後、生成物の行末空白を除去する。


### ブラインド比較の再現（Linux/WSL）

`ALE_ZIP` は上記の ahc007.zip への絶対パス。非公開シードの値や個別スコアは
出力せず、ローカルの一時ディレクトリは終了時に削除する。

```sh
python3 -m zipfile -e "$ALE_ZIP" ale007
(cd ale007/ahc007/tools && cargo build --release --locked --bin gen --bin vis --bin tester)
ALE_TOOLS="$PWD/ale007/ahc007/tools/target/release"
g++ -std=c++23 problems/ahc007/main.cpp -o ahc007
python3 problems/ahc007/blind_replay.py "$ALE_ZIP" ./ahc007 \
    "$ALE_TOOLS/gen" "$ALE_TOOLS/vis" "$ALE_TOOLS/tester" blind-result.json
```
