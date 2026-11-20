# ahc-workspace

AtCoder Heuristic Contest（AHC）の練習と事前準備用のC++コードです。
汎用の部品を選び、入力・状態・近傍・評価は問題に合わせて書くことを想定しています。

**[手法別の雛形](template/README.md) から、使いたい1本を選んでください。**
各ファイルに問題ごとに実装するTODOと、探索への接続を示しています。
雛形は未実装部分を含み、そのまま実行するためのコードではありません。

| ジャンル | 雛形の入口 |
|---|---|
| 焼きなまし・山登り | [全評価](template/sa/full-score.cpp)、[差分評価](template/sa/delta.cpp)、[行動列・期待値の評価](template/README.md#焼きなまし山登り) |
| ビームサーチ | [状態コピー型](template/beam/simple.cpp)、[Action型・木上型・可変手数型](template/README.md#ビームサーチ) |
| 破壊・修復（LNS） | [部分的に壊して作り直す](template/lns/destroy-repair.cpp) |
| 先読み（Rollout） | [候補の先を仮実行して1手選ぶ](template/rollout/deterministic.cpp) |

## 使い始める

コンテストごとに `AHCXXX` フォルダへリポジトリ全体を取得して使えます。
初回は `git clone` でフォルダごと作成します。以下はAHC004の例なので、参加する回の番号に置き換えてください。
コマンドはBash（WindowsではGit Bashなど）で実行します。

```sh
git clone https://github.com/tokotoko7777/ahc-workspace.git AHC004
cd AHC004
mkdir -p problems/ahc004 inputs outputs build
# 大会開始前に、使う版が公開済みであることを確認して記録する。
git rev-parse HEAD > build/source-ref.txt
# 例: 全評価の焼きなまし。使いたい手法の雛形に置き換えられる。
cp template/sa/full-score.cpp problems/ahc004/main.cpp
# コピー先のTODO（型・入力・初期解・評価・近傍・出力）を実装する。
# 公式のテスト入力を inputs/0000.txt に用意してから実行する。
g++ -O2 -std=c++17 -I . problems/ahc004/main.cpp -o build/solver
./build/solver < inputs/0000.txt > outputs/0000.txt
```

以降のコマンドも `AHC004/`（リポジトリのルート）で実行します。
取得済みのフォルダを更新する時は、編集を始める前にその中で `git pull --ff-only` を使います。
作業フォルダの配置と取得済みの場合の手順は [コンテスト開始時の準備](USAGE.md#コンテスト開始時の準備) を参照してください。

AHCの提出は1ファイルにまとめます。必要なヘッダを手で貼っても、次のコマンドで展開しても構いません。

```sh
# Python 3.9以上。追加パッケージは不要。
python3 scripts/bundle.py problems/ahc004/main.cpp \
  --source-ref "$(cat build/source-ref.txt)" -o build/submit.cpp
# ローカルincludeパスなしでコンパイルし、テスタで確認してからsubmit.cppを提出。
g++ -O2 -std=c++17 build/submit.cpp -o build/submit_solver
./build/submit_solver < inputs/0000.txt > outputs/submit_0000.txt
```

提出するファイルは `build/submit.cpp` です。
`--source-ref` には開始前に記録した版を渡し、提出時の `HEAD` で記録を上書きしません。
公開版の確認と出典の仕組みは [出典付き結合](docs/SOURCE_PROVENANCE.md)、
Windowsの既存クローンで改行差分が出る場合は [改行の確認](USAGE.md#windowsの改行) を参照してください。
[使い方](USAGE.md) に、初回の準備、実装箇所、評価値の契約、手動結合とコマンド展開の詳細をまとめています。

## 目的別の入口

| 目的 | 開くもの |
|---|---|
| 自分の問題用のmain.cppを書き始める | [手法別の雛形](template/README.md) |
| 手順と提出方法を確認する | [USAGE.md](USAGE.md) |
| 探索手法を選ぶ | [SEARCH_GUIDE.md](SEARCH_GUIDE.md) |
| 部品の引数・戻り値を調べる | [APIリファレンス](docs/LIBRARY_REFERENCE.md) |
| 実問題での接続を見る | [21本の探索使用例](examples/search/README.md) |
| 小さな動作例を実行する | [basic_usage.cpp](examples/basic_usage.cpp) |
| 差分更新・巻き戻しを検査する | [debug使用例](examples/debug/README.md) |

## 困りごとからコードを選ぶ

| 困りごと | コピーするファイル / 接続例 | 動く過去問例 |
|---|---|---|
| 完成解を少しずつ改善したい | [全評価型SA](template/sa/full-score.cpp)、[差分型SA](template/sa/delta.cpp) | [AHC006: 経路の差分評価](examples/search/ahc006_sa.cpp) |
| 同じ対象を複数箇所で覆うので、削除時の失点を数えにくい | [CoverageCounterの接続例](examples/coverage_usage.cpp) | [AHC004: 行・列のヒットを差し替える](examples/search/ahc004_genome_sa.cpp) |
| 重い状態を全候補にコピーしたくない | [ActionBeamの雛形](template/beam/action.cpp) | [AHC032: スタンプ配置](examples/search/ahc032_action_beam.cpp)、[AHC071: 行単位の構築・再構築](examples/search/ahc071_action_beam.cpp) |
| 盤面を進めて戻しながら探索したい | [TreeBeamの雛形](template/beam/tree.cpp) | [AHC011: apply/revertと履歴復元](examples/search/ahc011_tree_beam.cpp) |
| 優先辺を残しながら頂点を重複のないパスに分けたい | [GreedyPathForest](docs/LIBRARY_REFERENCE.md#greedypathforest) | [AHC036: コピー元の並びを作る](problems/ahc036/README.md) |
| 行動ごとに消費手数が違う | [可変手数ビームの雛形](template/beam/variable-cost-tree.cpp) | [AHC038: 把持・解放までを1行動にする](examples/search/ahc038_variable_cost_beam.cpp) |
| 解の一部を壊して作り直したい | [LNSの雛形](template/lns/destroy-repair.cpp) | [AHC002: 経路の部分修復](examples/search/ahc002_destroy_repair_lns.cpp)、[AHC059: ペア再挿入](examples/search/ahc059_lns.cpp) |
| 行動列を評価するたび、先頭から再実行している | [PrefixReplayの雛形](template/sa/prefix-replay.cpp) | [AHC058: 購入列の途中から再実行](examples/search/ahc058_prefix_sa.cpp) |
| 有限状態の確率遷移で1操作の変更を速く評価したい | [ForwardBackwardDPの雛形](template/sa/forward-backward.cpp) | 過去問の完成例は未追加。[APIと適用条件](docs/LIBRARY_REFERENCE.md#forwardbackwarddp) |
| 候補の先を仮実行して、最初の1手だけ選びたい | [決定的先読みの雛形](template/rollout/deterministic.cpp) | [AHC026](examples/search/ahc026_deterministic_rollout.cpp)、[AHC058](examples/search/ahc058_deterministic_rollout.cpp) |

`problems/ahcXXX/` が空でも、`examples/search/` にその回の実装がある場合があります。
実問題の入力形式は各例に従い、対話形式の例は対応するローカルテスタで実行します。

## 構成

- `library/`: 再利用できるC++ヘッダ101個。
- `template/`: 手法ごとに分類した10本の雛形。選んだ1本を問題用の `main.cpp` にコピーします。
- `examples/`: 実行できる使用例。
- `scripts/bundle.py`: 提出用1ファイルへの任意の展開コマンド。
- `problems/ahc001/` 〜 `problems/ahc074/`: 各回の作業フォルダ。AHC004以降は空です。

| 問題 | 状態 |
|---|---|
| [AHC001](problems/ahc001/) | 実装済み。公式シード30件で検証 |
| [AHC002](problems/ahc002/) | 実装済み。追加20件で検証 |
| [AHC003](problems/ahc003/) | 現状版を保存。合計50件で検証し、比較の目安は未達 |

各問題の既存 `main.cpp` はC++17の単独ファイルとしてコンパイルできます。
AHC003は対話問題なので、対応する公式ローカルテスタで実行します。
入力・出力・実行ファイル・公式ツールは各作業フォルダに置けますが、Gitには含めません。

## ライセンス

**自由に使ってください。** [MIT License](LICENSE) で公開しています。
利用・改変・再配布・商用利用ができます。コピーや再配布時は著作権表示と許諾表示を保持してください。
コンテストでの利用は参加する回のルールに従ってください。
Human Only向けに公開版の出典を残す方法は [出典付きで提出用ファイルを作る](docs/SOURCE_PROVENANCE.md) を参照してください。
参考リンクは [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) にまとめています。
