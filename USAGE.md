# 使い方: 雛形を選ぶ → 問題部分を書く → 1ファイルで提出

コマンドはリポジトリのルートで実行します。C++17と、コマンド展開を使う場合だけPython 3.9以上が必要です。
ヘッダの個別APIは [リファレンス](docs/LIBRARY_REFERENCE.md)、探索手法の判断は [SEARCH_GUIDE.md](SEARCH_GUIDE.md) に分けています。

## まず開くファイル

最初は [`main.cpp`](main.cpp) を開いてください。完成解をコピーして変更する焼きなましの雛形です。
問題固有の型・関数はTODO付きの宣言、探索ループは接続済みです。
**未編集の雛形は実行用サンプルではありません。TODOの型と関数を実装して使います。**

| 状況 | コピーするファイル |
|---|---|
| まず合法解から局所探索を始めたい | [main.cpp](main.cpp) |
| 差分評価で焼きなまし / 山登りしたい | [basic.cpp](template/search/local-search/basic.cpp) |
| 小さい状態でビームを組みたい | [simple.cpp](template/search/beam/simple.cpp) |
| 一部を壊して修復したい | [destroy-repair.cpp](template/search/local-search/destroy-repair.cpp) |
| 状態コピー削減、巻き戻し、可変手数、行動列評価、先読み | [雛形一覧](template/README.md) |
| 実問題の入力から出力まで読みたい | [問題別の完成例](examples/search/README.md) |
| 編集せず小さなコードを動かしたい | [basic_usage.cpp](examples/basic_usage.cpp)（入力不要） |

## 問題ごとに書く部分

```sh
cp main.cpp problems/ahc004/main.cpp
```

コピーしたファイルで、次を実装します。既に自分の解法がある場合は、必要なincludeと呼び出しだけ取り込めます。

| 箇所 | 書く内容 |
|---|---|
| `Problem` のフィールドと入力 | 固定入力、距離表などの前計算 |
| `State` | 変化する解と、差分更新に必要なcache |
| 初期解 | 制約を満たし、時間切れでも出力できる完成解 |
| 評価 | 正式な得点の全再計算。高速化時の検算にも使う |
| 近傍 / 遷移 | 問題に合う変更、合法性判定、採用時の更新 |
| 出力・探索定数 | 解の出力形式、制限時間、温度、seed、ビーム幅など |

評価の戻り値は手法によって異なります。ここを合わせてから差分化します。

| 接続先 | 渡す値 / 返す値 | 状態の変更 |
|---|---|---|
| `main.cpp` の `evaluate` | 完成解の絶対スコア | 変更しない |
| SAの `accept` / Runnerの `evaluate_move` | **改善量**。最大化: 新−旧、最小化: 旧−新 | 評価中は変更しない。採用時だけ反映 |
| LNSの `repair` | 修復後の**絶対スコア**。失敗は `nullopt` | 候補だけを修復する |
| ビームの評価 | 子状態の**順位値そのもの** | Action型は評価中に変更しない。Tree型はapply / revert |
| Rolloutの `evaluate_action` | その手を選んだ後の仮実行結果の評価値 | 実状態は変更しない |

雛形の局所探索は最大化を基本にしています。距離最小化なら、絶対スコアを `-distance` に揃えると使いやすくなります。
ビームの途中順位値と正式な最終得点は分けて管理します。

## 手元で実行する

TODOを実装した後、ルートをインクルードパスに追加してコンパイルします。

```sh
g++ -O2 -std=c++17 -Wall -Wextra -I . problems/ahc004/main.cpp -o /tmp/solver
/tmp/solver < input.txt > output.txt
```

`#include "library/xxx.hpp"` はコピー先に依存せず、`-I .` で見つかります。
元の `examples/search/*.cpp` は `../../library/` を使うものがあるので、別の深さへコピーした場合は
`library/` 形式へ直します。対話問題は、その問題のローカルテスタで確認します。

差分評価や巻き戻しを実装したら、[debugの完成例](examples/debug/README.md) のように全再計算と照合します。
入力・初期化時間も制限時間に含め、重い近傍・1層の展開・修復処理の内側でも必要に応じて時計を確認します。

## 提出用の1ファイルを作る

AHCに提出するのは、ローカルヘッダに依存しない **1つのC++ファイル** です。
手動で貼る方法とコマンドで展開する方法のどちらでも構いません。

### 手動で貼る

1. 自分のソースを別の `submit.cpp` へコピーする。
2. `#include "library/xxx.hpp"` を、そのヘッダの全文へ置き換える。ローカルヘッダ内のローカルincludeも展開する。
3. 同じヘッダを二重に貼らない。標準の `#include <...>` は残す。
4. ヘッダ内の出典コメントを残し、[LICENSE](LICENSE) の著作権・許諾表示をコメントとして先頭に含める。
5. 以下の「展開後の確認」を行う。

このリポジトリのヘッダにはinclude guardがないものもあります。単純に同じ内容を二重に貼ると再定義になります。
条件付きincludeがある場合は、提出時に必要なヘッダが有効な位置に残るように整理してください。

### コマンドで展開する（任意）

[`scripts/bundle.py`](scripts/bundle.py) はPython標準ライブラリだけで動きます。追加インストールは不要です。

```sh
python3 scripts/bundle.py problems/ahc004/main.cpp -o /tmp/submit.cpp
```

別のヘッダ置き場を使う場合は `-I` を追加できます。探索順は「include元のディレクトリ → 指定した `-I` → このリポジトリのルート」です。

```sh
python3 scripts/bundle.py problems/ahc004/main.cpp -I my_headers -o /tmp/submit.cpp
```

- `#include "..."` を再帰展開し、`#include <...>` はそのまま残します。第三者ヘッダを山括弧で読み込む場合は、提出環境で提供されている必要があります。
- ヘッダごとのguardを付けるので、複数箇所から読んでも定義は一度だけ有効になります。条件分岐内での読み込みにも対応するため、出力テキストに同じヘッダ本文が複数回現れる場合があります。
- このリポジトリのMIT表示を先頭に追加します。元ソースとヘッダは変更しません。
- 存在しないinclude・循環includeはエラーにします。マクロによるinclude、複数行のinclude、X-macroなど同じファイルを繰り返し評価する用途には対応しません。
- `-o` を省略すると標準出力へ書きます。入力ファイル自身へのリダイレクトはせず、別ファイルを指定してください。`-o` では入力・依存ヘッダへの上書きを検出します。

### 展開後の確認

```sh
# -I . を付けず、生成したファイルだけをコンパイルする。
g++ -O2 -std=c++17 -Wall -Wextra /tmp/submit.cpp -o /tmp/submit_solver
/tmp/submit_solver < input.txt > /tmp/submit_output.txt
```

この `/tmp/submit.cpp` を提出します。時間制限で終わる探索では反復数が変わるため、
ローカル版と出力が一致するとは限りません。合法性と得点をテスタで確認します。
生成後の修正は元の `main.cpp` に行い、再度展開します。

## ライブラリを探す・追加する

[APIリファレンス](docs/LIBRARY_REFERENCE.md) は目的別の索引から個々のヘッダへ進めます。
[探索ガイド](SEARCH_GUIDE.md) は手法の判断、[雛形一覧](template/README.md) はコードの接続を担当します。
新しい部品を追加する場合は、問題固有の処理を外へ出し、必要な契約と使用例を一緒に置きます。

結合スクリプトの検証:

```sh
python3 -m unittest discover -s tests -v
```
