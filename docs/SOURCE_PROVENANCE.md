# 出典付きで提出用ファイルを作る

2026-11-19に確認した [長期AHCの公式ルール](https://info.atcoder.jp/entry/long-ahc-llm-rules-ja) では、
Human Only部門で事前に生成AIを用いて作ったコードを使う場合、開始前に公開し、
提出コードの該当部分へ参照URLを記載する必要があります。参加回・部門の適用ルールも確認してください。

このリポジトリでは、**大会開始前に公開したコミット**を選び、その版へのURLを残す運用にします。
Gitのコミット日時だけではWebでの公開日時は証明できません。手元にあるだけのコミットや、
大会開始後に初めて公開したコミットを、開始前の公開版として扱わないでください。

## 開始前に版を決める

リポジトリを取得・更新し、使うライブラリや雛形が公開済みになった時点で、
リポジトリのルート（`AHCXXX/`）からコミットIDを記録します。

```sh
mkdir -p build
git rev-parse HEAD > build/source-ref.txt
cat build/source-ref.txt
```

GitHubで `https://github.com/tokotoko7777/ahc-workspace/tree/<記録したコミットID>` を開き、
第三者がコードを閲覧できることと、開始前に公開していたことを確認します。
`build/source-ref.txt` は提出まで残します。途中でライブラリを更新した場合も、
記録した版が現在のヘッダと一致しているか、結合時に検査されます。

## 雛形をコピーする

```sh
mkdir -p problems/ahc004
cp main.cpp problems/ahc004/main.cpp
```

雛形先頭には次のようなコピー元の記録があります。問題用に編集するときも残します。

```cpp
// AHC_SOURCE: main.cpp
// Source repository: https://github.com/tokotoko7777/ahc-workspace
```

手法別の雛形には `template/search/beam/simple.cpp` など、そのファイルのパスが入っています。
`AHC_SOURCE` はコピー元の指定です。編集後の全コードが事前公開版と同一だと主張するものではありません。

## 出典付きで結合する

```sh
python3 scripts/bundle.py problems/ahc004/main.cpp \
  --source-ref "$(cat build/source-ref.txt)" -o build/submit.cpp
g++ -O2 -std=c++17 build/submit.cpp -o build/submit_solver
```

生成結果には次のURLがコメントとして付きます。

- 展開した `library/` の各ヘッダ: `.../blob/<コミットID>/library/xxx.hpp`
- `AHC_SOURCE` で指定したコピー元: `.../blob/<コミットID>/main.cpp` など。問題固有の編集前の元コードとして記載します。

`--source-ref` はコミットID・タグ等を受け取り、ローカルGitから完全なコミットIDへ解決します。
各ライブラリヘッダがその版に存在し、手元と同じバイト列か検査します。違えば生成を止め、
既存の出力ファイルを維持します。雛形は問題用に編集するため、コピー元の存在を確認します。
公開の事実や公開時刻の検証、コンテスト参加資格の判定を行う機能ではありません。

URLはこの `tokotoko7777/ahc-workspace` を参照します。別のリポジトリのコードや
`-I` で追加した外部ヘッダは、その出典を手動で残してください。
この機能を使わない通常の結合でもリポジトリURLとMIT表示は付きますが、版は固定されません。

## 手動で結合する場合

コピーするヘッダや雛形のGitHub画面で、ブランチ名ではなく開始前に公開したコミットIDのURLを使い、
該当部分の直前へコメントとして貼ります。元からある出典と著作権・許諾表示も残します。

```cpp
// Source: https://github.com/tokotoko7777/ahc-workspace/blob/<公開済みコミットID>/library/coverage-counter.hpp
// この下にヘッダの内容を貼る。
```

[使用手順へ戻る](../USAGE.md#提出用の1ファイルを作る)
