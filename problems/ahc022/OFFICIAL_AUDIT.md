# AHC022 最終ケース照合と相対得点

対象は低配置費版の固定済みコミット
`573aa89a8c06bd2c6d4d8df5810b42bbe1931979`（公開ブランチ実装
`86f0b8b8dc3d75a877626672b36b2dec88da2e05`）。
実装や調整値を変更せず、同じC++23単一ファイルで測った。

## 本番入力の再現

- [公式問題文](https://atcoder.jp/contests/ahc022/tasks/ahc022_a)の
  [公表seedファイル](https://img.atcoder.jp/ahc022/seeds.txt)を、
  [公式ツール](https://img.atcoder.jp/ahc022/u4u4grds7n.zip)同梱
  `README_en.html`の通常手順どおり、**変更せずgenに渡す**。
- [公式アーカイブの入力メタデータ](https://img.atcoder.jp/ahc_standings/ahc022/input.csv)と
  seed列を順序込みで一致確認。ファイル番号0000〜2999、
  公式genの `L,N,S` も全3000件で一致し、30種類のS各100件。
  公開表には入力ファイル自体の個別ハッシュはないため、これは公式genの
  通常モードによる再現であり、未公開ファイルとの直接バイト比較ではない。
- genに `seed 0 0 S` を渡す上書きモードは採用しない。
  ヘッダ自体は全件一致しても、S抽選を飛ばして乱数列が変わるため、
  通常モードと2,074件の本文が異なった。このモードの得点は判定に使わない。
- 相対点は[公式の得点表](https://img.atcoder.jp/ahc_standings/ahc022/result.csv)
  の各列の0以上の最大値から再計算した。列順は
  [公式ビューア](https://img.atcoder.jp/ahc_standings/index.html)と同じ。
  historical MAXでJirotechの1位と2,808,216,532,758点を再現した。

## 結果

| 相対点の基準 | 低配置費版 | 歴史的最終1位・Jirotech |
| --- | ---: | ---: |
| 歴史的MAX固定 | 271,023,954,031 | 2,808,216,532,758 |
| 低配置費版も順位対象に追加 | 271,023,954,031 | 2,808,216,532,758 |

**未達**。歴史的ケース最大値を超えたケースは0件なので、
両基準は同じ。歴史的1063人に加えると仮想307位（同点なし）。
全3000件が合法かつ誤答0、独立採点一致、絶対点合計37,578,375,772、
壁時計最大0.224637秒。1位との差は主に費用競争であり、現在の版をマージしない。

## 物差し

同じケースの絶対点を A、歴史的な最大絶対点を M とする。
まず両者を固定Mで計算する。さらに自分を順位対象に追加したと仮定し、
各ケースで `max(M, 自分のA)` を全員に等しく適用して再比較する。
丸めは正整数について `(2*10^9*A + M)//(2*M)` とし、
不成立の負値は0とする。固定Mで自分のAがMを超えた場合は10^9を
超え得るので、その値は固定基準の診断であり、追加後の公式式ではない。

Rust testerの各ケース絶対点・誤答・配置費・計測費・回数は、出力から
独立に計算した整数式と照合する。時刻は対話テスタを含む単発壁時計。
外部に提出はしていない。個別ケースの得点や入力はコミットしない。

## 再現コマンド

公式ファイルを上記リンクから保存し、リポジトリルートで以下を実行。
`seedfile`、`input_csv`、`result_csv` は保存した絶対パス。
公式ツールのZIP SHA-256は
`d4bc4f50335717fe862a2d822c500fdb5d09addd638c5b3518e247ee660ff58d`。

```sh
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc022/main.cpp -o "$work/solver.cpp"
g++ -std=c++23 -O3 "$work/solver.cpp" -o "$work/solver"
python problems/ahc022/official-audit.py "$seedfile" "$input_csv" "$result_csv" \
  "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" \
  "$work/solver" "$work/audit"
```

測定環境はLinux、g++ 14.2.0、空けたCPU8に1ワーカーを固定。
単一ファイルSHA-256:
`02a2f39f575ab4b15db93c897202d5883b2141b239ee98198fb0bf18325b26c1`。
各元ファイル・バイナリ・3000入力連結のハッシュと集計は
[official-audit-summary.json](official-audit-summary.json)。
