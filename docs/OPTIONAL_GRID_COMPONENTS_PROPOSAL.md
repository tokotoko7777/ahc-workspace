# 固定した候補セル集合に対する連結成分集計

同じ4近傍グリッドで、障害物候補の開閉だけを何度も評価するための部品です。常時開いているセルの連結成分を一度だけ縮約し、各評価では候補セルとその隣接成分だけを探索します。面積はセル数として組み込み、人数・動物数など任意の独立した加算量はテンプレート引数で列数を指定できます。重みが不要なら `optional_grid_components<>` を使います。候補数に64セルの制限はありません。

グリッドをNセル、候補をkセル、重み列をDとすると、前処理は O(N(D+1))、1回の評価は O(1+k(D+1)) です。候補に接しない成分は `unaffected()` で一度だけ取得できます。全成分を毎回列挙する必要がある場合は、その出力にかかる時間が別途必要です。個々のセルの成分IDや重みの動的更新には対応しません。各列の集計途中で符号付き64ビット整数を越えないことが前提です。

独立した単純実装との照合は小盤面の全列挙、隣接候補、固定成分、二つの符号付き重み、65番目以降の候補と面積のみの利用を対象にしました。実際のソルバへの組み込み、処理速度の比較、競技上のスコア評価はまだ行っていません。

[ヘッダ](../library/optional-grid-components.hpp) / [使用例](../examples/optional_grid_components_usage.cpp) / [差分テスト](../tests/optional_grid_components_test.cpp)

候補の順番は `optional_cells()` が返す行優先のセル番号順です。各候補には `1`（開く）または `0`（閉じる）を渡します。候補どうしの辺も残すので、個々の変更を足し合わせるだけでは求められない分断も評価できます。前処理はグリッド・候補集合・重みを固定する必要があり、壁を置けるかどうかや、作業者の割当は判定しません。

## 再現

```sh
flags="-std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all -Ilibrary"
g++ $flags tests/optional_grid_components_test.cpp -o /tmp/optional_grid_components_test
/tmp/optional_grid_components_test
g++ $flags examples/optional_grid_components_usage.cpp -o /tmp/optional_grid_components_usage
/tmp/optional_grid_components_usage
```

人工盤面の16,293クエリで、独立した全盤面の連結性計算と成分別の面積・重み合計が一致しました。
