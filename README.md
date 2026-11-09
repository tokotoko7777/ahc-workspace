# ahc-workspace

AtCoder Heuristic Contest（AHC）の練習と事前準備用のC++コードです。

**自由に使ってください。** 本リポジトリのコードは [MIT License](LICENSE) で公開します。
利用・改変・再配布・商用利用ができます。コピーや再配布の際は、著作権表示と許諾表示を保持してください。
コンテストでの利用は、参加する回のルールに従ってください。

## 構成

- `library/`: 再利用できるC++ヘッダ100個。
- `problems/ahc001/` 〜 `problems/ahc074/`: 各回を解くための作業フォルダ。
- AHC001〜003には独立に実装した解法を置いています。AHC004以降は空の作業フォルダです。

| 問題 | 状態 |
|---|---|
| [AHC001](problems/ahc001/) | 実装済み。公式シード30件で検証 |
| [AHC002](problems/ahc002/) | 実装済み。追加20件で検証 |
| [AHC003](problems/ahc003/) | 現状版を保存。合計50件で検証し、比較の目安は未達 |

## コンパイル

各問題の `main.cpp` はC++17の単独ファイルとしてコンパイルできます。

```sh
g++ -O3 -std=c++17 problems/ahc001/main.cpp -o /tmp/ahc001
/tmp/ahc001 < input.txt > output.txt
```

AHC003は対話問題なので、実行には[公式問題ページ](https://atcoder.jp/contests/ahc003/tasks/ahc003_a)のローカルテスタを使います。
入力・出力・実行ファイル・公式ツールは各作業フォルダに置けますが、Gitには含めません。

ライブラリを使うプログラムでは、リポジトリのルートをインクルードパスに追加してください。

```sh
g++ -O3 -std=c++17 -I . problems/ahc004/main.cpp -o /tmp/ahc004
```

ライブラリ内の参考リンクについては [参考情報](THIRD_PARTY_NOTICES.md) を参照してください。
