# 探索の基本ガイド

まず状態・評価・合法手を小さく実装し、その形に合う探索を選びます。
部品ごとの呼び出し例は [USAGE.md](USAGE.md) にあります。
ここでは、新しい問題でどのヘッダを使い、どの処理を自分で書くかを整理します。

入力から出力までを含む21本の [問題別の使用例](examples/search/README.md) もあります。
ヘッダと問題固有のコードを分けて読める形で、提出時にヘッダを1ファイルへ貼り付けます。

## 最初に選ぶ

| 解の作り方 | まず使う部品 | 自分で用意する処理 |
|---|---|---|
| 完成した合法解を少しずつ変更する | [時間制限付き焼きなまし](library/time-based-simulated-annealing.hpp) | 近傍生成、改善量の評価、採用した変更の反映 |
| 良い候補を複数残しながら1手ずつ解を作る | [SimpleBeamSearch](library/simple-beam-search.hpp) | 子状態の生成、候補の順位値 |
| 確率的な未来を何度か試して今の手を選ぶ | [MonteCarloTreeSearch](library/monte-carlo-tree-search.hpp) | 合法手、確率遷移、終端判定、試行の報酬 |
| 各候補の先を決定的に仮実行して比べる | [DeterministicRolloutRunner](library/deterministic-rollout.hpp) | 合法手、その手を選んだ後の先読み評価 |

入力から必ず合法解を1つ作り、評価値を全再計算する関数も先に書きます。
局所探索ではその解を初期解にし、途中で良い解が得られたら別に保存します。
構築型の探索では、制限時間や候補切れで止まった時に出力できる完成解も確保してください。

## 共通の組み込み方

1. 問題フォルダの `main.cpp` に、変化する情報を持つ `State` と1手を表す `Move` / `Action` を書く。
2. 固定の入力は状態ごとにコピーせず、探索の外か `Problem` に置く。
3. 選んだヘッダの説明に合わせて、候補生成・評価・更新の処理をつなぐ。
4. 小さい入力で、合法性・得点・巻き戻しを確認してから探索量を増やす。
5. 提出時は必要なヘッダを展開し、単独の `main.cpp` をもう一度コンパイルする。

ローカルでは `#include "library/simple-beam-search.hpp"` のように読み込みます。
コンパイル方法は [README](README.md#コンパイル)、そのまま実行できる小さな例は
[basic_usage.cpp](examples/basic_usage.cpp) を参照してください。

## 焼きなましと山登り

焼きなましでは、近傍を作り、改善量を計算し、採用した時だけ現在解を変えます。
最大化の改善量は `新得点 - 旧得点`、最小化では `旧コスト - 新コスト` です。
温度は改善量と同じ尺度なので、実際の近傍でどの程度の悪化が出るかを見て調整します。

- [TimeBasedSimulatedAnnealing](USAGE.md#タイマー内蔵の焼きなまし): 時計と採用判定をまとめたい時。
- [SimulatedAnnealing](USAGE.md#進捗率を外から渡す焼きなまし): 複数フェーズで時計を共有し、進捗率を外から渡したい時。
- 同じヘッダの `TimeBasedAnnealingRunner`: `Problem` に近傍・評価・更新をまとめ、最良解や反復数も管理したい時。

`TimeBasedAnnealingRunner::run_hill_climbing()` は改善量が正の変更だけを採用します。
悪化を許さないので調整箇所は減りますが、1手では改善しない場所に止まりやすくなります。
自分でループを書く場合は、同点の移動を許すかどうかも明示してください。

近傍の評価が重い場合は、まず変わった部分だけを再計算します。
行動列を先頭からシミュレーションする評価なら
[PrefixReplay](USAGE.md#行動列を途中からだけ再生する) で共通の前半を再利用できます。
閾値による途中打ち切りは、残りを計算しても採用されないことを保証できる場合に使います。

## 大きく変更してから直す

細かい近傍だけで動けなくなる場合は、変更する範囲を広げます。

| 部品 | 使いどころ | コールバックで返すもの |
|---|---|---|
| [IteratedLocalSearch](library/iterated-local-search.hpp) | 摂動を加え、局所探索で再び改善する | `local_search` は合法な完成解を残し、その絶対スコアを返す |
| [LargeNeighborhoodSearch](library/large-neighborhood-search.hpp) | 一部を壊し、修復した完成解を採用する | `repair` は修復後の絶対スコア、修復不能なら `nullopt` |
| [MultiStart](USAGE.md#multistart) | 異なる初期解やseedから短い探索を繰り返す | 各試行で得られた最良解と得点を呼び出し側で比較する |

修復中は一時的に不正な状態になり得ます。失敗や時間切れの時に、前の合法解が残るようにします。
ILSの局所探索など重いコールバックの内側でも、渡された残り時間を確認してください。
LNSの返り値は改善量ではなく絶対スコアで、焼きなましの `accept` とは契約が異なります。

## ビームサーチの形式

| 部品 | 選ぶ目安 |
|---|---|
| [SimpleBeamSearch](USAGE.md#simplebeamsearch) | 状態が小さく、子状態をコピーして作れる。最初の実装に使いやすい |
| [ActionBeamSearch](library/action-beam-search.hpp) | 状態コピーが重い。軽い行動と次状態の順位値で先に候補を絞りたい |
| [TreeBeamSearch](USAGE.md#treebeamsearch) | 状態コピーを減らすため、1手進める `apply` と正確な `revert` を書ける |
| [CostTreeBeamSearch](USAGE.md#costtreebeamsearch) | 行動ごとに消費手数が異なり、同じ到着世代の候補を比較したい |
| [ChokudaiSearch](library/chokudai-search.hpp) | 深さごとの候補を残し、時間内に浅い層から繰り返し展開したい |

評価関数は「その候補の順位値」を返します。親からの得点差だけではありません。
途中の順位値と最終得点が異なる場合は、完成した解の良さを別に管理します。
幅を広げる前に、評価が良い途中状態から実際に良い完成解が作れるかを確認してください。

木上版の `revert` は、盤面だけでなく、得点・hash・cache・候補集合も戻します。
同じ盤面へ複数の手順で到達する場合は、対応する `step_with_key` と
[ZobristHash](USAGE.md#zobristhash) などで重複を減らせます。
64bit hashにも衝突はあり、鍵が同じ状態をまとめてよいかは問題側で判断します。

## モンテカルロ木探索

[MonteCarloTreeSearch](library/monte-carlo-tree-search.hpp) は試行結果を木に蓄積し、
次に調べる枝をUCTで選びます。`Problem` には `State`、`Action` と次の処理を書きます。

| 処理 | 契約 |
|---|---|
| `generate_actions(state)` | 合法手を列挙する |
| `is_terminal(state)` | 終端かどうかを返す |
| `sample_transition(state, action, rng)` | 仮の状態を更新し、同じ親・同じ行動から出た結果を区別する `uint64_t` のIDを返す |
| `rollout(state, rng)` | 仮の状態から先を試し、終端または打ち切り時の評価を `[0, 1]` に正規化して返す |

違う確率的結果に同じIDを返すと、その後の統計が混ざります。決定的遷移ならIDは0で構いません。
実際の状態は探索の外で保持し、`choose_action` が返した手を実環境へ適用します。
終端や合法手なしの場合に返る `nullopt` も処理してください。
この部品の時間制限は試行の区切りで確認されるため、1回の `rollout` が長すぎないようにします。

## 差分更新を検査してから高速化する

[検査例](examples/debug/README.md) では、経路swapと1マス更新について、
差分評価とは別に全再計算した得点・hash・補助情報を照合します。
採用しない手を戻した後や、複数の手を逆順に戻した後も検査してください。

まず固定seed・固定反復数で不一致を再現できるようにし、その後に時計確認の間引き、
状態コピーの削減、候補の絞り込みを進めます。固定seedでも、時間で終了する探索の反復数は
実行環境によって変わるため、同じ出力になるとは限りません。
