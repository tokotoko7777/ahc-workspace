# 編集して使うC++雛形

**問題固有の型・関数を実装して、自分の `main.cpp` にするためのファイルです。**
動作する架空の問題を埋め込まず、必要な契約と探索への接続を示しています。
TODOの関数は宣言だけなので、未編集ではリンクできません。
入出力込みの完成例は [examples/search](../examples/search/README.md) にあります。

未取得の場合は [コンテスト開始時の準備](../USAGE.md#コンテスト開始時の準備) から始めます。
コマンドはリポジトリのルート（`AHCXXX/`）で実行します。使いたい1本をコピーして編集します。

```sh
cp template/search/local-search/basic.cpp problems/ahc004/main.cpp
# 型・関数・定数のTODOを実装した後:
g++ -O2 -std=c++17 -I . problems/ahc004/main.cpp -o /tmp/solver
/tmp/solver < input.txt > output.txt
```

固定入力は `Problem`、探索で変化する情報は `State` に置きます。
各ファイルは相互依存せず、`library/` の必要なヘッダだけを使用します。
先頭の `AHC_SOURCE` コメントはコピー元の記録として残します。
[出典付き結合](../docs/SOURCE_PROVENANCE.md) では、指定した公開版の雛形URLになります。

## どれをコピーするか

| やりたいこと | 雛形 | 自分で書く中心部分 |
|---|---|---|
| 完成解をコピーして少し変更する | [ルートのmain.cpp](../main.cpp) | 合法な候補の生成、全スコア評価 |
| コピー・全評価を減らして焼きなまし / 山登り | [local-search/basic.cpp](search/local-search/basic.cpp) | Move生成、改善量、採用時の更新 |
| 一部を壊して作り直す | [local-search/destroy-repair.cpp](search/local-search/destroy-repair.cpp) | 破壊、修復、修復後の絶対スコア |
| 行動列の共通する前半を再利用する | [local-search/prefix-replay.cpp](search/local-search/prefix-replay.cpp) | 1手の決定的遷移、行動の等値比較、行動列の変更 |
| 確率遷移の期待値を1操作変更で改善する | [local-search/forward-backward.cpp](search/local-search/forward-backward.cpp) | 遷移確率・報酬、初期分布、操作変更 |
| 小さい状態をコピーしてビームを始める | [beam/simple.cpp](search/beam/simple.cpp) | 子状態の列挙、順位値 |
| 重い状態のコピーを選抜後だけにする | [beam/action.cpp](search/beam/action.cpp) | 軽いAction、子の順位値、適用 |
| 状態を進めて戻しながらビーム探索する | [beam/tree.cpp](search/beam/tree.cpp) | apply / revert、順位値 |
| 1行動で進む手数が異なるビーム | [beam/variable-cost-tree.cpp](search/beam/variable-cost-tree.cpp) | apply / revert、正の消費手数 |
| 各候補の先を仮実行して1手選ぶ | [advanced/deterministic-rollout.cpp](advanced/deterministic-rollout.cpp) | 合法手、先読み評価、実状態の更新 |

## 実装時に合わせる契約

- 得点の向き: 雛形は最大化を基本にしています。距離最小化なら負の距離へ統一すると接続しやすくなります。LNSは `options.maximize = false` も選べます。
- 評価値: SA Runnerは改善量、LNSは修復後の絶対スコア、ビームは子の絶対順位値を返します。
- 時間: 時計は入力前から開始します。外側の時間判定だけでは、重い1回の近傍・1層の展開・先読みを中断できません。必要なら `Problem` に時計への参照を持たせ、内側でも打ち切ります。
- 完成解: ビームの順位値と最終得点は別に扱います。初期状態と全生成子の完成判定を行い、時間切れ・候補切れでも保存済みの合法解を出力します。
- 更新: 差分型SAは評価中に現在解を変更せず、採用時だけ反映します。TreeBeamは得点・hash・cacheも含めて完全に巻き戻します。

提出時の手動貼り付けとコマンド展開は [USAGE.md](../USAGE.md#提出用の1ファイルを作る) を参照してください。
雛形を増やす時も、TODO・値の契約・呼び出し・完成解の出力までを1本にまとめます。
